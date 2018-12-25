#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <vector>

#define MSG_LEN 2048
#define FILESIZE_LEN 64
#define SOCK_ADDR "localhost"
#define SOCK_PORT 31337
#define FILENAME "file.txt"

class Client
{
public:
    Client(){}
    std::string sendMessage(std::string message);
    int start();
    std::string getResponce();
    bool active;
    int turn;
    int id;
    std::vector<std::string> cardset;

private:
    int sock;

};

#endif // CLIENT_H
