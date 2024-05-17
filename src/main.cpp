#include "ProjectSelectorWidget.h"
#include <QApplication>
#include <qicon.h>
#include <qtranslator.h>
#include <git2.h>

int main(int argc, char **argv)
{
#if defined(Q_OS_WIN)
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif

    git_libgit2_init();
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/Data/Globe.ico"));

    ProjectSelectorWidget selectorWidget;
    selectorWidget.show();

    int retVal = app.exec();
    git_libgit2_shutdown();
    return retVal;

}
