
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
struct SaveEntityV1 {
	float x;
	float y;
	float z;

    char id[512];

	int type;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SaveLevelHeaderVersion {
	u32 version;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SaveLevelHeaderV1 {
	u32 tileCount;
	size_t bytesPerTile;

    u32 entityCount;
    size_t bytesPerEntity;
};
#pragma pack(pop)

void saveLevel_version1_json(EditorState *editorState, char *utf8_full_file_name) {

    Platform_File_Handle handle = platform_begin_file_write_utf8_file_path(utf8_full_file_name);
    assert(!handle.has_errors);

    size_t offset = 0;

	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *e = &editorState->entities[i];

        MemoryArenaMark memMark = takeMemoryMark(&globalPerFrameArena);

#define writeVarString(format, ...) {char *s = easy_createString_printf(&globalPerFrameArena, format, __VA_ARGS__); size_t inBytes = easyString_getSizeInBytes_utf8(s); platform_write_file_data(handle, s, inBytes, offset); offset += inBytes; }
        writeVarString("{\n", "");
        writeVarString("id: \"%s\"\n", e->id);
        writeVarString("idHash: \"%s\"\n", e->idHash);
        writeVarString("type: \"%s\"\n", MyEntity_TypeStrings[(int)e->type]);
        writeVarString("pos: %f %f %f\n", e->pos.x, e->pos.y, e->pos.z);
        writeVarString("}\n", "");
#undef writeVarString
        releaseMemoryMark(&memMark);
	}

	platform_close_file(handle);
}

float3 getFloat3FromTokenizer(EasyTokenizer *tokenizer) {
    EasyToken t = lexGetNextToken(tokenizer);
    assert(t.type == TOKEN_FLOAT);
    
    float x = t.floatVal;

    t = lexGetNextToken(tokenizer);
    assert(t.type == TOKEN_FLOAT);
    
    float y = t.floatVal;

    t = lexGetNextToken(tokenizer);
    assert(t.type == TOKEN_FLOAT);
    
    float z = t.floatVal;

    return make_float3(x, y, z);
}

void loadSaveLevel_json(EditorState *state, char *fileName_utf8) {
    u8 *data = 0;
	size_t data_size = 0;

    bool worked = Platform_LoadEntireFile_utf8(fileName_utf8, (void **)&data, &data_size);

	if(worked && data) {
        //NOTE: Parse the json format
        EasyTokenizer tokenizer = lexBeginParsing(data, (EasyLexOptions)(EASY_LEX_EAT_SLASH_COMMENTS | EASY_LEX_OPTION_EAT_WHITE_SPACE));

        Entity entity = {};

        bool parsing = true;
        while(parsing) {
            EasyToken token = lexGetNextToken(&tokenizer);
            switch(token.type) {
                case TOKEN_NULL_TERMINATOR: {
                    parsing = false;
                } break;
                case TOKEN_OPEN_BRACKET: {
                    memset(&entity, 0, sizeof(Entity));
                } break;
                case TOKEN_WORD: {
                    if(easyString_stringsMatch_null_and_count("pos", token.at, token.size)) {
                        EasyToken t = lexGetNextToken(&tokenizer);
                        assert(t.type == TOKEN_COLON);
                        entity.pos = getFloat3FromTokenizer(&tokenizer);
                    }
                    if(easyString_stringsMatch_null_and_count("id", token.at, token.size)) {
                        EasyToken t = lexGetNextToken(&tokenizer);
                        assert(t.type == TOKEN_COLON);
                        // entity.id = getFloat3FromTokenizer(&tokenizer);
                    }
                    if(easyString_stringsMatch_null_and_count("idHash", token.at, token.size)) {
                        EasyToken t = lexGetNextToken(&tokenizer);
                        assert(t.type == TOKEN_COLON);
                        // entity.idHash = getFloat3FromTokenizer(&tokenizer);
                    }
                    if(easyString_stringsMatch_null_and_count("type", token.at, token.size)) {
                        EasyToken t = lexGetNextToken(&tokenizer);
                        assert(t.type == TOKEN_COLON);
                        // entity.type = getFloat3FromTokenizer(&tokenizer);
                    }
                } break;
                case TOKEN_CLOSE_BRACKET: {


                } break;
                case TOKEN_NEWLINE: {

                } break;
                default: {

                } break;
            }
        }
    }
}

void saveLevel_version1_binary(EditorState *editorState, char *utf8_full_file_name) {

	size_t toAllocate = sizeof(SaveLevelHeaderVersion) + sizeof(SaveLevelHeaderV1) + editorState->tileCount*sizeof(SaveTileV1);

	u8 *data = (u8 *)pushSize(&globalPerFrameArena, toAllocate);

	SaveLevelHeaderVersion *v = (SaveLevelHeaderVersion *)data;
	v->version = 1;
	SaveLevelHeaderV1 *header = (SaveLevelHeaderV1 *)(data + sizeof(SaveLevelHeaderVersion));
	header->tileCount = editorState->tileCount;
	header->bytesPerTile = sizeof(SaveTileV1);

	SaveTileV1 *tiles = (SaveTileV1 *)(data + sizeof(SaveLevelHeaderV1) + sizeof(SaveLevelHeaderVersion));

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

	Platform_File_Handle handle = platform_begin_file_write_utf8_file_path(utf8_full_file_name);

	assert(!handle.has_errors);

	platform_write_file_data(handle, data, toAllocate, 0);

	platform_close_file(handle);
}

void loadSaveLevel_binary(EditorState *state, char *fileName_utf8) {
	u8 *data = 0;
	size_t data_size = 0;

	u32 currentVersion = 1;

	bool worked = Platform_LoadEntireFile_utf8(fileName_utf8, (void **)&data, &data_size);

	if(worked && data) {
		SaveLevelHeaderVersion *v = (SaveLevelHeaderVersion *)data;

		if(v->version == 1) {
			SaveLevelHeaderV1 *h = (SaveLevelHeaderV1 *)(data + sizeof(SaveLevelHeaderVersion));

			SaveTileV1 *tiles = (SaveTileV1 *)(data + sizeof(SaveLevelHeaderV1) + sizeof(SaveLevelHeaderVersion));

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