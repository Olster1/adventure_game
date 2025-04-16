enum PlacementType {
    UNIT_PLACE_SAFE,
    UNIT_PLACE_EXACT,
};

float3 getSafeSpawnLocation(GameState *gameState, float3 offset, int pWidth, int pHeight) {
    int maxChunkSearch = 10;
    bool searching = true;
    int maxSafeFound = 0;
    float safeAreaSize = pHeight*pWidth;
    float2 chunkOffset = getChunkPosForWorldP(offset.xy);
    for(int chunkY = 0; chunkY < maxChunkSearch && searching; chunkY++) {
        for(int chunkX = 0; chunkX < maxChunkSearch && searching; chunkX++) {
            
            float2 chunkP = make_float2(chunkX + chunkOffset.x, chunkY + chunkOffset.y);
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, chunkP.x, chunkP.y, 0, true, true, &globalPerFrameArena);
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

                                float2 localP = make_float2(localX + x, localY + y);
                                assert(c->visited);
                                assert(localP.x < CHUNK_DIM && localP.y < CHUNK_DIM);
                                if(mapHeight == 1 && !c->visited[(int)localP.y*CHUNK_DIM + (int)localP.x]) {
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

                        if(safeFound > maxSafeFound) {
                            //NOTE: Keep track of the best one we've found so far
                            maxSafeFound = safeFound;
                            offset.xy = make_float2(chunkWorldP.x + localX, chunkWorldP.y + localY);
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

    return offset;
}

void markTileAsUsed(GameState *gameState, float3 boardP, bool markUnWalkable = false) {
     //NOTE: Now say we've used this cell up
     float2 chunkP = getChunkPosForWorldP(boardP.xy);
     Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, chunkP.x, chunkP.y, 0, true, true);

     float3 localP = getChunkLocalPos(boardP.x, boardP.y, boardP.z);

     assert(c->visited);
     assert(localP.x < CHUNK_DIM && localP.y < CHUNK_DIM);

     assert(!c->visited[(int)localP.y*CHUNK_DIM + (int)localP.x]);
     c->visited[(int)localP.y*CHUNK_DIM + (int)localP.x] = true;

     if(markUnWalkable) {
        Tile *tile = c->getTile(localP.x, localP.y, localP.z);
        if(tile) {
            assert((tile->flags & TILE_FLAG_WALKABLE));
            //NOTE: Mark unwalkable
            tile->flags &= (~TILE_FLAG_WALKABLE);
        }

     }
}

void placeKnightUnit(GameState *gameState, float3 offset, PlacementType type) {
    DEBUG_TIME_BLOCK();
    int unitCount = 6;
    int rowCount = 2;
    int halfUnitCount = unitCount / rowCount;
    int widthScale = 1;

    if(type == UNIT_PLACE_SAFE) {
        int pWidth = halfUnitCount*widthScale;
        int pHeight = rowCount;
        //NOTE: Find a safe location to spawn the knight unit
        offset = getSafeSpawnLocation(gameState, offset, pWidth, pHeight);
    }

    for(int i = 0; i < unitCount; i++) {
        float3 p = offset;

        p.x += (widthScale*(i % halfUnitCount));
        p.y += i / halfUnitCount;
        p.z = getMapHeight(p.x, p.y);
        float3 boardP = convertRealWorldToBlockCoords(p);
        addKnightEntity(gameState, boardP);

       markTileAsUsed(gameState, boardP);
    }
}

void placeHouse(GameState *gameState, float3 offset) {
    DEBUG_TIME_BLOCK();
    int pWidth = 2;
    int pHeight = 2;
        //NOTE: Find a safe location to spawn the hosue unit
    offset = getSafeSpawnLocation(gameState, offset, pWidth, pHeight);

    for(int y = 0; y < pHeight; y++) {
        for(int x = 0; x < pWidth; x++) {
            float3 p = offset;
            p.x += x;
            p.y += y;
            p.z = getMapHeight(p.x, p.y);
            float3 boardP = convertRealWorldToBlockCoords(p);
            markTileAsUsed(gameState, boardP, true); //NOTE: Mark chunk as not WALKABLE aswell
        }
    }

    float3 p = offset;
    p.z = getMapHeight(p.x, p.y);
    p = convertRealWorldToBlockCoords(p);
    p.x += 0.5f; //NOTE: We move it to the center of the two tiles
    p.y += 0.5f; //NOTE: We move it to the center of the two tiles
    addHouseEntity(gameState, p);
}

void placeCastle(GameState *gameState, float3 offset) {
    DEBUG_TIME_BLOCK();
    int pWidth = 6;
    int pHeight = 3 + 2; //NOTE: + 2 for safe place out the front
        //NOTE: Find a safe location to spawn the hosue unit
    offset = getSafeSpawnLocation(gameState, offset, pWidth, pHeight);

    for(int y = 0; y < pHeight; y++) {
        for(int x = 0; x < pWidth; x++) {
            float3 p = offset;
            p.x += x;
            p.y += y;
            p.z = getMapHeight(p.x, p.y);
            float3 boardP = convertRealWorldToBlockCoords(p);
            markTileAsUsed(gameState, boardP, y > 1); //NOTE: Mark chunk as not WALKABLE aswell only if it's the ones not one out front   
        }
    }

    float3 p = offset;
    p.z = getMapHeight(p.x, p.y);
    p = convertRealWorldToBlockCoords(p);
    p.x += 2.5f; //NOTE: We move it to the center of the two tiles
    p.y += 3.5f; //NOTE: We move it to the center of the two tiles
    addCastleEntity(gameState, p);
}

void initPlayerBoard(GameState *gameState) {
    DEBUG_TIME_BLOCK();
    gameState->cameraPos.x = CHUNK_DIM*0.5f;
    gameState->cameraPos.y = CHUNK_DIM*0.5f;
    
    MemoryArenaMark mark = takeMemoryMark(&globalPerFrameArena);

    float3 p = make_float3(-CHUNK_DIM, -CHUNK_DIM, 0);

    placeKnightUnit(gameState, p, UNIT_PLACE_SAFE);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeHouse(gameState, p);
    placeCastle(gameState, p);

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

    releaseMemoryMark(&mark);
}