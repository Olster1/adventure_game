enum PlacementType {
    UNIT_PLACE_SAFE,
    UNIT_PLACE_EXACT,
};

void placeKnightUnit(GameState *gameState, float3 offset, PlacementType type) {
    DEBUG_TIME_BLOCK();
    int unitCount = 6;
    int rowCount = 2;
    int halfUnitCount = unitCount / rowCount;
    int widthScale = 2;

    if(type == UNIT_PLACE_SAFE) {
        //NOTE: Find a safe location to spawn the knight unit
        int pWidth = halfUnitCount*widthScale;
        int pHeight = rowCount;

        int maxChunkSearch = 10;
        bool searching = true;
        float safeAreaSize = pHeight*pWidth;
        float2 chunkOffset = getChunkPosForWorldP(offset.xy);
        for(int chunkY = 0; chunkY < maxChunkSearch && searching; chunkY++) {
            for(int chunkX = 0; chunkX < maxChunkSearch && searching; chunkX++) {
                
                float2 chunkP = make_float2(chunkX + chunkOffset.x, chunkY + chunkOffset.y);
                Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, chunkP.x, chunkP.y, 0, true, true);
                if(c) { 
                    float3 chunkWorldP = getChunkWorldP(c);
                    for(int localY = 0; localY < (CHUNK_DIM - pHeight)  && searching; localY++) {
                        for(int localX = 0; localX < (CHUNK_DIM - pWidth) && searching; localX++) {
                            bool looking = true;
                            int safeFound = 0;
                            for(int y = 0; y < pHeight && looking; y++) {
                                for(int x = 0; x < pWidth && looking; x++) {
                                    float2 tileP = make_float2(chunkWorldP.x + localX + x, chunkWorldP.y + localY + y);
                                    float mapHeight = getMapHeight(tileP.x, tileP.y);
                                    if(mapHeight == 1) {
                                        Tile *tile = c->getTile(localX + x, localY + y, mapHeight - chunkWorldP.z);
                                        if(tile && (tile->flags & TILE_FLAG_WALKABLE)) {
                                            safeFound++;
                                        } else {
                                            looking = false;
                                        }                     
                                    } else {
                                        looking = false;
                                    }
                                }
                            }

                            if(safeFound == safeAreaSize) {
                                searching = false;
                                offset.xy = make_float2(chunkWorldP.x + localX, chunkWorldP.y + localY);
                            }
                        }
                    }
                }
            }
        }
    }

    for(int i = 0; i < unitCount; i++) {
        float3 p = offset;

        p.x += (widthScale*(i % halfUnitCount));
        p.y += i / halfUnitCount;

        p.z = getMapHeight(p.x, p.y);
        float3 boardP = convertRealWorldToBlockCoords(p);
        addKnightEntity(gameState, boardP);
    }
}

void initPlayerBoard(GameState *gameState) {
    DEBUG_TIME_BLOCK();
    gameState->cameraPos.x = CHUNK_DIM*0.5f;
    gameState->cameraPos.y = CHUNK_DIM*0.5f;

    float3 p = make_float3(0, 0, 0);

    placeKnightUnit(gameState, p, UNIT_PLACE_SAFE);

    // c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 1, 0, 0);
    // assert(c);
    // if(c) {
    //     float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
    //     addGoblinTntEntity(gameState, boardP);
    // }

    // c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 2, 0, 0);
    // assert(c);
    // if(c) {
    //     float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(2*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
    //     addPeasantEntity(gameState, boardP);
    // }

    // c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 3, 0, 0);
    // assert(c);
    // if(c) {
    //     float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(3*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
    //     addArcherEntity(gameState, boardP);
    // }

    // c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, 4, 0, 0);
    // assert(c);
    // if(c) {
    //     float2 boardP = convertRealWorldToBlockCenterWorld(make_float3(4*CHUNK_DIM + CHUNK_DIM*0.5f, CHUNK_DIM*0.5f, 0)).xy;
    //     addGoblinEntity(gameState, boardP);
    // }
}