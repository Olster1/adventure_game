
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

        result->timeAt = MAX_TIME_DAMAGE_SPLAT;
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
    while(*d) {
        ((*d)->timeAt) -= dt;

        {
            (*d)->offsetP.y += dt*2;
            float3 p = getRenderWorldP(e->pos);
            p = plus_float3(p, (*d)->offsetP);
            p.z = 1;

            float alpha = (*d)->timeAt;
            if(alpha < 0) {
                alpha = 0;
            }

            float4 color = make_float4(1, 1, 1, alpha / MAX_TIME_DAMAGE_SPLAT);

            p.x -= gameState->cameraPos.x; 
            p.y -= gameState->cameraPos.y;
            char *str = easy_createString_printf(&globalPerFrameArena, "%d", (*d)->damage);
            RenderDamageSplatItem r = {};
            r.string = str;
            r.p = p;
            r.color = color;
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
    }
}