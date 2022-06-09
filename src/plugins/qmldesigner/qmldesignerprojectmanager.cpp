/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "qmldesignerprojectmanager.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/target.h>
#include <projectstorage/filestatuscache.h>
#include <projectstorage/filesystem.h>
#include <projectstorage/nonlockingmutex.h>
#include <projectstorage/projectstorage.h>
#include <projectstorage/projectstorageupdater.h>
#include <projectstorage/qmldocumentparser.h>
#include <projectstorage/qmltypesparser.h>
#include <projectstorage/sourcepathcache.h>
#include <sqlitedatabase.h>
#include <qmlprojectmanager/qmlproject.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>

#include <asynchronousexplicitimagecache.h>
#include <imagecache/asynchronousimagefactory.h>
#include <imagecache/explicitimagecacheimageprovider.h>
#include <imagecache/imagecachecollector.h>
#include <imagecache/imagecacheconnectionmanager.h>
#include <imagecache/imagecachegenerator.h>
#include <imagecache/imagecachestorage.h>
#include <imagecache/timestampproviderinterface.h>

#include <coreplugin/icore.h>

#include <QQmlEngine>

namespace QmlDesigner {

namespace {

QString defaultImagePath()
{
    return Core::ICore::resourcePath("qmldesigner/welcomepage/images/newThumbnail.png").toString();
}

::QmlProjectManager::QmlBuildSystem *getQmlBuildSystem(::ProjectExplorer::Target *target)
{
    return qobject_cast<::QmlProjectManager::QmlBuildSystem *>(target->buildSystem());
}

class TimeStampProvider : public TimeStampProviderInterface
{
public:
    Sqlite::TimeStamp timeStamp(Utils::SmallStringView) const override
    {
        auto now = std::chrono::steady_clock::now();

        return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    }

    Sqlite::TimeStamp pause() const override
    {
        using namespace std::chrono_literals;
        return std::chrono::duration_cast<std::chrono::seconds>(1h).count();
    }
};

} // namespace

class PreviewImageCacheData
{
public:
    Sqlite::Database database{Utils::PathString{
                                  Core::ICore::cacheResourcePath("previewcache.db").toString()},
                              Sqlite::JournalMode::Wal,
                              Sqlite::LockingMode::Normal};
    ImageCacheStorage<Sqlite::Database> storage{database};
    ImageCacheConnectionManager connectionManager;
    ImageCacheCollector collector{connectionManager,
                                  ImageCacheCollectorNullImageHandling::DontCaptureNullImage};
    TimeStampProvider timeStampProvider;
    AsynchronousExplicitImageCache cache{storage};
    AsynchronousImageFactory factory{storage, timeStampProvider, collector};
};

class ProjectStorageData
{
public:
    ProjectStorageData(::ProjectExplorer::Project *project)
        : database{project->projectDirectory().pathAppended("projectstorage.db").toString()}
    {}

    Sqlite::Database database;
    ProjectStorage<Sqlite::Database> storage{database, database.isInitialized()};
    ProjectStorageUpdater::PathCache pathCache{storage};
    FileSystem fileSystem{pathCache};
    FileStatusCache fileStatusCache{fileSystem};
    QmlDocumentParser qmlDocumentParser{storage, pathCache};
    QmlTypesParser qmlTypesParser{pathCache, storage};
    ProjectStorageUpdater updater{
        fileSystem, storage, fileStatusCache, pathCache, qmlDocumentParser, qmlTypesParser};
};

class QmlDesignerProjectManagerProjectData
{
public:
    QmlDesignerProjectManagerProjectData(ImageCacheStorage<Sqlite::Database> &storage,
                                         ::ProjectExplorer::Project *project)
        : factory{storage, timeStampProvider, collector}
        , projectStorageData{project}
    {}

