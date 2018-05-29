#include "qwebgui.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss("myqss.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();
    QWebGui w;
    w.show();

    return a.exec();
}
