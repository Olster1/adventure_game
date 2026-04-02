void debug_addStringToCommandBuffer(DebugGameCommandBuffer *buffer, char *str) {
    int indexAt = buffer->lineAt++;

    if(indexAt >= arrayCount(buffer->lines)) {
        //NOTE: Wrap index
        indexAt = 0;
        buffer->lineAt = 1;
    }

    if(buffer->lineCount < arrayCount(buffer->lines)) {
        buffer->lineCount++;
    }

    assert(indexAt >= 0 && indexAt < arrayCount(buffer->lines));
    buffer->lines[indexAt] = str;
}

void debug_drawUpdateInputBuffer(Renderer *renderer, GameState *gameState, float fauxDimensionX, float fauxDimensionY) {
    DebugGameCommandBuffer *buffer =  &gameState->debug_commandBuffer;

    float openBufferHeight = (buffer->debug_InputBufferOpenState == DEBUG_COMMAND_BUFFER_OPEN_FULL) ? 0.9f*fauxDimensionY : 0.2f*fauxDimensionY;

    //NOTE: ADD THE string
    if(global_platformInput.textInput_utf8[0]) {
        stringBuffer_insertString(&buffer->debug_stringBuffer, (char *)global_platformInput.textInput_utf8);
    }

    if(global_platformInput.keyStates[PLATFORM_KEY_CURSOR_RIGHT].isDown) {
        stringBuffer_cursorRight(&buffer->debug_stringBuffer, 1);
    }
    if(global_platformInput.keyStates[PLATFORM_KEY_CURSOR_LEFT].isDown) {
        stringBuffer_cursorLeft(&buffer->debug_stringBuffer, 1);
    }

    if(global_platformInput.keyStates[PLATFORM_KEY_BACKSPACE].isDown) {
        stringBuffer_removeCharactersAtCursor(&buffer->debug_stringBuffer, 1);
    }

    if(global_platformInput.keyStates[PLATFORM_KEY_ENTER].pressedCount > 0) {
        //Parse command
        EasyTokenizer tokenizer = lexBeginParsing(buffer->debug_stringBuffer.string, EASY_LEX_OPTION_EAT_WHITE_SPACE);

        bool parsing = true;
        while(parsing) {
            EasyToken t = lexGetNextToken(&tokenizer);

            if(t.type == TOKEN_NULL_TERMINATOR) {
                parsing = false;
            } else if(t.type == TOKEN_WORD) {
                if(easyString_stringsMatch_null_and_count("clear", t.at, t.size)) {

                } else if(easyString_stringsMatch_null_and_count("load", t.at, t.size)) {
                    t = lexSeeNextToken(&tokenizer);
                    if(t.type == TOKEN_STRING) {
                        t = lexGetNextToken(&tokenizer);
                        char *fileName = nullTerminateArena(t.at, t.size, &globalPerFrameArena);
                    } else {
                        //NOTE: Print "Expected a String"
                        debug_addStringToCommandBuffer(buffer, "Expected a String");
                    }

                } else {
                    debug_addStringToCommandBuffer(buffer, "Command not found");
                }
            } else {
                // debug_addStringToCommandBuffer(buffer, "Command not found");
            }
        }
        clearStringBuffer(&buffer->debug_stringBuffer);
    }

    float16 orthoMatrix1 = make_ortho_matrix_bottom_left_corner(fauxDimensionX, fauxDimensionY, MATH_3D_NEAR_CLIP_PlANE, MATH_3D_FAR_CLIP_PlANE);
	pushMatrix(renderer, orthoMatrix1);


    pushShader(renderer, &pixelArtShader);
    pushRect(renderer, make_float3(0.5f*fauxDimensionX, fauxDimensionY - 0.5f*openBufferHeight, 0), make_float2(fauxDimensionX, openBufferHeight), MY_COLOR_SLATE);

	pushShader(renderer, &sdfFontShader);
	{
	    //NOTE: Render the input
	    float scale = 1.5f;
		draw_text(renderer, &gameState->font, buffer->debug_stringBuffer.string, 0, fauxDimensionY - openBufferHeight + scale*gameState->font.fontHeight, scale, MY_COLOR_WHITE);
	}

	{
	   float scale = 1.5f;
	   float atY = fauxDimensionY - openBufferHeight + (scale*gameState->font.fontHeight*(gameState->debug_commandBuffer.lineCount + 1));

       //TODO: Handle Ring buffer loop around
	   for(int i = gameState->debug_commandBuffer.lineAt; i < gameState->debug_commandBuffer.lineCount; i++) {
    		draw_text(renderer, &gameState->font, gameState->debug_commandBuffer.lines[i], 0, atY, scale, MY_COLOR_WHITE);
    		atY -= scale*gameState->font.fontHeight;
	   }
	   for(int i = 0; i < gameState->debug_commandBuffer.lineAt; i++) {
    		draw_text(renderer, &gameState->font, gameState->debug_commandBuffer.lines[i], 0, atY, scale, MY_COLOR_WHITE);
    		atY -= scale*gameState->font.fontHeight;
	   }
	}
}