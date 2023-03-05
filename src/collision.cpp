
void addCollisionEvent(Collider *a1, Entity *b) {
    CollideEventType type = COLLIDE_ENTER;

    CollideEvent *oldEvent = 0;

    for(int i = 0; i < a1->collideEventsCount; ++i) {
        CollideEvent *oldEvent = &a1->events[a1->collideEventsCount];
        if(oldEvent->entityHash == b->idHash && easyString_stringsMatch_nullTerminated(oldEvent->entityId, b->id)) {
            if(oldEvent->type == COLLIDE_EXIT) {
                type = COLLIDE_ENTER;
            } else {
                type = COLLIDE_STAY;
            }
            
            break;
        } 
    }

    //NOTE: Add a new event
    if(!oldEvent && a1->collideEventsCount < arrayCount(a1->events)) {
        CollideEvent *e = a1->events + a1->collideEventsCount++;
        
        e->type = type;
        e->hitThisFrame = true;
        e->entityId = b->id;
        e->entityHash = b->idHash;
        e->hitThisFrame = true;
    } else if(oldEvent) {
        assert(oldEvent);

        if(!oldEvent->hitThisFrame) {
            //NOTE: Don't override info that has been added this frame
            oldEvent->type = type;
            oldEvent->hitThisFrame = true;
        }
    }
}

void updateExitCollision(Collider *a1, Entity *b) {
    for(int i = 0; i < a1->collideEventsCount; ++i) {
        CollideEvent *oldEvent = &a1->events[a1->collideEventsCount];
        if(oldEvent->entityHash == b->idHash && easyString_stringsMatch_nullTerminated(oldEvent->entityId, b->id)) {
            if(oldEvent->type == COLLIDE_EXIT) {
                //NOTE: Remove event
                
            } else {
                oldEvent->type = COLLIDE_EXIT;
                oldEvent->hitThisFrame = true;
            }
            
            break;
        } 

    }
}

void prepareCollisions(Collider *a1) {
    for(int i = 0; i < a1->collideEventsCount; ++i) {
        CollideEvent *e = &a1->events[a1->collideEventsCount];
        e->hitThisFrame = false;
    }
}

void clearStaleCollisions(Collider *a1) {
    for(int i = 0; i < a1->collideEventsCount; ) {
        int addend = 1;
        CollideEvent *e = &a1->events[a1->collideEventsCount];

        if(!e->hitThisFrame) {
            //NOTE: Remove event
            a1->events[i] = a1->events[--a1->collideEventsCount];
            addend = 0;
        }
        i += addend;
    }
}

void updateTriggerCollision(Entity *a, Entity *b, Collider *a1, Collider *b1) {
    float3 aPos = getWorldPosition(a);
    float3 bPos = getWorldPosition(b);

    Rect2f aRect = make_rect2f_center_dim(plus_float3(aPos, a1->offset).xy, float3_hadamard(a1->scale, a->scale).xy);
    Rect2f bRect = make_rect2f_center_dim(plus_float3(bPos, b1->offset).xy, float3_hadamard(b1->scale, b->scale).xy);

    //NOTE: See if it hit anything
    Rect2f minowskiPlus = rect2f_minowski_plus(aRect, bRect, plus_float3(b->pos, b1->offset).xy);
    if(in_rect2f_bounds(minowskiPlus, a->pos.xy)) {
        addCollisionEvent(a1, b);
        addCollisionEvent(b1, a);
    } else {
        updateExitCollision(a1, b);
        updateExitCollision(b1, a);
    }
}


struct RayCastResult {
    float2 collisionNormal; //NOTE: Pointing outwards
    float distance; //NOTE: Between 0 - 1 of the incoming deltaP
	bool hit;

    Entity *a;
    Collider *colA;
    Entity *b;
    Collider *colB;
};

RayCastResult castAgainstSide(float2 startP, float2 normal, float2 startSide, float3 deltaP, float sideLength) {
    RayCastResult result = {};
    float2 relA = minus_float2(startP, startSide);

    float tValue = -relA.x / deltaP.x;

    if(tValue >= 0.0f && tValue <= 1.0f) {
        float yValue = lerp(startP.y, plus_float2(startP, deltaP.xy).y, make_lerpTValue(tValue)); 

        if(yValue >= 0.0f && yValue <= sideLength) {
            result.hit = true;
            result.distance = tValue;
            result.collisionNormal = normal;
        }
    }
    return result;
}

RayCastResult rayCast_rectangle(float3 startP, float3 deltaP, Rect2f rect) {
	RayCastResult result = {};

    //NOTE: LEFT 
    if(!result.hit) {
        float2 startSide = make_float2(rect.minX, rect.minY);
        float2 normal = make_float2(-1, 0);
        float sideLength =(rect.maxY - rect.minY);

        result = castAgainstSide(startP.xy, normal, startSide, deltaP, sideLength);
    }

    //NOTE: TOP
    if(!result.hit) {
        float2 startSide = make_float2(rect.minX, rect.maxY);
        float2 normal = make_float2(0, 1);
        float sideLength =(rect.maxX - rect.minX);

        result = castAgainstSide(startP.xy, normal, startSide, deltaP, sideLength);
    }

    //NOTE: RIGHT
    if(!result.hit) {
        float2 startSide = make_float2(rect.maxX, rect.maxY);
        float2 normal = make_float2(1, 0);
        float sideLength =(rect.maxY - rect.minY);

        result = castAgainstSide(startP.xy, normal, startSide, deltaP, sideLength);
    }

    //NOTE: BOTTOM
    if(!result.hit) {
        float2 startSide = make_float2(rect.minX, rect.maxY);
        float2 normal = make_float2(0, -1);
        float sideLength =(rect.maxX - rect.minX);

        result = castAgainstSide(startP.xy, normal, startSide, deltaP, sideLength);
    }
    

	return result;
}


