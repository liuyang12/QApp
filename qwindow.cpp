#include "qwindow.h"
#include "ui_qwindow.h"

QWindow::QWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QWindow)
{
    ui->setupUi(this);
}

QWindow::~QWindow()
{
    delete ui;
}
