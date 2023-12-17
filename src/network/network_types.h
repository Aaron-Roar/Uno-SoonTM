typedef enum NetToken {NETERROR, NETJOIN, NETREADY, NETSENDCARD, NETREQUESTCARD, NETLOBBY}NetToken;
typedef enum ErrorToken {}ErrorToken;
typedef enum UIToken {
    UINONE, 
    UINAME,
    UIJOIN,
    UILEAVE,
    UIREADY, 
    UITAKECARD, 
    UIGIVECARD,
    UIUNO 
}UIToken;
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
    char notification[100];
} Payloads;


typedef struct NetMsg {
    Player src;
    Player dst;

    NetToken token;
    Payloads payload;
} NetMsg;

