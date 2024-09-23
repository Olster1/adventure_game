enum EntityFlags {
    ENTITY_ACTIVE = 1 << 0,
    FIRE_BALL_COMPONENT = 1 << 1,
    LIGHT_COMPONENT = 1 << 2,
    ENTITY_FLAG_ATTACKING = 1 << 3,
    ENTITY_FLAG_ONCE_OFF_ATTACK = 1 << 4,
};

#define MY_ENTITY_TYPE(FUNC) \
FUNC(ENTITY_PLAYER)\
FUNC(ENTITY_ENEMY)\
FUNC(ENTITY_PIPE)\
FUNC(ENTITY_FIREBALL)\
FUNC(ENTITY_COIN)\
FUNC(ENTITY_TILE_MAP)\

typedef enum {
    MY_ENTITY_TYPE(ENUM)
} EntityType;

static char *MyEntity_TypeStrings[] = { MY_ENTITY_TYPE(STRING) };

enum ColliderIndex {
    PLATFORM_COLLIDER_INDEX = 0,
    ATTACK_COLLIDER_INDEX = 1,
    HIT_COLLIDER_INDEX = 2,
    ENTITY_COLLIDER_INDEX_COUNT
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

    CollideEventType type;

    //NOTE: Cached values from the other entity we can use without having to look up the entity in the updateEntities loop
    EntityType entityType;
    float damage;
    float2 hitDir;

    //@private
    bool hitThisFrame; //NOTE: Only used by the internal collision code 
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

struct DefaultEntityAnimations {
    Animation idle;
	Animation runForward;
	Animation runBack;
	Animation runSideward;
    Animation runSidewardBack;
	
	Animation attack;
    Animation attackBack;
	Animation hurt;
    Animation suprised;
	Animation die;
};

typedef struct Entity Entity; 

struct Entity {
    Entity *parent; 

    int colliderCount;
    Collider colliders[ENTITY_COLLIDER_INDEX_COUNT];

    char *id;
    int idHash;
    EntityType type;

    //NOTES: Could be flags
    bool spriteFlipped;
    bool grounded; 

    float damage; //NOTE: How much damage enemy can do
    float health;

    float3 pos;
    float deltaTLeft;
    float3 deltaPos; //NOTE: Used in collision loop

    float perlinNoiseLight; //NOTE: Used for the lights

    float speed; //NOTE: How fast the entity moves - used to scale direction vectors

    float3 scale;
    float3 velocity;
    float rotation;
    float targetRotation;
    u64 flags;

    //NOTE: Used by a star
    float3 lastSetPos;

    ///////////////////////
    //NOTE: For fireball
    float respawnTimer;

    DefaultEntityAnimations *animations; //NOTE: Points to animations saved in the gameState

    EasyAnimation_Controller animationController;
    EasyAiController *aStarController;
};
