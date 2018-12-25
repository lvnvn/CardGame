#include "server.h" 
#include <stdlib.h>

void Server::manageConnection(unsigned int fd) {
    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    int incom = accept(fd, (struct sockaddr*) &client_addr, (socklen_t*) &addrSize);
    if (incom < 0) {
        perror("Error in accept(): ");
        exit(-1);
    }
    //fcntl(incom, F_SETFL, O_NONBLOCK);

    printf("\nNew connection: \nfd = %d \nip = %s:%d\n", incom, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == 0) {
            clients[i] = incom;
            if(i == 2)
                game_is_on = 1;
            break;
        }
    }
}

void Server::manageClient(unsigned int fd, unsigned int client_id) {
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    int recvSize = recv(fd, msg, MSG_LEN, 0);
    if(recvSize > 0)
    {
        msg[recvSize] = '\0';
        //std::cout << "\ngot message: '" << msg << "' from client [" << client_id << "]";
        if(!strncmp(msg,"start",5))
        {
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            sprintf(response, "%u", (unsigned)client_id);
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"isover",6))
        {
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            if(game_is_on == 1)
                sprintf(response, "%d", -2);
            else
                sprintf(response, "%d", winner);
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"wait",4))
        {
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            if(!game_is_on)
                sprintf(response, "%u", 0);
            else
                sprintf(response, "%u", 1);
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"cardset",7))
        {
            std::string cardset = deck.takeSeven();
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            for(int i = 0; i < cardset.size(); i++)
                response[i] = cardset[i];
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"scard",5))
        {
            std::string startcard = "";
            if(start_card.rang == -1)
            {
                turn = 0;
                startcard = deck.takeCard();
                start_card.suit = (startcard.size()==3) ? startcard[2] : startcard[1];
                start_card.rang = (startcard.size()==3) ? std::stoi(startcard.substr(0,2)) : (startcard[0]-'0');
            }
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            if(start_card.rang > 9)
            {
                response[0] = start_card.rang/10 + '0';
                response[1] = start_card.rang%10 + '0';
                response[2] = start_card.suit;
            }
            else
            {
                response[0] = start_card.rang + '0';
                response[1] = start_card.suit;
            }
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"getcard",7))
        {
            std::string card = deck.takeCard();
            if(card == "none")
                game_is_on = 0;
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            for(int i = 0; i < card.size(); i++)
                response[i] = card[i];
            write(fd,response,sizeof(response));
            lastmove = 1;
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"turn",4))
        {
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            sprintf(response, "%u", turn);
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"finishmove",10))
        {
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            if(lastmove == -1 || lastmove == 2)
                sprintf(response,"Чтобы закончить ход необходимо взять карту");
            else
            {
                turn = (turn+1)%3;
                lastmove = -1;
                sprintf(response, "ok");
            }
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"putcard",7))
        {
            card cd;
            if(msg[9] == ' ')
            {
                cd.rang = msg[7] - '0';
                cd.suit = msg[8];
            } else
            {
                cd.rang = (msg[7]-'0')*10 + (msg[8]-'0');
                cd.suit = msg[9];
            }
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            if((abs(cd.rang - start_card.rang) == 1) || (abs(cd.rang - start_card.rang) == 12)) // карту можно положить
            {
                sprintf(response, "ok");
                start_card.rang = cd.rang;
                start_card.suit = cd.suit;
                if(lastmove <= 0) //lastmove == -1 || lastmove == 0
                    lastmove = 0;
                else if(lastmove > 0) //lastmove == 1 || lastmove == 2
                    lastmove = 2;
            }
            else
               sprintf(response, "Этой картой нельзя сходить");
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"win",3))
        {
            winner = client_id;
            game_is_on = 0;
            char response[FILESIZE_LEN];
            memset(response, 0, FILESIZE_LEN);
            sprintf(response, "0");
            write(fd,response,sizeof(response));
            //std::cout << "\nresponded: " << response;
        }
        if(!strncmp(msg,"end",3)) // disconnect
        {
            char response[FILESIZE_LEN];
            sprintf(response, "You are disconnected");
            write(fd,response,sizeof(response));
            getpeername(fd, (struct sockaddr*) &client_addr, (socklen_t*) &addrSize);
            printf("Client disconnected %s:%d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            close(fd);
            clients[client_id] = 0;
        }
    }
}

void Server::startServer()
{
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        std::cout << "Error in sock(): ";
        exit(-1);
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = PF_INET;
    client_addr.sin_port = htons(SOCK_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY; //any address for binding

    if (bind(listener, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0)
    {
        std::cout << "Error in bind():";
        exit(-1);
    }
    printf("Server is listening on the %d port...\n", SOCK_PORT);

    if (listen(listener, 3) < 0)
    {
        std::cout << "Error in listen(): ";
        exit(-1);
    }
    printf("Waiting for the connections...\n");

    for (;;) {
        fd_set readfds; //set of file descriptors that will monitor select()
        int max_fd;  //The largest file descriptor in the set
        int active_clients_count; //number of sockets for which select() has fixed activity

       // Empty the set and add the listener
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        max_fd = listener;

        //add to the set of all already connected clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i];
            if (fd > 0) {
                FD_SET(fd, &readfds);
            }
            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }

        active_clients_count = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (active_clients_count < 0 && (errno != EINTR))
        {
            std::cout << "Error in select():" << active_clients_count;
            exit(-1);
        }

        //if the listener was active than the connection came
        if (FD_ISSET(listener, &readfds)) {
            manageConnection(listener);
        }

        // check if there was activity on one of the clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i];
            if ((fd > 0) && FD_ISSET(fd, &readfds)) {
                manageClient(fd, i);
            }
        }
    }
}
