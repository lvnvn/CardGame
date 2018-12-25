#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <client.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void buttonClicked();
    void on_getcard_clicked();
    void on_rules_clicked();
    void on_finish_clicked();
    void updateOnWait();
    void gameLoop();
    void startCard();
    void updateCardset();

private:
    Ui::MainWindow *ui;
    QTimer* timer1;
    QTimer* timer2;
    Client client = Client();
    std::vector<QLabel*> card_labels;
    std::vector<QPushButton*> card_buttons;
};

#endif // MAINWINDOW_H
