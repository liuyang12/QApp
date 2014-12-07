#include "qwindow.h"
#include "ui_qwindow.h"

#include <QTcpSocket>
#include <QMessageBox>

QWindow::QWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QWindow)
{
    ui->setupUi(this);
    QMessageBox::warning(0, tr("info"), tr("Hello World!"), QMessageBox::Ok);

}

QWindow::~QWindow()
{
    delete ui;
}
