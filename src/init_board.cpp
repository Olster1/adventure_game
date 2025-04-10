void initPlayerBoard(GameState *gameState) {
    gameState->cameraPos.x = CHUNK_DIM*0.5f;
    gameState->cameraPos.y = CHUNK_DIM*0.5f;
    Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 0, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addKnightEntity(gameState, boardP);
    }

    c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 1, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addGoblinTntEntity(gameState, boardP);
    }

    c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 2, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(2*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addPeasantEntity(gameState, boardP);
    }

    c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 3, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(3*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addArcherEntity(gameState, boardP);
    }

    c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 4, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(4*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addGoblinEntity(gameState, boardP);
    }
}