#include "client.h"
#include <vector>
#include <iostream>

std::string Client::sendMessage(std::string message) {

    std::vector<char> msg(message.size(),0);
    for(int i = 0; i < message.size(); i++)
        msg[i] = message[i];
    send(sock, &msg[0], msg.size(), 0);
    //std::cout << "\nsent message: " << message;

    char response[FILESIZE_LEN];
    memset(response, 0, FILESIZE_LEN);
    int n = read(sock, response, sizeof(response));
    while(n<0)
        n = read(sock, response, sizeof(response));
    //std::cout << "\ngot response: " << response;

    return response;
}

/*std::string Client::getResponce()
{
    char response[FILESIZE_LEN];
    for(int i = 0; i < FILESIZE_LEN; i++)
        response[i] = 0;
    int n = read(sock, response, sizeof(response));
    while(n<0)
        n = read(sock, response, sizeof(response));
    //std::cout << "\ngot message: " << response;
    return response;
}*/


int Client::start() {
    srand(time(NULL));

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error in sock(): ");
        return sock;
    }

    struct hostent* host = gethostbyname(SOCK_ADDR);
    if (!host) {
        perror("Error in gethostbyname(): ");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    server_addr.sin_addr = *((struct in_addr*) host->h_addr_list[0]);

    if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error in connect():");
        return -1;
    }
    fcntl(sock, F_SETFL, O_NONBLOCK);

    /*if (sendMessage(sock,std::string("abcdef"))) {
    printf("Error in send_message");
    return -1;
    }

    printf("Client closed\n");
    return 0;*/
}




