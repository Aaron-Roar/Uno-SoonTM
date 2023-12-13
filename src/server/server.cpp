#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
    
const char total_cards = 100;
const char max_playes = 10;
//

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "cards.cpp"
#include "network.cpp"
    

int main() {

    ServerBackend::setup();
    char lobby_size = ServerBackend::creatLobby();
    printf("The game has begun\n");


    close(sockfd);
    return 0;
}

