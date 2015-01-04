#ifndef FRIENDINFO_H
#define FRIENDINFO_H

//#include <QWidget>

#include <QDialog>
#include <QMouseEvent>
#include <QTableWidget>
#include <QTableWidgetItem>


namespace Ui {
class friendinfo;
}

class friendinfo : public QDialog
{
    Q_OBJECT

public:
    explicit friendinfo(QWidget *parent = 0);
    ~friendinfo();

    QPoint dragPosition;
    bool flag;
    bool flag_tx;

private slots:
    //拖动窗口

    void initStatus();

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void on_newgroup_clicked();

    void on_gn_button_clicked();

    void on_confirm_button_clicked();

    void on_choose_tx_clicked();

    void getCell(int row,int col);

    //void on_tableWidget_itemClicked(QTableWidgetItem *item);

private:
    Ui::friendinfo *ui;

protected:

signals:
    void addfriendinfoSignal(void);     // 添加好友信号，应该更新好友列表


};

#endif // FRIENDINFO_H
