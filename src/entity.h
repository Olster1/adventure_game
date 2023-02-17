enum EntityFlags {
    ENTITY_ACTIVE = 1 << 0,
    FIRE_BALL_COMPONENT = 1 << 1,
};

enum EntityType {
    ENTITY_PLAYER,
    ENTITY_PIPE,
    ENTITY_FIREBALL,
    ENTITY_COIN
};

struct Entity {
    char *id;
    int idHash;
    EntityType type;
    
    float3 pos;
    float3 scale;
    float3 velocity;
    float rotation;
    float targetRotation;
    float collisionScale;
    u64 flags;

    ///////////////////////
    //NOTE: For fireball
    float respawnTimer;

    EasyAnimation_Controller animationController;
};

void updateFireballComponent() {

} 