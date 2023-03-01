enum TileSetType {
	TILE_SET_SWAMP
};

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