#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "deck.h"
#include <string>
#include <iostream>


#define MSG_LEN 2048
#define FILESIZE_LEN 100
#define SOCK_ADDR "localhost"
#define SOCK_PORT 31337
#define MAX_CLIENTS 10 // 10 games 3 players each



class Server
{
public:
    Server(){
        start_card.rang = -1;
    }
    void manageConnection(unsigned int fd);
    void manageClient(unsigned int fd, unsigned int client_id);
    void startServer();
private:
    int clients[MAX_CLIENTS] = { 0 };
    Deck deck = Deck();
    card start_card;

    bool game_is_on = 0;
    int winner = -1;
    int turn = -1; // чей ход
    int lastmove = -1; // -1 - не ходил; 0 - положил; 1 - взял; 2 - положил после того как взял
};

#endif // SERVER_H
