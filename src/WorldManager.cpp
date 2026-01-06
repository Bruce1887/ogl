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

    // Setup Cow
    EnemyData cowEnemyData; // default enemy data
    std::unique_ptr<EnemySpawner> cowSpawner = std::make_unique<EnemySpawner>(cowEnemyData, 2000, 1);
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

    // Setup Mange
    EnemyData mangeEnemyData; // default enemy data
    mangeEnemyData.m_maxHealth = 250.0f;
    mangeEnemyData.m_health = 250.0f;
    mangeEnemyData.m_attackDamage = 25.0f;
    mangeEnemyData.m_attackCooldown = 6.0f;
    mangeEnemyData.m_moveSpeed = 4.0f;

    std::unique_ptr<EnemySpawner> mangeSpawner = std::make_unique<EnemySpawner>(mangeEnemyData, 100, 5);
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

    // Setup Abbe

    EnemyData abbeEnemyData; // default enemy data
    abbeEnemyData.m_maxHealth = 150.0f;
    abbeEnemyData.m_health = 150.0f;
    abbeEnemyData.m_attackDamage = 15.0f;
    abbeEnemyData.m_attackCooldown = 4.0f;
    abbeEnemyData.m_moveSpeed = 8.0f;
    std::unique_ptr<EnemySpawner> abbeSpawner = std::make_unique<EnemySpawner>(abbeEnemyData, 150, 3);
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

    return true;
}

void WorldManager::update(float dt, InputManager *input)
{
    if (!m_scene || !m_player)
    {
        return;
    }

    std::vector<EnemyData *> allEnemies;
    for (auto &spawner : m_enemySpawners)
    {
        for (auto &enemy : spawner->m_enemyDataList)
        {
            allEnemies.push_back(&enemy);
        }
    }

    if (input->keyboardInput.getKeyState(OOGABOOGA_ATTACK_KEY).readAndClear())
    {
        int hits = m_player->attack(allEnemies);
        DEBUG_PRINT("Hit " << hits << " enemies!");
    }

    // Update player
    if (m_player && m_chunkManager)
    {
        m_player->update(dt, input, m_chunkManager.get());
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