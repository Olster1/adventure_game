#define MY_TILE_MAP_TYPE(FUNC) \
FUNC(TILE_SET_SWAMP)\

typedef enum {
    MY_TILE_MAP_TYPE(ENUM)
} TileSetType;

static char *MyTileMap_TypeStrings[] = { MY_TILE_MAP_TYPE(STRING) };

struct MapTile {
	int x;
	int y;

	int xId;
	int yId;

	TileSetType type;
};

struct MapTileFindResult {
    bool found;
    int indexAt;
};  


struct TileSet {
	TileSetType type;
	Texture **tiles;
	int count;

	int countX;
	int countY;

	int tileSizeX;
	int tileSizeY;
};