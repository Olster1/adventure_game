void updateCamera(EditorState *editorState, float dt) {
	float2 cameraOffset = make_float2(0, 0);

	if(editorState->shakeTimer >= 0) {

		float offset = perlin1d(editorState->shakeTimer, 40, 3)*(1.0f - editorState->shakeTimer);
		//NOTE: Update the camera position offset
		cameraOffset.x = offset;
		cameraOffset.y = offset;

		editorState->shakeTimer += dt;

		if(editorState->shakeTimer >= 1.0f) {
			editorState->shakeTimer = -1.0f;
		}
	}

	if(editorState->movingCamera) {
		editorState->cameraPos.x = lerp(editorState->cameraPos.x, editorState->player->pos.x + cameraOffset.x, make_lerpTValue(0.4f));
		editorState->cameraPos.y = lerp(editorState->cameraPos.y, editorState->player->pos.y + cameraOffset.y, make_lerpTValue(0.4f));
	}

}