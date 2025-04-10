void initPlayerBoard(GameState *gameState) {
    gameState->cameraPos.x = CHUNK_DIM*0.5f;
    gameState->cameraPos.y = CHUNK_DIM*0.5f;
    Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 0, 0, 0);
    assert(c);
    if(c) {
        float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
        addKnightEntity(gameState, boardP);
    }
    
}