#include "GL.h"
#include "OpenGLWindow.h"

#include "Particles.h"

//#include "print.h"

int main(){

    constexpr int width = 1080;
    constexpr int height = 1080;

    OpenGLWindow window;
    window.gl_version_major = 4;
    window.gl_version_minor = 3;
    window.open(width, height, "Particles", false);
    window.vertical_synchronization = true;

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Particles simulation;
    simulation.start();
    simulation.step(); // update the velocities

    while(!glfwWindowShouldClose(window.window)){

        glfwPollEvents();

		if(glfwGetKey(window.window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(window.window, true);
		}

        // -------- Start of Frame -------- //

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- Interactions ---- //

        bool run_step = false;
        if(glfwGetKey(window.window, GLFW_KEY_SPACE)){
            run_step = true;
        }

        // ---- Simulate ---- //

        if(run_step){
            simulation.step();
        }

        // ---- Render ---- //

        simulation.render();

        // -------- End of Frame -------- //

        window.synchronize();
    }

    window.close();
}
