#include "Particles.h"

#include "utils.h"

Particles::Particles(){

    const char* const code_compute_step = R"(
    #version 450

    layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

    layout(std140, binding = 0) buffer ParticleBuffer{
        vec4 Position2Velocity2[];
    };

    layout(location = 0) uniform uint total_particles;
    layout(location = 1) uniform float delta_time;

    void simulate(inout vec4 particle){

        const float max_speed = 0.25;
        const float repulsion_radius = 0.01;

        vec2 barycenter = vec2(0., 0.);
        vec2 barycentric_velocity = vec2(0., 0.);
        vec2 neighbor_repulsion = vec2(0., 0.);

        for(int ipart = 1; ipart != 64; ++ipart){
            vec4 neighbor = Position2Velocity2[(gl_GlobalInvocationID.x + ipart) % 64];

            barycenter += neighbor.xy;
            barycentric_velocity += neighbor.zw;

            if(min(distance(neighbor.xy, particle.xy), 1.) < repulsion_radius){
                neighbor_repulsion -= neighbor.xy - particle.xy;
            }
        }

        barycenter /= 63;
        barycentric_velocity /= 63;

        vec2 toward_barycenter = (barycenter - particle.xy) / 100.;
        vec2 toward_barycentric_velocity = (barycentric_velocity - particle.zw) / 100.;

        particle.wz += toward_barycenter;
        particle.wz += toward_barycentric_velocity;
        particle.wz += neighbor_repulsion;

        particle.wz = particle.wz / length(particle.wz) * min(length(particle.wz), max_speed);
    }

    void step(inout vec4 particle){
        particle.xy = particle.xy + particle.zw * delta_time;

        particle.x += (float(particle.x < -1.) - float(particle.x > 1.)) * 2.;
        particle.y += (float(particle.y < -1.) - float(particle.y > 1.)) * 2.;
    }

    void main(){
        simulate(Position2Velocity2[gl_GlobalInvocationID.x].xyzw);
        step(Position2Velocity2[gl_GlobalInvocationID.x].xyzw);
    }

    )";

    const char* const code_vertex = R"(
    #version 450

    out vec2 particle_velocity;

    layout(std140, binding = 0) buffer ParticleBuffer{
        vec4 Position2Velocity2[];
    };

    void main(){

        gl_Position = vec4(Position2Velocity2[gl_VertexID].xy, 0.0, 1.0);
        particle_velocity = Position2Velocity2[gl_VertexID].zw;
    }
    )";

    const char* const code_geometry = R"(
    #version 450

    #define M_PI 3.1415926535897932384626433832795

    in vec2 particle_velocity[];
    out float fading;

    layout(points) in;
    layout(triangle_strip, max_vertices = 12) out;

    layout(location = 0) uniform float time;

    vec2 rotate(in vec2 axis, in float angle){
        return mat2(cos(angle), sin(angle), - sin(angle), cos(angle)) * axis;
    }

    void main(){

        const float body_width = 0.0125;
        const float body_height = 0.05125;
        const float body_height_factor = 0.125;
        const float body_height_vary_factor = 0.005;

        const float wing_height = 0.01;

        const float unique_hash = gl_PrimitiveIDIn / 64. * M_PI;

        // orthogonal to velocity and towards its right
        vec2 orthogonal = normalize(cross(vec3(0., 0., -1.0), vec3(particle_velocity[0].xy, 0.)).xy);
        vec4 pos = gl_in[0].gl_Position;

        // ---- Body ---- //

        vec4 pos_shoulder_left = vec4(pos.xy
                                        - orthogonal * body_width,
                                        0., 1.);
        vec4 pos_shoulder_right = vec4(pos.xy
                                        + orthogonal * body_width,
                                        0., 1.);
        vec4 pos_tail = vec4(pos.xy
                            - body_height * normalize(particle_velocity[0].xy)
                            - body_height_factor * particle_velocity[0].xy
                            - body_height_vary_factor * (sin(2. * time + unique_hash * M_PI) + 1.0) * normalize(particle_velocity[0].xy),
                            0., 1.);

        gl_Position = pos_shoulder_left;
        fading = 0.;
        EmitVertex();

        gl_Position = pos_tail;
        fading = 1.0;
        EmitVertex();

        gl_Position = pos_shoulder_right;
        fading = 0.;
        EmitVertex();
        EndPrimitive();

        // ---- Left Wing ---- //

        float left_wing_angle = (sin(2. * time + unique_hash * M_PI) + 1) / 2. * M_PI / 3.;
        vec2 direction_left_wing = rotate(orthogonal, left_wing_angle);
        vec4 pos_articulation_left_wing = vec4(pos_shoulder_left.xy
                                - 2. * body_width * direction_left_wing,
                                0., 1.);
        vec4 pos_tip_left_wing = vec4(pos_shoulder_left.xy
                                + wing_height * normalize(particle_velocity[0].xy)
                                - 2.5 * body_width * direction_left_wing,
                                0., 1.);

        gl_Position = pos_shoulder_left;
        fading = 1.;
        EmitVertex();

        gl_Position = pos_articulation_left_wing;
        fading = 0.;
        EmitVertex();

        gl_Position = pos_tip_left_wing;
        fading = 0.;
        EmitVertex();
        EndPrimitive();

        // ---- Right Wing ---- //

        float right_wing_angle = - left_wing_angle;
        vec2 direction_right_wing = rotate(orthogonal, right_wing_angle);
        vec4 pos_articulation_right_wing = vec4(pos_shoulder_right.xy
                                + 2. * body_width * direction_right_wing,
                                0., 1.);
        vec4 pos_tip_right_wing = vec4(pos_shoulder_right.xy
                                + wing_height * normalize(particle_velocity[0].xy)
                                + 2.5 * body_width * direction_right_wing,
                                0., 1.);

        gl_Position = pos_shoulder_right;
        fading = 1.;
        EmitVertex();

        gl_Position = pos_articulation_right_wing;
        fading = 0.;
        EmitVertex();

        gl_Position = pos_tip_right_wing;
        fading = 0.;
        EmitVertex();

    }
    )";

    const char* const code_fragment = R"(
    #version 450

    in float fading;
    out vec4 out_color;

    void main(){

        vec3 fragment_color = vec3(1.0, 0.0, 0.5) * (1. - fading);

        out_color = vec4(fragment_color, 0.5);
    }
    )";

    ogl::create_shader(compute_step_shader, GL_COMPUTE_SHADER, code_compute_step);
    ogl::create_shader(render_vertex, GL_VERTEX_SHADER, code_vertex);
    ogl::create_shader(render_geometry, GL_GEOMETRY_SHADER, code_geometry);
    ogl::create_shader(render_fragment, GL_FRAGMENT_SHADER, code_fragment);

    ogl::create_program(compute_step_program, compute_step_shader);
    ogl::create_program(render_program, render_vertex, render_geometry, render_fragment);
}

