#define PORT     8080

const char total_cards = 100;
const char max_playes = 10;
//

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "../cards.cpp"

#include "../network/network_types.h"
#include "../network/network.cpp"
#include "../render/pregame.cpp"
    

//A temporary game loop for a pregame session
char creatLobby() {
    struct sockaddr_in cliaddr{0};
    NetMsg client_msg{0};

    while(1) {
        //Check if full lobby
        if(ServerBackend::player_count > 10)
            return ServerBackend::player_count;

        //All client ready begin game!!
        if(ServerBackend::readyStatus() == 1) {
            printf("starting the game\n");
            return ServerBackend::player_count;
        }

        //Take new msg from client
        cliaddr = {0};
        client_msg = {0};
        recvfrom(sockfd, (NetMsg*)&client_msg, sizeof(NetMsg), MSG_WAITALL, (struct sockaddr*)&cliaddr, &ServerBackend::addr_size);

        char token = client_msg.token;

        //New client to join
        if(token == NETJOIN) {
            ServerBackend::addClient(client_msg.src, cliaddr);
        }

        //Client wants to toggle ready status
        else if(token == NETREADY) {
            ServerBackend::readyUpClient(client_msg.src, cliaddr);
        }

        NetMsg lobby{0};
        memcpy(&lobby.payload, &ServerBackend::server_lobby, sizeof(LobbyStatus));
        ServerBackend::broadCastMsg(lobby);


        Render::renderLobby(ServerBackend::server_identity, ServerBackend::server_lobby);


    }

}

int main() {

    strcpy(ServerBackend::server_identity.name, "Server\0");
    ServerBackend::setup();
    char lobby_size = creatLobby();
    printf("The game has begun\n");


    close(sockfd);
    return 0;
}

