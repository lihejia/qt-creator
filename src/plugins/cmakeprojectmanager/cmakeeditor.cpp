// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cmakeeditor.h"

#include "cmakeautocompleter.h"
#include "cmakefilecompletionassist.h"
#include "cmakeindenter.h"
#include "cmakeprojectconstants.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreplugintr.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildsystem.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projecttree.h>
#include <texteditor/textdocument.h>
#include <texteditor/texteditoractionhandler.h>
#include <utils/textutils.h>

#include <QTextDocument>

#include <functional>

using namespace Core;
using namespace ProjectExplorer;
using namespace TextEditor;

namespace CMakeProjectManager::Internal {

//
// CMakeEditor
//

class CMakeEditor : public TextEditor::BaseTextEditor
{
public:
    void contextHelp(const HelpCallback &callback) const final;
};

void CMakeEditor::contextHelp(const HelpCallback &callback) const
{
    int pos = position();

    QChar chr;
    do {
        --pos;
        if (pos < 0)
            break;
        chr = characterAt(pos);
        if (chr == QLatin1Char('(')) {
            BaseTextEditor::contextHelp(callback);
            return;
        }
    } while (chr.unicode() != QChar::ParagraphSeparator);

    ++pos;
    chr = characterAt(pos);
    while (chr.isSpace()) {
        ++pos;
        chr = characterAt(pos);
    }
    int begin = pos;

    do {
        ++pos;
        chr = characterAt(pos);
    } while (chr.isLetterOrNumber() || chr == QLatin1Char('_'));
    int end = pos;

    while (chr.isSpace()) {
        ++pos;
        chr = characterAt(pos);
    }

    // Not a command
    if (chr != QLatin1Char('(')) {
        BaseTextEditor::contextHelp(callback);
        return;
    }

    const QString id = "command/" + textAt(begin, end - begin).toLower();
    callback({{id, Utils::Text::wordUnderCursor(editorWidget()->textCursor())},
              {},
              {},
              HelpItem::Unknown});
}

//
// CMakeEditorWidget
//

class CMakeEditorWidget final : public TextEditorWidget
{
public:
    ~CMakeEditorWidget() final = default;

private:
    void findLinkAt(const QTextCursor &cursor,
                    const Utils::LinkHandler &processLinkCallback,
                    bool resolveTarget = true,
                    bool inNextSplit = false) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
};

void CMakeEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    showDefaultContextMenu(e, Constants::M_CONTEXT);
}

static bool mustBeQuotedInFileName(const QChar &c)
{
    return c.isSpace() || c == '"' || c == '(' || c == ')';
}

static bool isValidFileNameChar(const QString &block, int pos)
{
    const QChar c = block.at(pos);
    return !mustBeQuotedInFileName(c) || (pos > 0 && block.at(pos - 1) == '\\');
}

static QString unescape(const QString &s)
{
    QString result;
    int i = 0;
    const int size = s.size();
    while (i < size) {
        const QChar c = s.at(i);
        if (c == '\\' && i < size - 1) {
            const QChar nc = s.at(i + 1);
            if (mustBeQuotedInFileName(nc)) {
                result += nc;
                i += 2;
                continue;
            }
        }
        result += c;
        ++i;
    }
    return result;
}

