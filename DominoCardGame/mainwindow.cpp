/*
Домино́ — карточная игра. Применяется 52−карточная колода. Участвуют от 2 до 6 игроков. Цель игры − избавиться от всех карт на руках.

Всем участвующим в игре раздаётся по 7 карт, а оставшаяся колода кладётся взакрытую рядом и верхняя карта снимается.
На эту карту по очереди игрок, сидящий следом за сдающим, должен положить карту либо серию карт, которые составляют
последовательности с ней, вне зависимости от масти. Например: на 7 можно положить 8 либо 6, либо 6, 5, 4, либо 8, 9, 8, 7 и т. д.
Туз является «поворотной» картой: его и на него можно положить и короля, и двойку.

Игрок обязан снести карту, если есть такая возможность.
Если игрок не может выложить ни одну из карт, то он берёт карту из колоды.
Если она подходит, то он кладёт её на кон и тянет следующую (либо выкладывает карту с рук, если теперь это возможно),
если не подходит, то забирает себе и передаёт ход дальше.

Выигрывает тот, кто первым избавится от своих карт на любой стадии игры,
даже если остались ещё карты в колоде. Если таких нет, то считается что игра не состоялась (ничья).

РЕАЛИЗАЦИЯ ИГРЫ ДЛЯ ТРЕХ ИГРОКОВ
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include "client.h"
#include <iostream>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    client.start();
    client.id = std::stoi(client.sendMessage("start")); // get message about id
    ui->statusBar->showMessage("Вы игрок номер " + QString::number(client.id+1));
    ui->turn->setText("Ожидание других игроков...");

    timer1 = new QTimer(this);
    QObject::connect(timer1, SIGNAL(timeout()), this, SLOT(updateOnWait()));
    timer1->start(1000);

    std::string cardset = client.sendMessage("cardset"); // get 7 random cards
    card_buttons.resize(7);
    card_labels.resize(7);
    client.cardset.resize(7);
    for(int i = 0; i < 7; i++)
    {
        int j = 0;
        while(cardset[j] != ' ')
            j++;
        int rang = std::stoi(cardset.substr(0,j-1));
        char suit = cardset[j-1];
        client.cardset[i] = std::to_string(rang) + std::string(1,suit);
        cardset = cardset.substr(j+1,cardset.size()-j-1);

        card_labels[i] = new QLabel();
        QString path = "/home/daria/cours_nets/DominoCardGame/cards/PNG/" + QString::number(rang) + suit + ".png";
        QPixmap pm(path); // path to image file
        pm = pm.scaled(100,153);
        card_labels[i]->setPixmap(pm);
        ui->cards->addWidget(card_labels[i],0,i);

        card_buttons[i] = new QPushButton("Выбрать "+QString::number(i));
        connect(card_buttons[i], SIGNAL(clicked()), this, SLOT(buttonClicked()));
        ui->cards->addWidget(card_buttons[i],1,i);
    }
}

void MainWindow::updateCardset()
{
    while (QLayoutItem* item = ui->cards->takeAt(0))
        {
            QWidget* widget;
            if(widget = item->widget())
                delete widget;
            delete item;
        }

    for(int i = 0; i < client.cardset.size(); i++)
    {
        QString path = "/home/daria/cours_nets/DominoCardGame/cards/PNG/" + QString::fromUtf8(client.cardset[i].c_str()) + ".png";
        QPixmap pm(path); // path to image file
        pm = pm.scaled(100,153);
        card_labels[i] = new QLabel();
        card_labels[i]->setPixmap(pm);
        ui->cards->addWidget(card_labels[i],0,i);

        card_buttons[i] = new QPushButton("Выбрать "+QString::number(i));
        connect(card_buttons[i], SIGNAL(clicked()), this, SLOT(buttonClicked()));
        card_buttons[i]->setText("Выбрать "+QString::number(i));
        ui->cards->addWidget(card_buttons[i],1,i);
    }
}

void MainWindow::updateOnWait()
{
    client.active = std::stoi(client.sendMessage("wait"));
    if(client.active)
    {
        timer1->stop();

        startCard();
        timer2 = new QTimer(this);
        QObject::connect(timer2, SIGNAL(timeout()), this, SLOT(gameLoop()));
        timer2->start(1000);
    }
}

void MainWindow::startCard()
{
    std::string startcard = client.sendMessage("scard");
    QString path = "/home/daria/cours_nets/DominoCardGame/cards/PNG/" + QString::fromUtf8(startcard.c_str()) + ".png";
    QPixmap pm(path); // path to image file
    pm = pm.scaled(100,153);
    ui->startcard->setPixmap(pm);
    ui->label->setText("Вам нужно положить свою карту на эту:");

}

void MainWindow::gameLoop()
{
    client.turn = std::stoi(client.sendMessage("turn"));

    //-2 - игра не окончена; -1 - ничья, другое число - выиграл пользователь с этим id
    int is_over = std::stoi(client.sendMessage("isover"));
    if(is_over == -1)
    {
        timer2->stop();
        ui->turn->setText("Больше карт в колоде нет. Игра окончена: ничья.");
        return;
    }
    if(is_over >= 0 && is_over != client.id)
    {
        timer2->stop();
        ui->turn->setText("Игра окончена: победил игрок номер" + QString::number(is_over));
        return;
    }
    if(client.turn != client.id)
        ui->turn->setText("Ход игрока " + QString::number(client.turn+1));
    else
        ui->turn->setText("Ваш ход!");
    startCard();
    updateCardset();
}

void MainWindow::buttonClicked()
{
    QPushButton *button = (QPushButton *)sender();
    int card_number = (button->text().size()==9) ? (button->text()[8].unicode()-'0') :
        ((button->text()[8].unicode()-'0')*10 + button->text()[9].unicode()-'0');
    if(client.turn != client.id)
        return;
    std::string message = "putcard" + client.cardset[card_number];
    if(client.cardset[card_number].size()==2)  // 'putcard7C ' or 'putcard10C' for same msg size
        message += " ";
    std::string response = client.sendMessage(message);
    ui->res->setText(QString::fromUtf8(response.c_str()));
    if(response == "ok")
    {
        QString path = "/home/daria/cours_nets/DominoCardGame/cards/PNG/" + QString::fromUtf8(client.cardset[card_number].c_str()) + ".png";
        QPixmap pm(path); // path to image file
        pm = pm.scaled(100,153);
        ui->startcard->setPixmap(pm);

        client.cardset.erase(client.cardset.begin()+card_number);
        card_buttons.erase(card_buttons.begin()+card_number);
        card_labels.erase(card_labels.begin()+card_number);

        if(client.cardset.size() == 0)
        {
            client.sendMessage("win");
            timer2->stop();
            updateCardset();
            ui->turn->setText("Игра окончена: вы победили!");
        }
    }
}

void MainWindow::on_getcard_clicked()
{
    if(client.turn != client.id)
        return;
    std::string newcard = client.sendMessage("getcard");
    if(newcard == "none")
    {
        timer2->stop();
        ui->turn->setText("Больше карт в колоде нет. Игра окончена: ничья.");
        return;
    }
    client.cardset.push_back(newcard);
    QString path = "/home/daria/cours_nets/DominoCardGame/cards/PNG/" + QString::fromUtf8(newcard.c_str())+ ".png";
    QPixmap pm(path); // path to image file
    pm = pm.scaled(100,153);

    int n = card_labels.size();
    QLabel* newlabel = new QLabel();
    card_labels.push_back(newlabel);
    card_labels[n]->setPixmap(pm);
    ui->cards->addWidget(card_labels[n],0,n);

    QPushButton* newbutton = new QPushButton("Выбрать "+QString::number(n));
    card_buttons.push_back(newbutton);
    connect(card_buttons[n], SIGNAL(clicked()), this, SLOT(buttonClicked()));
    ui->cards->addWidget(card_buttons[n],1,n);
}

void MainWindow::on_rules_clicked()
{
    QLabel* newlabel = new QLabel();
    newlabel->setContentsMargins(20,20,20,20);
    newlabel->setText(" Домино́ — карточная игра. Применяется 52−карточная колода. Участвуют от 2 до 6 игроков. Цель игры − избавиться от всех карт на руках.\nВсем участвующим в игре раздаётся по 7 карт, а оставшаяся колода кладётся взакрытую рядом и верхняя карта снимается.\nНа эту карту по очереди игрок, сидящий следом за сдающим, должен положить карту либо серию карт, которые составляют\nпоследовательности с ней, вне зависимости от масти. Например: на 7 можно положить 8 либо 6, либо 6, 5, 4, либо 8, 9, 8, 7 и т. д.\nТуз является «поворотной» картой: его и на него можно положить и короля, и двойку.\n\n Игрок обязан снести карту, если есть такая возможность.\nЕсли игрок не может выложить ни одну из карт, то он берёт карту из колоды.\nЕсли она подходит, то он кладёт её на кон и тянет следующую (либо выкладывает карту с рук, если теперь это возможно),\nесли не подходит, то забирает себе и передаёт ход дальше.\n\n   Выигрывает тот, кто первым избавится от своих карт на любой стадии игры, даже если остались ещё карты в колоде.\nЕсли таких нет, то считается что игра не состоялась (ничья).\n\nЭТА РЕАЛИЗАЦИЯ ИГРЫ - ДЛЯ ТРЕХ ИГРОКОВ");
    newlabel->show();
}

void MainWindow::on_finish_clicked()
{
    if(client.turn != client.id)
        return;
    std::string response = client.sendMessage("finishmove");
    ui->fres->setText(QString::fromUtf8(response.c_str()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
