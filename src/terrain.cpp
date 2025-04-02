
Chunk *Terrain::generateChunk(int x, int y, int z, uint32_t hash) {
    Chunk *chunk = (Chunk *)pushStruct(&global_long_term_arena, Chunk);
    *chunk = Chunk();
    assert(chunk);
    memset(chunk, 0, sizeof(Chunk));

    chunk->x = x;
    chunk->y = y;
    chunk->z = z;
    chunk->generateState = CHUNK_NOT_GENERATED;

    chunk->next = chunks[hash];
    chunks[hash] = chunk;

    return chunk;
}

TileType getLandscapeValue(int worldX, int worldY, int worldZ) {
    float maxHeight = 3.0f;
    int height = round(maxHeight*mapSimplexNoiseTo11(SimplexNoise_fractal_2d(8, worldX, worldY, 0.05f)));

    TileType type = TILE_TYPE_NONE;
    if(height >= 0 && worldZ <= height) {
        type = TILE_TYPE_SOLID;
    }

    return type;
}

bool hasGrassyTop(int worldX, int worldY, int worldZ) {
    float perlin = mapSimplexNoiseTo01(SimplexNoise_fractal_3d(8, worldX, worldY, worldZ, 0.001f));
    bool result = perlin > 0.5f;
    return result;
}

static TileMapCoords global_tileLookup[16] = {
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

static TileMapCoords global_tileLookup_elevated[16] = {
    {3, 4}, // 0000 - No beach tiles
    {3, 2}, // 0001 - Top
    {3, 0}, // 0010 - Bottom
    {3, 1}, // 0011 - Top & Bottom
    {0, 4}, // 0100 - Right
    {0, 2}, // 0101 - Top & Right
    {0, 0}, // 0110 - Bottom & Right
    {0, 1}, // 0111 - Top, Bottom, Right
    {2, 4}, // 1000 - Left
    {2, 2}, // 1001 - Top & Left
    {2, 0}, // 1010 - Bottom & Left
    {2, 1}, // 1011 - Top, Bottom, Left
    {1, 4}, // 1100 - Left & Right
    {1, 2}, // 1101 - Top, Right, Left
    {1, 0}, // 1110 - Bottom, Right, Left
    {1, 1}  // 1111 - Surrounded by beach tiles
};

void Terrain::fillChunk(AnimationState *animationState, Chunk *chunk) {
    assert(chunk);
    assert(chunk->generateState & CHUNK_NOT_GENERATED);
    chunk->generateState = CHUNK_GENERATING;

    float3 chunkP = getChunkWorldP(chunk);
    int chunkWorldX = roundChunkCoord(chunkP.x);
    int chunkWorldY = roundChunkCoord(chunkP.y);
    int chunkWorldZ = roundChunkCoord(chunkP.z);

    for(int z = 0; z < CHUNK_DIM; ++z) {
        for(int y = 0; y < CHUNK_DIM; ++y) {
            for(int x = 0; x < CHUNK_DIM; ++x) {
                int worldX = chunkWorldX + x;
                int worldY = chunkWorldY + y;
                int worldZ = chunkWorldZ + z;

                if(worldX == -13 && worldY == 16 && worldZ == 0) {
                    int h = 0;
                }
                

                TileType type = getLandscapeValue(worldX, worldY, worldZ);

                if(type == TILE_TYPE_SOLID) {
                    if(worldZ > 0) {
                        int h = 0;
                    }

                    Animation *animation = 0;
                    TileMapCoords tileCoords = {};
                    TileMapCoords tileCoordsSecondary = {};
                    u32 flags = 0;

                    u8 bits = 0;
                    if (getLandscapeValue(worldX, worldY + 1, worldZ) == TILE_TYPE_SOLID) bits |= 1 << 0;
                    if (getLandscapeValue(worldX, worldY - 1, worldZ) == TILE_TYPE_SOLID) bits |= 1 << 1;
                    if (getLandscapeValue(worldX + 1, worldY, worldZ) == TILE_TYPE_SOLID) bits |= 1 << 2;
                    if (getLandscapeValue(worldX - 1, worldY, worldZ) == TILE_TYPE_SOLID) bits |= 1 << 3;

                    if(worldZ == 0) {
                        animation = &animationState->waterAnimation;
                        tileCoords = global_tileLookup[bits];
                        tileCoords.x += 5;
                        type = TILE_TYPE_BEACH;

                    } else if(worldZ > 0) {
                        // type = TILE_TYPE_NONE;
                        tileCoords = global_tileLookup_elevated[bits];
                        tileCoordsSecondary = global_tileLookup[bits];
                        flags |= TILE_FLAG_SHADOW;
                        type = TILE_TYPE_ROCK;
                        if(bits == 0b0101 || bits == 0b1101 || bits == 0b1001 || bits == 0b0001 || 
                            bits == 0b0000 || bits == 0b0100 || bits == 0b1000 || bits == 0b1100) {
                            flags |= TILE_FLAG_FRONT_FACE;
                            
                        }

                        if(hasGrassyTop(worldX, worldY, worldZ)) {
                            flags |= TILE_FLAG_GRASSY_TOP;
                            

                            //NOTE: Get new bits based on whether grass is next to it
                            u8 bits2 = 0;
                            if (hasGrassyTop(worldX, worldY + 1, worldZ) && (bits & (1 << 0))) bits2 |= 1 << 0;
                            if (hasGrassyTop(worldX, worldY - 1, worldZ) && (bits & (1 << 1))) bits2 |= 1 << 1;
                            if (hasGrassyTop(worldX + 1, worldY, worldZ) && (bits & (1 << 2))) bits2 |= 1 << 2;
                            if (hasGrassyTop(worldX - 1, worldY, worldZ) && (bits & (1 << 3))) bits2 |= 1 << 3;
        
                            tileCoordsSecondary = global_tileLookup[bits2];
                        }
                    }

                    
                    if(type == TILE_TYPE_ROCK && getLandscapeValue(worldX, worldY, worldZ + 1) == TILE_TYPE_SOLID && getLandscapeValue(worldX, worldY - 1, worldZ) == TILE_TYPE_SOLID) {
                        //NOTE: If hidden don't bother making this a tile
                        // type = TILE_TYPE_NONE;
                    }

                    if(type != TILE_TYPE_NONE) {
                        // assert(z == 0);
                        chunk->tiles[z*CHUNK_DIM*CHUNK_DIM + y*CHUNK_DIM + x] = Tile(type, &animationState->animationItemFreeListPtr, animation, tileCoords, tileCoordsSecondary, flags);
                    }
                }
            }
        }
    }

    chunk->generateState = CHUNK_GENERATED;

}

Chunk *Terrain::getChunk(AnimationState *animationState, int x, int y, int z, bool shouldGenerateChunk, bool shouldGenerateFully) {
    uint32_t hash = getHashForChunk(x, y, z);

    Chunk *chunk = chunks[hash];

    bool found = false;

    while(chunk && !found) {
        if(chunk->x == x && chunk->y == y && chunk->z == z) {
            found = true;
            break;
        }
        chunk = chunk->next;
    }

    if((!chunk && shouldGenerateChunk)) {
        chunk = generateChunk(x, y, z, hash);
    }

    if(chunk && shouldGenerateFully && (chunk->generateState & CHUNK_NOT_GENERATED)) {
        //NOTE: Launches multi-thread work
        fillChunk(animationState, chunk);
    } 

    return chunk;
}