void CMakeEditorWidget::findLinkAt(const QTextCursor &cursor,
                                   const Utils::LinkHandler &processLinkCallback,
                                   bool/* resolveTarget*/,
                                   bool /*inNextSplit*/)
{
    Utils::Link link;

    int line = 0;
    int column = 0;
    convertPosition(cursor.position(), &line, &column);

    const QString block = cursor.block().text();

    // check if the current position is commented out
    const int hashPos = block.indexOf(QLatin1Char('#'));
    if (hashPos >= 0 && hashPos < column)
        return processLinkCallback(link);

    // find the beginning of a filename
    QString buffer;
    int beginPos = column - 1;
    while (beginPos >= 0) {
        if (isValidFileNameChar(block, beginPos)) {
            buffer.prepend(block.at(beginPos));
            beginPos--;
        } else {
            break;
        }
    }

    // find the end of a filename
    int endPos = column;
    while (endPos < block.size()) {
        if (isValidFileNameChar(block, endPos)) {
            buffer.append(block.at(endPos));
            endPos++;
        } else {
            break;
        }
    }

    if (buffer.isEmpty())
        return processLinkCallback(link);

    const Utils::FilePath dir = textDocument()->filePath().absolutePath();
    buffer.replace("${CMAKE_CURRENT_SOURCE_DIR}", dir.path());
    buffer.replace("${CMAKE_CURRENT_LIST_DIR}", dir.path());

    if (auto project = ProjectTree::currentProject()) {
        buffer.replace("${CMAKE_SOURCE_DIR}", project->projectDirectory().path());
        if (auto bs = ProjectTree::currentBuildSystem()) {
            buffer.replace("${CMAKE_BINARY_DIR}", bs->buildConfiguration()->buildDirectory().path());

            // Get the path suffix from current source dir to project source dir and apply it
            // for the binary dir
            const QString relativePathSuffix = textDocument()
                                                   ->filePath()
                                                   .parentDir()
                                                   .relativePathFrom(project->projectDirectory())
                                                   .path();
            buffer.replace("${CMAKE_CURRENT_BINARY_DIR}",
                           bs->buildConfiguration()
                               ->buildDirectory()
                               .pathAppended(relativePathSuffix)
                               .path());
        }
    }
    // TODO: Resolve more variables

    Utils::FilePath fileName = dir.withNewPath(unescape(buffer));
    if (fileName.isRelativePath())
        fileName = dir.pathAppended(fileName.path());
    if (fileName.exists()) {
        if (fileName.isDir()) {
            Utils::FilePath subProject = fileName.pathAppended("CMakeLists.txt");
            if (subProject.exists())
                fileName = subProject;
            else
                return processLinkCallback(link);
        }
        link.targetFilePath = fileName;
        link.linkTextStart = cursor.position() - column + beginPos + 1;
        link.linkTextEnd = cursor.position() - column + endPos;
    }
    processLinkCallback(link);
}

static TextDocument *createCMakeDocument()
{
    auto doc = new TextDocument;
    doc->setId(Constants::CMAKE_EDITOR_ID);
    doc->setMimeType(QLatin1String(Constants::CMAKE_MIMETYPE));
    return doc;
}

//
// CMakeEditorFactory
//

CMakeEditorFactory::CMakeEditorFactory()
{
    setId(Constants::CMAKE_EDITOR_ID);
    setDisplayName(::Core::Tr::tr("CMake Editor"));
    addMimeType(Constants::CMAKE_MIMETYPE);
    addMimeType(Constants::CMAKE_PROJECT_MIMETYPE);

    setEditorCreator([] { return new CMakeEditor; });
    setEditorWidgetCreator([] { return new CMakeEditorWidget; });
    setDocumentCreator(createCMakeDocument);
    setIndenterCreator([](QTextDocument *doc) { return new CMakeIndenter(doc); });
    setUseGenericHighlighter(true);
    setCommentDefinition(Utils::CommentDefinition::HashStyle);
    setCodeFoldingSupported(true);

    setCompletionAssistProvider(new CMakeFileCompletionAssistProvider);
    setAutoCompleterCreator([] { return new CMakeAutoCompleter; });

    setEditorActionHandlers(TextEditorActionHandler::UnCommentSelection
            | TextEditorActionHandler::JumpToFileUnderCursor
            | TextEditorActionHandler::Format);

    ActionContainer *contextMenu = ActionManager::createMenu(Constants::M_CONTEXT);
    contextMenu->addAction(ActionManager::command(TextEditor::Constants::JUMP_TO_FILE_UNDER_CURSOR));
    contextMenu->addSeparator(Context(Constants::CMAKE_EDITOR_ID));
    contextMenu->addAction(ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION));
}

} // CMakeProjectManager::Internal
