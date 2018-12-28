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
            /*if(i == 2)
                game_is_on = 1;*/
            if(i % 3 == 2)
                game_is_on[i/3] = 1;
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
        int game_number = client_id/3;
        msg[recvSize] = '\0';
        char response[FILESIZE_LEN];
        memset(response, 0, FILESIZE_LEN);
        std::cout << "\n\ngot message: '" << msg << "' from client [" << client_id << "]";
        if(!strncmp(msg,"start",5))
        {
            sprintf(response, "%u", (unsigned)client_id);
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"isover",6))
        {
            if(game_is_on[game_number])
                sprintf(response, "%d", -2);
            else
                sprintf(response, "%d", winners[game_number]);
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"wait",4))
        {
            if(!game_is_on[game_number])
                sprintf(response, "%u", 0);
            else
                sprintf(response, "%u", 1);
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"cardset",7))
        {
            std::string cardset = decks[game_number].takeSeven();
            for(int i = 0; i < cardset.size(); i++)
                response[i] = cardset[i];
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"scard",5))
        {
            std::string startcard = "";
            if(start_cards[game_number].rang == -1)
            {
                turn[game_number] = game_number*3;
                startcard = decks[game_number].takeCard();
                start_cards[game_number].suit = (startcard.size()==3) ? startcard[2] : startcard[1];
                start_cards[game_number].rang = (startcard.size()==3) ? std::stoi(startcard.substr(0,2)) : (startcard[0]-'0');
            }
            if(start_cards[game_number].rang > 9)
            {
                response[0] = start_cards[game_number].rang/10 + '0';
                response[1] = start_cards[game_number].rang%10 + '0';
                response[2] = start_cards[game_number].suit;
            }
            else
            {
                response[0] = start_cards[game_number].rang + '0';
                response[1] = start_cards[game_number].suit;
            }
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"getcard",7))
        {
            std::string card = decks[game_number].takeCard();
            if(card == "none")
            {
                game_is_on[game_number] = 0;
                winners[game_number] = -1;
            }
            for(int i = 0; i < card.size(); i++)
                response[i] = card[i];
            write(fd,response,sizeof(response));
            lastmove[game_number] = 1;
        }
        if(!strncmp(msg,"turn",4))
        {
            sprintf(response, "%u", turn[game_number]);
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"finishmove",10))
        {
            if(lastmove[game_number] == -1 || lastmove[game_number] == 2)
                sprintf(response,"Чтобы закончить ход необходимо взять карту!");
            else
            {
                turn[game_number] = (turn[game_number] % 3 != 2) ? (turn[game_number] + 1) : (game_number * 3);
                lastmove[game_number] = -1;
                sprintf(response, "Ваш ход завершён.");
            }
            write(fd,response,sizeof(response));
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
            if((abs(cd.rang - start_cards[game_number].rang) == 1) || (abs(cd.rang - start_cards[game_number].rang) == 12)) // карту можно положить
            {
                sprintf(response, "ok");
                start_cards[game_number].rang = cd.rang;
                start_cards[game_number].suit = cd.suit;
                if(lastmove[game_number] <= 0) //lastmove == -1 || lastmove == 0
                    lastmove[game_number] = 0;
                else if(lastmove[game_number] > 0) //lastmove == 1 || lastmove == 2
                    lastmove[game_number] = 2;
            }
            else
               sprintf(response, "Этой картой нельзя сходить");
            write(fd,response,sizeof(response));
        }
        if(!strncmp(msg,"win",3))
        {
            winners[game_number] = client_id;
            game_is_on[game_number] = 0;
            sprintf(response, "0");
            write(fd,response,sizeof(response));
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
        std::cout << "\nresponded: " << response << " game_is_on: " << game_is_on[game_number] << " turn " << turn[game_number] << " winner " << winners[game_number];
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
