#include "qwebgui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWebGui w;
    w.show();

    return a.exec();
}
