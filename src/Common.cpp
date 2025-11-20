#include "Common.h"

RenderingContext *rContext = nullptr;
GLsizei window_X = 640;
GLsizei window_Y = 480;
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
        if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
            glfwSetWindowShouldClose(wdw, GLFW_TRUE);

        g_InputManager->movementInput.updateMovement(key, action, mods); });

    // set cursor position callback (for mouse movement input)
    glfwSetCursorPosCallback(g_window, [](GLFWwindow * /*window*/, double xpos, double ypos)
                             {
        // double inverted_y = -ypos + static_cast<double>(window_Y); // Invert Y axis to match OpenGL coordinates
        g_InputManager->mouseMoveInput.updateDeltas(xpos, ypos); });

    // set scroll callback (for mouse scroll input)
    glfwSetScrollCallback(g_window, [](GLFWwindow * /*window*/, double xoffset, double yoffset)
                          {
        g_InputManager->scrollInput.updateScroll(xoffset, yoffset); });
}

int checkTextureUnits()
{
    GLint maxTextureUnits;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits); // Fragment shader
    if (maxTextureUnits < NUM_TEXTURE_UNITS)
    {
        std::cout << "Warning: You have " << maxTextureUnits << " texture image units (fragment shader). You need " << NUM_TEXTURE_UNITS << std::endl;
        return -1;
    }

    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxTextureUnits); // Vertex shader
    if (maxTextureUnits < NUM_TEXTURE_UNITS)
    {
        std::cout << "Warning: You have " << maxTextureUnits << " texture image units (vertex shader). You need " << NUM_TEXTURE_UNITS << std::endl;
        return -1;
    }
    return 0;
}

int oogaboogaInit(const std::string &windowname)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
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
        std::cerr << "Failed to create window: " << e_msg << std::endl;

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
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Print OpenGL version
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    // GLCALL(glClearColor(0.2f, 0.1f, 0.2f, 1.0f)); // dark purple background color
    GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // black background color

    // Enable blending and set the blend function
    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable multisampling for anti-aliasing (maybe redundant but cant hurt)
    glEnable(GL_MULTISAMPLE);

    if (checkTextureUnits() != 0)
        return -1;
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

    return 0;
}