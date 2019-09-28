#ifndef H_OPENGLWINDOW
#define H_OPENGLWINDOW

// gl3w is not requiered in this file but it should always be included before GLFW
// and other files may include gl3w after OpenGLWindow.h
#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

struct OpenGLWindow{

	OpenGLWindow();
	~OpenGLWindow();

	void open(int width, int height, const char* name,
            bool full_screen = false);
	void close();

    void synchronize();
    void cursor_screen_coordinates(float& coord_x, float& coord_y);

    // ---- State Storage ---- //

    int gl_version_major = 3;
    int gl_version_minor = 3;
    bool vertical_synchronization = true;
	GLFWwindow* window = nullptr;
};

#endif
