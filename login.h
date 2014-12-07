#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();

    QString LogNumber;         //
    QString LogPassword;

private slots:
    void on_buttonConfirm_accepted();

    void on_EditNumber_textChanged(const QString &arg1);

    void on_EditPassword_textChanged(const QString &arg1);

private:
    Ui::login *ui;
};

#endif // LOGIN_H
