
Chunk *Terrain::generateChunk(int x, int y, uint32_t hash) {
    Chunk *chunk = (Chunk *)pushStruct(&global_long_term_arena, Chunk);
    *chunk = Chunk();
    assert(chunk);
    memset(chunk, 0, sizeof(Chunk));

    chunk->x = x;
    chunk->y = y;
    chunk->generateState = CHUNK_NOT_GENERATED;

    chunk->next = chunks[hash];
    chunks[hash] = chunk;

    return chunk;
}

TileType getLandscapeValue(int worldX, int worldY) {
    float perlinValue = mapSimplexNoiseTo01(SimplexNoise_fractal_2d(8, worldX, worldY, 0.3f));

    TileType type = TILE_TYPE_NONE;
    if(perlinValue < 0.33f) {
        type = TILE_TYPE_BEACH;
    } else {
        type = TILE_TYPE_WATER;
    }

    return type;
}

void Terrain::fillChunk(AnimationState *animationState, Chunk *chunk) {
    assert(chunk);
    assert(chunk->generateState & CHUNK_NOT_GENERATED);
    chunk->generateState = CHUNK_GENERATING;

    float2 chunkP = getChunkWorldP(chunk);
    int chunkWorldX = roundChunkCoord(chunkP.x);
    int chunkWorldY = roundChunkCoord(chunkP.y);

    for(int y = 0; y < CHUNK_DIM; ++y) {
        for(int x = 0; x < CHUNK_DIM; ++x) {
            int worldX = chunkWorldX + x;
            int worldY = chunkWorldY + y;

            TileType type = getLandscapeValue(worldX, worldY);

            Animation *animation = 0;
            TileMapCoords tileCoords = {};

            if(type == TILE_TYPE_BEACH) {
                animation = &animationState->waterAnimation;

                TileMapCoords tileLookup[16] = {
                    {3, 3}, // 0000 - No beach tiles
                    {3, 2}, // 0001 - Top
                    {3, 0}, // 0010 - Bottom
                    {3, 1}, // 0011 - Top & Bottom
                    {0, 3}, // 0100 - Right
                    {0, 2}, // 0101 - Top & Right
                    {0, 0}, // 0110 - Bottom & Right
                    {0, 1}, // 0111 - Top, Bottom, Right
                    {2, 3}, // 1000 - Left
                    {2, 2}, // 1001 - Top & Left
                    {2, 0}, // 1010 - Bottom & Left
                    {2, 1}, // 1011 - Top, Bottom, Left
                    {1, 3}, // 1100 - Left & Right
                    {1, 2}, // 1101 - Top, Right, Left
                    {1, 0}, // 1110 - Bottom, Right, Left
                    {1, 1}  // 1111 - Surrounded by beach tiles
                };
                
                u8 bits = 0;
                if (getLandscapeValue(worldX, worldY + 1) == TILE_TYPE_BEACH) bits |= 1 << 0;
                if (getLandscapeValue(worldX, worldY - 1) == TILE_TYPE_BEACH) bits |= 1 << 1;
                if (getLandscapeValue(worldX + 1, worldY) == TILE_TYPE_BEACH) bits |= 1 << 2;
                if (getLandscapeValue(worldX - 1, worldY) == TILE_TYPE_BEACH) bits |= 1 << 3;
                
                // Fast lookup
                tileCoords = tileLookup[bits];
                tileCoords.x += 5;


            } 

            if(type != TILE_TYPE_NONE) {
                chunk->tiles[y*CHUNK_DIM + x] = Tile(type, &animationState->animationItemFreeListPtr, animation, tileCoords);
            }

        }
    }

    chunk->generateState = CHUNK_GENERATED;

}

Chunk *Terrain::getChunk(AnimationState *animationState, int x, int y, bool shouldGenerateChunk, bool shouldGenerateFully) {
    uint32_t hash = getHashForChunk(x, y);

    Chunk *chunk = chunks[hash];

    bool found = false;

    while(chunk && !found) {
        if(chunk->x == x && chunk->y == y) {
            found = true;
            break;
        }
        chunk = chunk->next;
    }

    if((!chunk && shouldGenerateChunk)) {
        chunk = generateChunk(x, y, hash);
    }

    if(chunk && shouldGenerateFully && (chunk->generateState & CHUNK_NOT_GENERATED)) {
        //NOTE: Launches multi-thread work
        fillChunk(animationState, chunk);
    } 

    return chunk;
}