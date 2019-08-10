#ifndef H_DIFFUSION
#define H_DIFFUSION

#include "GL.h"

struct Diffusion{

    // Diffusion::Diffusion uses OpenGL functions ie GL3W needs to be
    // initialized (cf. OpenGLWindow / initialize_GL3W) before creating a
    // Diffusion object
    Diffusion();
    ~Diffusion();

    void start(const int simulation_width, const int simulation_height);
    void reset();

    enum InteractionType {SINK = 0, CLEAR = 1, SOURCE = 2};
    // center_x, center_y and radius must be given in OpenGL screen coordinates ie [-1;1]
    void interaction(const float center_x, const float center_y,
            const float radius, InteractionType interaction_value_id);

    void step(const float R);

    ogl::Shader vertex_full_screen_quad;
    ogl::Shader vertex_quad;
    ogl::Shader fragment_set_value;
    ogl::Shader fragment_interaction;
    ogl::Shader fragment_step;

    ogl::Program program_reset;
    ogl::Program program_source_sink;
    ogl::Program program_step;

    int width = 0;
    int height = 0;

    float min_initial_max[3] = {0.f, 0.5f, 1.f};

    bool current_state = false;
    ogl::Framebuffer framebuffer_state[2];
    ogl::Texture texture_state[2];

    ogl::Framebuffer framebuffer_source_sink;
    ogl::Texture texture_source_sink;

    ogl::VertexArray vertex_array_empty;
};

#endif
