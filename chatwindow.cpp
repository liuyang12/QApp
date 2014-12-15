#include "chatwindow.h"
#include "ui_chatwindow.h"

chatWindow::chatWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chatWindow)
{
    ui->setupUi(this);
}

chatWindow::~chatWindow()
{
    delete ui;
}
