#include "GL.h"
#include "OpenGLWindow.h"

#include "TextureRenderer.h"

#include "print.h"

// Tutorial on compute shaders
// https://zestedesavoir.com/tutoriels/1554/introduction-aux-compute-shaders/

int main(){

    constexpr int width = 640;
    constexpr int height = 480;

    OpenGLWindow window;
    window.gl_version_major = 4;
    window.gl_version_minor = 3;
    window.open(width, height, "Particles", false);
    window.vertical_synchronization = true;

    ogl::Texture compute_texture;
    glBindTexture(GL_TEXTURE_2D, compute_texture.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 640, 480, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    int workgroup_count[3];
    int workgroup_size[3];
    int workgroup_invocations;

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, workgroup_count);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, workgroup_count + 1);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, workgroup_count + 2);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, workgroup_size);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, workgroup_size + 1);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, workgroup_size + 2);

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);

    print("Maximal number of work groups that can be dispatched in each dimension", workgroup_count[0], workgroup_count[1], workgroup_count[2]);
    print("Maximal size of a work group in each dimension", workgroup_size[0], workgroup_size[1], workgroup_size[2]);
    print("Number of invocations in a single local work group ie product of the work group size in all dimensions", workgroup_invocations);

    const char* const compute_code = R"(
    #version 430

    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba32f, binding = 0) uniform image2D output_image;

    void main(){
        ivec2 output_coordinates = ivec2(gl_GlobalInvocationID);

        vec4 output_color = vec4(0., 0.5, 1., 1.);
        if( (gl_WorkGroupID.x & 1u) != (gl_WorkGroupID.y & 1u) ){
            output_color = vec4(1., 0.5, 0., 1.);
        }

        imageStore(output_image, output_coordinates, output_color);
    }
    )";

    ogl::Shader compute_shader;
    ogl::Program compute_program;
    ogl::create_shader(compute_shader, GL_COMPUTE_SHADER, compute_code);
    ogl::create_program(compute_program, compute_shader);

    // Generating the texture
    glUseProgram(compute_program.handle);
    glBindTexture(GL_TEXTURE_2D, compute_texture.handle);
    glBindImageTexture(0, compute_texture.handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(40, 30, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindImageTexture(GL_TEXTURE0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    TextureRenderer renderer(TextureRenderer::RGB);

    while(!glfwWindowShouldClose(window.window)){

        glfwPollEvents();

		if(glfwGetKey(window.window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(window.window, true);
		}

        // -------- Start of Frame -------- //

        // ---- Interactions ---- //

        // ---- Render ---- //

        renderer.render(window, compute_texture, TextureRenderer::RGB);

        // -------- End of Frame -------- //

        window.synchronize();
    }

    window.close();
}
