void playCutWoodSound(GameState *gameState) {
    int soundIndex = random_between_int(0, arrayCount(gameState->soundAssets.woodChopSounds));
    assert(soundIndex < arrayCount(gameState->soundAssets.woodChopSounds));
    playSound(&gameState->soundAssets.woodChopSounds[soundIndex]);

}

void playSwordAttackSound(GameState *gameState) {
    int soundIndex = random_between_int(0, arrayCount(gameState->soundAssets.swordAttack));
    assert(soundIndex < arrayCount(gameState->soundAssets.swordAttack));
    playSound(&gameState->soundAssets.swordAttack[soundIndex])->volume = 0.3f;
}

void playHammerSound(GameState *gameState) {
    int soundIndex = random_between_int(0, arrayCount(gameState->soundAssets.hammerBuilding));
    assert(soundIndex < arrayCount(gameState->soundAssets.hammerBuilding));
    playSound(&gameState->soundAssets.hammerBuilding[soundIndex])->volume = 0.5f;
}

void playSuccessSound(GameState *gameState) {
    playSound(&gameState->soundAssets.successSound)->volume = 0.5f;
}

void playFootstepSound(GameState *gameState) {
    int soundIndex = random_between_int(0, arrayCount(gameState->soundAssets.footsteps));
    assert(soundIndex < arrayCount(gameState->soundAssets.footsteps));
    playSound(&gameState->soundAssets.footsteps[soundIndex])->volume = 0.3f;
}

void playArrowSound(GameState *gameState) {
    playSound(&gameState->soundAssets.arrowSound)->volume = 0.5f;
}

void playArrowHitSound(GameState *gameState) {
    playSound(&gameState->soundAssets.arrowHitSound)->volume = 0.5f;
}

void startAttackingEntity(GameState *gameState, Entity *e) {
    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
    EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->attackSide, 0.08f);
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_ATTACK_ENEMY, (item0->animation->frameCount / 2));
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_SWORD_SOUND, 1);
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_SWORD_SOUND, 8);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);
}


void startCuttingTree(GameState *gameState, Entity *e) {
    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
    EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->attackSide, 0.08f);
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_CUT_WOOD_SOUND, item0->animation->frameCount / 2);
    EasyAnimation_ListItem *item1 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->attackSide, 0.08f);
    easyAnimation_addActionForFrame(item1, ANIMATION_ACTION_PLAY_CUT_WOOD_SOUND, item1->animation->frameCount / 2);
    EasyAnimation_ListItem *item2 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->attackSide, 0.08f);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_PLAY_CUT_WOOD_SOUND, item2->animation->frameCount / 2);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_CUT_TREE, (item2->animation->frameCount / 2) + 1);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);
}


void startBuildingTower(GameState *gameState, Entity *e, float3 houseOrigin) {
    Entity *tower = placeTowerByPeasent(gameState, houseOrigin);

    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
    EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_BUILD_SOUND, item0->animation->frameCount / 2);
    EasyAnimation_ListItem *item1 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item1, ANIMATION_ACTION_PLAY_BUILD_SOUND, item1->animation->frameCount / 2);
    easyAnimation_addActionForFrame(item1, ANIMATION_ACTION_BUILD_TOWER_INTERMEDIATE, (item1->animation->frameCount / 2) + 1);
    EasyAnimation_ListItem *item2 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_PLAY_BUILD_SOUND, item2->animation->frameCount / 2);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_BUILD_TOWER, (item2->animation->frameCount / 2) + 1, tower);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);
}

void startBuildingHouse(GameState *gameState, Entity *e, float3 houseOrigin) {
    Entity *house = placeHouseByPeasent(gameState, houseOrigin);

    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
    EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_BUILD_SOUND, item0->animation->frameCount / 2);
    EasyAnimation_ListItem *item1 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item1, ANIMATION_ACTION_PLAY_BUILD_SOUND, item1->animation->frameCount / 2);
    easyAnimation_addActionForFrame(item1, ANIMATION_ACTION_BUILD_HOUSE_INTERMEDIATE, (item1->animation->frameCount / 2) + 1);
    EasyAnimation_ListItem *item2 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->work, 0.08f);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_PLAY_BUILD_SOUND, item2->animation->frameCount / 2);
    easyAnimation_addActionForFrame(item2, ANIMATION_ACTION_BUILD_HOUSE, (item2->animation->frameCount / 2) + 1, house);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);
}

