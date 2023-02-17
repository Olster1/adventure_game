typedef enum {
	EASY_TRANSFORM_STATIC_ID,
	EASY_TRANSFORM_TRANSIENT_ID,
	EASY_TRANSFORM_NO_ID,
} EasyTransform_IdType;

typedef struct EasyTransform EasyTransform;
typedef struct EasyTransform {
	float3 pos;
	float3 scale;
	Quaternion Q;

	EasyTransform *parent;

	EasyTransform_IdType idType;
	int id;
	bool markForDeletion;
} EasyTransform;

static int GLOBAL_transformID_static = 0;
static int GLOBAL_transformID_transient = 0; 

static inline void easyTransform_initTransform(EasyTransform *t, float3 pos, EasyTransform_IdType idType) {
	// t->T = mat4();
	t->pos = pos;
	t->scale = v3(1, 1, 1);
	t->Q = identityQuaternion();

	t->parent = 0;

	//NOTE: The id for each entity is stored on the Transform. So the static id doesn't get really big when you save the file, we have to types of ids. 
	//NOTE: If we changed to just a uuid type scheme we could get rid of this
	if(idType == EASY_TRANSFORM_STATIC_ID) {
		t->id = GLOBAL_transformID_static++;
		t->idType = idType; 
	} else if(idType == EASY_TRANSFORM_TRANSIENT_ID) {
		t->id = GLOBAL_transformID_transient++;
		t->idType = idType;
	} else if(idType == EASY_TRANSFORM_NO_ID) {
		t->id = 0;
		t->idType = idType;
	}
	
	t->markForDeletion = false;
}

static inline void easyTransform_initTransform_withScale(EasyTransform *t, float3 pos, float3 scale, EasyTransform_IdType idType) {
	easyTransform_initTransform(t, pos, idType);
	t->scale =  scale;
}


#define easyTransform_getTransform(T) easyTransform_getTransform_(T, true)
#define easyTransform_getTransform_withoutPosition(T) easyTransform_getTransform_(T, false)
static inline Matrix4 easyTransform_getTransform_(EasyTransform *T, bool withPosition) {
	float16 result = float16_identity();
	EasyTransform *parent = T;

	/*
	//NOTE(ollie): The way the transform works is that it keeps the translate out seperatley,
					so things can just add the translation & doesn't effect the rotation of it 
	*/

	float3 translation = v3(0, 0, 0);
	while(parent) {

		float16 thisT = float16_scale(float16_identity(), parent->scale);
		thisT = float16_multiply(quaternionToMatrix(parent->Q), thisT);
		

		//NOTE(ollie): Add the translation
		translation = plus_float3(parent->pos, translation);

		//NOTE(ollie): Combine it with the result
		result = float16_multiply(result, thisT);

		parent = parent->parent;
	}

	if(withPosition) {
		//NOTE(ollie): Add the translation component. See comment above to see why
		result = Mat4Mult(float16_set_pos(float16_identity translation), result);
	}

	return result;
	
}

static inline float16 easyTransform_getWorldRotation(EasyTransform *T) {
	float16 result = mat4();

	EasyTransform *parent = T->parent;
	Quaternion q = T->Q;
	while(parent) {
		q = quaternion_mult(parent->Q, q);
		parent = parent->parent;
	}

	result = quaternionToMatrix(q);

	return result;
}

static inline float3 easyTransform_getWorldScale(EasyTransform *T) {
	EasyTransform *parent = T->parent;
	float3 s = T->scale;
	while(parent) {
		s = float3_hadamard(s, parent->scale);
		parent = parent->parent;
	}

	return s;
}

static inline V3 easyTransform_getWorldPos(EasyTransform *T) {
	V3 pos = T->pos;
	EasyTransform *parent = T->parent;

	int parentCount = 0;
	while(parent) {
		parentCount++;
		pos = v3_plus(pos, parent->pos);
		parent = parent->parent;
	}

	return pos;
}

static inline void easyTransform_setWorldPos(EasyTransform *T, float3 worldPos) {
	EasyTransform *parent = T->parent;

	float3 pos = make_float3(0, 0, 0);

	while(parent) {
		pos = plus_float3(pos, parent->pos);
		parent = parent->parent;
	}

	//NOTE(ollie): Take away the remainding postion to get it to be the world pos
	pos = minus_float3(worldPos, pos);

	//NOTE(ollie): Set the world pos
	T->pos = pos;
}

static inline float3 easyTransform_getZAxis(EasyTransform *T) {
	float16 t = easyTransform_getTransform(T); //@speed dont know how expensive this could be?
	float3 result = easyMath_getZAxis(t);
	return result;
}

static inline void easyTransform_assignAsParent(EasyTransform *child, EasyTransform *parent) {
	float3 parentP = easyTransform_getWorldPos(parent);
	float3 childP = easyTransform_getWorldPos(child);
	child->pos = minus_float3(childP, parentP);

	float3 parentScale = easyTransform_getWorldScale(parent);
	float3 childScale = easyTransform_getWorldScale(child);

	child->scale = make_float3(childScale.x / parentScale.x, childScale.y / parentScale.y, childScale.z / parentScale.z);


	//After we've got their world positions
	child->parent = parent;

}