/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
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
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef DEBUGGER_DEBUGGERMANAGER_H
#define DEBUGGER_DEBUGGERMANAGER_H

#include "debugger_global.h"
#include "debuggerconstants.h"

#include <coreplugin/ssh/sshconnection.h>

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QAction;
class QDebug;
class QDockWidget;
class QLabel;
class QMessageBox;
class QPoint;
class QIcon;
QT_END_NAMESPACE

namespace Core {
class IOptionsPage;
}

namespace TextEditor {
class ITextEditor;
class FontSettings;
}

namespace CPlusPlus {
    class Snapshot;
}

namespace Debugger {
namespace Internal {

class DebuggerOutputWindow;
class DebuggerPlugin;

class BreakHandler;
class BreakpointData;
class ModulesHandler;
class RegisterHandler;
class SourceFilesWindow;
struct StackFrame;
class StackHandler;
class Symbol;
class SnapshotHandler;
class ThreadsHandler;
class WatchData;
class WatchHandler;
class IDebuggerEngine;
class GdbEngine;
class ScriptEngine;
class PdbEngine;
class CdbDebugEngine;
class CdbDebugEnginePrivate;
struct DebuggerManagerActions;
class CdbDebugEventCallback;
class CdbDumperHelper;
class CdbDumperInitThread;
class CdbExceptionLoggerEventCallback;
class GdbEngine;
class TcfEngine;
class QmlEngine;
class CdbDebugEngine;
class CdbDebugEnginePrivate;
class TrkGdbAdapter;
class BreakpointMarker;
} // namespace Internal

class DEBUGGER_EXPORT DebuggerStartParameters
{
public:
    DebuggerStartParameters();
    void clear();

    QString executable;
    QString coreFile;
    QStringList processArgs;
    QStringList environment;
    QString workingDirectory;
    QString buildDirectory;
    qint64 attachPID;
    bool useTerminal;
    QString crashParameter; // for AttachCrashedExternal
    // for remote debugging
    QString remoteChannel;
    QString remoteArchitecture;
    QString symbolFileName;
    QString serverStartScript;
    QString sysRoot;
    QString debuggerCommand;
    int toolChainType;
    QByteArray remoteDumperLib;
    QString qtInstallPath;

    QString dumperLibrary;
    QStringList dumperLibraryLocations;
    Core::SshServerInfo sshserver;
    DebuggerStartMode startMode;
};

typedef QSharedPointer<DebuggerStartParameters> DebuggerStartParametersPtr;

DEBUGGER_EXPORT QDebug operator<<(QDebug str, const DebuggerStartParameters &);

// Flags for initialization
enum DebuggerEngineTypeFlags
{
    GdbEngineType     = 0x01,
    ScriptEngineType  = 0x02,
    CdbEngineType     = 0x04,
    PdbEngineType     = 0x08,
    TcfEngineType     = 0x10,
    QmlEngineType     = 0x20,
    AllEngineTypes = GdbEngineType
        | ScriptEngineType
        | CdbEngineType
        | PdbEngineType
        | TcfEngineType
        | QmlEngineType
};

QDebug operator<<(QDebug d, DebuggerState state);

//
// DebuggerManager
//

struct DebuggerManagerPrivate;

class DEBUGGER_EXPORT DebuggerManager : public QObject
{
    Q_OBJECT

public:
    explicit DebuggerManager(Internal::DebuggerPlugin *plugin);
    ~DebuggerManager();

    friend class Internal::IDebuggerEngine;
    friend class Internal::DebuggerPlugin;
    friend class Internal::CdbDebugEventCallback;
    friend class Internal::CdbDumperHelper;
    friend class Internal::CdbDumperInitThread;
    friend class Internal::CdbExceptionLoggerEventCallback;
    friend class Internal::GdbEngine;
    friend class Internal::ScriptEngine;
    friend class Internal::PdbEngine;
    friend class Internal::TcfEngine;
    friend class Internal::QmlEngine;
    friend class Internal::CdbDebugEngine;
    friend class Internal::CdbDebugEnginePrivate;
    friend class Internal::TrkGdbAdapter;
    friend class Internal::BreakpointMarker;

    DebuggerState state() const;
    QList<Core::IOptionsPage*> initializeEngines(unsigned enabledTypeFlags);

    QLabel *statusLabel() const;
    Internal::IDebuggerEngine *currentEngine() const;

    DebuggerStartParametersPtr startParameters() const;
    qint64 inferiorPid() const;

    QMessageBox *showMessageBox(int icon, const QString &title, const QString &text,
        int buttons = 0);

    bool debuggerActionsEnabled() const;
    unsigned debuggerCapabilities() const;

    bool checkDebugConfiguration(int toolChain,
                                 QString *errorMessage,
                                 QString *settingsCategory = 0,
                                 QString *settingsPage = 0) const;

    const CPlusPlus::Snapshot &cppCodeModelSnapshot() const;

    QIcon locationMarkIcon() const;

    static DebuggerManager *instance();

public slots:
    void startNewDebugger(const DebuggerStartParametersPtr &sp);
    void exitDebugger();
    void abortDebugger();

