#include "qwebgui.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString qss_file;
#ifdef Q_OS_WIN
    qss_file = qApp->applicationDirPath() + "\\myqss.qss";
#else
    qss_file = "myqss.qss";
#endif
    QFile qss(qss_file);
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();
    QWebGui w;
    w.show();

    return a.exec();
}
