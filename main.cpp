#include "qwindow.h"
#include <QApplication>
#include <stdio.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWindow w;
    w.show();

    printf("Hello World!\n");

    return a.exec();
}
