#include <stdio.h>
const char total_cards = 100;
const char max_playes = 10;

//Error Numbers
//1: Players Has Too Many cards and cannot add any more!!
//2: Player does not have the card to remove
//3: Player to contact does not exist

//R:1 B:2 G:3 Y:4
typedef struct Card {
    char color;
    char number;
} Card;

char compareCard(Card card1, Card card2) {
    if(card1.color == card2.color && card1.number == card2.number)
        return 0;
    return 1;
}


typedef struct Cards {
    char length;
    Card cards[total_cards];

    char addCard(Card card) {
        int i = 0;
        while(i < total_cards) {
            if(compareCard(cards[i], (Card){0}) == 0) {
                cards[i] = card;
                return 0;
            }

            i += 1;
        }
        perror("[!](1) Player had too many cards to add another\n");
        return 1; //Error 1 (Too many cards cant add more)
    }

    char removeCard(Card card) {
        int i = 0;
        while(i < total_cards) {
            if(compareCard(cards[i], card) == 0) {
                cards[i] = {0};
                return 0;
            }

            i += 1;
        }
        perror("[!](2) Card to remove was not in the players deck\n");
        return 2; //Error 2 (The card to remove is not in the cards)
    }
} Cards;

typedef struct Player {
    char id;
    char name[10];
    Cards hand; //Could support multiple decks but would have to 
                 //determine which deck for client to put card when 
                 //recieved
} Player;


typedef struct Server {
    char server_id;
    Player client;

    Player player_list[max_playes];

    char serverInit(char server_id) {
        client.id = 47;
        client.hand = (Cards){0};
        return 0;
        //Make first conact and populate player_list
    }

    char sendCard(char id, Card card) {
        int i = 0;
        while(i < max_playes) {
            if(player_list[i].id == id){
                //networking stuff here instead
                player_list[i].hand.addCard(card);
                client.hand.removeCard(card);
                //
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
            if(player_list[i].id == id){
                //networking stuff here instead
                player_list[i].hand.removeCard(card);
                client.hand.addCard(card);
                //
                return 0;
            }
            i += 1;
        }

        perror("[!](3) Unable to request card, Player to contact does not exist\n");
        return 3; //Error 3 (Player to contact doesnt exist)

    }
} Server;

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
//////////////////////////////

int main() {
    Server server;
    server.serverInit(12);

    //dealer
    Player* dealer = &server.player_list[0];
    dealer->id = 12;
    dealer->hand.cards[0] = createCard(1, 1);
    dealer->hand.cards[1] = createCard(2, 1);
    dealer->hand.cards[2] = createCard(3, 1);
    dealer->hand.cards[3] = createCard(4, 1);
    //


    printCards(*dealer);

    Card b1 = (Card){.color = 2, .number = 1};
    server.requestCard(12, b1);
    server.sendCard(13, b1);

    printCards(*dealer);

}
