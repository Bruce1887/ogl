#pragma once

#include <cstdint>
#include <memory>
#include <atomic>

#include "../Frametimer.h"   // frametimer utility already in project
#include "GameClock.h"       // game clock
#include "Entity.h"          // base entity type (forward usage)
 

// Forward declarations for subsystems (actual definitions live elsewhere)
class World;
class Server;   // server-side network handler
class Client;   // client-side network handler
class Scene;    // renderer/scene bridge

// Mode the engine is running in
enum class RunMode : uint8_t
{
    SERVER,
    CLIENT,
    LOCAL    // single-player / combined server+client
};

// GameLoop: engine heart. Owns/coordinates World, Clock, Network and the main loop.
class GameLoop
{
public:
    GameLoop(RunMode mode = RunMode::LOCAL);
    ~GameLoop();

    // Initialize subsystems (window, GL, world, network, resources).
    // Returns true on success.
    bool Initialize();

    // Main loop. Blocks until shutdown requested.
    void Run();

    // Request a graceful shutdown from any thread.
    void Shutdown() { m_running.store(false); }

    // Server-side fixed tick update. Advances authoritative simulation.
    // Called at server tick rate (e.g. 20..60 Hz).
    void ServerTick(float deltaTime);

    // Client-side render/update. Handles input, interpolation and rendering.
    // Called every frame on the client.
    void ClientRender(float deltaTime);

    // Accessors
    RunMode GetMode() const { return m_mode; }

private:
    // Internal helpers
    bool initializeWindowAndGL();            // platform / GL init (client)
    bool initializeNetwork();                // start server or client networking
    void mainLoopLocal();                    // single-process loop (local)
    void mainLoopClient();                   // client-only loop
    void mainLoopServer();                   // server-only loop

private:
    RunMode m_mode;

    // Core subsystems (owned here or injected)
    std::unique_ptr<World> m_world;          // game world (entities, chunks)
    std::unique_ptr<GameClock> m_clock;      // authoritative clock (server-owned)
    std::unique_ptr<Server> m_server;        // present when running as server
    std::unique_ptr<Client> m_client;        // present when running as client
    std::unique_ptr<Scene> m_scene;          // rendering bridge (client)

    // Frame timing
    FrameTimer m_framer;                      // utility that computes dt / tick timing

    // Main loop control
    std::atomic<bool> m_running{false};      // set to false to stop Run()
    // Target server tick rate (Hz) — server tick loop will use this
    const float m_serverTickRate = 30.0f;

    // Misc flags / config could be added here (window handle, config path, etc.)
};