#ifndef H_OPENGLWINDOW
#define H_OPENGLWINDOW

// gl3w is not requiered in this file but it should always be included before GLFW
// and other files may include gl3w after gl_window.h
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

    // ---- State Storage ---- //

    int gl_version_major = 3;
    int gl_version_minor = 3;
    bool vertical_synchronization = true;
	GLFWwindow* window = nullptr;
};

// Returns the coordinates of the mouse cursor in OpenGL coordinates
void cursor_coordinates(const OpenGLWindow& window, float& coord_x, float& coord_y);

#endif
