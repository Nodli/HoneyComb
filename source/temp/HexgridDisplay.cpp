#include "HexGrid.h"

#include "GL.h"
#include "OpenGLWindow.h"

#include "Geometry.h"
#include "Shader.h"
#include "Orbiter.h"
#include "Framebuffer.h"

#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct Application{

	Application();
	~Application();

	void update(const float time);

	HexGrid<float> hexgrid;

	OpenGLWindow display;

	//Orbiter orbiter;
	//Shader mesh_shader;
	//ValuedCollection hexgrid_mesh;

	float grid_scale = 1.1f;
};

Application::Application()
: display(3, 3){


    //hexgrid.buildLayered(10);

    //orbiter.target_position = {0.f, 0.f, 0.f};
    //orbiter.camera_radius = 2.f;
    //orbiter.setPolarAngle(0.f);
    //orbiter.setAzimutalAngle(0.f);

    const char* const header = R"(
    #version 450
    )";

    const char* const vertex = R"(
	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec3 in_instance_position;
	layout(location = 3) in float in_instance_value;

	layout(location = 0) uniform mat4 u_matrix_mvp;

	out vec3 interp_color;

	void main(){

		vec4 valued_position = vec4(in_position + in_instance_position, 1.);
		valued_position.y *= in_instance_value;

		gl_Position = u_matrix_mvp * valued_position;

		//interp_color = abs(in_normal);
        interp_color = abs(in_normal) * dot(vec3(1., 1., 1.), in_normal);
	}
    )";

    const char* const fragment = R"(
	in vec3 interp_color;

	out vec4 out_color;

	void main(){

		out_color = vec4(interp_color, 0.25);
	}
    )";

	//mesh_shader.set(header, vertex, fragment);

	//hexgrid_mesh.setShape(HEXAGONAL_PRISM);
    //hexgrid_mesh.setPositionValue();


/*
    // setup rendering context
    const glm::ivec2 dim = display.dimensions();
    glViewport(0, 0, dim.x, dim.y);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 0.f);
*/
}

Application::~Application(){
    //display.close();
}

void Application::update(const float time){

}

/*
void Application::run(){

    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);

	initialize();

    // create the texture attachment
    const glm::ivec2 dim = display.dimensions();
    const glm::ivec2 buffer_dim = {dim.x / 2, dim.y / 2};

    Framebuffer bufferA(buffer_dim.x, buffer_dim.y);
    Framebuffer bufferB(buffer_dim.x, buffer_dim.y);
    GLenum buffer_attachments[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};

    Shader horizontal_bloom_framebuffer;
    Shader vertical_bloom_framebuffer;
    Shader draw_framebuffer;

    horizontal_bloom_framebuffer.set(Header::Version450().c_str(),
            Vertex::ScreenSquare().c_str(),
            Fragment::HorizontalBloomTexture().c_str());
    vertical_bloom_framebuffer.set(Header::Version450().c_str(),
            Vertex::ScreenSquare().c_str(),
            Fragment::VerticalBloomTexture().c_str());
    draw_framebuffer.set(Header::Version450().c_str(),
            Vertex::ScreenSquare().c_str(),
            Fragment::TextureSample().c_str());

	while(!glfwWindowShouldClose(display->window)){

		glfwPollEvents();

		if(glfwGetKey(display->window, GLFW_KEY_ESCAPE)){
			glfwSetWindowShouldClose(display->window, true);
		}

		// start of frame -------------------------------------------------

		if(glfwGetKey(display->window, GLFW_KEY_SPACE)){
			simulate();
		}

		orbiter.processInput(display->window);
		glm::mat4 view_projection_matrix = orbiter.viewProjection(display->aspectRatio());

        // render things on the framebuffers
        glViewport(0, 0, buffer_dim.x, buffer_dim.y);

        glBindFramebuffer(GL_FRAMEBUFFER, bufferA.framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffers(1, buffer_attachments);

        glUseProgram(shader.program);
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view_projection_matrix));
        glBindVertexArray(hexgrid_mesh.vao);
		hexgrid_mesh.draw();

        // apply bloom
        // horizontal
        glBindFramebuffer(GL_FRAMEBUFFER, bufferB.framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffers(1, buffer_attachments);

        glUseProgram(horizontal_bloom_framebuffer.program);
        glBindTexture(GL_TEXTURE_2D, bufferA.texture[0]);
        glBindVertexArray(dummyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // vertical
        glBindFramebuffer(GL_FRAMEBUFFER, bufferA.framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffers(1, buffer_attachments);

        glUseProgram(vertical_bloom_framebuffer.program);
        glBindTexture(GL_TEXTURE_2D, bufferB.texture[0]);
        glBindVertexArray(dummyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render the framebuffer on the screen
        glViewport(0, 0, dim.x, dim.y);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(draw_framebuffer.program);
        glBindTexture(GL_TEXTURE_2D, bufferA.texture[0]);
        glBindVertexArray(dummyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// end of frame ---------------------------------------------------

		glfwSwapBuffers(display->window);
	}

    glDeleteVertexArrays(1, &dummyVAO);
}
*/

int main(){
	Application app;

    //app.display.open(1920, 1080, "HexgridDisplay");
}
