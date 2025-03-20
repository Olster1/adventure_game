//NOTE: Helper funciton since the simplex lib I'm using for 3d noise maps between -1 -> 1, and we want 0 -> 1
float mapSimplexNoiseTo01(float value) {
    value += 1;
    value *= 0.5f;

    assert(value >= 0 && value <= 1);

    return value;
}

enum TileType {
    tileTypeNone,
    tileTypeGrass,
    tileTypeStone,
    tileTypeDirt,
};

struct Tile {
    TileType type = tileTypeNone;

    Tile() {
    }

    Tile(TileType type) {
        this->type = type;
    }
};


enum ChunkGenerationState {
    CHUNK_NOT_GENERATED = 1 << 0, 
    CHUNK_GENERATING = 1 << 1, 
    CHUNK_GENERATED = 1 << 2, 
    CHUNK_MESH_DIRTY = 1 << 3, 
    CHUNK_MESH_BUILDING = 1 << 4, 
};

#define CHUNK_DIM 16

struct Chunk {
    int x = 0;
    int y = 0;

    volatile int64_t generateState = 0; //NOTE: Chunk might not be generated, so check first when you get one

    //NOTE: 16 x 16
    //NOTE: Z Y
    Tile tiles[CHUNK_DIM*CHUNK_DIM];

    // Entity *entities;
    Chunk *next = 0;

    Tile *getTile(int x, int y) {
        Tile *t = 0;
        if(x >= 0 && y >= 0 && x < CHUNK_DIM && y < CHUNK_DIM) {
            t = &tiles[y*CHUNK_DIM + x];
        }
        return t;
    }

    Chunk() {
        generateState = CHUNK_NOT_GENERATED;
        x = 0;
        y = 0;
        next = 0;
    }

};

#define CHUNK_LIST_SIZE 4096

float2 getChunkWorldP(Chunk *c) {
    float2 worldP = make_float2(c->x*CHUNK_DIM, c->y*CHUNK_DIM);
    return worldP;
}

float2 getTileWorldP(Chunk *c, int x, int y) {
    float2 worldP = make_float2(c->x*CHUNK_DIM + x, c->y*CHUNK_DIM + y);
    return worldP;
}


float2 convertRealWorldToBlockCoords(float2 p) {
    //NOTE: The origin is at the center of a block
    //NOTE: So 0.5 & 1.4 should both map to 1 - I think the +0.5
    //NOTE: So -0.5 & -1.4 should both map to -1 - I think the -0.5
    //NOTE: So -0.4 & 0.4 should both map to 0 - I think the -0.5
    p.x = round(p.x);
    p.y = round(p.y);

    return p;
}

int roundChunkCoord(float value) {
    return floor(value);
}

float2 getChunkPosForWorldP(float2 p) {
    p.x = round(p.x);
    p.y = round(p.y);

    p.x = roundChunkCoord((float)p.x / (float)CHUNK_DIM);
    p.y = roundChunkCoord((float)p.y / (float)CHUNK_DIM);

    return p;
}


uint32_t getHashForChunk(int x, int y) {
    int values[2] = {x, y};
    uint32_t hash = get_crc32((char *)values, arrayCount(values)*sizeof(int));
    hash = hash & (CHUNK_LIST_SIZE - 1);
    assert(hash < CHUNK_LIST_SIZE);
    assert(hash >= 0);
    return hash;
}

struct Terrain {
    Chunk *chunks[CHUNK_LIST_SIZE];

    Terrain() {
        memset(chunks, 0, arrayCount(chunks)*sizeof(Chunk *));
    }

    Chunk *generateChunk(int x, int y, uint32_t hash) {
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

    void fillChunk(Chunk *chunk) {
        assert(chunk);
        assert(chunk->generateState & CHUNK_NOT_GENERATED);
        chunk->generateState = CHUNK_GENERATING;

        float2 chunkP = getChunkWorldP(chunk);
        int chunkWorldX = roundChunkCoord(chunkP.x);
        int chunkWorldY = roundChunkCoord(chunkP.y);

        for(int y = 0; y < CHUNK_DIM; ++y) {
            for(int x = 0; x < CHUNK_DIM; ++x) {
                TileType type = tileTypeNone;

                int worldX = chunkWorldX + x;
                int worldY = chunkWorldY + y;

                float perlinValue = mapSimplexNoiseTo01(SimplexNoise_fractal_2d(8, worldX, worldY, 0.3f));

                if(perlinValue < 0.33f) {
                    type = tileTypeGrass;
                } else if(perlinValue < 0.66f) {
                    type = tileTypeDirt;
                } else if(perlinValue < 1.0f) {
                    type = tileTypeStone;
                }

                chunk->tiles[y*CHUNK_DIM + x] = Tile(type);

            }
        }

        chunk->generateState = CHUNK_GENERATED;
    
    }
    
    Chunk *getChunk(int x, int y, bool shouldGenerateChunk = true, bool shouldGenerateFully = true) {
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
            fillChunk(chunk);
        } 
    
        return chunk;
    }
};

