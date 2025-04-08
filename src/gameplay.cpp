enum GameTurnType {
    GAME_TURN_PLAYER_KNIGHT,
    GAME_TURN_PLAYER_GOBLIN,
};

struct GamePlay {
    GameTurnType turnOn;
    float turnTime;
    float maxTurnTime;
    int turnCount;
    int maxTurnCount;
};

GamePlay init_gameplay() {
    GamePlay gamePlay = {};

    gamePlay.turnOn = GAME_TURN_PLAYER_KNIGHT;

    gamePlay.turnTime = 0;
    gamePlay.maxTurnTime = 60*5; //NOTE: 5 mintues
    gamePlay.maxTurnCount = 10;
    gamePlay.turnCount = 0;

    return gamePlay;
}