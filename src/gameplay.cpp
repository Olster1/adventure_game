enum GameTurnType {
    GAME_TURN_PLAYER_0,
    GAME_TURN_PLAYER_1,
};

struct GamePlay {
    GameTurnType turnOn;
    float turnTime;
    float maxTurnTime;
};

GamePlay init_gameplay() {
    GamePlay gamePlay = {};

    gamePlay.turnOn = GAME_TURN_PLAYER_0;

    gamePlay.turnTime = 0;
    gamePlay.maxTurnTime = 60*5; //NOTE: 5 mintues

    return gamePlay;
}