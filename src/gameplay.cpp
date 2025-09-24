enum GameTurnType {
    GAME_TURN_PLAYER_KNIGHT,
    GAME_TURN_PLAYER_GOBLIN,
};

enum GameTurnPhase {
    GAME_TURN_PHASE_COMMAND, //Cast spells, activate hero abilities, use command abilities.
    GAME_TURN_PHASE_MOVE,
    GAME_TURN_PHASE_SHOOT,
    GAME_TURN_PHASE_CHARGE,
    GAME_TURN_PHASE_FIGHT,
    GAME_TURN_PHASE_BATTLESHOCK,
};

enum BuildingCostType {
    BUILDING_COST_KNIGHT_HOUSE = 0,
    BUILDING_COST_KNIGHT_TOWER = 1,

    BUILDING_COST_TOTAL_COUNT,
};

struct BuildingCost {
    int wood;
    int stone;
    BuildingCostType buildingType;
};


BuildingCost initBuildingCost(int wood, int stone, BuildingCostType buildingType) {
    BuildingCost result = {};

    result.wood = wood;
    result.stone = stone;
    result.buildingType = buildingType;

    return result;
}
struct GamePlay {
    bool boardInited;

    GameTurnType turnOn;
    float turnTime;
    float maxTurnTime;
    int turnCount;
    int maxTurnCount;
    GameTurnPhase phase;
    int treeCount;
    int stoneCount;
    
    BuildingCost buildingCosts[BUILDING_COST_TOTAL_COUNT];
};

GamePlay init_gameplay() {
    GamePlay gamePlay = {};

    gamePlay.turnOn = GAME_TURN_PLAYER_KNIGHT;
    gamePlay.phase = GAME_TURN_PHASE_MOVE;
    gamePlay.boardInited = false;

    gamePlay.turnTime = 0;
    gamePlay.maxTurnTime = 60*5; //NOTE: 5 mintues
    gamePlay.maxTurnCount = 10;
    gamePlay.turnCount = 0;
    gamePlay.treeCount = 0;
    gamePlay.stoneCount = 0;

    gamePlay.buildingCosts[BUILDING_COST_KNIGHT_HOUSE] = initBuildingCost(3, 2, BUILDING_COST_KNIGHT_HOUSE);
    gamePlay.buildingCosts[BUILDING_COST_KNIGHT_TOWER] = initBuildingCost(6, 4, BUILDING_COST_KNIGHT_TOWER);

    return gamePlay;
}
