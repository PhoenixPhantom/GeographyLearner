#include <QApplication>
#include "MainWidget.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    MainWidget mainWidget;
    mainWidget.show();
    mainWidget.setWindowState(Qt::WindowMaximized);
    return app.exec();
}
