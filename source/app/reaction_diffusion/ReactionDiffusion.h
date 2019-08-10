#ifndef H_REACTION_DIFFUSION
#define H_REACTION_DIFFUSION

#include "GL.h"

struct ReactionDiffusion{

    // ReactionDiffusion::ReactionDiffusion uses OpenGL functions ie GL3W needs to be
    // initialized (cf. OpenGLWindow / initialize_GL3W) before creating a
    // ReactionDiffusion object
    ReactionDiffusion();
    ~ReactionDiffusion();

    void start(const int simulation_width, const int simulation_height);
    void reset();

    // The variables center_x, center_y and radius must be given in OpenGL
    // screen coordinates ie within [-1;1]
    void point(const float center_x, const float center_y, const float radius,
            const float value_u, const float value_v);
    void square(const float center_x, const float center_y, const float size,
            const float value_u, const float value_v);

    void step();

    ogl::Shader vertex_full_screen_quad;
    ogl::Shader vertex_quad;
    ogl::Shader fragment_set_value;
    ogl::Shader fragment_disc_set_value;
    ogl::Shader fragment_step;

    ogl::Program program_reset;
    ogl::Program program_point;
    ogl::Program program_square;
    ogl::Program program_step;

    int width = 0;
    int height = 0;

    float default_value_u = 0.4201f;
    float default_value_v = 0.2878f;

    float sim_Ru = 0.125;
    float sim_Rv = 0.0625; // sim_Ru / 2.f
    float sim_F = 0.0380;
    float sim_R = 0.0610;

    bool current_state = false;
    ogl::Framebuffer framebuffer_state[2];
    ogl::Texture texture_state[2];

    ogl::VertexArray vertex_array_empty;
};

#endif
