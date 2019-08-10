#include "ReactionDiffusion.h"

#include <cmath>
#include "print.h"

ReactionDiffusion::ReactionDiffusion(){

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

    out vec2 out_value;
    layout(location = 1) uniform vec2 value;

    void main(){
        out_value = value;
    }
    )";

    const char* const code_disc_set_value = R"(
    #version 450

    in vec2 texture_coord;
    out vec2 out_value;
    layout(location = 1) uniform vec2 center;
    layout(location = 2) uniform float radius;
    layout(location = 3) uniform vec2 value;

    void main(){

        const float distance_x = (texture_coord.x - center.x);
        const float distance_y = (texture_coord.y - center.y);
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
    out vec2 out_value;
    layout(location = 0) uniform sampler2D previous_state;
    layout(location = 1) uniform vec4 RFk; // vec4(Ru, Rv, F, k)

    vec2 laplacian9(in vec2 current_value){

        vec2 texel_size = vec2(1.0) / textureSize(previous_state, 0);

        return (1.0 / 6.0) *

                (4 * (texture(previous_state, vec2(texture_coord.x + texel_size.x, texture_coord.y)).xy
                    + texture(previous_state, vec2(texture_coord.x - texel_size.x, texture_coord.y)).xy
                    + texture(previous_state, vec2(texture_coord.x, texture_coord.y + texel_size.y)).xy
                    + texture(previous_state, vec2(texture_coord.x, texture_coord.y - texel_size.y)).xy)

                + texture(previous_state, vec2(texture_coord.x - texel_size.x, texture_coord.y - texel_size.y)).xy
                + texture(previous_state, vec2(texture_coord.x + texel_size.x, texture_coord.y - texel_size.y)).xy
                + texture(previous_state, vec2(texture_coord.x - texel_size.x, texture_coord.y + texel_size.y)).xy
                + texture(previous_state, vec2(texture_coord.x + texel_size.x, texture_coord.y + texel_size.y)).xy

                - 20 * current_value);
    }


    void main(){

        vec2 current_value = texture(previous_state, texture_coord).xy;
        vec2 laplacian_value = laplacian9(current_value);

        // Diffusion
        out_value = current_value + RFk.xy * laplacian_value; // Diffusion

        // Reaction
        out_value.r += - current_value.x * current_value.y * current_value.y + RFk.z * (1. - current_value.x);
        out_value.g += current_value.x * current_value.y * current_value.y - (RFk.z + RFk.w) * current_value.y; // Reaction for V
    }
    )";

    // ---- Shader Creation ---- //

    ogl::create_shader(vertex_full_screen_quad, GL_VERTEX_SHADER, code_full_screen_quad);
    ogl::create_shader(vertex_quad, GL_VERTEX_SHADER, code_quad);
    ogl::create_shader(fragment_set_value, GL_FRAGMENT_SHADER, code_set_value);
    ogl::create_shader(fragment_disc_set_value, GL_FRAGMENT_SHADER, code_disc_set_value);
    ogl::create_shader(fragment_step, GL_FRAGMENT_SHADER, code_step_forward);

    ogl::create_program(program_reset, vertex_full_screen_quad, fragment_set_value);
    ogl::create_program(program_point, vertex_quad, fragment_disc_set_value);
    ogl::create_program(program_square, vertex_quad, fragment_set_value);
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

    glBindTexture(GL_TEXTURE_2D, 0);
}

ReactionDiffusion::~ReactionDiffusion(){
}

void ReactionDiffusion::start(const int simulation_width, const int simulation_height){

    width = simulation_width;
    height = simulation_height;

    // ---- Initializing state textures and framebuffer ---- //

    float initial_data[width * height * 2];
    for(int idata = 0; idata != width * height; ++idata){
        initial_data[2 * idata] = default_value_u;
        initial_data[2 * idata + 1] = default_value_v;
    }

    glBindTexture(GL_TEXTURE_2D, texture_state[0].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, initial_data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[0].handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_state[0].handle, 0);

    glBindTexture(GL_TEXTURE_2D, texture_state[1].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, initial_data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[1].handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_state[1].handle, 0);

    // ---- Clean bindings ---- //

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ReactionDiffusion::reset(){

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)current_state].handle);
    const GLenum state_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &state_attachment);

    glUseProgram(program_reset.handle);

    constexpr unsigned int location_value = 1;
    glUniform2f(location_value, default_value_u, default_value_v);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

void ReactionDiffusion::point(const float center_x, const float center_y, const float radius,
        const float value_u, const float value_v){

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)current_state].handle);
    const GLenum state_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &state_attachment);

    glUseProgram(program_point.handle);

    constexpr unsigned int location_min_max_coord = 0;
    float min_x = std::max(center_x - radius, -1.f);
    float max_x = std::min(center_x + radius, 1.f);
    float min_y = std::max(center_y - radius, -1.f);
    float max_y = std::min(center_y + radius, 1.f);
    glUniform4f(location_min_max_coord, min_x, min_y, max_x, max_y);

    constexpr unsigned int location_center = 1;
    glUniform2f(location_center, center_x, center_y);

    constexpr unsigned int location_radius = 2;
    glUniform1f(location_radius, radius);

    constexpr unsigned int location_value = 3;
    glUniform2f(location_value, value_u, value_v);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ReactionDiffusion::square(const float center_x, const float center_y, const float size,
        const float value_u, const float value_v){

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)current_state].handle);
    const GLenum state_attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &state_attachment);

    glUseProgram(program_square.handle);

    constexpr unsigned int location_min_max_coord = 0;
    float min_x = std::max(center_x - size, -1.f);
    float max_x = std::min(center_x + size, 1.f);
    float min_y = std::max(center_y - size, -1.f);
    float max_y = std::min(center_y + size, 1.f);
    glUniform4f(location_min_max_coord, min_x, min_y, max_x, max_y);

    constexpr unsigned int location_value = 1;
    glUniform2f(location_value, value_u, value_v);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ReactionDiffusion::step(){
    glViewport(0, 0, width, height);

    glUseProgram(program_step.handle);

    constexpr unsigned int location_previous_state = 0;
    constexpr unsigned int unit_previous_state = 0;
    glActiveTexture(GL_TEXTURE0 + unit_previous_state);
    glBindTexture(GL_TEXTURE_2D, texture_state[(int)current_state].handle);
    glUniform1i(location_previous_state, unit_previous_state);

    constexpr unsigned int location_RFk = 1;
    glUniform4f(location_RFk, sim_Ru, sim_Rv, sim_F, sim_R);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_state[(int)!current_state].handle);
    const GLenum attachment = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &attachment);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    current_state = !current_state;
}
