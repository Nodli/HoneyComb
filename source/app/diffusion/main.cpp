#include "GL.h"
#include "OpenGLWindow.h"

#include "Diffusion.h"
#include "TextureRenderer.h"

//#include "print.h"

int main(){

    // ---------- PARAMETERS ---------- //

    constexpr int sim_width = 100;
    constexpr int sim_height = 100;
    constexpr float sim_maxR = 0.35; // < 0.5 to have a numerically stable simulation
    constexpr float sim_minT = 0.f;
    constexpr float sim_maxT = 1.f;

    constexpr int render_width = 500;
    constexpr int render_height = 500;

    constexpr int steps_per_render = 5;

    constexpr float interaction_radius = 0.1f;

    // ---------- END PARAMETERS ---------- //

    OpenGLWindow window;
    window.open(render_width, render_height, "Diffusion", false);
    window.vertical_synchronization = true;

    Diffusion simulation;
    simulation.min_initial_max[0] = sim_minT;
    simulation.min_initial_max[1] = (sim_maxT + sim_minT) / 2.f;
    simulation.min_initial_max[2] = sim_maxT;
    simulation.start(sim_width, sim_height);

    TextureRenderer renderer(TextureRenderer::R);

    // Dynamic Parameter
    Diffusion::InteractionType interaction_type = Diffusion::CLEAR;

    while(!glfwWindowShouldClose(window.window)){
        glfwPollEvents();

		if(glfwGetKey(window.window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(window.window, true);
		}

        // -------- Start of Frame -------- //

        // ---- Interactions ---- //

        if(glfwGetKey(window.window, GLFW_KEY_1)){
            interaction_type = Diffusion::SINK;
        }
        if(glfwGetKey(window.window, GLFW_KEY_2)){
            interaction_type = Diffusion::CLEAR;
        }
        if(glfwGetKey(window.window, GLFW_KEY_3)){
            interaction_type = Diffusion::SOURCE;
        }

        bool show_source_sink_texture = false;
        if(glfwGetKey(window.window, GLFW_KEY_TAB)){
            show_source_sink_texture = true;
        }

        if(glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_LEFT)){
            float pos_x, pos_y;
            window.cursor_screen_coordinates(pos_x, pos_y);

            simulation.interaction(pos_x, pos_y, interaction_radius, interaction_type);
        }

        bool step_this_frame = false;
        if(glfwGetKey(window.window, GLFW_KEY_SPACE)){
            step_this_frame = true;
        }

        bool reset_this_frame = false;
        if(glfwGetKey(window.window, GLFW_KEY_R)){
            reset_this_frame = true;
        }

        // ---- Running Simulation ---- //

        if(reset_this_frame){
            simulation.reset();
        }

        if(step_this_frame){
            for(int istep = 0; istep != steps_per_render; ++istep){
                simulation.step(sim_maxR);
            }
        }

        // ---- Render Simulation ---- //

        if(show_source_sink_texture){
            renderer.render(window, simulation.texture_source_sink);
        }else{
            renderer.render(window, simulation.texture_state[(int)simulation.current_state]);
        }

        // -------- End of Frame -------- //

        window.synchronize();
    }

    window.close();
}
