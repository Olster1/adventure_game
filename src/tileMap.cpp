
MapTile getDefaultMapTile(EditorState *state, TileSetType type, int x, int y, int xId, int yId, bool collidable) {
	MapTile tile = {};
	tile.x = x;
	tile.y = y;

	tile.xId = xId;
	tile.yId = yId;
	
	tile.type = type;
	tile.collidable = collidable;

	return tile;
}

MapTileFindResult findMapTile(EditorState *editorState, MapTile tile) {
    MapTileFindResult result = {};

    //NOTE: Draw the tile map
	for(int i = 0; i < editorState->tileCount; ++i) {
		MapTile t = editorState->tiles[i];

        if(t.x == tile.x && t.y == tile.y) {
            result.found = true;
            result.indexAt = i;
			result.tile = t;
            break;
        }
    }

    return result;                    
}

bool isSameMapTile(MapTile tileA, MapTile tileB) {
	bool result = false;
	if(tileA.xId == tileB.xId && tileA.yId == tileB.yId && tileA.type == tileB.type) {
		result = true;
	}
	return result;
}

void removeMapTile(EditorState *editorState, int indexAt) {
    editorState->tiles[indexAt] = editorState->tiles[--editorState->tileCount]; 
}

TileSet buildTileSet(Texture **tiles, int count, TileSetType type, int countX, int countY, int tileSizeX, int tileSizeY) {
	TileSet result = {};

	result.type = type;
	result.tiles = tiles;
	result.count = count;

	result.countX = countX;
	result.countY = countY;

	result.tileSizeX = tileSizeX;
	result.tileSizeY = tileSizeY;

	return result;
}