void checkCutTree(GameState *gameState, bool clicked, float3 mouseP, Entity *e, float3 offset) {
    float3 p = convertRealWorldToBlockCoords(e->pos);
    if(gameState->selectedMoveType == MOVE_TYPE_CHOP && clicked && mouseP.x == (p.x + offset.x) && mouseP.y == (p.y + offset.y) && mouseP.z == (p.z + offset.z)) {
        float3 checkP = p;
        checkP.x += offset.x;
        checkP.y += offset.y;
        checkP.z += offset.z;
        Tile *tile = getTileFromWorldP(gameState, checkP);

        if(tile->flags & TILE_FLAG_TREE) {
            e->movementTargetPosition = checkP;
            startCuttingTree(gameState, e);
        }
    }
}

float2 worldCameraToScreen01(GameState *state, float2 p) {
    p.x = inverse_lerp(-0.5f*state->planeSizeX*state->zoomLevel, 0.5f*state->planeSizeX*state->zoomLevel, make_lerpTValue(p.x));
    p.y = inverse_lerp(-0.5f*state->planeSizeY*state->zoomLevel, 0.5f*state->planeSizeY*state->zoomLevel, make_lerpTValue(p.y));
    return p;
}

void hurtEntity(GameState *gameState, Entity *e, int damage) {
    e->health -= damage;
    printf("%d\n", damage);

    DamageSplat *d = getDamageSplat(gameState, e);
    if(d) {
        d->damage = damage;
    }

    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->hurt, 0.08f);
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->hurt, 0.08f);
    //NOTE: And back to the idle animation
    easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);


}

void updateArrows() {

}

void updateAnimationActions(GameState *gameState, Entity *e) {
     EasyAnimation_ListItem *listItem = easyAnimation_getCurrentAnimationItem(&e->animationController);
    if(listItem) {
        //NOTE: Process any actions that the animation controller has
        for(int i = 0; i < listItem->actionCount; ++i) {
            EasyAnimationAction *action = listItem->actions + i;

            if(!action->hasRunActionForLoop && listItem->frameIndex == action->actionFrame) {
                action->hasRunActionForLoop = true;
                //NOTE: See which action it was

                if(action->actionId == ANIMATION_ACTION_PLAY_CUT_WOOD_SOUND) {
                    playCutWoodSound(gameState);
                } else if(action->actionId == ANIMATION_ACTION_ATTACK_ENEMY) {
                    Tile *tile = getTileFromWorldP(gameState, e->movementTargetPosition);

                    if(tile->flags & TILE_FLAG_ENEMY) {
                        assert(e->movementTargetEntityId > 0);
                        Entity *attackEntity = findEntityFromTile(tile, e->movementTargetEntityId);
                        if(attackEntity) {
                            hurtEntity(gameState, attackEntity, e->damage);
                        }
                    } else {
                        assert(false);
                    }
                } else if(action->actionId == ANIMATION_ACTION_PLAY_FOOTSTEPS) {
                    playFootstepSound(gameState);
                } else if(action->actionId == ANIMATION_ACTION_PLAY_SWORD_SOUND) {
                    playSwordAttackSound(gameState);
                } else if(action->actionId == ANIMATION_ACTION_PLAY_BUILD_SOUND) {
                    playHammerSound(gameState);
                 } else if(action->actionId == ANIMATION_ACTION_BUILD_TOWER) {
                    Entity *towerEntity = (Entity *)action->payload;
                    if(towerEntity) {
                        easyAnimation_emptyAnimationContoller(&towerEntity->animationController, &gameState->animationState.animationItemFreeListPtr);
                        easyAnimation_addAnimationToController(&towerEntity->animationController, &gameState->animationState.animationItemFreeListPtr, &gameState->towerAnimation, 0.08f);

                        addArcherOnTower(gameState, towerEntity->pos);
                    }
                } else if(action->actionId == ANIMATION_ACTION_BUILD_HOUSE) {
                    Entity *houseEntity = (Entity *)action->payload;
                    if(houseEntity) {
                        easyAnimation_emptyAnimationContoller(&houseEntity->animationController, &gameState->animationState.animationItemFreeListPtr);
                        easyAnimation_addAnimationToController(&houseEntity->animationController, &gameState->animationState.animationItemFreeListPtr, &gameState->houseAnimation, 0.08f);
                        // playSuccessSound(gameState);
                    }
                } else if(action->actionId == ANIMATION_ACTION_CUT_TREE) {
                    //NOTE: Finished cutting tree so chop it down
                    Tile *tile = getTileFromWorldP(gameState, e->movementTargetPosition);

                    assert(tile->flags & TILE_FLAG_TREE);

                    tile->flags &= ~TILE_FLAG_TREE;
                    tile->flags |= TILE_FLAG_TREE_CUT;

                    gameState->gamePlay.treeCount += 3;

                    if(gameState->uiOnScreenItemCount < arrayCount(gameState->uiOnScreenItems)) {
                        UiOnScreenItem *ui = gameState->uiOnScreenItems + gameState->uiOnScreenItemCount++;
                        ui->startP = e->movementTargetPosition.xy;

                        ui->startP.x -= gameState->cameraPos.x;
                        ui->startP.y -= gameState->cameraPos.y;

                        ui->startP = worldCameraToScreen01(gameState, ui->startP);
                        ui->tAt = 0;

                    } else {
                        assert(false);
                    }
                }
            }
        }
    }
}

