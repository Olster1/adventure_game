
DamageSplat *getDamageSplat(GameState *gameState, Entity *e) {
    DamageSplat *result = 0;

    if(gameState->freeListDamageSplats) {
        result = gameState->freeListDamageSplats;
        gameState->freeListDamageSplats = gameState->freeListDamageSplats->next;
        
    } else {
        result = pushStruct(&globalPerEntityLoadArena, DamageSplat);
    }
    
    if(result) {
        //NOTE: Clear the damage splat
        DamageSplat temp = {};
        *result = temp;

        result->timeAt = 1;
        result->next = e->damageSplats;
        e->damageSplats = result;
    }

    return result;
} 



void renderDamageSplats(GameState *gameState, Entity *e, float dt) {
    if(!gameState->perFrameDamageSplatArray) {
        gameState->perFrameDamageSplatArray = initResizeArrayArena(RenderDamageSplatItem, &globalPerFrameArena);
    }

    DamageSplat **d = &e->damageSplats;
    float3 offsets[4] = {make_float3(0, 1, 0), make_float3(0, -1, 0), make_float3(1, 0, 0), make_float3(-1, 0, 0)};
    int count = 0;
    while(*d) {
        ((*d)->timeAt) -= dt;

        {
            float3 p = getRenderWorldP(e->pos);
            p = plus_float3(p, offsets[count % arrayCount(offsets)]);
            p.y += 1;
            p.z = 1;

            p.x -= gameState->cameraPos.x; 
            p.y -= gameState->cameraPos.y;
            char *str = easy_createString_printf(&globalPerFrameArena, "%d", (*d)->damage);
            RenderDamageSplatItem r = {};
            r.string = str;
            r.p = p;
            pushArrayItem(&gameState->perFrameDamageSplatArray, r, RenderDamageSplatItem);
        }

        if((*d)->timeAt <= 0) {
            DamageSplat *temp = *d;
            *d = (*d)->next;

            temp->next = gameState->freeListDamageSplats;
            gameState->freeListDamageSplats = temp;
        } else {
            d = &((*d)->next);
        }
        count++;
    }
}