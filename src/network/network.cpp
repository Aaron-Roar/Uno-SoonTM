#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

unsigned int addr_len = sizeof(struct sockaddr_in);
int sockfd;
struct sockaddr_in servaddr, clientaddr;

int indexOfPlayer(Player player, LobbyStatus lobby) {
    int i = 0;
    while(i < max_playes) {
        if(lobby.players[i].id == player.id) {
            return i;
        }

        i += 1;
    }

    //Client not in the list
    return -1;
}

NetMsg checkMsg() {
    //Setting file descriptor
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sockfd, &rfd);

    //Timeout 0-1 second
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    //Check if the fd has data ready to read
    int ready = select(1, &rfd, NULL, NULL, &tv);
    if(ready) {
        //If ready read a msg
        NetMsg srv_msg{0};
        recvfrom(sockfd, (NetMsg*)&srv_msg, sizeof(NetMsg), MSG_WAITALL, NULL, NULL);
        return srv_msg;
    }
    //Not ready return 0
    return NetMsg{0};
}

//Client Interface
namespace ClientBackend {

    Player client{0};
    LobbyStatus client_lobby{0};

    //Checks if server msg in buffer and returns msg if exist
    
    
    //Updates the clients lobby with a new lobby version
    void updateLobby(LobbyStatus lobby) {
        int i = 0;
        while(i < 10) {
            client_lobby.players[i] = lobby.players[i];
            client_lobby.ready_states[i] = lobby.ready_states[i];
            i += 1;
        }
    }

    //Set the clients initial networking properties
    char setup() {
        servaddr = {0};
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = INADDR_ANY;
    
        //Asking hardware for a fd for networking
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        return 1;
    }
    
