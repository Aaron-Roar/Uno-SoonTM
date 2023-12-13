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

