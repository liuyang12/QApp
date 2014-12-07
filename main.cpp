#include "qwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWindow w;
    w.show();

    return a.exec();
}
