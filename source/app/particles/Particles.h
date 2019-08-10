#ifndef H_PARTICLES
#define H_PARTICLES

#include "GL.h"
#include "Timer.h"

struct Particles{
    Particles();
    ~Particles();

    struct ParticleData{
        float position_x;
        float position_y;
        float velocity_x;
        float velocity_y;
    };

    void start();
    void step();
    void render();

    ogl::Buffer position;
    ogl::VertexArray position_format;

    ogl::Shader compute_step_shader;
    ogl::Program compute_step_program;

    ogl::Shader render_vertex;
    ogl::Shader render_geometry;
    ogl::Shader render_fragment;
    ogl::Program render_program;

    Timer timer;
    float previous_step_time;

    const int particles_per_group = 64; // local_size_x in compute shaders
    const int number_of_groups = 1;
    const int number_of_particles = number_of_groups * particles_per_group;
};

#endif