    void setSimpleDockWidgetArrangement(const QString &activeLanguage);

    void setBusyCursor(bool on);

    void gotoLocation(const Debugger::Internal::StackFrame &frame, bool setLocationMarker);
    void fileOpen(const QString &file);
    void resetLocation();

    void interruptDebuggingRequest();

    void executeJumpToLine();
    void executeRunToLine();
    void executeRunToFunction();
    void breakByFunction(const QString &functionName);
    void breakByFunctionMain();
    void activateFrame(int index);
    void selectThread(int index);
    void activateSnapshot(int index);
    void removeSnapshot(int index);

    void executeStep();
    void executeStepOut();
    void executeStepNext();
    void executeContinue();
    void executeReturn();
    void detachDebugger();
    void makeSnapshot();
    void frameUp();
    void frameDown();

    void addToWatchWindow();
    void updateWatchData(const Debugger::Internal::WatchData &data);

    void sessionLoaded();
    void aboutToUnloadSession();
    void aboutToSaveSession();
    QVariant sessionValue(const QString &name);
    void setSessionValue(const QString &name, const QVariant &value);

    void assignValueInDebugger();
    void assignValueInDebugger(const QString &expr, const QString &value);

    void executeDebuggerCommand();
    void executeDebuggerCommand(const QString &command);

    void watchPoint();
    void setRegisterValue(int nr, const QString &value);

    void showStatusMessage(const QString &msg, int timeout = -1); // -1 forever
    void clearCppCodeModelSnapshot();

    static const char *stateName(int s);

public slots: // FIXME
    void showDebuggerOutput(const QString &msg)
        { showDebuggerOutput(LogDebug, msg); }
    void ensureLogVisible();
    void updateWatchersWindow();
    void updateWatchersHeader(int section, int oldSize, int newSize);
    void activateBreakpoint(int index);

//private slots:  // FIXME
    void showDebuggerOutput(int channel, const QString &msg);
    void showDebuggerInput(int channel, const QString &msg);
    void showApplicationOutput(const QString &data, bool onStdErr);

    void reloadSourceFiles();
    void sourceFilesDockToggled(bool on);

    void reloadModules();
    void modulesDockToggled(bool on);
    void loadSymbols(const QString &moduleName);
    void loadAllSymbols();

    void reloadRegisters();
    void registerDockToggled(bool on);
    void clearStatusMessage();
    void attemptBreakpointSynchronization();
    void reloadFullStack();
    void operateByInstructionTriggered();
    void startFailed();

public:
    Internal::ModulesHandler *modulesHandler() const;
    Internal::BreakHandler *breakHandler() const;
    Internal::RegisterHandler *registerHandler() const;
    Internal::StackHandler *stackHandler() const;
    Internal::ThreadsHandler *threadsHandler() const;
    Internal::WatchHandler *watchHandler() const;
    Internal::SnapshotHandler *snapshotHandler() const;

private:
    Internal::SourceFilesWindow *sourceFileWindow() const;
    QWidget *threadsWindow() const;

    Internal::DebuggerManagerActions debuggerManagerActions() const;

    void notifyInferiorStopped();
    void notifyInferiorRunning();
    void notifyInferiorExited();
    void notifyInferiorPidChanged(qint64);

    void cleanupViews();

    void setState(DebuggerState state, bool forced = false);

    //
    // internal implementation
    //
    bool qtDumperLibraryEnabled() const;
    QString qtDumperLibraryName() const;
    QStringList qtDumperLibraryLocations() const;
    void showQtDumperLibraryWarning(const QString &details = QString());
    bool isReverseDebugging() const;
    QAbstractItemModel *threadsModel();

    Q_SLOT void loadSessionData();
    Q_SLOT void saveSessionData();
    Q_SLOT void dumpLog();
    Q_SLOT void fontSettingsChanged(const TextEditor::FontSettings &settings);

public:
    // stuff in this block should be made private by moving it to
    // one of the interfaces
    void requestModuleSymbols(const QString &moduleName);
    void showModuleSymbols(const QString &moduleName,
        const QList<Internal::Symbol> &symbols);

signals:
    void debuggingFinished();
    void inferiorPidChanged(qint64 pid);
    void stateChanged(int newstatus);
    void statusMessageRequested(const QString &msg, int timeout); // -1 for 'forever'
    void applicationOutputAvailable(const QString &output, bool onStdErr);
    void messageAvailable(const QString &output, bool isError);
    void emitShowOutput(int channel, const QString &output);
    void emitShowInput(int channel, const QString &input);

private:
    void init();
    void runTest(const QString &fileName);
    Q_SLOT void createNewDock(QWidget *widget);

    void aboutToShutdown();

    //void toggleBreakpoint(const QString &fileName, int lineNumber);
    void setToolTipExpression(const QPoint &mousePos,
        TextEditor::ITextEditor *editor, int cursorPos);
    void openTextEditor(const QString &titlePattern,
        const QString &contents);

    DebuggerManagerPrivate *d;
};

} // namespace Debugger

#endif // DEBUGGER_DEBUGGERMANAGER_H
