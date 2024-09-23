//NOTE: This is for A* algorithm 

/////////////////////////// API //////////////////////
/*

SETTING UP API:
EasyAiController *easyAi_initController(Memory_Arena *allocationArena);
void easyAi_pushNode(EasyAiController *controller, float3 pos);
EasyAi_Node *easyAi_removeNode(EasyAiController *controller, float3 pos);

Per game loop check API: 

EasyAi_A_Star_Result easyAi_update_A_star(EasyAiController *controller, float3 currentPos, float3 dest);


*/
//////////////////////////////////////////////

#define EASY_AI_NODE_COUNT 1024

typedef struct  EasyAi_Node EasyAi_Node;


typedef enum {
	EASY_AI_IDLE,
	EASY_AI_MOVE_TOWARDS,
	EASY_AI_ATTACK,
	EASY_AI_COOL_DOWN,
	EASY_AI_HURT,
	EASY_AI_DIE,
} EasyAi_Mode;

typedef struct EasyAi_Node {
	float3 pos;
	bool canSeePlayerFrom; //wether on this node the entity can see the player

	EasyAi_Node *next;
} EasyAi_Node;

typedef struct EasyAi_SearchNode EasyAi_SearchNode;
typedef struct  EasyAi_SearchNode {
	float3 pos;
	float3 cameFrom;

	EasyAi_SearchNode *next;
	EasyAi_SearchNode *prev; 
} EasyAi_SearchNode;

typedef struct {
	Memory_Arena *allocationArena;
	EasyAi_Node *validBoardHash[EASY_AI_NODE_COUNT]; //this is the valid states when we're doing a * routine wether it's visited or not
	EasyAi_Node *boardHash[EASY_AI_NODE_COUNT]; //this is the state of the ai board

	EasyAi_Node *freeList;

	EasyAi_Mode aiMode;
	float waitTimer;
	
	float3 searchBouys[8]; 
	int searchBouysCount;
	int bouyIndexAt;	
} EasyAiController;

static EasyAiController *easyAi_initController(Memory_Arena *allocationArena) {
	EasyAiController *result = pushStruct(allocationArena, EasyAiController);

	easyMemory_zeroStruct(result, EasyAiController);

	result->freeList = 0;
	result->allocationArena = allocationArena;
	result->aiMode = EASY_AI_IDLE;
	result->waitTimer = 0;

	result->bouyIndexAt = 0;
	result->searchBouysCount = 0;

	return result;
}

static int easyAi_hasSearchBouy(EasyAiController *controller, float3 position) {
	int result = -1;
	for(int i = 0; i < controller->searchBouysCount && result < 0; ++i) {
	    float3 *value = &controller->searchBouys[i];
		if(value->x == position.x && value->y == position.y && value->z == position.z) {
			result = i;
			break;
		}
	}
	return result;
}

static bool easyAi_pushSearchBouy(EasyAiController *controller, float3 position) {
	bool result = false;
	if(controller->searchBouysCount < arrayCount(controller->searchBouys)) {
		controller->searchBouys[controller->searchBouysCount++] = position;
		result = true;
	} 
	return result;
}

static void easyAi_removeSearchBouy(EasyAiController *controller, int index) {
	assert(controller->searchBouysCount > index);

	//move everything down
	for(int i = index; i < (controller->searchBouysCount - 1); ++i) {
		controller->searchBouys[i] = controller->searchBouys[i + 1];	
	}
	controller->searchBouysCount--;

}

static bool easyAi_pushNode(EasyAiController *controller, float3 pos, EasyAi_Node **boardHash, bool canSeePlayerFrom) {
	pos.z = 0;
	int hashKey = (int)pos.x*13 + (int)pos.y*3 + (int)pos.z*7;
	hashKey = hashKey & (arrayCount(controller->boardHash) - 1);

	assert(hashKey < arrayCount(controller->boardHash));

	EasyAi_Node **node = &boardHash[hashKey];

	bool shouldAdd = true;

	while(*node && shouldAdd) {

		int x = (*node)->pos.x;
		int y = (*node)->pos.y;
		int z = (*node)->pos.z;

		if(x == pos.x && y == pos.y && z == pos.z) {
			shouldAdd = false;
			// easyConsole_addToStream(DEBUG_globalEasyConsole, "Node already exists");
		} else {
			node = &((*node)->next);	
		}
	}

	if(shouldAdd) {
		EasyAi_Node *newNode = 0;
		if(controller->freeList) {
			newNode = controller->freeList;
			controller->freeList = newNode->next;
		} else {
			newNode = pushStruct(controller->allocationArena, EasyAi_Node);	
		}
		
		newNode->pos = pos;
		newNode->canSeePlayerFrom = canSeePlayerFrom;

		*node = newNode;
	}

	return shouldAdd;

}

