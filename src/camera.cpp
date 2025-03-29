void updateCamera(GameState *gameState, float dt) {
	float2 cameraOffset = make_float2(0, 0);

	if(gameState->shakeTimer >= 0) {

		float offset = SimplexNoise_fractal_1d(40, gameState->shakeTimer, 3)*(1.0f - gameState->shakeTimer);
		//NOTE: Update the camera position offset
		cameraOffset.x = offset;
		cameraOffset.y = offset;

		gameState->shakeTimer += dt;

		if(gameState->shakeTimer >= 1.0f) {
			gameState->shakeTimer = -1.0f;
		}
	}

	if(gameState->cameraFollowPlayer) {
		gameState->cameraPos.x = lerp(gameState->cameraPos.x, gameState->player->pos.x + cameraOffset.x, make_lerpTValue(0.4f));
		gameState->cameraPos.y = lerp(gameState->cameraPos.y, gameState->player->pos.y + cameraOffset.y, make_lerpTValue(0.4f));
	} else {
		float speed = 10*dt;
		if(global_platformInput.keyStates[PLATFORM_KEY_UP].isDown) {
			gameState->cameraPos.y += speed; 
		}
		if(global_platformInput.keyStates[PLATFORM_KEY_DOWN].isDown) {
			gameState->cameraPos.y -= speed; 
		}
		if(global_platformInput.keyStates[PLATFORM_KEY_LEFT].isDown) {
			gameState->cameraPos.x -= speed; 
		}
		if(global_platformInput.keyStates[PLATFORM_KEY_RIGHT].isDown) {
			gameState->cameraPos.x += speed; 
		}
		
	}

}