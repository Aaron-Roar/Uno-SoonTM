#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
    
#define PORT     8090
#define MAXLINE 1024

typedef enum NetToken {NETERROR, NETJOIN, NETREADY, NETSENDCARD, NETREQUESTCARD, NETLOBBY}NetToken;
typedef enum ErrorToken {}ErrorToken;
typedef enum UIToken {UINONE, UIREADY, UICARD, UIUNO, UINAME}UIToken;
typedef enum CardToken {}CardToken;


typedef struct Player {
    char id;
    char name[20];
    Cards hand; //Could support multiple decks but would have to 
                 //determine which deck for client to put card when 
                 //recieved
} Player;

typedef struct CardTransfer {
    Player destination;
    Card card;
} CardTransfer;

typedef struct LobbyStatus {
    Player players[10];
    char ready_states[10];

} LobbyStatus;

typedef union Payloads {
    CardTransfer transfer;
    LobbyStatus lobby;
    Player player_info;
} Payloads;

typedef struct NetMsg {
    Player src;

    NetToken token;
    Payloads payload;
} NetMsg;


unsigned int addr_len = sizeof(struct sockaddr_in);
int sockfd;
struct sockaddr_in servaddr, clientaddr;

//Client Interface
namespace ClientBackend {

    Player client;
    LobbyStatus lobby;
    
    void createIdentity(char* name) {
        int i = 0;
        while(i < 20) {
    
            client.name[i] = name[i];
            i += 1;
        }
        client.name[19] = '\0';
    
    }
    
    void updateLobby(LobbyStatus some_lobby) {
        int i = 0;
        while(i < 10) {
            lobby.players[i] = some_lobby.players[i];
            lobby.ready_states[i] = some_lobby.ready_states[i];
            i += 1;
        }
    }
    
    char joinServer() {
        //Setting server details
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
    
    
        //Telling server we exist and our name
        NetMsg first_contact;
        first_contact.token = NETJOIN;
        first_contact.src = client;
        sendto(sockfd, (NetMsg*)&first_contact, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    
    
        //Getting our assigned id from server
        char id;
        recvfrom(sockfd, (char*)&id, sizeof(char), MSG_WAITALL, NULL, NULL);
        client.id = id;

        NetMsg lobby_state;
        recvfrom(sockfd, (NetMsg*)&lobby_state, sizeof(NetMsg), MSG_WAITALL, NULL, NULL);
        updateLobby(lobby_state.payload.lobby);

        return 0;
        //Make first conact and populate lobby
    }
    
    void readyUp() {
        NetMsg ready_msg;
        ready_msg.token = NETREADY;
        ready_msg.src = client;
    
        sendto(sockfd, (NetMsg*)&ready_msg, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    
    }
    
    char sendCard(char id, Card card) {
        int i = 0;
        while(i < max_playes) {
            if(lobby.players[i].id == id){
                NetMsg card_to_transfer;
                card_to_transfer.token = NETSENDCARD;
                card_to_transfer.payload.transfer.destination = lobby.players[i];
                card_to_transfer.payload.transfer.card = card;
    
                sendto(sockfd, (NetMsg*)&card_to_transfer, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                return 0;
            }
            i += 1;
        }
    
        perror("[!](3) Unable to send card, Player to contact does not exist\n");
        return 3; //Error 3 (Player to contact doesnt exist)
    }
    
    char requestCard(char id, Card card) {
        int i = 0;
        while(i < max_playes) {
            if(lobby.players[i].id == id){
                NetMsg card_to_transfer;
                card_to_transfer.token = NETREQUESTCARD;
                card_to_transfer.payload.transfer.destination = lobby.players[i];
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
    LobbyStatus lobby = {0};
    struct sockaddr_in address_list[10];
    char ready_check[10] = {0};
    
    //Connected players
    char player_count = 0;
    
    //Configuring initial values
    unsigned int addr_size = sizeof(sockaddr_in);
    
    //FD reference for socket
    int sockfd;
    
    //Server Address
    Player server_identity;

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
    
    int indexOfClient(Player client) {
        int i = 0;
        while(i < max_playes) {
            if(lobby.players[i].id == client.id) {
                printf("Client at index: %d, had id: %d\n", i, client.id);
                return i;
            }
    
            i += 1;
        }
    
        //Client not in the list
        return -1;
    }
    
    
    Player* addClient(Player client, struct sockaddr_in cliaddr) {
        int i = 0;
        while(i < max_playes) {
    
            //Placing client info in next available location
            if(lobby.players[i].id == 0) {
    
                //Copying client address information to address list
                address_list[i].sin_family = cliaddr.sin_family;
                address_list[i].sin_port = cliaddr.sin_port;
                address_list[i].sin_addr = cliaddr.sin_addr;
                printf("Clients Assigned id: %d\n", lobby.players[i].id);
    
    
    
                //Sending client their newly generated id
                client.id = (char)(rand()%20) + 1;
                lobby.players[i] = client;
                sendto(sockfd, (char*)&client.id, sizeof(char), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, sizeof(cliaddr));
    
                printf("Sending Lobby Status to client\n");
                NetMsg lobby_state;
                lobby_state.token = NETLOBBY;
                lobby_state.src = server_identity;
                lobby_state.payload.lobby = lobby;
    
                sendto(sockfd, (NetMsg*)&lobby_state, sizeof(NetMsg), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, sizeof(cliaddr));
    
                player_count += 1;
                return &lobby.players[i];
            }
            i += 1;
        }
        return 0;
    
        //Refused connection too many player INOFRM CLIENT
    }
    
    
    void readyUpClient(Player client, struct sockaddr_in cliaddr) {
        int index = indexOfClient(client);
        printf("Index of client: %d\n", index);
    
        //Client wasnt in the list add them to the lobby
        if(index < 0) {
            addClient(client, cliaddr);
        }
        ready_check[index] = 1;
    
    }
    
    char readyStatus() {
        char ready_sum = 0;
        int i = 0;
        while(i < max_playes) {
            if(ready_check[i] != 0)
                ready_sum += 1;
            i += 1;
        }
        printf("Ready sum: %d, player count: %d\n", ready_sum, player_count);
        if(ready_sum >= player_count && ready_sum > 0)
            return 1;
        return 0;
    }
    
    char creatLobby() {
        struct sockaddr_in cliaddr;
        while(1) {
            //Check if full lobby
            if(player_count > 10)
                return player_count;
    
            //All client ready begin game!!
            if(readyStatus() == 1) {
                printf("starting the game\n");
                return player_count;
            }
    
            //Take new msg from client
            cliaddr = {0};
            NetMsg client_msg;
            recvfrom(sockfd, (NetMsg*)&client_msg, sizeof(NetMsg), MSG_WAITALL, (struct sockaddr*)&cliaddr, &addr_size);
    
            char token = client_msg.token;
            printf("Client sent message of type: %d\n", token);
    
            //New client to join
            if(token == NETJOIN) {
                printf("Assing new client to server\n");
                addClient(client_msg.src, cliaddr);
            }
    
            //Client wants to toggle ready status
            else if(token == NETREADY) {
                printf("Readying up a client\n");
                readyUpClient(client_msg.src, cliaddr);
            }
    
    
        }
    
    }
}
