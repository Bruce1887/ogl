#include "Common.h"
#include "game/Audio.h"

RenderingContext *rContext = nullptr;
GLsizei window_X = 1024;
GLsizei window_Y = 768;
GLFWwindow *g_window = nullptr;
GLFWmonitor *g_monitor = nullptr;
InputManager *g_InputManager = nullptr;

void setupDefaultGLFWCallbacks()
{
    // we need to have set up these first
    assert(g_window != nullptr);
    assert(g_InputManager != nullptr);
    assert(rContext != nullptr);
    // assert (g_monitor != nullptr); // g_monitor may be null if not fullscreen

    // set window-resize-callback (resize viewport)
    glfwSetFramebufferSizeCallback(g_window, [](GLFWwindow * /*window*/, int width, int height)
                                   { glViewport(0, 0, width, height); });

    // set key-callback (for movement input)
    glfwSetKeyCallback(g_window, [](GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
                       {         
        // Close window on Ctrl+W
        if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
            glfwSetWindowShouldClose(wdw, GLFW_TRUE);
        
        // Update movement input
        const InputUpdate movementUpdate = KeyboardUpdate{ key, action, mods };
        g_InputManager->keyboardInput.movementInput.update(movementUpdate); 

        // Update other key states
        for(KeyState &ks : g_InputManager->keyboardInput.keyStates){
            if(ks.key() == key && action == GLFW_RELEASE){
                ks.update(true);
            }
        } });

    // set cursor position callback (for mouse movement input)
    glfwSetCursorPosCallback(g_window, [](GLFWwindow * /*window*/, double xpos, double ypos)
                             {
                                const InputUpdate mousePosUpdate = MousePosUpdate{ xpos, ypos };
                                g_InputManager->mouseMoveInput.update(mousePosUpdate); });

    // set scroll callback (for mouse scroll input)
    glfwSetScrollCallback(g_window, [](GLFWwindow * /*window*/, double xoffset, double yoffset)
                          { 
        const InputUpdate scrollUpdate = ScrollUpdate{ xoffset, yoffset }; 
        g_InputManager->scrollInput.update(scrollUpdate); });

    // set mouse button callback (for attack input)
    glfwSetMouseButtonCallback(g_window, [](GLFWwindow * /*window*/, int button, int action, int /*mods*/)
                               {
        const InputUpdate buttonUpdate = ButtonUpdate{ button, action };
        g_InputManager->mouseButtonInput.update(buttonUpdate);
        /*
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            g_InputManager->attackInput.triggerAttack(); 
        }
        */ });
}

int checkTextureUnits()
{
    GLint maxTextureUnits;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits); // Fragment shader
    if (maxTextureUnits < REQUIRED_NUM_TEXTURE_UNITS)
    {
        DEBUG_PRINT("Warning: Fragment shader supports " << maxTextureUnits << " texture units. Required " << REQUIRED_NUM_TEXTURE_UNITS << ". Proceeding with " << maxTextureUnits << ".");
        // Allow continuation; engine will just be limited to this number.
        // TODO: Propagate actual limit instead of using NUM_TEXTURE_UNITS constant.
    }

    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxTextureUnits); // Vertex shader
    if (maxTextureUnits < REQUIRED_NUM_TEXTURE_UNITS)
    {
        DEBUG_PRINT("Warning: Vertex shader supports " << maxTextureUnits << " texture units. Required " << REQUIRED_NUM_TEXTURE_UNITS << ". Proceeding with " << maxTextureUnits << ".");
    }
    return 0;
}

int oogaboogaInit(const std::string &windowname)
{
    if (!glfwInit())
    {
        DEBUG_PRINT("Failed to initialize GLFW");
        return -1;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA (multi-sample anti-aliasing)

    // monitor = glfwGetPrimaryMonitor(); // for fullscreen maybe?

    g_window = glfwCreateWindow(window_X, window_Y, windowname.c_str(), g_monitor, NULL);

    if (!g_window)
    {
        const char **e_msg = nullptr;
        glfwGetError(e_msg);
        DEBUG_PRINT("Failed to create window: " << e_msg);

        return -1;
    }
    glfwMakeContextCurrent(g_window);

    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    rContext = new RenderingContext();
    rContext->makeCurrent();

    g_InputManager = new InputManager();
    setupDefaultGLFWCallbacks();

    // Enable V-Sync.
    glfwSwapInterval(1);

    // ####### NO GL FUNCTION CALLS UNTIL GLAD HAS LOADED THE FUNCTIONS #######
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        DEBUG_PRINT("Failed to initialize GLAD");
        return -1;
    }

    // Print OpenGL version
    DEBUG_PRINT("OpenGL " << glGetString(GL_VERSION));

    // GLCALL(glClearColor(0.2f, 0.1f, 0.2f, 1.0f)); // dark purple background color
    GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // black background color
    // glClearColor(0.5f, 0.7f, 0.9f, 1.0f);

    // Enable blending and set the blend function
    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable multisampling for anti-aliasing (maybe redundant but cant hurt)
    glEnable(GL_MULTISAMPLE);

    if (checkTextureUnits() != 0)
        return -1;

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // Cull back faces


    // Initialize audio system
    if (!Audio_Init())
    {
        DEBUG_PRINT("Failed to initialize audio system");
        return -1;
    }

    return 0;
}

int oogaboogaExit()
{
    glfwDestroyWindow(g_window);
    glfwTerminate();

    if (rContext)
    {
        delete rContext;
        rContext = nullptr;
    }

    if (g_InputManager)
    {
        delete g_InputManager;
        g_InputManager = nullptr;
    }

    if(s_audio_context || s_audio_device)
        Audio_Shutdown();

    return 0;
}
