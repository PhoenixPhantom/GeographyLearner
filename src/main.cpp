#include <QApplication>
#include "app/LearnerWidget.h"


int main(int argc, char **argv)
{
#if defined(Q_OS_WIN)
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif
    QApplication app(argc, argv);
    LearnerWidget mainWidget;
    mainWidget.show();
    mainWidget.setWindowState(Qt::WindowMaximized);
    return app.exec();
}
