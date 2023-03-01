void DEBUG_runUnitTests(EditorState *state) {
    EditorGui *gui = &state->editorGuiState;

    //NOTE: Make sure array is power of two
    assert((arrayCount(gui->undoRedoBlocks) & (arrayCount(gui->undoRedoBlocks) - 1)) == 0);


    assert(wrapUndoRedoIndex(gui, 6) == 6);
    assert(wrapUndoRedoIndex(gui, 1024) == 0);
    assert(wrapUndoRedoIndex(gui, 1026) == 2);
    assert(wrapUndoRedoIndex(gui, 1024*2) == 0);
    assert(wrapUndoRedoIndex(gui, 1026*2) == 4);
}