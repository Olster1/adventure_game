enum EntityFlags {
    ENTITY_ACTIVE = 1 << 0,
    FIRE_BALL_COMPONENT = 1 << 1,
    LIGHT_COMPONENT = 1 << 2,
};

enum EntityType {
    ENTITY_PLAYER,
    ENTITY_PIPE,
    ENTITY_FIREBALL,
    ENTITY_COIN
};

enum ColliderFlag {
    COLLIDER_ACTIVE = 1 << 0,
    COLLIDER_TRIGGER = 1 << 1,
};

enum CollideEventType {
    COLLIDE_ENTER,
    COLLIDE_STAY,
    COLLIDE_EXIT,
};

struct CollideEvent {
    char *entityId;
    int entityHash;

    bool hitThisFrame;

    CollideEventType type;
};

struct Collider {
    // ColliderType type; //NOTE: Assume everything is a rectangle

    int collideEventsCount;
    CollideEvent events[8];

    float3 offset;
    float3 scale; //NOTE: This is a percentage of the parent scale;

    u32 flags;
};

Collider make_collider(float3 offset, float3 scale, u32 flags) {
    Collider c = {};

    c.flags = flags;
    c.offset = offset;
    c.scale = scale;

    return c;
}

typedef struct Entity Entity; 

struct Entity {
    Entity *parent; 

    int colliderCount;
    Collider colliders[2];

    char *id;
    int idHash;
    EntityType type;

    //NOTES: Could be flags
    bool spriteFlipped;
    bool grounded; 

    float3 pos;
    float deltaTLeft;
    float3 deltaPos; //NOTE: Used in collision loop

    float perlinNoiseLight; //NOTE: Used for the lights

    float3 scale;
    float3 velocity;
    float rotation;
    float targetRotation;
    u64 flags;

    ///////////////////////
    //NOTE: For fireball
    float respawnTimer;

    EasyAnimation_Controller animationController;
};
