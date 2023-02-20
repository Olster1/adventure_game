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

struct EditorGui {
    GuiInteraction currentInteraction; 
};