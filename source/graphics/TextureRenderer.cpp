#include "TextureRenderer.h"

#include <cassert>

#include "print.h"

TextureRenderer::TextureRenderer(RenderMode mode){

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

    const char* const code_fragment[] = {

        // RenderMode = R
        R"(
            #version 450

            in vec2 texture_coord;
            out vec4 out_color;

            layout(location = 0) uniform sampler2D simulation_data;

            void main(){
                float value = texture(simulation_data, texture_coord).r;

                out_color = vec4(value, value, value, 1.0);
            }
        )",

        // RenderMode = RG
        R"(
            #version 450

            in vec2 texture_coord;
            out vec4 out_color;

            layout(location = 0) uniform sampler2D simulation_data;

            void main(){
                vec2 value = texture(simulation_data, texture_coord).rg;

                out_color = vec4(value.r, value.g, 0.5f, 1.0);
            }
        )",

        // RenderMode = RGB
        R"(
            #version 450

            in vec2 texture_coord;
            out vec4 out_color;

            layout(location = 0) uniform sampler2D input_texture;

            void main(){
                out_color = vec4(texture(input_texture, texture_coord).rgb, 1.);
            }
        )",

        // RenderMode = RGBA
        R"(
            #version 450

            in vec2 texture_coord;
            out vec4 out_color;

            layout(location = 0) uniform sampler2D input_texture;

            void main(){
                out_color = texture(input_texture, texture_coord);
            }
        )"
    };

    // ---- Shader code ---- //

    ogl::create_shader(vertex, GL_VERTEX_SHADER, code_vertex);
    ogl::create_shader(fragment,
            GL_FRAGMENT_SHADER,
            code_fragment[mode]);

    ogl::create_program(program, vertex, fragment);
}



TextureRenderer::TextureRenderer(const char* const fragment_code){
}

TextureRenderer::~TextureRenderer(){
}

void TextureRenderer::render(const OpenGLWindow& window, const ogl::Texture& texture){

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

