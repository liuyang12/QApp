#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::on_buttonConfirm_accepted()
{

}

void login::on_EditNumber_textChanged(const QString &arg1)
{

}

void login::on_EditPassword_textChanged(const QString &arg1)
{

}
