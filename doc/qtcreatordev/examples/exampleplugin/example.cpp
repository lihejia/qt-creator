#include "example.h"

#include "exampleconstants.h"
#include "examplefunctions.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

//! [test include]
#ifdef WITH_TESTS
#include <QtTest>
#endif
//! [test include]

namespace Example {
namespace Internal {

//! [plugin tests]
#ifdef WITH_TESTS
class MyPluginTests : public QObject
{
    Q_OBJECT

private slots:
    void testMyTest()
    {
        // a failing test
        QVERIFY(false);
    }
};
#endif
//! [plugin tests]

ExamplePlugin::ExamplePlugin()
{
    // Create your members
}

ExamplePlugin::~ExamplePlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool ExamplePlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

//! [add action]
    auto action = new QAction(tr("Example Action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, &QAction::triggered, this, &ExamplePlugin::triggerAction);
//! [add action]
//! [add menu]
    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("Example"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
//! [add menu]

//! [register tests]
#ifdef WITH_TESTS
    addTest<MyPluginTests>();
#endif
    //! [register tests]
    return true;
}

void ExamplePlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag ExamplePlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

//! [slot implementation]
void ExamplePlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Action Triggered"),
                             tr("This is an action from Example."));
}
//! [slot implementation]

} // namespace Internal

//! [exported function]
int addOne(int i)
{
    return i; // that is wrong!
}
//! [exported function]

} // namespace Example

//! [include moc]
#ifdef WITH_TESTS
#include "example.moc"
#endif
//! [include moc]