void updateCollisions(Entity *a, Entity *b, Collider *a1, Collider *b1, float *shortestDistance, RayCastResult *shortestRayCastResult) {
    float3 aPos = getWorldPosition(a);
    float3 bPos = getWorldPosition(b);

    Rect2f aRect = make_rect2f_center_dim(plus_float3(aPos, a1->offset).xy, float3_hadamard(a1->scale, a->scale).xy);
    Rect2f bRect = make_rect2f_center_dim(plus_float3(bPos, b1->offset).xy, float3_hadamard(b1->scale, b->scale).xy);

    //NOTE: See if it hit anything
    Rect2f minowskiPlus = rect2f_minowski_plus(aRect, bRect, plus_float3(b->pos, b1->offset).xy);

    //NOTE: Ray cast agains minkowski rectangle
    RayCastResult r = rayCast_rectangle(aPos, a->deltaPos, minowskiPlus);

    if(r.hit && r.distance < *shortestDistance) {
        //NOTE: See if shortest distance
        *shortestRayCastResult = r;
        shortestRayCastResult->a = a;
        shortestRayCastResult->b = b;
        shortestRayCastResult->colA = a1;
        shortestRayCastResult->colB = b1;

        *shortestDistance = r.distance;
    }
}

void updateEntityCollisions(EditorState *editorState, float dt) {
    for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *a = &editorState->entities[i];

		for(int colliderIndex = 0; colliderIndex < a->colliderCount; ++colliderIndex) {
			Collider *colA = &a->colliders[colliderIndex];

			prepareCollisions(colA);
		}

        a->deltaPos.xy = scale_float2(dt, a->velocity.xy);
        a->deltaTLeft = 1.0f;
	}

	//NOTE: Collision detection
	for(int iterationIndex = 0; iterationIndex < 4; ++iterationIndex) {
		for(int i = 0; i < editorState->entityCount; ++i) {

			Entity *a = &editorState->entities[i];

            float shortestDistance = 1.0f; //NOTE: Percentage
            RayCastResult shortestRayCastResult = {};

            //NOTE: Update TILE collisions first
            if(float3_magnitude(a->deltaPos) > 0.0f) { 
                float3 entWorldP = getWorldPosition(a);

                for(int i = 0; i < editorState->tileCount; ++i) {
                    MapTile t = editorState->tiles[i];

                    float2 tileP = make_float2(t.x + 0.5f, t.y + 0.5f);

                    Rect2f tileRect = make_rect2f_center_dim(tileP, make_float2(1, 1));
                    Rect2f entRect = make_rect2f_center_dim(entWorldP.xy, a->scale.xy);

                    Rect2f minowskiPlus = rect2f_minowski_plus(tileRect, entRect, tileP);

                    //NOTE: Ray cast agains minkowski rectangle
                    RayCastResult r = rayCast_rectangle(entWorldP, a->deltaPos, minowskiPlus);

                    if(r.hit && r.distance < shortestDistance) {
                        //NOTE: See if shortest distance
                        shortestRayCastResult = r;
                        shortestDistance = r.distance;
                    }
                }
            }

			//NOTE: Process other entity collisions
            for(int j = 0; j  < editorState->entityCount; ++j) {
                if(i == j) {
                    continue;
                }

                Entity *b = &editorState->entities[j];

                for(int colliderIndex = 0; colliderIndex < a->colliderCount; ++colliderIndex) {
                    for(int colliderIndex1 = 0; colliderIndex1 < b->colliderCount; ++colliderIndex1) {
                        Collider *colA = &a->colliders[colliderIndex];
                        Collider *colB = &b->colliders[colliderIndex1];

                        // if((colA->flags & COLLIDER_ACTIVE) && (colB->flags & COLLIDER_ACTIVE)) 
                        {
                            //NOTE: See if trigger
                            if((colA->flags & COLLIDER_TRIGGER) || (colB->flags & COLLIDER_TRIGGER)) {
                                updateTriggerCollision(a, b, colA, colB);
                            } else { 
                                //TODO: continous collision
                               updateCollisions(a, b, colA, colB, &shortestDistance, &shortestRayCastResult);
                            }
                        }
                    }
                }
            }

            //NOTE: Update position and velocity
            if(shortestRayCastResult.hit) {

                addCollisionEvent(shortestRayCastResult.colA, shortestRayCastResult.b);
                addCollisionEvent(shortestRayCastResult.colB, shortestRayCastResult.a);

                float2 moveVector = scale_float2(shortestRayCastResult.distance, a->deltaPos.xy);
                a->pos.xy = plus_float2(moveVector,  a->pos.xy);

                //NOTE: Take the component out of the velocity
                a->velocity.xy = minus_float2(a->velocity.xy, scale_float2(-float2_dot(a->velocity.xy, shortestRayCastResult.collisionNormal), shortestRayCastResult.collisionNormal));

                //NOTE: Remove distance left to travel
                a->deltaTLeft -= shortestRayCastResult.distance;
                if(a->deltaTLeft > 0) {
                    a->deltaPos.xy = scale_float2(dt*a->deltaTLeft, a->velocity.xy);
                } else {
                    a->deltaPos = make_float3(0, 0, 0);
                    a->deltaTLeft = 0;
                }
            }
        }
    }

	for(int i = 0; i < editorState->entityCount; ++i) {
		Entity *a = &editorState->entities[i];

		for(int colliderIndex = 0; colliderIndex < a->colliderCount; ++colliderIndex) {
			Collider *colA = &a->colliders[colliderIndex];

			clearStaleCollisions(colA);
		}
	}
}

