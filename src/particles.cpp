struct Particle {
    TransformX T;
    float3 dP;
    float lifeTime;
    bool alive;
};

struct ParticlerId {
    int id; 
};

ParticlerId makeParticlerId(int id) {
    ParticlerId result = {};
    result.id = id;
    return result;
}

bool particlerIdsMatch(ParticlerId id, ParticlerId id1) {
    return id.id == id1.id;
}

struct Particler {
    ParticlerId id;
    Particle particles[255];
    int count;
    int indexAt;

    Rect3f spawnBox;
    TextureHandle *imageHandle;
    float4 uvCoords;

    int colorCount;
    float4 colors[4]; //NOTE: Moves through colors as lifespan increases

    float lastTimeCreation;
    float lifespan;
    float lifeAt;
    float tAt;
    float spawnRate; //NOTE: seconds per particle
};

Particler initParticler(float lifespan, float spawnRate, Rect3f spawnBox, TextureHandle *imageHandle, float4 uvCoords, ParticlerId id) {
    Particler p = {};

    p.count = 0;
    p.lifespan = lifespan;
    p.spawnRate = 1.0f / spawnRate;
    p.lastTimeCreation = 0;
    p.tAt = 0;
    p.lifeAt = 0;
    p.indexAt = 0;
    p.spawnBox = spawnBox;
    p.imageHandle = imageHandle;
    p.uvCoords = uvCoords;
    p.id = id;

    return p;   
}

bool addColorToParticler(Particler *p, float4 color) {
    bool result = false;
    if(p->colorCount < arrayCount(p->colors)) {
        p->colorCount++;
        //NOTE: We push them on in reverse order for the gradient lerping. Because it makes it less confusing when thinking about that code
        for(int i = (p->colorCount - 1); i > 0; --i) {
            p->colors[i] = p->colors[i - 1];    
        }
        p->colors[0] = color;
        result = true;
    }
    return result;
}

bool updateParticler(Renderer *renderer, Particler *particler, float3 cameraPos, float dt) {
    particler->tAt += dt;
    particler->lifeAt += dt;

    bool isDead = (particler->lifeAt >= particler->lifespan);

    float diff = (particler->tAt - particler->lastTimeCreation);
    if(!isDead && diff >= particler->spawnRate) {
        int numberOfParticles = diff / particler->spawnRate;

        assert(numberOfParticles > 0);

        for(int i = 0; i < numberOfParticles; ++i) {
            Particle *p = 0;

            if(particler->count < arrayCount(particler->particles)) {
                p = &particler->particles[particler->count++];
                
            } else {
                //NOTE: Already full so use the ring buffer index
                assert(particler->indexAt < arrayCount(particler->particles));
                p = &particler->particles[particler->indexAt++];

                if(particler->indexAt >= arrayCount(particler->particles)) {
                    particler->indexAt = 0;
                }
            }

            assert(p);

            float x = lerp(particler->spawnBox.minX, particler->spawnBox.maxX, make_lerpTValue((float)rand() / RAND_MAX));
            float y = lerp(particler->spawnBox.minY, particler->spawnBox.maxY, make_lerpTValue((float)rand() / RAND_MAX));
            float z = lerp(particler->spawnBox.minZ, particler->spawnBox.maxZ, make_lerpTValue((float)rand() / RAND_MAX));
            p->T.pos = make_float3(x, y, z);
            p->T.scale = make_float3(random_between_float(0.3f, 1.0f), random_between_float(0.3f, 1.0f), 0.1f);

            float dpMargin = 0.8f;

            p->dP = make_float3(random_between_float(-dpMargin, dpMargin), 2, random_between_float(-dpMargin, dpMargin)); //NOTE: Straight up
            p->lifeTime = MAX_PARTICLE_LIFETIME; //Seconds
            p->alive = true;
        }

        particler->lastTimeCreation = particler->tAt;
    }
    
    int deadCount = 0;
    for(int i = 0; i < particler->count; ++i) {
        Particle *p = &particler->particles[i];

        if(p->alive) {
            float3 accelForFrame = scale_float3(dt, make_float3(0, 0, 0));

            //NOTE: Integrate velocity
            p->dP = plus_float3(p->dP, accelForFrame); //NOTE: Already * by dt 

            //NOTE: Apply drag
            // p->dP = scale_float3(0.95f, p->dP);

            //NOTE: Get the movement vector for this frame
            p->T.pos = plus_float3(p->T.pos, scale_float3(dt, p->dP));

            float3 drawP = p->T.pos;

            drawP.x -= cameraPos.x;
            drawP.y -= cameraPos.y;
            drawP.z = 1;

            float4 color = (particler->colorCount == 1) ? particler->colors[0] : make_float4(1, 1, 1, 1);

            //NOTE Update whether the particle should still be alive
            p->lifeTime -= dt;

            if(p->lifeTime <= 0) {
                p->alive = false;
            }

            if(particler->colorCount > 1) {
                //NOTE: This is lerping between the color gradients if there is more than one color
                float v = p->lifeTime / MAX_PARTICLE_LIFETIME;
                assert(v >= -dt && v < 1.0f);
                if(v < 0) {
                    v = 0;
                }

                float pos = lerp(0, particler->colorCount - 1, make_lerpTValue(v));
              
                int index = floor(pos);
                int index1 = (index + 1 < particler->colorCount) ? index + 1 : index;
               
                assert(index >= 0 && index < particler->colorCount);
                assert(index1 >= 0 && index1 < particler->colorCount);

                float t = pos - index;
                t = clamp(0.0f, 1.0f, t);
                color = lerp_float4(particler->colors[index], particler->colors[index1],  t);
            }

            //NOTE: Draw the particle
            pushTexture(renderer, particler->imageHandle, drawP, p->T.scale.xy, color, particler->uvCoords);
          
        } else {
            deadCount++;
        }
    }

    bool shouldRemove = false;

    if(isDead && deadCount == particler->count){
        //NOTE: Particle system should be removed
        shouldRemove = true;
    }

    return shouldRemove;

}

static int global_particleId = 0;

struct ParticlerParent {
    int particlerCount;
    Particler particlers[256];
};

Particler *getNewParticleSystem(ParticlerParent *parent, float3 startP, TextureHandle *imageHandle, float4 uvCoords) {
    Particler *p = 0;
    assert(parent->particlerCount < arrayCount(parent->particlers));
    if(parent->particlerCount < arrayCount(parent->particlers)) {
        float lifespan = 60.0f; //NOTE: Seconds for the _particler_ not particles. Particle lifespan is set by MAX_PARTICLE_LIFESPAN
        float spawnRatePerSecond = 100;
        parent->particlers[parent->particlerCount++] = initParticler(lifespan, spawnRatePerSecond, make_rect3f_center_dim(startP, make_float3(3, 1, 1)), imageHandle, uvCoords, makeParticlerId(global_particleId++));
        p = &parent->particlers[parent->particlerCount - 1];
    }

    return p;
}