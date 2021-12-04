#define NOMINMAX

#include "Diffusion.h"

#include <cmath>
#include "print.h"

#include <cstdlib>


Diffusion::Diffusion(){

    // ---- Shader code ---- //

    const char* const code_full_screen_quad = R"(
    #version 450

    out vec2 texture_coord;

    void main(){

        // gl_VertexID = 0, 1, 2, 3, 4, 5 -> square_vertex = 0, 1, 2, 3, 2, 1
        const int square_vertex = gl_VertexID - (gl_VertexID / 3) * (2 * gl_VertexID - 6);

        gl_Position = vec4(-1.0 + (square_vertex & 1) * 2.0,
                            -1.0 + ((square_vertex & 2) >> 1) * 2.0,
                            0.0,
                            1.0);
        texture_coord = vec2(0.0 + (square_vertex & 1),
                            0.0 + ((square_vertex & 2) >> 1));
    }
    )";

    const char* const code_quad = R"(
    #version 450

    out vec2 texture_coord;
    layout(location = 0) uniform vec4 min_max_coord; // vec4(min_x, min_y, max_x, max_y)

    void main(){

        // gl_VertexID = 0, 1, 2, 3, 4, 5 -> square_vertex = 0, 1, 2, 3, 2, 1

        const int square_vertex = gl_VertexID - (gl_VertexID / 3) * (2 * gl_VertexID - 6);

        const vec2 diff_min_max = vec2(min_max_coord.z - min_max_coord.x,
                                        min_max_coord.w - min_max_coord.y);

        gl_Position = vec4(min_max_coord.x + (square_vertex & 1) * diff_min_max.x,
                            min_max_coord.y + ((square_vertex & 2) >> 1) * diff_min_max.y,
                            0.0,
                            1.0);

        texture_coord = vec2(min_max_coord.x + (square_vertex & 1) * diff_min_max.x,
                            min_max_coord.y + ((square_vertex & 2) >> 1) * diff_min_max.y);
    }
    )";

    const char* const code_set_value = R"(
    #version 450

    out float out_value;
    layout(location = 1) uniform float value;

    void main(){
        out_value = value;
    }
    )";

    const char* const code_interaction_set_value = R"(
    #version 450

    in vec2 texture_coord;
    out float out_value;
    layout(location = 1) uniform vec2 position;
    layout(location = 2) uniform float radius;
    layout(location = 3) uniform float value;

    void main(){

        const float distance_x = (texture_coord.x - position.x);
        const float distance_y = (texture_coord.y - position.y);
        const float distance = distance_x * distance_x + distance_y * distance_y;

        if(distance > (radius * radius)){
            discard;

        }else{
            out_value = value;

        }
    }
    )";

    const char* const code_step_forward = R"(
    #version 450

    in vec2 texture_coord;
    out float out_value;
    layout(location = 0) uniform sampler2D previous_state;
    layout(location = 1) uniform sampler2D source_sink;
    layout(location = 2) uniform float R;

    float get_value(vec2 coord){

        float source_sink_value = texture(source_sink, coord).r;
        float previous_value = texture(previous_state, coord).r;

        return float(source_sink_value != 0.5) * source_sink_value + float(source_sink_value == 0.5) * previous_value;
    }

    float laplacian9(in float current_value){

        vec2 texel_size = vec2(1.0) / textureSize(previous_state, 0);

        return (1.0 / 6.0) *

                (4 * (get_value(vec2(texture_coord.x + texel_size.x, texture_coord.y)).r
                    + get_value(vec2(texture_coord.x - texel_size.x, texture_coord.y)).r
                    + get_value(vec2(texture_coord.x, texture_coord.y + texel_size.y)).r
                    + get_value(vec2(texture_coord.x, texture_coord.y - texel_size.y)).r)

                + get_value(vec2(texture_coord.x - texel_size.x, texture_coord.y - texel_size.y)).r
                + get_value(vec2(texture_coord.x + texel_size.x, texture_coord.y - texel_size.y)).r
                + get_value(vec2(texture_coord.x - texel_size.x, texture_coord.y + texel_size.y)).r
                + get_value(vec2(texture_coord.x + texel_size.x, texture_coord.y + texel_size.y)).r

                - 20 * current_value);
    }

    void main(){

        float current_value = get_value(texture_coord).r;
        out_value = current_value + R * laplacian9(current_value);
    }
    )";

    // ---- Shader Creation ---- //

    ogl::create_shader(vertex_full_screen_quad, GL_VERTEX_SHADER, code_full_screen_quad);
    ogl::create_shader(vertex_quad, GL_VERTEX_SHADER, code_quad);
    ogl::create_shader(fragment_set_value, GL_FRAGMENT_SHADER, code_set_value);
    ogl::create_shader(fragment_interaction, GL_FRAGMENT_SHADER, code_interaction_set_value);
    ogl::create_shader(fragment_step, GL_FRAGMENT_SHADER, code_step_forward);

    ogl::create_program(program_reset, vertex_full_screen_quad, fragment_set_value);
    ogl::create_program(program_source_sink, vertex_quad, fragment_interaction);
    ogl::create_program(program_step, vertex_full_screen_quad, fragment_step);

    // ---- Texture Parameters Setup ---- //

    glBindTexture(GL_TEXTURE_2D, texture_state[0].handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture_state[1].handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture_source_sink.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Diffusion::~Diffusion(){
}

void Diffusion::start(const int simulation_width, const int simulation_height){

    width = simulation_width;
    height = simulation_height;

    // ---- Initializing state textures and framebuffer ---- //

    // TODO (Nodli) : VLAs are not supported on windows, instead use malloc / new or glMapBufferRange to let the driver do the allocation
    float* initial_data = (float*)malloc(width * height * sizeof(float));
    for(int idata = 0; idata != width * height; ++idata){
        initial_data[idata] = min_initial_max[CLEAR];
    }

    glBindTexture(GL_TEXTURE_2D, texture_state[0].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, initial_data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[0].handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_state[0].handle, 0);

    // Value initialization not requiered
    glBindTexture(GL_TEXTURE_2D, texture_state[1].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[1].handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_state[1].handle, 0);

    // ---- Initializing source - sink texture and framebuffer ---- //

    glBindTexture(GL_TEXTURE_2D, texture_source_sink.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, initial_data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_source_sink.handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_source_sink.handle, 0);

    // ---- Clean bindings ---- //

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    free(initial_data);
}

void Diffusion::reset(){

    glViewport(0, 0, width, height);

    // ---- Reset diffusion values ---- //

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)current_state].handle);
    const GLenum state_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &state_attachment);

    glUseProgram(program_reset.handle);

    constexpr unsigned int location_value = 1;
    glUniform1f(location_value, min_initial_max[1]);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ---- Reset Source Sink values ---- //

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_source_sink.handle);
    const GLenum source_sink_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &source_sink_attachment);

    glUniform1f(location_value, min_initial_max[1]);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Diffusion::interaction(const float coord_x, const float coord_y,
        const float radius, InteractionType interaction_value_id){

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_source_sink.handle);
    const GLenum source_sink_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &source_sink_attachment);

    glUseProgram(program_source_sink.handle);

    constexpr unsigned int location_min_max_coord = 0;
    float min_x = std::max(coord_x - radius, -1.f);
    float max_x = std::min(coord_x + radius, 1.f);
    float min_y = std::max(coord_y - radius, -1.f);
    float max_y = std::min(coord_y + radius, 1.f);
    glUniform4f(location_min_max_coord, min_x, min_y, max_x, max_y);

    constexpr unsigned int location_position = 1;
    glUniform2f(location_position, coord_x, coord_y);

    constexpr unsigned int location_radius = 2;
    glUniform1f(location_radius, radius);

    constexpr unsigned int location_value = 3;
    glUniform1f(location_value, min_initial_max[interaction_value_id]);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Diffusion::step(const float R){
    glViewport(0, 0, width, height);

    glUseProgram(program_step.handle);

    constexpr unsigned int location_previous_state = 0;
    constexpr unsigned int unit_previous_state = 0;
    glActiveTexture(GL_TEXTURE0 + unit_previous_state);
    glBindTexture(GL_TEXTURE_2D, texture_state[(int)current_state].handle);
    glUniform1i(location_previous_state, unit_previous_state);

    constexpr unsigned int location_source_sink = 1;
    constexpr unsigned int unit_source_sink = 1;
    glActiveTexture(GL_TEXTURE0 + unit_source_sink);
    glBindTexture(GL_TEXTURE_2D, texture_source_sink.handle);
    glUniform1i(location_source_sink, unit_source_sink);

    constexpr unsigned int location_R = 2;
    glUniform1f(location_R, R);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)!current_state].handle);
    const GLenum attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &attachment);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    current_state = !current_state;
}
