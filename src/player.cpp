#define DEFAULT_PLAYER_ANIMATION_SPEED 0.1

void updatePlayerMoveKey(EditorState *editorState, PlatformKeyType type, float2 moveVector, float2 *impluse, bool *playerMoved) {
		if(global_platformInput.keyStates[type].isDown && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		*impluse = plus_float2(moveVector, *impluse);
		*playerMoved = true;
	}
}

void updatePlayerInput(EditorState *editorState) {
	if(!editorState->cameraFollowPlayer) {
		return;
	}
    
	bool playerMoved = false;

	float2 impluse = make_float2(0, 0);

	//NOTE: Move up
	updatePlayerMoveKey(editorState, PLATFORM_KEY_UP, make_float2(0, 1), &impluse, &playerMoved);
	updatePlayerMoveKey(editorState, PLATFORM_KEY_DOWN, make_float2(0, -1), &impluse, &playerMoved);
	updatePlayerMoveKey(editorState, PLATFORM_KEY_LEFT, make_float2(-1, 0), &impluse, &playerMoved);
	updatePlayerMoveKey(editorState, PLATFORM_KEY_RIGHT, make_float2(1, 0), &impluse, &playerMoved);

	//NOTE: Update the walking animation 
	if(playerMoved) {
		Animation *animation = 0;
		float margin = 0.1f;
		editorState->player->spriteFlipped = false;

		if((impluse.x > margin  && impluse.y < -margin) || (impluse.x < -margin && impluse.y > margin) || ((impluse.x > margin && impluse.y < margin && impluse.y > -margin))) { //NOTE: The extra check is becuase the front & back sideways animation aren't matching - should flip one in the aesprite
			editorState->player->spriteFlipped = true;
		}

		if(impluse.y > margin) {
			if(impluse.x < -margin || impluse.x > margin) {
				animation = &editorState->playerbackwardSidewardRun;	
			} else {
				animation = &editorState->playerRunbackwardAnimation;
			}
		} else if(impluse.y < -margin) {
			if(impluse.x < -margin || impluse.x > margin) {
				animation = &editorState->playerforwardSidewardRun;	
			} else {
				animation = &editorState->playerRunForwardAnimation;
			}
		} else {
			animation = &editorState->playerRunsidewardAnimation;
		}

		//NOTE: Push the Run Animation
		if(editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && editorState->player->animationController.lastAnimationOn != animation)  {
			easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
			easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, animation, DEFAULT_PLAYER_ANIMATION_SPEED);	
		}
	}

	impluse = normalize_float2(impluse);
	impluse = scale_float2(editorState->player->speed, impluse); 

	editorState->player->velocity.xy = plus_float2(editorState->player->velocity.xy, impluse);
	

	//NOTE: IDLE ANIMATION
	if(!playerMoved && editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerIdleAnimation && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation)  {
		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	
	}
	
	// //NOTE: JUMP ANIMATION
	// if(global_platformInput.keyStates[PLATFORM_KEY_SPACE].pressedCount > 0 && editorState->player->grounded) {
	// 	editorState->player->velocity.y = 10.0f;
	// 	easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
	// 	easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerJumpAnimation, 0.08f);	
	// 	easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerFallingAnimation, 0.08f);	
	// } else {
	// 	if(editorState->player->animationController.lastAnimationOn == &editorState->playerJumpAnimation && editorState->player->grounded)  {
	// 		//NOTE: LANDED ANIMATION
	// 		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
	// 		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	
	// 	} else if(editorState->player->animationController.lastAnimationOn != &editorState->playerJumpAnimation && !editorState->player->grounded && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
	// 		//NOTE: FALLING ANIMATION
	// 		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
	// 		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerFallingAnimation, 0.08f);	
	// 	}
	// }

	//NOTE: Attack 
	if(global_platformInput.keyStates[PLATFORM_KEY_X].pressedCount > 0 && editorState->player->animationController.lastAnimationOn != &editorState->playerAttackAnimation) {
		//NOTE: SpriteFlipped = false
		editorState->player->velocity.x = editorState->player->speed;
		editorState->player->colliders[ATTACK_COLLIDER_INDEX].offset = make_float3(0.5f, 0, 0);

		if(editorState->player->spriteFlipped) {
			editorState->player->velocity.x = -editorState->player->speed;
			editorState->player->colliders[ATTACK_COLLIDER_INDEX].offset = make_float3(-0.5f, 0, 0);
		} 

		//NOTE: Make active
		editorState->player->colliders[ATTACK_COLLIDER_INDEX].flags |= COLLIDER_ACTIVE;

		easyAnimation_emptyAnimationContoller(&editorState->player->animationController, &editorState->animationItemFreeListPtr);
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerAttackAnimation, 0.08f);	
		easyAnimation_addAnimationToController(&editorState->player->animationController, &editorState->animationItemFreeListPtr, &editorState->playerIdleAnimation, 0.08f);	

	} 
}