bool isTryingToBuild(GameState *gameState) {
    bool result = false;
    if(gameState->selectedMoveType == MOVE_TYPE_BUILD_HOUSE || gameState->selectedMoveType == MOVE_TYPE_BUILD_TOWER) {
        result = true;
    }
    return result;
}

void updateEntity(GameState *gameState, Renderer *renderer, Entity *e, float dt, float3 mouseWorldP, bool clicked) {
    DEBUG_TIME_BLOCK();
    float3 p = getWorldPosition(e);

    {
        float2 chunkP =  getChunkPosForWorldP(p.xy);
        float3 localP = getChunkLocalPos(p.x, p.y, p.z);

        int margin = CHUNK_REVEAL_MARGIN;

        if(localP.x < margin) {
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, &gameState->textureAtlas, chunkP.x - 1, chunkP.y, 0, true, true);
            assert(c);
        }
        if(localP.x >= (CHUNK_DIM - margin)) {
            // float2 chunkP =  getChunkPosForWorldP(p.xy);
            // float3 localP = getChunkLocalPos(p.x, p.y, p.z);
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, &gameState->textureAtlas, chunkP.x + 1, chunkP.y, 0, true, true);
            assert(c);
        }
        if(localP.y < margin) {
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, &gameState->textureAtlas, chunkP.x, chunkP.y - 1, 0, true, true);
            assert(c);
        }
        if(localP.y >= (CHUNK_DIM - margin)) {
            Chunk *c = gameState->terrain.getChunk(&gameState->lightingOffsets, &gameState->animationState, &gameState->textureAtlas, chunkP.x, chunkP.y + 1, 0, true, true);
            assert(c);
        }
    }

    refreshParticlers(gameState, e);

    if(e->flags & ENTITY_SELECTED) {
        Particler *p = 0;

        for(int i = 0; i < e->particlerCount && !p; i++) {
            Particler *pTemp = e->particlers[i];
            if(pTemp && pTemp->flags & ENTITY_SELECTED) {
                assert(pTemp->id.id == e->particlerIds[i].id);
                {
                    p = pTemp;
                    break;
                }
            }
        }

        if(p) {
            //NOTE: Reset the lifespan so it keeps going
            resetParticlerLife(p);
            updateParticlerWorldPosition(p, e->pos);
        }


        // if(e->type == ENTITY_PEASANT && !easyAnimation_getCurrentAnimation(&e->animationController, &e->animations->attackSide)) {
        //     float3 mouseP = convertRealWorldToBlockCoords(mouseWorldP);
        //     checkCutTree(gameState, clicked, mouseP, e, make_float3(0, 1, 0));
        //     checkCutTree(gameState, clicked, mouseP, e, make_float3(0, -1, 0));
        //     checkCutTree(gameState, clicked, mouseP, e, make_float3(1, 0, 0));
        //     checkCutTree(gameState, clicked, mouseP, e, make_float3(-1, 0, 0));
        // }
    }

    updateAnimationActions(gameState, e);

    if(e->fireTimer >= 0) {
        e->fireTimer += dt;

        Particler *p = 0;

        for(int i = 0; i < e->particlerCount && !p; i++) {
            Particler *pTemp = e->particlers[i];
            if(pTemp->flags & ENTITY_ON_FIRE) {
                assert(pTemp->id.id == e->particlerIds[i].id);
                {
                    p = pTemp;
                    break;
                }
            }
        }

        if(p) {
            //NOTE: Reset the lifespan so it keeps going
            resetParticlerLife(p);
            updateParticlerWorldPosition(p, e->pos);
        }

        float burnTime = 0;

        if(e->type == ENTITY_CASTLE) {
            burnTime = TIME_TILL_CASTLE_BURNS;
        } else if(e->type == ENTITY_HOUSE) {
            burnTime = TIME_TILL_HOUSE_BURNS;
        } else {
            assert(false);
        }

        if(e->fireTimer >= burnTime) {
            e->fireTimer = -1; //NOTE: Stop burning
            //NOTE: Stop Particle system
            p->lifeAt = p->lifespan;

            easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
            if(e->type == ENTITY_CASTLE) {
                easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &gameState->castleBurntAnimation, 0.08f);
            } else if(e->type == ENTITY_HOUSE) {
                easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &gameState->houseBurntAnimation, 0.08f);
            } else {
                assert(false);
            }
        }
    }


    //NOTE: Check if the player tried to move the units
    int selectedEntityIndex = isEntitySelected(gameState, e);
    if(selectedEntityIndex >= 0) {
        gameState->selectedEntityIds[selectedEntityIndex].isValidPos = false;

        float3 offset = minus_float3(gameState->selectedEntityIds[selectedEntityIndex].worldPos, getOriginSelection(gameState));
        float3 targetP = plus_float3(mouseWorldP, offset);
        float3 targetRounded = convertRealWorldToBlockCoords(targetP);

        int movementAction = MOVEMENT_ACTION_NONE;
        FloodFillResult searchResult = {};
        bool addSelectionResult = false;

        //NOTE: Update if they entity can shoot
        if((e->flags & ENTITY_CAN_SHOOT)) {
            float3 targetP = mouseWorldP;
            float3 targetRounded = convertRealWorldToBlockCoords(targetP);

            Tile *tile = getTileFromWorldP(gameState, targetRounded);
            bool isAttackEnemy = (tile->flags & TILE_FLAG_ENEMY) && (e->flags & ENTITY_SELECTABLE);

            //NOTE: Check if can reach the entity
            float dist = float2_magnitude(minus_float2(targetRounded.xy, gameState->selectedEntityIds[selectedEntityIndex].worldPos.xy));
            if(dist < 10 && isAttackEnemy) {
                addSelectionResult = true;
                movementAction = MOVEMENT_ACTION_SHOOT;
            }
        }

        if((e->flags & ENTITY_CAN_WALK)) {
            //NOTE: Path finding
            Tile *tile = getTileFromWorldP(gameState, targetRounded);
            if(tile) {
                bool isTree = (tile->flags & TILE_FLAG_TREE) && e->type == ENTITY_PEASANT;
                bool isAttackEnemy = (tile->flags & TILE_FLAG_ENEMY) && e->type == ENTITY_KNIGHT;
                bool tryingToBuild = isTryingToBuild(gameState);

                if(isTree || isAttackEnemy || tryingToBuild) {
                    int offsetCount = 4;
                    float3 offsets_[4] = {make_float3(1, 0, 0), make_float3(-1, 0, 0), make_float3(0, -1, 0), make_float3(0, 1, 0)};
                    float3 *offsets = offsets_;

                    if(tryingToBuild) {
                        offsetCount = 8;
                        //NOTE: These assumes all buildings being built are 2 x 2 area
                        float3 offsets_[8] = {make_float3(0, 0, 0), make_float3(1, 0, 0), make_float3(2, 1, 0), make_float3(2, 2, 0),
                                      make_float3(0, 3, 0), make_float3(1, 3, 0), make_float3(-1, 1, 0), make_float3(-1, 2, 0)};
                        offsets = offsets_;
                    }

                    for(int i = 0; i < offsetCount && !searchResult.foundNode; ++i) {
                        float3 t = plus_float3(targetRounded, offsets[i]);
                        searchResult = floodFillSearch(gameState, convertRealWorldToBlockCoords(p), t, e->maxMoveDistance);
                        if(searchResult.foundNode) {
                            assert(searchResult.cameFrom);
                            if(isTree) {
                                movementAction = MOVEMENT_ACTION_CUT_TREE;
                            } else if(isAttackEnemy) {
                                movementAction = MOVEMENT_ACTION_FIGHT_ENEMY;
                            } else if(tryingToBuild) {
                                movementAction = (gameState->selectedMoveType == MOVE_TYPE_BUILD_HOUSE) ? MOVEMENT_ACTION_BUILD_HOUSE : MOVEMENT_ACTION_BUILD_TOWER;
                            }
                        }
                    }
                } else {
                    //NOTE: Just walking, no actions
                    searchResult = floodFillSearch(gameState, convertRealWorldToBlockCoords(p), targetRounded, e->maxMoveDistance);
                }

                if(tryingToBuild && searchResult.foundNode) {
                    //NOTE: Check there is enough space for the building
                    //NOTE: These assumes all buildings being built are 2 x 2 area
                    float3 offsets[HOUSE_DIM_Y_BY_PESEANT*HOUSE_DIM_X];

                    int offsetCount = 0;
                    for(int y = 0; y < HOUSE_DIM_Y_BY_PESEANT; y++) {
                        for(int x = 0; x < HOUSE_DIM_X; x++) {
                            offsets[offsetCount++] = make_float3(x, y, 0);
                        }
                    }

                    bool enoughRoomForBuilding = true;
                    float targetHeight = getMapHeight(targetRounded.x, targetRounded.y);
                    for(int i = 0; i < arrayCount(offsets) && enoughRoomForBuilding; ++i) {
                        float3 tileWorldP = plus_float3(targetRounded, offsets[i]);
                        tileWorldP = convertRealWorldToBlockCoords(tileWorldP);
                        Tile *tile = getTileFromWorldP(gameState, tileWorldP);
                        float height = getMapHeight(tileWorldP.x, tileWorldP.y);
                        if(!(isTileValidWalkPosition(tile) && (int)targetHeight == (int)height && tile->entityOccupation == 0)) {
                            enoughRoomForBuilding = false;
                        }
                    }

                    if(!enoughRoomForBuilding) {
                        //NOTE: Not a valid position so get rid of it
                        searchResult.foundNode = 0;
                    }

                }
            }
        }

        if(searchResult.foundNode || addSelectionResult) {
            if(searchResult.foundNode) {
                assert(searchResult.cameFrom);
            }

            gameState->selectedMoveCount++;
            gameState->selectedEntityIds[selectedEntityIndex].isValidPos = true;
            gameState->selectedEntityIds[selectedEntityIndex].floodFillResult = searchResult;
            gameState->selectedEntityIds[selectedEntityIndex].movementAction = movementAction;
            gameState->selectedEntityIds[selectedEntityIndex].targetPosition = targetRounded;
        }
    }

    //NOTE: Update the entity move cycle
    if(e->moves != 0) {
        e->turnComplete = true;
        float3 lastP = convertRealWorldToBlockCoords(e->pos);

        float3 target = e->moves->move;
        float3 dir = normalize_float3(minus_float3(target, e->pos));
        float speed = e->speed*dt;
        e->pos = plus_float3(scale_float3(speed, dir), e->pos);

        if(!easyAnimation_getCurrentAnimation(&e->animationController, &e->animations->run)) {
            easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
            EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->run, 0.08f);
            easyAnimation_addActionForFrame(item0, ANIMATION_ACTION_PLAY_FOOTSTEPS, 0);
        }

        if(float3_magnitude(minus_float3(target, e->pos)) < 0.1f) {
            e->pos = target;
            EntityMove *move = e->moves;
            //NOTE: Move to the next in the list
            e->moves = move->next;


            if(!e->moves) {
                if(e->movementAction == MOVEMENT_ACTION_FIGHT_ENEMY) {
                    startAttackingEntity(gameState, e);
                } else if(e->movementAction == MOVEMENT_ACTION_CUT_TREE) {
                    startCuttingTree(gameState, e);
                } else if(e->movementAction == MOVEMENT_ACTION_BUILD_HOUSE) {
                    gameState->selectedMoveType = MOVE_TYPE_NONE;
                    startBuildingHouse(gameState, e, e->movementTargetPosition);
                } else if(e->movementAction == MOVEMENT_ACTION_BUILD_TOWER) {
                    startBuildingTower(gameState, e, e->movementTargetPosition);
                } else {
                    easyAnimation_emptyAnimationContoller(&e->animationController, &gameState->animationState.animationItemFreeListPtr);
                    EasyAnimation_ListItem *item0 = easyAnimation_addAnimationToController(&e->animationController, &gameState->animationState.animationItemFreeListPtr, &e->animations->idle, 0.08f);
                }
            }

            //NOTE: Add to the free list
            move->next = gameState->freeEntityMoves;
            gameState->freeEntityMoves = move;
        }

        float3 currentP = convertRealWorldToBlockCoords(e->pos);

        if(!sameFloat3(currentP, lastP)) {
            markBoardAsEntityOccupied(gameState, currentP, e);
            markBoardAsEntityUnOccupied(gameState, lastP, e);
        }

        assert(tileIsOccupied(gameState, currentP));
    }
}

