char *makeEntityId(EditorState *editorState) {
    __time64_t t;
    __time64_t timeSinceEpoch = _time64(&t);
    char *result = easy_createString_printf(&global_long_term_arena, "%ld-%d-%d", timeSinceEpoch, editorState->randomIdStartApp, editorState->randomIdStart);

    //NOTE: This would have to be locked in threaded application
    editorState->randomIdStart++;

    return result;
}  

float16 getModelToWorldTransform(Entity *e_) {
    float16 result = float16_identity();

    Entity *e = e_;

    while(e) {
        //NOTE: ROTATION
        float16 local = float16_angle_aroundZ(lerp(0, 2*PI32, make_lerpTValue(e->rotation)));

        if(e == e_) { //NOTE: Only scale by the orignal entity
            //NOTE: SCALE
            local = float16_scale(local, e->scale);
        }

        //NOTE: POS
        local = float16_set_pos(local, e->pos);

        //NOTE: Cocat to end matrix
        result = float16_multiply(local, result);

        e = e->parent;
    }

    return result;
}

float3 getWorldPosition(Entity *e_) {
    float3 result = e_->pos;

    Entity *e = e_->parent;

    while(e) {
        result = plus_float3(result, e->pos);

        e = e->parent;
    }

    return result;
}

float16 getModelToViewTransform(Entity *e_, float3 cameraPos) {

    float16 result = getModelToWorldTransform(e_);

    result = float16_multiply(result, float16_set_pos(float16_identity(), float3_negate(cameraPos)));

    return result;

}

Entity *addFireballEnemy(EditorState *state) {
    Entity *e = 0;
    if(state->entityCount < arrayCount(state->entities)) {
        e = &state->entities[state->entityCount++];

        e->id = makeEntityId(state);
        e->idHash = get_crc32_for_string(e->id);
        
        e->type = ENTITY_FIREBALL;

        e->velocity = make_float3(-1, 0, 0);
        e->pos = make_float3(0, 0, 10);
        e->flags |= ENTITY_ACTIVE;
        e->respawnTimer = 3;
        e->scale = make_float3(2, 1, 1);

        e->colliders[e->colliderCount++] = make_collider(make_float3(0, 0, 0), make_float3(1, 1, 0), COLLIDER_ACTIVE | COLLIDER_TRIGGER);

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationItemFreeListPtr, &state->fireballIdleAnimation, 0.3f);
		
    }
    return e;
} 