static EasyAi_Node *easyAi_hasNode(EasyAiController *controller, float3 pos, EasyAi_Node **boardHash, bool canSeePlayerFrom) {
	pos.z = 0;
	int hashKey = (int)pos.x*13 + (int)pos.y*3 + (int)pos.z*7;
	hashKey = hashKey & (arrayCount(controller->boardHash) - 1);

	assert(hashKey < arrayCount(controller->boardHash));

	EasyAi_Node **node = &boardHash[hashKey];

	EasyAi_Node *found = 0;

	while(*node && !found) {

		int x = (*node)->pos.x;
		int y = (*node)->pos.y;
		int z = (*node)->pos.z;

		if(x == pos.x && y == pos.y && z == pos.z && ((*node)->canSeePlayerFrom || canSeePlayerFrom)) {
			found = *node;
		} else {
			node = &((*node)->next);	
		}
	}

	return found;
}

static EasyAi_Node *easyAi_removeNode(EasyAiController *controller, float3 pos, EasyAi_Node **boardHash) {
	pos.z = 0;
	int hashKey = (int)pos.x*13 + (int)pos.y*3 + (int)pos.z*7;
	hashKey = hashKey & (arrayCount(controller->boardHash) - 1);

	assert(hashKey < arrayCount(controller->boardHash));

	EasyAi_Node **node = &boardHash[hashKey];

	EasyAi_Node *found = 0;

	while(*node && !found) {

		int x = (*node)->pos.x;
		int y = (*node)->pos.y;
		int z = (*node)->pos.z;

		if(x == pos.x && y == pos.y && z == pos.z) {
			found = *node;
		} else {
			node = &((*node)->next);	
		}
	}

	if(found) {
		*node = found->next;

		found->next = controller->freeList;
		controller->freeList = found->next;
	}

	return found;
}

static void easyAi_pushOnQueue(EasyAiController *controller, EasyAi_SearchNode *queue, float3 pos, float3 cameFrom, bool canSeePlayerFrom) {

	if(easyAi_hasNode(controller, pos, controller->boardHash, canSeePlayerFrom) && !easyAi_hasNode(controller, pos, controller->validBoardHash, true)) {
		EasyAi_SearchNode *node = pushStruct(controller->allocationArena, EasyAi_SearchNode);
		node->pos = pos;
		node->cameFrom = cameFrom;

		queue->next->prev = node;
		node->next = queue->next;

		queue->next = node;
		node->prev = queue;

		//push node to say you visited it
		easyAi_pushNode(controller, pos, controller->validBoardHash, true);
	}
		
}

static EasyAi_SearchNode *easyAi_popOffQueue(EasyAiController *controller, EasyAi_SearchNode *queue) {
	EasyAi_SearchNode *result = 0;

	if(queue->prev != queue) { //something on the queue
		result = queue->prev;

		queue->prev = result->prev;
		queue->prev->next = queue;
	} 

	return result;
}


typedef struct {
	float3 nextPos;
	bool found;
} EasyAi_A_Star_Result;

//pass in destenation position & returns the next position to move to 
static EasyAi_A_Star_Result easyAi_update_A_star(EasyAiController *controller, float3 currentPos, float3 dest) {
	EasyAi_A_Star_Result result = {};

	//clear valid array
	for(int i = 0; i < arrayCount(controller->validBoardHash); ++i) {
	    controller->validBoardHash[i] = 0;
	}

	
	// easyMemory_zeroSize(controller->validBoardHash, arrayCount(controller->validBoardHash)*sizeof(EasyAi_Node));

	MemoryArenaMark memMark = takeMemoryMark(controller->allocationArena);
	////////////////////////////////////////////

	//start at dest and find the quickest way to get to the current position
	//FIFO queue 
	EasyAi_SearchNode queue = {};

	queue.next = queue.prev = &queue; 

	easyAi_pushOnQueue(controller, &queue, dest, currentPos, false);

	bool searching = true;
	while(searching) {	

		EasyAi_SearchNode *node = easyAi_popOffQueue(controller, &queue);

		if(node) {

			int x = node->pos.x;
			int y = node->pos.y;
			int z = node->pos.z;

			if(x == currentPos.x && y == currentPos.y) {
				result.nextPos = node->cameFrom;
				result.found = true;
				searching = false;
				break;
			} else {
				//push on more directions
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(1, 0, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(-1, 0, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(0, 1, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(0, -1, 0)), node->pos, true);


				//Diagonal movements
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(1, 1, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(1, -1, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(-1, -1, 0)), node->pos, true);
				easyAi_pushOnQueue(controller, &queue, plus_float3(node->pos, make_float3(-1, 1, 0)), node->pos, true);
			}
		} else {
			searching = false;
			result.found = false;
			break;
		}
	}

	/////////////////////////////////
	releaseMemoryMark(&memMark);

	return result;

}

//NOTE: Searches all nodes - visible to the player and not
static EasyAi_Node *easyAi_getClosestNode(EasyAiController *aiController, float3 pos) {
	float closetDistance = FLT_MAX;
	EasyAi_Node *closetNode = 0;

	for(int i = 0; i < arrayCount(aiController->boardHash); ++i) {
	    EasyAi_Node *n = aiController->boardHash[i];

	    while(n) {
	    	float distanceSqr = float3_magnitude_sqr(minus_float3(pos, n->pos));

	    	if(distanceSqr < closetDistance) {
	    		closetDistance = distanceSqr;
	    		closetNode = n;
	    	}

	        n = n->next;
	    }
	}

	return closetNode;
}