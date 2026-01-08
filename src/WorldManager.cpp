#include "WorldManager.h"
#include "game/Audio.h"

bool WorldManager::initialize()
{
    if (!initializeCamera())
    {
        DEBUG_PRINT("Failed to initialize camera");
        return false;
    }

    if (!initializeTerrain())
    {
        DEBUG_PRINT("Failed to initialize terrain");
        return false;
    }

    if (!initializeEntities())
    {
        DEBUG_PRINT("Failed to initialize entities");
        return false;
    }

    SoundPlayer &sp = SoundPlayer::getInstance();
    sp.PlayMusic(sp.LoadWav(AUDIO_DIR / "piraten.wav"), true);

    advanceWave(); // start first wave

    return true;
}

bool WorldManager::initializeCamera()
{
    // Camera setup
    CameraConfiguration camConfig{
        .fov = 50.0f,
        .aspect = (float)WINDOW_X / (float)WINDOW_Y,
        .near = 0.1f,
        .far = 2000.0f};

    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Lighting configuration
    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
        .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
        .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)};

    LightSource lightSource = LightSource::fromConfig(lightCfg);
    m_scene = std::make_unique<Scene>(camera, std::move(lightSource));

    // Create skybox
    m_scene->m_skybox = std::make_unique<Skybox>();

    // Create camera controller
    m_camController = std::make_unique<ThirdPersonCamera>();

    return true;
}

bool WorldManager::initializeTerrain()
{
    // Create terrain generator
    m_terrainGen = std::make_unique<TerrainGenerator>();

    // Create and compile terrain shader
    auto terrainShader = std::make_shared<Shader>();
    terrainShader->addShader("Terrain.vert", ShaderType::VERTEX);
    terrainShader->addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
    terrainShader->createProgram();

    // Load terrain textures
    auto groundTex = Texture::CreateTexture2D(TEXTURE_DIR / "ground.jpg", "u_texture0");
    auto grassTex = Texture::CreateTexture2D(TEXTURE_DIR / "grass.jpg", "u_texture1");
    auto mountainTex = Texture::CreateTexture2D(TEXTURE_DIR / "mountain.jpg", "u_texture2");
    auto blueWaterTex = Texture::CreateTexture2D(TEXTURE_DIR / "blueWater.jpg", "u_texture3");
    auto whiteWaterTex = Texture::CreateTexture2D(TEXTURE_DIR / "whiteWater.jpg", "u_texture4");

    std::vector<std::shared_ptr<Texture>> terrainTextures = {
        groundTex, grassTex, mountainTex, blueWaterTex, whiteWaterTex};

    // Create chunk manager
    m_chunkManager = std::make_unique<TerrainChunkManager>(m_terrainGen.get(), terrainTextures);
    m_chunkManager->setShader(terrainShader);

    // Setup fog
    updateFogSettings();

    // Setup camera min height function based on terrain height
    m_camController->setMinHeightFunction([this](float x, float z)
                                          {
                                              return m_chunkManager->getPreciseHeightAt(x, z) + 2.0f; // TODO: why return 2.0f offset?
                                          });

    return true;
}

bool WorldManager::initializeEntities()
{
    // Initialize game clock (1 game day = 10 minutes real time)
    m_gameClock = std::make_unique<GameClock>(600.0f);

    // Create player
    PlayerData playerData;
    playerData.m_position = glm::vec3(100.0f, 0.0f, 100.0f);

    m_player = std::make_unique<Player>(playerData);

    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeIdle.obj", AnimationState::IDLE, 0.5f));
    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeRun1.obj", AnimationState::WALKING, 0.2f));
    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeRun2.obj", AnimationState::WALKING, 0.2f));
    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack1.obj", AnimationState::ATTACK, m_player->m_playerData.m_attackCooldown / 3.0f));
    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack2.obj", AnimationState::ATTACK, m_player->m_playerData.m_attackCooldown / 3.0f));
    m_player->m_playerRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack3.obj", AnimationState::ATTACK, m_player->m_playerData.m_attackCooldown / 3.0f));

    EntitySounds playerSounds{.m_attackSound = SoundPlayer::getInstance().LoadWav(AUDIO_DIR / "swordSwoosh.wav")};
    m_player->setEntitySounds(playerSounds);
    
    // Preload explosion sound
    m_explosionSound = SoundPlayer::getInstance().LoadWav(AUDIO_DIR / "explosion.wav");
    
    float fogStart = m_renderDistance * m_fogStart;
    float fogEnd = m_renderDistance * m_fogEnd;
    m_player->m_playerRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);

    initializeEnemySpawners();

    return true;
}

