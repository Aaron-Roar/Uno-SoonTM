//Global Variables
const char total_cards = 100;
const char max_playes = 10;
//

#include <stdio.h>
#include "cards.cpp"
#include "network.cpp"

FILE* user_input_file;
char user_input_buffer[20];

long prev_meta_state = 0;
long current_meta_state = 0;


//Error Numbers
//1: Players Has Too Many cards and cannot add any more!!
//2: Player does not have the card to remove
//3: Player to contact does not exist



///////////////////////////////////Testing
Card createCard(char color, char number) {
    return (Card){
        .color = color,
        .number = number
    };
}
void printCard(Card card) {
    printf("Color: %d Number: %d\n", card.color, card.number);
}
void printCards(Player player) {
    Cards hand = player.hand;

    int i = 0;
    while(i < total_cards) {
        if(compareCard(hand.cards[i], (Card){0}) != 0) {
            printCard(hand.cards[i]);
        }
        i += 1;
    }
}

void printLobby(LobbyStatus lobby) {
    int i = 0;
    while(i < max_playes) {
        printf("%s\n", lobby.players[i].name);
        i += 1;
    }
}

//////////////////////////////

typedef union UIPayload {

    Card card;

    //Data such as calling UNO or a name
    char info[20];
} UIPayload;

typedef struct UserInput {
    UIToken token;
    UIPayload user_input;
}UserInput;

UserInput readUserInput() {
    memset(user_input_buffer, 0, sizeof(char)*20);

    user_input_file = fopen("../client_input.txt", "r");
    fgets(user_input_buffer, 20, user_input_file);
    fclose(user_input_file);


    UserInput uio;
    memset(&uio, 0, sizeof(UserInput));

    if(user_input_buffer[0] == '#') {
        //Build Card
        uio.user_input.card = createCard(user_input_buffer[1], user_input_buffer[2]);

        user_input_file = fopen("../client_input.txt", "w");
        fclose(user_input_file);
        memset(user_input_buffer, 0, sizeof(char)*20);
        return uio;
    }

    else if(user_input_buffer[0] == '@') {
        //BuildINFO ready ect
        uio.token = UINAME;
        int i = 0;
        while(i < 20) {
            uio.user_input.info[i] = user_input_buffer[i + 1];
            i += 1;
        }
        uio.user_input.info[19] = '\0';

        //Erasing file contents for next input reading
        user_input_file = fopen("../client_input.txt", "w");
        fclose(user_input_file);
        memset(user_input_buffer, 0, sizeof(char)*20);
        return uio;
    }

    memset(user_input_buffer, 0, sizeof(char)*20);
    return uio;

}


//Would compaer the entire data for the game and see if its changed
char checkState(Player client) {
    long state = 0;

    int i = 0;
    while(i < sizeof(Player)/sizeof(char)) {
        state += ((char*)&client)[i];
        i += 1;
    }

    prev_meta_state = current_meta_state;
    current_meta_state = state;

    if(current_meta_state == prev_meta_state) {
        //Do nothing game has not changed
        return 0;
    }

    return 1;
}

char update(UserInput uio) {
    if(uio.token == UINONE) {
    }
    else if(uio.token == UINAME) {
        ClientBackend::createIdentity(uio.user_input.info);
    }

    char state = checkState(ClientBackend::client);
    if(state == 1)
        return 1;
    return 0;
}

int main() {

    //Structure to store user input
    UserInput uio;

    while(1) {
        //Read user input, looking for string
        memset(&uio, 0, sizeof(UserInput));
        uio = readUserInput();
                
        char state = update(uio);

        if(state == 1)
            printf("Name:%s\n", ClientBackend::client.name);

    }

//    ClientBackend::joinServer();

//    //Lobby Loop
//    while(1) {
//        uio = readUserInput();
//        //Check Server Msgs
//
//        if(uio.info[0] == NETREADY)
//            ClientBackend::readyUp();
//
//        printLobby(ClientBackend::lobby);
//    }


//    close(sockfd);

    return 0;

}
