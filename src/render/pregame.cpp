namespace Render {
    void renderLobby(Player self, LobbyStatus lobby) {
        printf("-------------------------------------\n");
        printf("<>UserName: %s<>ID: %d<>\n", self.name, self.id);
    
        int i = 0;
        while(i < 10) {
            Player some_player = lobby.players[i];
            if(some_player.id != 0) {
                printf("|Player: %s|Ready: %d|\n", some_player.name, lobby.ready_states[i]);
            }
            i += 1;
        }
        printf("-------------------------------------\n");
    }
}
