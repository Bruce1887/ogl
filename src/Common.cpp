#include "Common.h"

GLsizei window_X = 640;
GLsizei window_Y = 480;
GLFWwindow *window = nullptr;
GLFWmonitor *monitor = nullptr;

void setupDefaultGLFWCallbacks()
{
    // set window-resize-callback (resize viewport)
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * /*window*/, int width, int height)
                                   { glViewport(0, 0, width, height); });
    glfwSetKeyCallback(window, [](GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
                       {
            // std::cout << "key: " << key << ", action: " << action  << ", mods:" << mods << std::endl;
            if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(wdw, GLFW_TRUE); });
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

    // monitor = glfwGetPrimaryMonitor(); // for fullscreen maybe?

    window = glfwCreateWindow(window_X, window_Y, windowname.c_str(), monitor, NULL);

    if (!window)
    {
        const char **e_msg = nullptr;
        glfwGetError(e_msg);
        std::cerr << "Failed to create window: " << e_msg << std::endl;

        return -1;
    }

    setupDefaultGLFWCallbacks();

    glfwMakeContextCurrent(window);

    // Enable V-Sync.
    glfwSwapInterval(1);

    // ####### NO GL FUNCTION CALLS UNTIL GLAD HAS LOADED THE FUNCTIONS #######
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        // glfwTerminate();
        return -1;
    }

    // Print OpenGL version
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    GLCALL(glClearColor(0.2f, 0.1f, 0.2f, 1.0f)); // dark purple background color

    // Enable blending and set the blend function
    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return 0;
}

int oogaboogaExit()
{
    // Cleanup

    glfwTerminate();

    return 0;
}