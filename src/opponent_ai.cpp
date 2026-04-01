void updateAiIfNeed(GameState *gameState, float dt) {
    if(!gameState->gamePlay.aiTakingTurn && gameState->gamePlay.turnOn == GAME_TURN_PLAYER_GOBLIN) {
        gameState->gamePlay.aiTakingTurn = true;
        gameState->gamePlay.turnOn = GAME_TURN_PLAYER_KNIGHT;
        printf("HEY\n");
    }
}