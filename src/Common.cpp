#include "Common.h"

GLsizei window_X = 640;
GLsizei window_Y = 480;
GLFWwindow *window = nullptr;
GLFWmonitor *monitor = nullptr;

int init(const std::string &windowname){
	    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // monitor = glfwGetPrimaryMonitor(); // for fullscreen maybe?

    window = glfwCreateWindow(window_X, window_Y, windowname.c_str(), monitor, NULL);

    if (!window)
    {
        const char **e_msg = nullptr;
        glfwGetError(e_msg);
        std::cerr << "Failed to create window: " << e_msg << std::endl;

        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, [](GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
                       {
            std::cout << "key: " << key << ", action: " << action  << ", mods:" << mods << std::endl;
            if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(wdw, GLFW_TRUE); });

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);    

    // ####### NO GL FUNCTION CALLS UNTIL GLAD HAS LOADED THE FUNCTIONS #######
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }    

    GLCALL(glClearColor(0.2f, 0.1f, 0.2f, 1.0f)); // dark purple background color

    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    // set window-resize-callback (resize viewport)
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * /*window*/, int width, int height)
                                   { glViewport(0, 0, width, height); });

	return 0;
}

int exit() {
	// Cleanup (nothing to do yet)
	
	return 0;
}