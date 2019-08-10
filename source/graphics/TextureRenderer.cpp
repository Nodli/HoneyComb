#include "TextureRenderer.h"

#include <cassert>

#include "print.h"

TextureRenderer::TextureRenderer(RenderMode initial_mode)
: current_render_mode(initial_mode){

    const char* const code_vertex = R"(
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

    #include "TextureRenderer.code"

    // ---- Shader code ---- //

    ogl::create_shader(vertex, GL_VERTEX_SHADER, code_vertex);
    ogl::create_shader(fragment[current_render_mode],
            GL_FRAGMENT_SHADER,
            code_fragment[current_render_mode]);

    ogl::create_program(program, vertex, fragment[current_render_mode]);
}

TextureRenderer::~TextureRenderer(){
}

void TextureRenderer::render(const OpenGLWindow& window, const ogl::Texture& texture, RenderMode mode){

    // Change the fragment shader used by the program
    if(mode != current_render_mode){

        // Create the fragment shader when necessary
        if(fragment[mode].handle == 0){

            #include "TextureRenderer.code"

            ogl::create_shader(fragment[mode], GL_FRAGMENT_SHADER, code_fragment[mode]);
        }

        // Relink the program with the new fragment shader
        glDetachShader(program.handle, fragment[current_render_mode].handle);
        glAttachShader(program.handle, fragment[mode].handle);
        glLinkProgram(program.handle);
        ogl::program_error(program.handle, "TextureRenderer::render()");

        current_render_mode = mode;
    }

    // Rendering the texture
    int width;
    int height;
    glfwGetWindowSize(window.window, &width, &height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(program.handle);

    constexpr int location_simulation_data = 0;
    constexpr int unit_simulation_data = 0;
    glActiveTexture(GL_TEXTURE0 + unit_simulation_data);

    glBindTexture(GL_TEXTURE_2D, texture.handle);

    glUniform1i(location_simulation_data, unit_simulation_data);

    glBindVertexArray(vertex_array_empty.handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