    ImageCacheConnectionManager connectionManager;
    ImageCacheCollector collector{connectionManager,
                                  ImageCacheCollectorNullImageHandling::DontCaptureNullImage};
    TimeStampProvider timeStampProvider;
    AsynchronousImageFactory factory;
    ProjectStorageData projectStorageData;
    ::ProjectExplorer::Target *activeTarget = nullptr;
};

QmlDesignerProjectManager::QmlDesignerProjectManager()
    : m_imageCacheData{std::make_unique<PreviewImageCacheData>()}
{
    auto editorManager = ::Core::EditorManager::instance();
    QObject::connect(editorManager, &::Core::EditorManager::editorOpened, [&](auto *editor) {
        editorOpened(editor);
    });
    QObject::connect(editorManager, &::Core::EditorManager::currentEditorChanged, [&](auto *editor) {
        currentEditorChanged(editor);
    });
    QObject::connect(editorManager, &::Core::EditorManager::editorsClosed, [&](const auto &editors) {
        editorsClosed(editors);
    });
    auto sessionManager = ::ProjectExplorer::SessionManager::instance();
    QObject::connect(sessionManager,
                     &::ProjectExplorer::SessionManager::projectAdded,
                     [&](auto *project) { projectAdded(project); });
    QObject::connect(sessionManager,
                     &::ProjectExplorer::SessionManager::aboutToRemoveProject,
                     [&](auto *project) { aboutToRemoveProject(project); });
    QObject::connect(sessionManager,
                     &::ProjectExplorer::SessionManager::projectRemoved,
                     [&](auto *project) { projectRemoved(project); });
}

QmlDesignerProjectManager::~QmlDesignerProjectManager() = default;

void QmlDesignerProjectManager::registerPreviewImageProvider(QQmlEngine *engine) const
{
    auto imageProvider = std::make_unique<ExplicitImageCacheImageProvider>(m_imageCacheData->cache,
                                                                           QImage{defaultImagePath()});

    engine->addImageProvider("project_preview", imageProvider.release());
}

void QmlDesignerProjectManager::editorOpened(::Core::IEditor *) {}

void QmlDesignerProjectManager::currentEditorChanged(::Core::IEditor *)
{
    if (!m_projectData || !m_projectData->activeTarget)
        return;

    ::QmlProjectManager::QmlBuildSystem *qmlBuildSystem = getQmlBuildSystem(
        m_projectData->activeTarget);

    if (qmlBuildSystem) {
        m_imageCacheData->collector.setTarget(m_projectData->activeTarget);
        m_imageCacheData->factory.generate(qmlBuildSystem->mainFilePath().toString().toUtf8());
    }
}

void QmlDesignerProjectManager::editorsClosed(const QList<::Core::IEditor *> &) {}

namespace {

QtSupport::QtVersion *getQtVersion(::ProjectExplorer::Target *target)
{
    if (target)
        return QtSupport::QtKitAspect::qtVersion(target->kit());

    return {};
}

QtSupport::QtVersion *getQtVersion(::ProjectExplorer::Project *project)
{
    return getQtVersion(project->activeTarget());
}

Utils::FilePath qmlPath(::ProjectExplorer::Target *target)
{
    auto qt = QtSupport::QtKitAspect::qtVersion(target->kit());
    if (qt)
        return qt->qmlPath();

    return {};
}

void projectQmldirPaths(::ProjectExplorer::Target *target, QStringList &qmldirPaths)
{
    ::QmlProjectManager::QmlBuildSystem *buildSystem = getQmlBuildSystem(target);

    const Utils::FilePath pojectDirectoryPath = buildSystem->canonicalProjectDir();
    const QStringList importPaths = buildSystem->importPaths();
    const QDir pojectDirectory(pojectDirectoryPath.toString());

    for (const QString &importPath : importPaths)
        qmldirPaths.push_back(QDir::cleanPath(pojectDirectory.absoluteFilePath(importPath))
                              + "/qmldir");
}

bool skipPath(const std::filesystem::path &path)
{
    auto directory = path.filename();
    qDebug() << path.string().data();

    bool skip = directory == "QtApplicationManager" || directory == "QtInterfaceFramework"
                || directory == "QtOpcUa" || directory == "Qt3D" || directory == "Qt3D"
                || directory == "Scene2D" || directory == "Scene3D" || directory == "QtWayland"
                || directory == "Qt5Compat";
    if (skip)
        qDebug() << "skip" << path.string().data();

    return skip;
}

void qtQmldirPaths(::ProjectExplorer::Target *target, QStringList &qmldirPaths)
{
    const QString installDirectory = qmlPath(target).toString();

    const std::filesystem::path installDirectoryPath{installDirectory.toStdString()};

    auto current = std::filesystem::recursive_directory_iterator{installDirectoryPath};
    auto end = std::filesystem::end(current);
    for (; current != end; ++current) {
        const auto &entry = *current;
        auto path = entry.path();
        if (current.depth() < 3 && !current->is_regular_file() && skipPath(path)) {
            current.disable_recursion_pending();
            continue;
        }
        if (path.filename() == "qmldir") {
            qmldirPaths.push_back(QString::fromStdU16String(path.generic_u16string()));
        }
    }
}

QStringList qmlDirs(::ProjectExplorer::Target *target)
{
    if (!target)
        return {};

    QStringList qmldirPaths;
    qmldirPaths.reserve(100);

    qtQmldirPaths(target, qmldirPaths);
    projectQmldirPaths(target, qmldirPaths);

    std::sort(qmldirPaths.begin(), qmldirPaths.end());
    qmldirPaths.erase(std::unique(qmldirPaths.begin(), qmldirPaths.end()), qmldirPaths.end());

    return qmldirPaths;
}

QStringList qmlTypes(::ProjectExplorer::Target *target)
{
    if (!target)
        return {};

    QStringList qmldirPaths;
    qmldirPaths.reserve(2);

    const QString installDirectory = qmlPath(target).toString();

    qmldirPaths.append(installDirectory + "/builtins.qmltypes");
    qmldirPaths.append(installDirectory + "/jsroot.qmltypes");

    qmldirPaths.append(
        Core::ICore::resourcePath("qmldesigner/projectstorage/fake.qmltypes").toString());

    return qmldirPaths;
}

} // namespace

void QmlDesignerProjectManager::projectAdded(::ProjectExplorer::Project *project)
{
    if (qEnvironmentVariableIsSet("QDS_ACTIVATE_PROJECT_STORAGE")) {
        m_projectData = std::make_unique<QmlDesignerProjectManagerProjectData>(m_imageCacheData->storage,
                                                                               project);
        m_projectData->activeTarget = project->activeTarget();

        QObject::connect(project, &::ProjectExplorer::Project::fileListChanged, [&]() {
            fileListChanged();
        });

        QObject::connect(project,
                         &::ProjectExplorer::Project::activeTargetChanged,
                         [&](auto *target) { activeTargetChanged(target); });

        QObject::connect(project,
                         &::ProjectExplorer::Project::aboutToRemoveTarget,
                         [&](auto *target) { aboutToRemoveTarget(target); });

        if (auto target = project->activeTarget(); target)
            activeTargetChanged(target);
    }
}

void QmlDesignerProjectManager::aboutToRemoveProject(::ProjectExplorer::Project *)
{
    if (m_projectData) {
        m_imageCacheData->collector.setTarget(m_projectData->activeTarget);
        m_projectData.reset();
    }
}

void QmlDesignerProjectManager::projectRemoved(::ProjectExplorer::Project *) {}

void QmlDesignerProjectManager::fileListChanged()
{
    update();
}

void QmlDesignerProjectManager::activeTargetChanged(ProjectExplorer::Target *target)
{
    if (m_projectData)
        return;

    QObject::disconnect(m_projectData->activeTarget, nullptr, nullptr, nullptr);

    m_projectData->activeTarget = target;

    if (target) {
        QObject::connect(target, &::ProjectExplorer::Target::kitChanged, [&]() { kitChanged(); });
        QObject::connect(getQmlBuildSystem(target),
                         &::QmlProjectManager::QmlBuildSystem::projectChanged,
                         [&]() { projectChanged(); });
    }

    update();
}

void QmlDesignerProjectManager::aboutToRemoveTarget(ProjectExplorer::Target *target)
{
    QObject::disconnect(target, nullptr, nullptr, nullptr);
    QObject::disconnect(getQmlBuildSystem(target), nullptr, nullptr, nullptr);
}

void QmlDesignerProjectManager::kitChanged()
{
    QStringList qmldirPaths;
    qmldirPaths.reserve(100);

    update();
}

void QmlDesignerProjectManager::projectChanged()
{
    update();
}

void QmlDesignerProjectManager::update()
{
    if (!m_projectData)
        return;

    m_projectData->projectStorageData.updater.update(qmlDirs(m_projectData->activeTarget),
                                                     qmlTypes(m_projectData->activeTarget));
}

} // namespace QmlDesigner