Particles::~Particles(){
}

void Particles::start(){
    timer.start();
    previous_step_time = timer.get<float, Timer::seconds>();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, position.handle);

    ParticleData* particle_positions = new ParticleData[number_of_particles];
    for(unsigned int ipos = 0; ipos != number_of_particles; ++ipos){
        particle_positions[ipos].position_x = rand_normalized() * 2.f - 1.f;
        particle_positions[ipos].position_y = rand_normalized() * 2.f - 1.f;
        particle_positions[ipos].velocity_x = rand_normalized() / 10.f + 0.001f;
        particle_positions[ipos].velocity_y = rand_normalized() / 10.f + 0.001f;
    }

    glBufferData(GL_SHADER_STORAGE_BUFFER, number_of_particles * sizeof(ParticleData), particle_positions, GL_STATIC_DRAW);

    delete[] particle_positions;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Particles::step(){

    float current_time = timer.get<float, Timer::seconds>();

    glUseProgram(compute_step_program.handle);

    constexpr unsigned int binding_particle_data = 0;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_particle_data, position.handle);

    constexpr unsigned int location_total_particles = 0;
    glUniform1ui(location_total_particles, number_of_particles);

    constexpr unsigned int location_delta_time = 1;
    glUniform1f(location_delta_time, current_time - previous_step_time);

    glDispatchCompute(number_of_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    previous_step_time = current_time;
}

void Particles::render(){

    glBindVertexArray(position_format.handle);
    glBindBuffer(GL_ARRAY_BUFFER, position.handle);

    constexpr unsigned int particle_position_component = 2;
    constexpr unsigned int particle_color_component = 2;
    constexpr unsigned int particle_data_component = particle_position_component + particle_color_component;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, particle_position_component, GL_FLOAT, GL_FALSE, particle_data_component * sizeof(float), NULL);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, particle_color_component, GL_FLOAT, GL_FALSE, particle_data_component * sizeof(float), NULL);

    glUseProgram(render_program.handle);

    constexpr unsigned int location_time = 0;
    //glUniform1f(location_time, timer.seconds());
    glUniform1f(location_time, previous_step_time);

    glDrawArrays(GL_POINTS, 0, number_of_particles);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
