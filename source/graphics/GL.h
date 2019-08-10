#ifndef H_GL
#define H_GL

#include "GL/gl3w.h"

namespace ogl{

	void initialize_GL3W(const int gl_version_major, const int gl_version_minor);

    struct Shader{
        Shader();
        ~Shader();

        GLuint handle = 0;
    };

	void create_shader(Shader& shader,
            const GLenum shader_type,
            const char* const shader_code,
            const GLint shader_length = -1);
    void load_shader(Shader& shader,
            const char* const shader_code,
            const GLint shader_length = -1);

    struct Program{
        Program();
        ~Program();

        GLuint handle;
    };

    template<typename ... ShaderArguments>
    void create_program(Program& program, const ShaderArguments& ... shader);

    struct Texture{
        Texture();
        ~Texture();

        GLuint handle;
    };

    struct Framebuffer{
        Framebuffer();
        ~Framebuffer();

        GLuint handle;
    };

    struct Buffer{
        Buffer();
        ~Buffer();

        GLuint handle;
    };

    struct VertexArray{
        VertexArray();
        ~VertexArray();

        GLuint handle;
    };

    template<typename GLType>
    void swap_handle(GLType& A, GLType& B);

    // ---- Error Detection ---- //

    void error_message(const GLenum error_code);
	bool error(const char* const log_label = nullptr);
	bool shader_error(const GLuint shader, const char* log_label = nullptr);
	bool program_error(const GLuint program, const char* log_label = nullptr);
	void framebuffer_error_message(const GLenum error_code);
	bool framebuffer_error(const GLenum target, const char* log_label = nullptr);
	bool named_framebuffer_error(const GLint framebuffer, const char* log_label = nullptr);

    // ---- Error Message Callback ---- //

    void source_message(const GLenum source_code);
    void type_message(const GLenum type_code);
    void severity_message(const GLenum severity_code);
    void message_callback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam);
    void initialize_debug_callback();
}

#include "GL.inl"

#endif
