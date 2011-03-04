/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Author: Nicolas Arnaud-Cormos, KDAB (nicolas.arnaud-cormos@kdab.com)
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef VALGRINDENGINE_H
#define VALGRINDENGINE_H

#include "valgrindtoolbase_global.h"

#include <analyzerbase/ianalyzerengine.h>

#include <utils/environment.h>

#include <valgrind/valgrindrunner.h>

#include <QString>
#include <QByteArray>
#include <QFutureInterface>

namespace Analyzer {

class AnalyzerSettings;

namespace Internal {

class VALGRINDTOOLBASE_EXPORT ValgrindEngine : public IAnalyzerEngine
{
    Q_OBJECT
public:
    explicit ValgrindEngine(ProjectExplorer::RunConfiguration *runConfiguration);
    virtual ~ValgrindEngine();

    void start();
    void stop();

    QString executable() const;

protected:
    virtual QString progressTitle() const = 0;
    virtual QStringList toolArguments() const = 0;
    virtual Valgrind::ValgrindRunner *runner() = 0;

    AnalyzerSettings *m_settings;
    QFutureInterface<void> *m_progress;

private slots:
    void runnerFinished();

    void receiveStandardOutput(const QByteArray &);
    void receiveStandardError(const QByteArray &);
    void receiveProcessError(const QString &, QProcess::ProcessError);

private:
    QString m_workingDirectory;
    QString m_executable;
    QString m_commandLineArguments;
    Utils::Environment m_environment;
    bool m_isStopping;
};

} // namespace Internal
} // namespace Analyzer

#endif // VALGRINDENGINE_H
