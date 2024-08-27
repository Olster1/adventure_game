
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

#pragma pack(push, 1)
struct SaveTileV1 {
	float x;
	float y;
	float z;

	int xId;
	int yId;

	int type;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TileMapLevelHeaderVersion {
	u32 version;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TileMapLevelHeaderV1 {
	u32 tileCount;
	size_t bytesPerTile;
};
#pragma pack(pop)

// static Settings_To_Save load_settings(char *utf8_full_file_name) {


// }	

void saveTileMapLevel_version1(EditorState *editorState, char *utf8_full_file_name) {

	size_t toAllocate = sizeof(TileMapLevelHeaderVersion) + sizeof(TileMapLevelHeaderV1) + editorState->tileCount*sizeof(SaveTileV1);

	u8 *data = (u8 *)pushSize(&globalPerFrameArena, toAllocate);

	TileMapLevelHeaderVersion *v = (TileMapLevelHeaderVersion *)data;
	v->version = 1;
	TileMapLevelHeaderV1 *header = (TileMapLevelHeaderV1 *)(data + sizeof(TileMapLevelHeaderVersion));
	header->tileCount = editorState->tileCount;
	header->bytesPerTile = sizeof(SaveTileV1);

	SaveTileV1 *tiles = (SaveTileV1 *)(data + sizeof(TileMapLevelHeaderV1) + sizeof(TileMapLevelHeaderVersion));

	for(int i = 0; i < editorState->tileCount; ++i) {
		MapTile t = editorState->tiles[i];
		SaveTileV1 tile = {};
		tile.x = t.x;
		tile.y = t.y;
		tile.xId = t.xId;
		tile.yId = t.yId;
		tile.type = t.type;
		tiles[i] = tile;
	}

	printf("SAVE FILE NAME: %s\n", utf8_full_file_name);	
	Platform_File_Handle handle = platform_begin_file_write_utf8_file_path(utf8_full_file_name);

	assert(!handle.has_errors);

	platform_write_file_data(handle, data, toAllocate, 0);

	platform_close_file(handle);
}

void loadTileMapLevel(EditorState *state, char *fileName_utf8) {
	u8 *data = 0;
	size_t data_size = 0;

	u32 currentVersion = 1;

	bool worked = Platform_LoadEntireFile_utf8(fileName_utf8, (void **)&data, &data_size);

	if(worked && data) {
		TileMapLevelHeaderVersion *v = (TileMapLevelHeaderVersion *)data;

		if(v->version == 1) {
			TileMapLevelHeaderV1 *h = (TileMapLevelHeaderV1 *)(data + sizeof(TileMapLevelHeaderVersion));

			SaveTileV1 *tiles = (SaveTileV1 *)(data + sizeof(TileMapLevelHeaderV1) + sizeof(TileMapLevelHeaderVersion));

			assert(h->bytesPerTile == sizeof(SaveTileV1));

			for(int i = 0; i < h->tileCount; ++i) {
				SaveTileV1 t = tiles[i];

				assert(state->tileCount < arrayCount(state->tiles));
                MapTile *tile = state->tiles + state->tileCount++;

				tile->x = t.x;
				tile->y = t.y;
				tile->xId = t.xId;
				tile->yId = t.yId;
				//TODO: More robust way of mapping save type to in-game type
				tile->type = (TileSetType)t.type;
			}
		}
	}

	if(data) {
		platform_free_memory(data);	
	}
}