void renderGameUILogic(Renderer *renderer, GameState *gameState, float3 renderP, Entity *e,  float dt) {
    //NOTE: This renders the little exclamation mark the entities have above their head if they still have a turn left
    if((e->flags & ENTITY_SELECTABLE) && !e->turnComplete && gameState->gamePlay.turnOn == GAME_TURN_PLAYER_KNIGHT) {
         if(!gameState->perFrameDamageSplatArray) {
            gameState->perFrameDamageSplatArray = initResizeArrayArena(RenderDamageSplatItem, &globalPerFrameArena);
        }

        char *str = easy_createString_printf(&globalPerFrameArena, "%d", e->maxMoveDistance);
        float3 p = renderP;
        p.y += 0.5f;
        p.z = RENDER_Z;
        RenderDamageSplatItem r = {};
        r.string = str;
        r.p = p;
        r.color = make_float4(1, 1, 1, 1);
        r.sprite = &gameState->splatTexture;
        r.scale = make_float2(0.7, 0.7);

        pushArrayItem(&gameState->perFrameDamageSplatArray, r, RenderDamageSplatItem);
    }
}

void pushEntityHealthBar(Renderer *renderer, GameState *gameState, float3 renderP, Entity *e,  float dt) {
    if(!gameState->perFrameHealthBarArray) {
        gameState->perFrameHealthBarArray = initResizeArrayArena(RenderDamageSplatItem, &globalPerFrameArena);
    }

    char *str = easy_createString_printf(&globalPerFrameArena, "%d", e->maxMoveDistance);
    float3 p = renderP;
    // p.y = 0.5f;
    p.z = RENDER_Z;
    RenderDamageSplatItem r = {};
    r.string = str;
    r.p = p;
    r.color = make_float4(1, 1, 1, 1);
    r.sprite = &gameState->healthBar;
    r.scale = make_float2(0.7, 0.35);

    pushArrayItem(&gameState->perFrameHealthBarArray, r, RenderDamageSplatItem);
}


