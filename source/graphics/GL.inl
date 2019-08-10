#ifndef INL_GL
#define INL_GL

#include "print.h"

inline void attach_shader_to_program(const ogl::Program& program){
}

template<typename ... ShaderArguments>
void attach_shader_to_program(const ogl::Program& program, const ogl::Shader& shader, const ShaderArguments& ... otherShader){
    glAttachShader(program.handle, shader.handle);
    attach_shader_to_program(program, otherShader...);
}

template<typename ... ShaderArguments>
void ogl::create_program(ogl::Program& program, const ShaderArguments& ... otherShader){

    attach_shader_to_program(program, otherShader...);

    glLinkProgram(program.handle);
    ogl::program_error(program.handle, "create_program");
}

template<typename GLType>
void ogl::swap_handle(GLType& A, GLType& B){
    GLuint temp_handle = A.handle;
    A.handle = B.handle;
    B.handle = temp_handle;
}

#endif
