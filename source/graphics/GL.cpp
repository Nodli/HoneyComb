#include "GL.h"
#include "print.h"

#include <cassert>

void ogl::initialize_GL3W(const int gl_version_major, const int gl_version_minor){

	if(gl3wInit()){
		print("GL3W error: initialization failed");
	}

	if(!gl3wIsSupported(gl_version_major, gl_version_minor)){
		print("GL3W error: gl version", gl_version_major, ".", gl_version_minor, "is not supported");
	}

	print("GL3W detection : gl", glGetString(GL_VERSION));
	print("GL3W detection : GLSL", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

ogl::Shader::Shader(){
}

ogl::Shader::~Shader(){
    glDeleteShader(handle);
}

void ogl::create_shader(Shader& shader,
        const GLenum shader_type,
        const char* const shader_code,
        const GLint shader_length){

    assert(shader.handle == 0);

    shader.handle = glCreateShader(shader_type);

	glShaderSource(shader.handle, 1, (const GLchar**) &shader_code, (const GLint*) &shader_length);
	glCompileShader(shader.handle);

	shader_error(shader.handle, "CreateShader");
}

void ogl::load_shader(Shader& shader,
        const char* const shader_code,
        const GLint shader_length){

    assert(shader.handle != 0);

	glShaderSource(shader.handle, 1, (const GLchar**) &shader_code, (const GLint*) &shader_length);
	glCompileShader(shader.handle);

	shader_error(shader.handle, "CreateShader");
}

ogl::Program::Program(){
    handle = glCreateProgram();
}

ogl::Program::~Program(){
    glDeleteProgram(handle);
}

ogl::Texture::Texture(){
    glGenTextures(1, &handle);
}

ogl::Texture::~Texture(){
    glDeleteTextures(1, &handle);
}

ogl::Framebuffer::Framebuffer(){
    glGenFramebuffers(1, &handle);
}

ogl::Framebuffer::~Framebuffer(){
    glDeleteFramebuffers(1, &handle);
}

ogl::Buffer::Buffer(){
    glGenBuffers(1, &handle);
}

ogl::Buffer::~Buffer(){
    glDeleteBuffers(1, &handle);
}

ogl::VertexArray::VertexArray(){
    glCreateVertexArrays(1, &handle);
}

ogl::VertexArray::~VertexArray(){
    glDeleteVertexArrays(1, &handle);
}

void ogl::error_message(const GLenum error_code){
    switch(error_code){
        case GL_INVALID_ENUM:
            print("GL_INVALID_ENUM",
                    "An unacceptable value is specified for an enumerated argument");
            break;

        case GL_INVALID_VALUE:
            print("GL_INVALID_VALUE",
                    "A numeric argument is out of range");
            break;

        case GL_INVALID_OPERATION:
            print("GL_INVALID_OPERATION",
                    "The specified operation is not allowed in the current state");
            break;

        case GL_STACK_OVERFLOW:
            print("GL_STACK_OVERFLOW",
                    "An attempt has been made to perform an operation that would cause an internal stack to overflow");
            break;

        case GL_STACK_UNDERFLOW:
            print("GL_STACK_UNDERFLOW",
                    "An attempt has been made to perform an operation that would cause an internal stack to underflow");
            break;

        case GL_OUT_OF_MEMORY:
            print("GL_OUT_OF_MEMORY",
                    "There is not enough memory left to execute the command");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            print("GL_INVALID_FRAMEBUFFER_OPERATION",
                    "The framebuffer object is not complete");
            break;

        case GL_CONTEXT_LOST:
            print("GL_CONTEXT_LOST",
                    "The OpenGL context has been lost, due to a graphics card reset");
            break;
    }
}

bool ogl::error(const char* print_label){

    GLenum error_status = GL_NO_ERROR;
    bool any_error = false;

    while((error_status = glGetError()) != GL_NO_ERROR){
        any_error = true;
		print(print_label, "OpenGL Error with code", error_status);
        error_message(error_status);
    }


    return any_error;
}

bool ogl::shader_error(const GLuint shader, const char* print_label){

	// retrieving shader status
	GLint shader_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);

	if(!shader_status){

		// retrieving shader error print
		int shader_print_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shader_print_length);

		std::string shader_print;
		shader_print.resize(shader_print_length);
		glGetShaderInfoLog(shader, shader_print_length, NULL, (char*)shader_print.data());

		print(print_label, "OpenGL Error with shader", shader);
		print(shader_print);

		return false;
	}

	return true;
}

bool ogl::program_error(const GLuint program, const char* print_label){

	// retrieving program status
	GLint linking_status;
	glGetProgramiv(program, GL_LINK_STATUS, &linking_status);

	if(!linking_status){

		// retrieving program error print
		int program_print_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_print_length);

		std::string program_print;
		program_print.resize(program_print_length);
		glGetProgramInfoLog(program, program_print_length, NULL, (char*)program_print.data());

		print(print_label, "OpenGL Error with program", program);
		print(program_print);

		return false;
	}

	return true;
}