bool WorldManager::initializeEnemySpawners()
{
    float fogStart = m_renderDistance * m_fogStart;
    float fogEnd = m_renderDistance * m_fogEnd;

    // Setup Cow - unlocks at wave 1 (basic enemy)
    EnemyData cowEnemyData; // default enemy data
    SpawnerConfig cowSpawnerConfig{
        .m_maxEnemies = 10,
        .m_spawnInterval = 0.1f,
        .m_enemiesPerWaveFactor = 2, // doubles each wave
    };
    std::unique_ptr<EnemySpawner> cowSpawner = std::make_unique<EnemySpawner>(cowEnemyData, cowSpawnerConfig);
    cowSpawner->setMinHeightFunction([this](float x, float z)
                                     { return m_chunkManager->getPreciseHeightAt(x, z); });
    // Add animation frames
    cowSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "cow" / "cow.obj", AnimationState::IDLE, 1.0f));
    cowSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "cow" / "cow_walk1.obj", AnimationState::WALKING, 0.5f));
    cowSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "cow" / "cow_walk2.obj", AnimationState::WALKING, 0.5f));
    cowSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "cow" / "cow.obj", AnimationState::ATTACK, 1.0f));
    // set fog uniforms
    cowSpawner->m_animatedInstanceRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);
    // set sounds
    EntitySounds cowSounds{.m_attackSound = SoundPlayer::getInstance().LoadWav(AUDIO_DIR / "cow_moo.wav")};
    cowSpawner->setEntitySounds(cowSounds);
    // add to world manager
    m_enemySpawners.push_back(std::move(cowSpawner));

    // Setup Abbe - unlocks at wave 2 (faster enemy)
    EnemyData abbeEnemyData; // default enemy data
    abbeEnemyData.m_maxHealth = 75.0f;
    abbeEnemyData.m_health = 75.0f;
    abbeEnemyData.m_attackDamage = 9.0f;
    abbeEnemyData.m_attackCooldown = 4.0f;
    abbeEnemyData.m_moveSpeed = 10.0f;
    abbeEnemyData.killScore = 3; // Abbe gives moderate points
    SpawnerConfig abbeSpawnerConfig{
        .m_maxEnemies = 5,
        .m_spawnInterval = 0.3f,
        .m_minSpawnDistance = 200.0f,
        .m_maxSpawnDistance = 250.0f,
        .m_despawnThreshold = 400.0f,
        .m_enemiesPerWaveIncrement = 5,
    };
    std::unique_ptr<EnemySpawner> abbeSpawner = std::make_unique<EnemySpawner>(abbeEnemyData, abbeSpawnerConfig);
    abbeSpawner->setMinHeightFunction([this](float x, float z)
                                      { return m_chunkManager->getPreciseHeightAt(x, z); });
    // Add animation frames
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeIdle.obj", AnimationState::IDLE, 0.5f));
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeRun1.obj", AnimationState::WALKING, 0.2f));
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeRun2.obj", AnimationState::WALKING, 0.2f));
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack1.obj", AnimationState::ATTACK, 0.1f));
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack2.obj", AnimationState::ATTACK, 0.1f));
    abbeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "abbe" / "abbeAttack3.obj", AnimationState::ATTACK, 0.1f));
    abbeSpawner->m_animatedInstanceRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);
    // set sounds
    EntitySounds abbeSounds{.m_attackSound = SoundPlayer::getInstance().LoadWav(AUDIO_DIR / "swordAttack.wav")};
    abbeSpawner->setEntitySounds(abbeSounds);
    m_enemySpawners.push_back(std::move(abbeSpawner));

    // Setup Mange - unlocks at wave 3 (stronger enemy)
    EnemyData mangeEnemyData; // default enemy data
    mangeEnemyData.m_maxHealth = 300.0f;
    mangeEnemyData.m_health = 300.0f;
    mangeEnemyData.m_attackDamage = 25.0f;
    mangeEnemyData.m_attackCooldown = 6.0f;
    mangeEnemyData.m_moveSpeed = 4.0f;
    mangeEnemyData.killScore = 10; // Mange gives more points
    SpawnerConfig mangeSpawnerConfig{
        .m_maxEnemies = 1,
        .m_spawnInterval = 2.0f,
        .m_enemiesPerWaveIncrement = 2,        
    };
    std::unique_ptr<EnemySpawner> mangeSpawner = std::make_unique<EnemySpawner>(mangeEnemyData, mangeSpawnerConfig);
    mangeSpawner->setMinHeightFunction([this](float x, float z)
                                       { return m_chunkManager->getPreciseHeightAt(x, z); });
    // Add animation frames
    mangeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "MangeMob" / "MangeMob.obj", AnimationState::IDLE, 0.5f));
    mangeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "MangeMob" / "MangeWalk1.obj", AnimationState::WALKING, 0.5f));
    mangeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "MangeMob" / "MangeWalk2.obj", AnimationState::WALKING, 0.5f));
    mangeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "MangeMob" / "MangeAttack1.obj", AnimationState::ATTACK, 0.3));
    mangeSpawner->m_animatedInstanceRenderer->addAnimationFrame(AnimatedInstanceRenderer::createAnimatedInstanceFrame(MODELS_DIR / "MangeMob" / "MangeAttack2.obj", AnimationState::ATTACK, 0.3));

    // set sounds
    EntitySounds mangeSounds{.m_attackSound = SoundPlayer::getInstance().LoadWav(AUDIO_DIR / "gun_explosion.wav")};
    mangeSpawner->setEntitySounds(mangeSounds);
    // set fog uniforms
    mangeSpawner->m_animatedInstanceRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);
    m_enemySpawners.push_back(std::move(mangeSpawner));

    return true;
}

