#ifndef MESSDIALOG_H
#define MESSDIALOG_H

#include <QWidget>
#include <QTimer>
#include <QDesktopWidget>
#include "classConstant.h"

namespace Ui {
class MessDialog;
}

class MessDialog : public QWidget
{
    Q_OBJECT

public:
    explicit MessDialog(QWidget *parent = 0);
    ~MessDialog();

    void SetMessage(QString Title, QString MainDoc,
                    QString YesButton, QString NoButton, FriendInfo hostInfo);
    void DynamicShow();             //渐变显示

private slots:
    void MessDialogShow();          //淡入效果
    void MessDialogStay();          //停留效果
    void MessDialogClose();         //淡出效果

    void on_CloseButton_clicked();

    void on_WorkButton_clicked();

private:
    Ui::MessDialog *ui;

    QTimer *timeShow;
    QTimer *timeStay;
    QTimer *timeClose;

    QPoint Location;
    QRect deskRect;

    FriendInfo FriendChat;
    QString BoxTitle;
};

#endif // MESSDIALOG_H
