//NOTE: Helper funciton since the simplex lib I'm using for 3d noise maps between -1 -> 1, and we want 0 -> 1
float mapSimplexNoiseTo01(float value) {
    value += 1;
    value *= 0.5f;

    assert(value >= 0 && value <= 1);

    return value;
}
float mapSimplexNoiseTo11(float value) {
    return value;
}

enum TileType {
    TILE_TYPE_NONE,
    TILE_TYPE_WATER,
    TILE_TYPE_BEACH,
    TILE_TYPE_ROCK,
    TILE_TYPE_SOLID,
    TILE_TYPE_WATER_ROCK,

};

struct TileMapCoords {
    int x;
    int y;
};

enum TileFlags {
    TILE_FLAG_FRONT_FACE = 1 << 0, //NOTE: This is whether we should render the front face for 3d
    TILE_FLAG_GRASSY_TOP = 1 << 1, //NOTE: Wether a block should have an additional grassy top - controlled by perlin noise if it is a rock type
    TILE_FLAG_SHADOW = 1 << 2, //NOTE: Wether a block should have an additional grassy top - controlled by perlin noise if it is a rock type
    TILE_FLAG_FRONT_GRASS = 1 << 3, 
    TILE_FLAG_FRONT_BEACH = 1 << 4, 
    TILE_FLAG_TREE = 1 << 5,  //NOTE: Wether this tile has a tree
};

struct Tile {
    TileType type = TILE_TYPE_NONE;
    EasyAnimation_Controller *animationController = 0;
    TileMapCoords coords;
    TileMapCoords coordsSecondary;
    u32 flags = 0;
    u8 lightingMask = 0; //NOTE: Minecraft like lighting data bottom 4 bits are top surface, next 4 bits are the front facing surface 

    Tile() {

    }

    Tile(TileType type, EasyAnimation_ListItem **freeList, Animation *animation, TileMapCoords coords, TileMapCoords coordsSecondary, u32 flags, u8 lightingMask) {
        this->type = type;
        this->coords = coords;
        this->coordsSecondary = coordsSecondary;
        this->flags = flags;
        this->lightingMask = lightingMask;
        if(type == TILE_TYPE_BEACH || type == TILE_TYPE_WATER_ROCK) {
            assert(animation);
            animationController = pushStruct(&global_long_term_arena, EasyAnimation_Controller);
            easyAnimation_initController(animationController);
            EasyAnimation_ListItem *anim = easyAnimation_addAnimationToController(animationController, freeList, animation, 0.08f);
        }
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
    int z = 0;

    volatile int64_t generateState = 0; //NOTE: Chunk might not be generated, so check first when you get one

    //NOTE: 16 x 16 x 16
    //NOTE: Z Y
    Tile tiles[CHUNK_DIM*CHUNK_DIM*CHUNK_DIM];

    // Entity *entities;
    Chunk *next = 0;

    Tile *getTile(int x, int y, int z) {
        Tile *t = 0;
        if(x >= 0 && y >= 0 && z >= 0 && x < CHUNK_DIM && y < CHUNK_DIM && z < CHUNK_DIM) {
            //TODO
            t = &tiles[z*CHUNK_DIM*CHUNK_DIM + y*CHUNK_DIM + x];
        }
        return t;
    }

    Chunk() {
        generateState = CHUNK_NOT_GENERATED;
        x = 0;
        y = 0;
        z = 0;
        next = 0;
    }

};

#define CHUNK_LIST_SIZE 4096

float3 getChunkWorldP(Chunk *c) {
    float3 worldP = make_float3(c->x*CHUNK_DIM, c->y*CHUNK_DIM, c->z*CHUNK_DIM);
    return worldP;
}

float3 getTileWorldP(Chunk *c, int x, int y, int z) {
    int zComp = c->z*CHUNK_DIM + z;
    float3 worldP = make_float3(c->x*CHUNK_DIM + x, c->y*CHUNK_DIM + y + zComp, 0);
    return worldP;
}


float3 convertRealWorldToBlockCoords(float3 p) {
    //NOTE: The origin is at the center of a block
    //NOTE: So 0.5 & 1.4 should both map to 1 - I think the +0.5
    //NOTE: So -0.5 & -1.4 should both map to -1 - I think the -0.5
    //NOTE: So -0.4 & 0.4 should both map to 0 - I think the -0.5
    p.x = round(p.x);
    p.y = round(p.y);
    p.z = round(p.z);

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


uint32_t getHashForChunk(int x, int y, int z) {
    DEBUG_TIME_BLOCK()
    int values[3] = {x, y, z};
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

    Chunk *generateChunk(int x, int y, int z, uint32_t hash);
    void fillChunk(AnimationState *animationState, Chunk *chunk);
    Chunk *getChunk(AnimationState *animationState, int x, int y, int z, bool shouldGenerateChunk = true, bool shouldGenerateFully = true);
       
};