void WorldManager::update(float dt, InputManager *input)
{
    if (!m_scene || !m_player)
    {
        return;
    }

    // Update screen flash timer
    if (m_screenFlashTimer > 0.0f)
    {
        m_screenFlashTimer -= dt;
        if (m_screenFlashTimer < 0.0f)
            m_screenFlashTimer = 0.0f;
    }

    // Update game clock and wave system
    if (m_gameClock)
    {
        m_gameClock->Update(dt);
        updateWaveSystem(dt);
    }

    std::vector<EnemyData *> allEnemies;
    for (auto &spawner : m_enemySpawners)
    {
        for (auto &enemy : spawner->m_enemyDataList)
        {
            allEnemies.push_back(&enemy);
        }
    }

    // Auto-attack: attack if any enemy is in the attack zone (in front of player)
    bool enemyInAttackZone = false;
    glm::vec3 playerPos = m_player->m_playerData.m_position;
    float yaw = m_player->m_playerData.m_yaw;
    float attackRangeOffset = m_player->m_playerData.m_attackRangeOffset;
    float attackRange = m_player->m_playerData.m_attackRange;
    
    // Calculate attack circle center (in front of player)
    glm::vec3 attackCircleCenter = playerPos + glm::vec3(
        sin(glm::radians(yaw)) * attackRangeOffset,
        0.0f,
        cos(glm::radians(yaw)) * attackRangeOffset
    );
    
    for (EnemyData* e : allEnemies)
    {
        if (!e->isDead())
        {
            glm::vec3 toEnemy = e->m_position - attackCircleCenter;
            toEnemy.y = 0.0f;
            if (glm::length(toEnemy) <= attackRange)
            {
                enemyInAttackZone = true;
                break;
            }
        }
    }
    
    if (enemyInAttackZone)
    {
        int hits = m_player->attack(allEnemies);
    }

    // Special attack - J key
    if (input->keyboardInput.getKeyState(OOGABOOGA_SPECIAL_ATTACK_KEY).readAndClear())
    {
        if (m_player->m_playerData.isSpecialAttackReady())
        {
            m_player->specialAttack(allEnemies);
            triggerScreenFlash();
        }
    }

#ifdef DEBUG
    if (input->keyboardInput.getKeyState(OOGABOOGA_DEBUG_WILDCARD_KEY).readAndClear())
    {
        DEBUG_PRINT("DEBUG: Advancing to next wave");
        advanceWave();
    }
#endif

    // Update player
    if (m_player && m_chunkManager)
    {
        m_player->update(dt, input, m_chunkManager.get());

        // Check for player death (UI handles the death screen, sound, and score submission)
        if (m_player->m_playerData.m_health <= 0.0f && !m_scorePosted)
        {
            m_isGameOver = true;
            m_scorePosted = true;

            // Play death sound (reusing explosion sound since it works)
            SoundPlayer::getInstance().PlaySFX(m_explosionSound, std::nullopt, true);

            float timeSurvived = m_gameClock ? m_gameClock->GetTotalElapsedSeconds() : 0.0f;
            int playerScore = m_player->getScore();

            DEBUG_PRINT("=== GAME OVER ===");
            DEBUG_PRINT("Time survived: " << timeSurvived << " seconds");
            DEBUG_PRINT("Player score: " << playerScore);
            DEBUG_PRINT("Wave reached: " << m_currentWave);
        }
    }

    // Update enemies
    for (auto &spawner : m_enemySpawners)
    {
        spawner->updateAll(dt, *m_player);
    }

    // Update camera
    if (m_camController)
    {
        m_camController->update(m_scene->m_activeCamera, m_player->m_playerData, dt);
    }

    // Update terrain chunks based on camera position
    if (m_chunkManager)
    {
        m_chunkManager->updateChunks(m_scene->m_activeCamera.m_Position);
    }
}

