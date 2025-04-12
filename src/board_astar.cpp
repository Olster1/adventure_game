struct FloodFillEvent {
    int x; 
    int y;
	int z;
    FloodFillEvent *next;
    FloodFillEvent *prev;
};

#define MAX_ASTAR_ARRAY_LENGTH MAX_MOVE_DISTANCE*MAX_MOVE_DISTANCE*MAX_HEIGHT_LEVEL

void pushOnFloodFillQueue(FloodFillEvent *queue, bool *visited, int x, int y, int z, float3 origin) {
	int index = MAX_MOVE_DISTANCE*MAX_MOVE_DISTANCE*z + MAX_MOVE_DISTANCE*(y - origin.y) + (x - origin.x);
	
	if(index < MAX_ASTAR_ARRAY_LENGTH && !visited[index]) { //TODO
		FloodFillEvent *node = pushStruct(&globalPerFrameArena, FloodFillEvent);
		node->x = x;
        node->y = y;
		node->z = z;

		queue->next->prev = node;
		node->next = queue->next;

		queue->next = node;
		node->prev = queue;

		//push node to say you visited it
		
        visited[index] = true;
	}
}

FloodFillEvent *popOffFloodFillQueue(FloodFillEvent *queue) {
	FloodFillEvent *result = 0;

	if(queue->prev != queue) { //something on the queue
		result = queue->prev;

		queue->prev = result->prev;
		queue->prev->next = queue;
	} 

	return result;
}


FloodFillEvent *floodFillSearch(GameState *gameState, float3 startP, float3 goalP) {
    bool *visited = pushArray(&globalPerFrameArena, MAX_ASTAR_ARRAY_LENGTH, bool);

	//NOTE: Need this because the visited array is local coordinates
	float3 origin = startP;	
	origin.x -= 0.5f*MAX_MOVE_DISTANCE;
	origin.y -= 0.5f*MAX_MOVE_DISTANCE;

    FloodFillEvent queue = {};
    queue.next = queue.prev = &queue; 
	pushOnFloodFillQueue(&queue, visited, startP.x, startP.y, startP.z, origin);

	bool searching = true;
	FloodFillEvent *foundNode = 0;
	int maxSearchTime = 1000;
	int searchCount = 0;
	while(searching && searchCount < maxSearchTime) {	
		FloodFillEvent *node = popOffFloodFillQueue(&queue);
		if(node) {
			int x = node->x;
			int y = node->y;
			int z = node->z;

			if(x == goalP.x && y == goalP.y && z == goalP.z) {
              //NOTE: Found goal
			  searching = false;
			  foundNode = node;
			} else {
				//push on more directions   
				pushOnFloodFillQueue(&queue, visited, x + 1, y, z, origin);
				pushOnFloodFillQueue(&queue, visited, x - 1, y, z, origin);
				pushOnFloodFillQueue(&queue, visited, x, y + 1, z, origin);
				pushOnFloodFillQueue(&queue, visited, x, y - 1, z, origin);
				
			}
		} else {
			searching = false;
			break;
		}
		searchCount++;
		printf("%d\n", searchCount);
	}
    return foundNode;
}

