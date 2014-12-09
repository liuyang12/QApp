#ifndef QWINDOW_H
#define QWINDOW_H

#include <QMainWindow>

namespace Ui {
class QWindow;
}

class QWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QWindow(QWidget *parent = 0);
    ~QWindow();

private slots:
    void on_button_SearchFriend_clicked();

    void on_action_Logout_triggered();

private:
    Ui::QWindow *ui;
};

#endif // QWINDOW_H
