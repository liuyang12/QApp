#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QDialog>

namespace Ui {
class chatWindow;
}

class chatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit chatWindow(QWidget *parent = 0);
    ~chatWindow();

private:
    Ui::chatWindow *ui;
};

#endif // CHATWINDOW_H