void WorldManager::render()
{
    if (!m_scene)
    {
        return;
    }

    // Enable 3D rendering state
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    // Render player
    if (m_player)
    {
        m_player->render(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config);
    }

    // Render enemy
    for (const auto &spawner : m_enemySpawners)
    {
        spawner->renderAll(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config);
    }

    // Render terrain chunks
    if (m_chunkManager)
    {
        for (const auto &chunk : m_chunkManager->m_chunks)
        {
            chunk->render(
                m_scene->m_activeCamera.getViewMatrix(),
                m_scene->m_activeCamera.getProjectionMatrix(),
                &m_scene->m_lightSource.config);
        }

        // Render global water
        m_chunkManager->renderWater(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config,
            m_scene->m_activeCamera.m_Position,
            m_renderDistance);

        // Render trees (instanced)
        m_chunkManager->renderTrees(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config);
    }

    // Render skybox and scene effects
    m_scene->renderScene();

    // Render screen flash overlay (if active)
    renderScreenFlash();
}

void WorldManager::setRenderDistance(float distance)
{
    m_renderDistance = distance;
    updateFogSettings();
}

void WorldManager::updateFogSettings()
{
    assert(m_chunkManager && "Chunk manager must be initialized before updating fog settings");

    float fogStart = m_renderDistance * m_fogStart;
    float fogEnd = m_renderDistance * m_fogEnd;

    // Update chunk manager fog settings
    m_chunkManager->setFogUniforms(m_fogColor, fogStart, fogEnd);

    // Update entity fog settings if they exist
    if (m_player)
    {
        m_player->m_playerRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);
    }

    for (auto &spawner : m_enemySpawners)
    {
        spawner->m_animatedInstanceRenderer->updateFogUniforms(m_fogColor, fogStart, fogEnd);
    }
}

void WorldManager::updateWaveSystem(float dt)
{
    m_waveTimer += dt;

    if (m_waveTimer >= m_waveDuration)
    {
        m_waveTimer = 0.0f;
        advanceWave();
    }
}

void WorldManager::advanceWave()
{
    m_currentWave++;

    DEBUG_PRINT("=== WAVE " << m_currentWave << " STARTED ===");
    DEBUG_PRINT("Game time: " << m_gameClock->GetTimeOfDayHours() << " hours");

#ifdef DEBUG
    assert(!m_enemySpawners.empty() && "No enemy spawners initialized!");
#endif

    size_t num_spawners = m_enemySpawners.size();

    for (size_t i = 0; i < num_spawners; i++)
    {
        EnemySpawner &spawner = *m_enemySpawners[i];
        // The first rounds we spawn only a single enemy type. After that, all spawners are active.
        if ((i + 1) == m_currentWave || m_currentWave > num_spawners)
        {
            DEBUG_PRINT("Activating spawner " << i << " for wave " << m_currentWave);
            spawner.activate();
            spawner.upgrade();
            DEBUG_PRINT("Spawner " << i << " max enemies increased to: " << spawner.m_spawnerConfig.m_maxEnemies);
        }        
    }
}

void WorldManager::triggerScreenFlash()
{
    m_screenFlashTimer = m_screenFlashDuration;

    // Play explosion sound
    SoundPlayer::getInstance().PlaySFX(m_explosionSound, std::nullopt, true);
}

void WorldManager::initializeFlashEffect()
{
    if (m_flashInitialized)
        return;

    // Load the flash shader
    m_flashShader = std::make_unique<Shader>();
    m_flashShader->addShader("ScreenFlash.vert", ShaderType::VERTEX);
    m_flashShader->addShader("ScreenFlash.frag", ShaderType::FRAGMENT);
    m_flashShader->createProgram();

    // Create fullscreen quad vertices (NDC coordinates)
    float quadVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f};

    glGenVertexArrays(1, &m_flashVAO);
    glGenBuffers(1, &m_flashVBO);

    glBindVertexArray(m_flashVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_flashVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindVertexArray(0);

    m_flashInitialized = true;
}

void WorldManager::renderScreenFlash()
{
    if (m_screenFlashTimer <= 0.0f)
        return;

    // Initialize on first use
    if (!m_flashInitialized)
        initializeFlashEffect();

    // Calculate alpha based on remaining time (fade out) - starts at full brightness
    float alpha = m_screenFlashTimer / m_screenFlashDuration;

    // Use additive blending for extra bright flashbang effect
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for BRIGHT flash

    m_flashShader->bind();
    m_flashShader->setUniform("u_flashColor",
                              glm::vec4(m_screenFlashColor.r, m_screenFlashColor.g, m_screenFlashColor.b, alpha));

    glBindVertexArray(m_flashVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    m_flashShader->unbind();

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}