    //Request to join the server providing a name and receiving back 
    //an id
    char joinServer() {
        if(client.id != 0)
            return client.id;
    
        //Telling server we exist and our name
        NetMsg first_contact{0};
        first_contact.token = NETJOIN;
        first_contact.src = client;
        sendto(sockfd, (NetMsg*)&first_contact, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
        printf("Sent server join request\n");
    
        //Server sends the lobby status and client id
        NetMsg lobby_state = {0};
        recvfrom(sockfd, (NetMsg*)&lobby_state, sizeof(NetMsg), MSG_WAITALL, NULL, NULL);
        printf("recieved lobby info\n");
        client.id = lobby_state.dst.id;
        updateLobby(lobby_state.payload.lobby);

        return 0;
        //Make first conact and populate lobby
    }
    
    //Tells the server the client is ready to start the game
    void readyUp() {
        NetMsg ready_msg;
        ready_msg.token = NETREADY;
        ready_msg.src = client;
    
        sendto(sockfd, (NetMsg*)&ready_msg, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    
    }
    
    //Sends a card to the server that can be asked to redirect to a 
    //player
    char sendCard(char id, Card card) {
        int i = 0;
        while(i < max_playes) {
            if(client_lobby.players[i].id == id){
                NetMsg card_to_transfer;
                card_to_transfer.token = NETSENDCARD;
                card_to_transfer.payload.transfer.destination = client_lobby.players[i];
                card_to_transfer.payload.transfer.card = card;
    
                sendto(sockfd, (NetMsg*)&card_to_transfer, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                return 0;
            }
            i += 1;
        }
    
        perror("[!](3) Unable to send card, Player to contact does not exist\n");
        return 3; //Error 3 (Player to contact doesnt exist)
    }
    
    //Asks the server for a card or redirect to ask a player for a 
    //card
    char requestCard(char id, Card card) {
        int i = 0;
        while(i < max_playes) {
            if(client_lobby.players[i].id == id){
                NetMsg card_to_transfer;
                card_to_transfer.token = NETREQUESTCARD;
                card_to_transfer.payload.transfer.destination = client_lobby.players[i];
                card_to_transfer.payload.transfer.card = card;
    
                sendto(sockfd, (NetMsg*)&card_to_transfer, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                return 0;
            }
            i += 1;
        }
    
        perror("[!](3) Unable to send card, Player to contact does not exist\n");
        return 3; //Error 3 (Player to contact doesnt exist)
    }

}


namespace ServerBackend {
    //Index corrosponding lists
    LobbyStatus server_lobby = {0};
    struct sockaddr_in address_list[10];
    
    //Connected players
    char player_count = 0;
    
    //Configuring initial values
    unsigned int addr_size = sizeof(sockaddr_in);
    
    
    //Server Address
    Player server_identity;

    //Setteing networking properties of server
    void setup() {
        //Server Address
        servaddr = {0};
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(PORT);

        // Creating socket file descriptor
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if ( sockfd < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Bind the socket with the server address
        int error = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        if ( error < 0 )
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
    }
    
    char updateClientLobbies(LobbyStatus lobby) {
        NetMsg msg;
        msg.token = NETLOBBY;
        msg.src = server_identity;
        msg.payload.lobby = lobby;

        int i = 0;
        while(i < 10) {
            if(server_lobby.players[i].id != 0) {
                msg.dst = server_lobby.players[i];
                sendto(sockfd, (NetMsg*)&msg, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&address_list[i], sizeof(sockaddr_in));
            }
            i += 1;
        }
    }    
    
    //Adds a new client to be in the servers lobby and address tables
    Player* addClient(Player client, struct sockaddr_in cliaddr) {
        if(client.id != 0) {
            printf("Client already connected!!");
            NetMsg lobby_state;
            lobby_state.dst = client;
            lobby_state.token = NETLOBBY;
            lobby_state.src = server_identity;
            lobby_state.payload.lobby = server_lobby;
    
            sendto(sockfd, (NetMsg*)&lobby_state, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, sizeof(sockaddr_in));
            return 0;
        }

        int i = 0;
        while(i < max_playes) {
    
            //Placing client info in next available location
            if(server_lobby.players[i].id == 0) {
    
                //Copying client address information to address list
                memcpy(&address_list[i], &cliaddr, sizeof(sockaddr_in));
    
    
    
                //Sending client their newly generated id
                client.id = (char)(rand()%253) + 1;
                server_lobby.players[i] = client;
    
                //Send client their id and lobby info
                NetMsg lobby_state;
                lobby_state.dst = server_lobby.players[i];
                lobby_state.token = NETLOBBY;
                lobby_state.src = server_identity;
                lobby_state.payload.lobby = server_lobby;
    
                sendto(sockfd, (NetMsg*)&lobby_state, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, sizeof(sockaddr_in));
    
                player_count += 1;
                return &server_lobby.players[i];
            }
            i += 1;
        }
        return 0;
    
        //Refused connection too many player INOFRM CLIENT
    }
    
    
    //Readies ip a client
    void readyUpClient(Player client, struct sockaddr_in cliaddr) {
        int index = indexOfPlayer(client, server_lobby);
    
        //Client wasnt in the list add them to the lobby
        if(index < 0) {
            addClient(client, cliaddr);
        }
        server_lobby.ready_states[index] = 1;
    
    }
    
    //Checking if all clients are ready
    char readyStatus() {
        char ready_sum = 0;
        int i = 0;
        while(i < max_playes) {
            if(server_lobby.ready_states[i] != 0)
                ready_sum += 1;
            i += 1;
        }

        if(ready_sum >= player_count && ready_sum > 0)
            return 1;
        return 0;
    }

    char broadCastMsg(NetMsg msg) {
        msg.src = server_identity;
        msg.token = NETLOBBY;

        int i = 0;
        while(i < 10) {
            if(server_lobby.players[i].id != 0) {
                msg.dst = server_lobby.players[i];
                sendto(sockfd, (NetMsg*)&msg, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&address_list[i], sizeof(sockaddr_in));
                printf("SentBroadCast\n");
            }
            i += 1;
        }
        return 1;
    }
    
}