void ogl::framebuffer_error_message(const GLenum error_code){
	switch(error_code){
		case GL_FRAMEBUFFER_UNDEFINED:
			print("GL_FRAMEBUFFER_UNDEFINED",
					"is returned if the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			print("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT",
					"is returned if any of the framebuffer attachment points are framebuffer incomplete");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			print("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT",
					"is returned if the framebuffer does not have at least one image attached to it");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			print("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER",
					"is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			print("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER",
					"is returned if the value of GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER");
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			print("GL_FRAMEBUFFER_UNSUPPORTED",
					"is returned if the combination of internal formats of the attaches images violates an implementation-dependant set of restrictions");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			print("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE",
					"is returned if the value of GL_RENDER_BUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES\n",
					"is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_LOCATIONS is not GL_TRUE for all attached textures2: The value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_LOCATIONS is not GL_TRUE for all attached textures");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			print("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS",
					"is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target");
			break;
	}
}

bool ogl::framebuffer_error(const GLenum target, const char* print_label){
	GLenum framebuffer_status = glCheckFramebufferStatus(target);

	if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE){
		print(print_label, "OpenGL Error : framebuffer", target, "is incomplete");
		framebuffer_error_message(framebuffer_status);

		return false;
	}

	return true;
}

bool ogl::named_framebuffer_error(const GLint framebuffer, const char* print_label){
	GLenum framebuffer_status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);

	if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE){

		if(framebuffer){
			print(print_label, "OpenGL Error : framebuffer", framebuffer, "is incomplete");
		}else{
			print(print_label, "OpenGL Error : fallback target GL_FRAMEBUFFER is incomplete");
		}

		framebuffer_error_message(framebuffer_status);

		return false;
	}

	return true;
}

void ogl::source_message(const GLenum source_code){
    switch(source_code){
        case GL_DEBUG_SOURCE_API:
            print("GL_DEBUG_SOURCE_API",
                    "Generated by calls to the OpenGL API");
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            print("GL_DEBUG_SOURCE_WINDOW_SYSTEM",
                    "Generated by calls to a window-system API");
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            print("GL_DEBUG_SOURCE_SHADER_COMPILER",
                    "Generated by a compiler for a shading language");
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            print("GL_DEBUG_SOURCE_THIRD_PARTY",
                    "Generated by an application associated with OpenGL");
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            print("GL_DEBUG_SOURCE_APPLICATION",
                    "Generated by the user of this application");
            break;

        case GL_DEBUG_SOURCE_OTHER:
            print("GL_DEBUG_SOURCE_OTHER",
                    "Generated by some unknown source");
            break;

    }
}

void ogl::type_message(const GLenum type_code){
    switch(type_code){
        case GL_DEBUG_TYPE_ERROR:
            print("GL_DEBUG_TYPE_ERROR",
                    "An error, typically from the API");
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            print("GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR",
                    "Some behavior marked deprecated has been used");
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            print("GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR",
                    "Something has invoked undefined behavior");
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            print("GL_DEBUG_TYPE_PORTABILITY",
                    "Some functionality the user relies upon is not portable");
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            print("GL_DEBUG_TYPE_PERFORMANCE",
                    "Code has triggered possible performance issues");
            break;

        case GL_DEBUG_TYPE_MARKER:
            print("GL_DEBUG_TYPE_MARKER",
                    "Command stream annotation");
            break;

        case GL_DEBUG_TYPE_PUSH_GROUP:
            print("GL_DEBUG_TYPE_PUSH_GROUP",
                    "Group pushing");
            break;

        case GL_DEBUG_TYPE_POP_GROUP:
            print("GL_DEBUG_TYPE_POP_GROUP",
                    "Group poping");
            break;

        case GL_DEBUG_TYPE_OTHER:
            print("GL_DEBUG_TYPE_OTHER",
                    "Unknown type");
            break;
    }
}

void ogl::severity_message(const GLenum severity_code){
    switch(severity_code){
        case GL_DEBUG_SEVERITY_HIGH:
            print("GL_DEBUG_SEVERITY_HIGH",
                    "All OpenGL Errors, shader compilation / linking errors, or highly-dangerous undefined behavior");
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            print("GL_DEBUG_SEVERITY_MEDIUM",
                    "Major performance warnings, shader compilation / linking warnings, or the use of deprecated functionality");
            break;

        case GL_DEBUG_SEVERITY_LOW:
            print("GL_DEBUG_SEVERITY_LOW",
                    "Redundant state change performance warning, or unimportant undefined behavior");
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            print("GL_DEBUG_SEVERITY_NOTIFICATION",
                    "Anything that isn't an error or performance issue");
            break;
    }
}

void ogl::message_callback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam){

    print("GL CALLBACK for ID :", id, (type == GL_DEBUG_TYPE_ERROR ? "which is a GL_ERROR" : ""));
    source_message(source);
    type_message(type);
    severity_message(severity);
    print("MESSAGE", message);

    error();
}

void ogl::initialize_debug_callback(){
    //glEnable(GL_DEBUG_OUTPUT);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, 0);
}
