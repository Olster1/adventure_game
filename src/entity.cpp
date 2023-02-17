char *makeEntityId(EditorState *editorState) {
    __time64_t t;
    __time64_t timeSinceEpoch = _time64(&t);
    char *result = easy_createString_printf(&global_long_term_arena, "%ld-%d-%d", timeSinceEpoch, editorState->randomIdStartApp, editorState->randomIdStart);

    //NOTE: This would have to be locked in threaded application
    editorState->randomIdStart++;

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
        e->collisionScale = 1;

        easyAnimation_initController(&e->animationController);
		easyAnimation_addAnimationToController(&e->animationController, &state->animationItemFreeListPtr, &state->fireballIdleAnimation, 0.3f);
		
    }
    return e;
} 

