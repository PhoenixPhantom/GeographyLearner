#include <QApplication>
#include <qicon.h>
#include <qtranslator.h>
#if MAKE_EDITOR
#include "editor/EditorWidget.h"
#else
#include "app/LearnerWidget.h"
#endif

int main(int argc, char **argv)
{
#if defined(Q_OS_WIN)
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/Data/Globe.ico"));

#if !(MAKE_EDITOR)
    LearnerWidget mainWidget;
    mainWidget.showMaximized();
    //mainWidget.setWindowState(Qt::WindowMaximized);
#endif

#if MAKE_EDITOR
    EditorWidget editor;
    //editor.show();
    editor.showMaximized();
    //editor.setWindowState(Qt::WindowMaximized);
#endif

    return app.exec();
}
