
MapTileFindResult findMapTile(EditorState *editorState, MapTile tile) {
    MapTileFindResult result = {};

    //NOTE: Draw the tile map
	for(int i = 0; i < editorState->tileCount; ++i) {
		MapTile t = editorState->tiles[i];

        if(t.x == tile.x && t.y == tile.y) {
            result.found = true;
            result.indexAt = i;
            break;
        }
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