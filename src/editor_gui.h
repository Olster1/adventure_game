enum InteractionType {
    TILE_SELECTION_SWAMP
};

struct EditorGuiId {
    InteractionType type;
    int a;
    int b;
};

struct GuiInteraction {
    EditorGuiId id;

    bool active;

};


struct UndoRedoBlock {
    union {
        struct {
            MapTile lastMapTile;
            MapTile mapTile;
        };
    };
};

struct EditorGui {
    GuiInteraction currentInteraction; 

    //NOTE: Ring buffer
    int undoRedoTotalCount; //NOTE: Fills up then stays full 
    int undoRedoEndOfRingBuffer; //NOTE: Where the next most out of date spot is 
    int undoRedoCursorAt; //NOTE: Where the next most out of date spot is 
    UndoRedoBlock undoRedoBlocks[1028];
};