void renderEntity(GameState *gameState, Renderer *renderer, Entity *e, float16 fovMatrix, float dt) {

    Texture *t = 0;

    if(easyAnimation_isControllerValid(&e->animationController)) {
        t = easyAnimation_updateAnimation_getTexture(&e->animationController, &gameState->animationState.animationItemFreeListPtr, dt);
    }

    float3 renderWorldP = getRenderWorldP(e->pos);
    renderWorldP.x += e->offsetP.x * e->scale.x;
    renderWorldP.y += e->offsetP.y * e->scale.y;

    renderWorldP.x -= gameState->cameraPos.x;
    renderWorldP.y -= gameState->cameraPos.y;

    float4 color = make_float4(1, 1, 1, 1);

    // NOTE: color the entity that you have selected
    if(isEntitySelected(gameState, e) >= 0) {
        // color = make_float4(1, 1, 1, 1);
        entityRenderSelected(gameState, e);
    } else {
        e->flags &= ~(ENTITY_SELECTED);
    }


    //NOTE: Draw position above player
    float3 tileP = convertRealWorldToBlockCoords(e->pos);
    char *str = easy_createString_printf(&globalPerFrameArena, "(%d %d %d)", (int)tileP.x, (int)tileP.y, (int)tileP.z);
    // pushShader(renderer, &sdfFontShader);
	// draw_text(renderer, &gameState->font, str, renderWorldP.x, renderWorldP.y, 0.02, make_float4(0, 0, 0, 1));

    if(t) {
        float3 sortPos = e->pos;
        sortPos.y -= e->sortYOffset;
        renderWorldP.z = RENDER_Z;

        {
            //NOTE: This draws the position of the sort index position
            /*
            float3 a = sortPos;
            a.y += a.z;
            a.z = 2;
            a.x -= gameState->cameraPos.x;
            a.y -= gameState->cameraPos.y;
            pushRect(renderer, a, make_float2(0.3f, 0.3f), make_float4(1, 0, 0, 1));
            */
        }

        pushEntityTexture(renderer, t->handle, renderWorldP, e->scale.xy, color, t->uvCoords, getSortIndex(sortPos, RENDER_LAYER_3));

        // if(e->healthBarTimer > 0)
        {
            e->healthBarTimer -= dt;

            //NOTE: Draw the health bar
            pushEntityHealthBar(renderer, gameState, renderWorldP, e, dt);

        }

    }
    renderGameUILogic(renderer, gameState, renderWorldP, e, dt);
    renderDamageSplats(gameState, e, dt);
}


void pushAllEntityLights(GameState *gameState, float dt) {
    //NOTE: Push all lights for the renderer to use
	for(int i = 0; i < gameState->entityCount; ++i) {
		Entity *e = &gameState->entities[i];

		if(hasEntityFlag(e, ENTITY_ACTIVE) && hasEntityFlag(e, LIGHT_COMPONENT)) {
            float3 worldPos = getWorldPosition(e);
            //NOTE: Update the flicker
            e->perlinNoiseLight += dt;

            if(e->perlinNoiseLight > 1.0f) {
                e->perlinNoiseLight = 0.0f;
            }

            float value = SimplexNoise_fractal_1d(40, e->perlinNoiseLight, 3);

            //NOTE: Push light
            pushGameLight(gameState, worldPos, make_float4(1, 0.5f, 0, 1), value);
		}
	}
}

