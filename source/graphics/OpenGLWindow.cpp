#include "OpenGLWindow.h"
#include "GL.h"
#include "print.h"

OpenGLWindow::OpenGLWindow(){
	if(!glfwInit()){
		print("GLFW error : initialization failed");
		glfwTerminate();
	}
}

OpenGLWindow::~OpenGLWindow(){
	// https://www.glfw.org/docs/latest/window.html
	// All windows remaining when glfwTerminate is called are destroyed as well.
	glfwTerminate();
}

void OpenGLWindow::open(int width, int height, const char* name,
        bool full_screen){

    if(window){
        print("OpenGLWindow error : A window is already open");

    }else{

        // requesting specific window caracteristics
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        if(!vertical_synchronization){
            glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
        }else{
            glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        }

        GLFWmonitor* monitor = nullptr;
        if(full_screen){
            monitor = glfwGetPrimaryMonitor();
        }

        window = glfwCreateWindow(width, height, name, monitor, nullptr);

        if(!window){
            print("GLFW error : window creation failed");

        }else{
            glfwMakeContextCurrent(window);
            ogl::initialize_GL3W(gl_version_major, gl_version_minor);
            // ogl::initialize_debug_callback();
        }
    }
}

void OpenGLWindow::close(){
	if(window){
		glfwDestroyWindow(window);
		window = nullptr;
	}
}

void OpenGLWindow::synchronize(){
    if(vertical_synchronization){
        glfwSwapBuffers(window);

    }else{
        glFlush();

    }
}

void cursor_coordinates(const OpenGLWindow& window, float& coord_x, float& coord_y){

    // Coordinates in pixels inside the window with origin at top left
    double mouse_x;
    double mouse_y;
    glfwGetCursorPos(window.window, &mouse_x, &mouse_y);

    int window_width;
    int window_height;
    glfwGetWindowSize(window.window, &window_width, &window_height);

    coord_x = (mouse_x / window_width) * 2.f - 1.f;
    coord_y = (1.f - mouse_y / window_height) * 2.f - 1.f;
}
