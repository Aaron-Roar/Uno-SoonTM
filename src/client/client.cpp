//Global Variables
const char total_cards = 100;
const char max_playes = 10;
//
 #define PORT 8080

#include <stdio.h>

#include "../cards.cpp"

#include "../network/network_types.h"
#include "../network/network.cpp"
#include "../render/pregame.cpp"

FILE* user_input_file;
char user_input_file_name[100];
char user_input_buffer[20];

long prev_meta_state = 0;
long current_meta_state = 0;

typedef union UIPayload {

    Card card;

    //Data such as calling UNO or a name
    char info[20];
} UIPayload;

typedef struct UserInput {
    UIToken token;
    UIPayload user_input;
}UserInput;

//Error Numbers
//1: Players Has Too Many cards and cannot add any more!!
//2: Player does not have the card to remove
//3: Player to contact does not exist



///////////////////////////////////Testing
Card* addCard(Card card) {
    int i = 0;
    while(i < total_cards) {
        if(ClientBackend::client.hand.cards[i].number == 0) {
            ClientBackend::client.hand.cards[i] = card;
            return &ClientBackend::client.hand.cards[i];
        }
        i += 1;
    }
    return 0;
}

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
void createIdentity(char* name) {
    int i = 0;
    while(i < 20) {

        if(name[i] != '\n')
            ClientBackend::client.name[i] = name[i];

        i += 1;
    }
    ClientBackend::client.name[19] = '\0';

}


UserInput readUserInput() {
    memset(user_input_buffer, 0, sizeof(char)*20);

    user_input_file = fopen(user_input_file_name, "r");
    fgets(user_input_buffer, 20, user_input_file);
    fclose(user_input_file);

    user_input_file = fopen(user_input_file_name, "w");
    fclose(user_input_file);


    UserInput uio;
    memset(&uio, 0, sizeof(UserInput));


    if(user_input_buffer[0] - 48 == UINAME) {
        //BuildINFO ready ect
        uio.token = UINAME;
        int i = 0;
        while(i < 20) {
            uio.user_input.info[i] = user_input_buffer[i + 1];
            i += 1;
        }
        uio.user_input.info[19] = '\0';

    }
    else if(user_input_buffer[0] - 48 == UIJOIN) {
        uio.token = UIJOIN;
    }
    else if(user_input_buffer[0] - 48 == UILEAVE) {
        uio.token = UILEAVE;
    }
    else if(user_input_buffer[0] - 48 == UIREADY) {
        uio.token = UIREADY;
    }
    else if(user_input_buffer[0] - 48 == UITAKECARD) {
        uio.token = UITAKECARD;
        //BUILD CARD
        //Provide who in input
    }
    else if(user_input_buffer[0] - 48 == UIGIVECARD) {
        uio.token = UIGIVECARD;
        //BUILD CARD
        //Provide who in input
    }
    else if(user_input_buffer[0] - 48 == UIUNO) {
        uio.token = UIUNO;
    }
    //Erasing file contents for next input reading
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

char updateUIO(UserInput uio) {
    if(uio.token == UINONE) {
    }
    else if(uio.token == UINAME) {
        if(ClientBackend::client.id == 0)
            createIdentity(uio.user_input.info);
    }
    else if(uio.token == UIJOIN) {
        ClientBackend::joinServer();
    }
    else if(uio.token == UIREADY) {
        ClientBackend::readyUp();
    }

    char state = checkState(ClientBackend::client);
    if(state == 1)
        return 1;
    return 0;
}
char updateNetwork(NetMsg msg) {
    if(msg.token == NETLOBBY) {
        memcpy(&ClientBackend::client_lobby, &msg.payload.lobby, sizeof(LobbyStatus));
        printf("Got Lobby\n");
    }
    else if(msg.token == NETREQUESTCARD) {
        //RemoveCard
        ClientBackend::sendCard(0, msg.payload.transfer.card);
    }
    else if(msg.token == NETSENDCARD) {
        //AddCard
        addCard(msg.payload.transfer.card);
    }

    return 1;
}

char update(UserInput uio, NetMsg msg) {
    return updateUIO(uio)||updateNetwork(msg);
}




int main(int argc, char* argv[]) {
    strcpy(user_input_file_name, argv[1]);
    //Structure to store user input
    UserInput uio;
    ClientBackend::setup();

    while(1) {
        //Read user input
        memset(&uio, 0, sizeof(UserInput));
        uio = readUserInput();

        //Read server input
        NetMsg srv_msg = checkMsg();


        //Update game state and server
        char delta_state = update(uio, srv_msg);

        Render::renderLobby(ClientBackend::client, ClientBackend::client_lobby);
    }


    return 0;

}
