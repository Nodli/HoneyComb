#include "GL.h"
#include "OpenGLWindow.h"

#include "ReactionDiffusion.h"
#include "TextureRenderer.h"

#include "utils.h"
//#include "print.h"

// Reference for reaction-diffusion and the simulation parameters
// https://mrob.com/pub/comp/xmorphia/

int main(){

    // ---------- PARAMETERS ---------- //

    constexpr int sim_width = 500;
    constexpr int sim_height = 500;

    constexpr float sim_F = 0.0260;
    constexpr float sim_R = 0.0530;

    constexpr int render_width = 500;
    constexpr int render_height = 500;

    constexpr float interaction_radius = 0.1f;
    constexpr float interaction_u_value_uv[2] = {1.f, 0.f};
    constexpr float interaction_v_value_uv[2] = {0.f, 1.f};

    constexpr int steps_per_render = 5;

    // ---------- END PARAMETERS ---------- //

    OpenGLWindow window;
    window.open(render_width, render_height, "Diffusion", false);
    window.vertical_synchronization = true;

    ReactionDiffusion simulation;
    simulation.sim_F = sim_F;
    simulation.sim_R = sim_R;
    simulation.start(sim_width, sim_height);

    TextureRenderer renderer(TextureRenderer::RG);

    // Dynamic Parameters
    float interaction_value_uv[2] = {interaction_v_value_uv[0], interaction_v_value_uv[1]};

    while(!glfwWindowShouldClose(window.window)){
        glfwPollEvents();

		if(glfwGetKey(window.window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(window.window, true);
		}

        // -------- Start of Frame -------- //

        // ---- Interactions ---- //

        if(glfwGetKey(window.window, GLFW_KEY_1)){
            interaction_value_uv[0] = interaction_u_value_uv[0];
            interaction_value_uv[1] = interaction_u_value_uv[1];
        }
        if(glfwGetKey(window.window, GLFW_KEY_2)){
            interaction_value_uv[0] = interaction_v_value_uv[0];
            interaction_value_uv[1] = interaction_v_value_uv[1];
        }
        if(glfwGetKey(window.window, GLFW_KEY_3)){
            interaction_value_uv[0] = rand_normalized();
            interaction_value_uv[1] = rand_normalized();
        }

        if(glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_LEFT)){
            float pos_x, pos_y;
            window.cursor_screen_coordinates(pos_x, pos_y);

            simulation.point(pos_x, pos_y, interaction_radius,
                    interaction_value_uv[0], interaction_value_uv[1]);
        }

        if(glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_RIGHT)){
            float pos_x, pos_y;
            window.cursor_screen_coordinates(pos_x, pos_y);

            simulation.square(pos_x, pos_y, interaction_radius,
                    interaction_value_uv[0], interaction_value_uv[1]);
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
                simulation.step();
            }
        }

        // ---- Render Simulation ---- //

        renderer.render(window, simulation.texture_state[(int)simulation.current_state]);

        // -------- End of Frame -------- //

        window.synchronize();
    }

    window.close();
}
