#ifndef H_TEXTURE_RENDERER
#define H_TEXTURE_RENDERER

#include "GL.h"
#include "OpenGLWindow.h"

struct TextureRenderer{

    enum RenderMode {R = 0, RG = 1, RGB = 2, RGBA = 3};

    TextureRenderer(RenderMode initial_mode = RGB);
    ~TextureRenderer();

    void render(const OpenGLWindow& window, const ogl::Texture& texture, RenderMode mode);

    RenderMode current_render_mode;

    ogl::Shader vertex;
    ogl::Shader fragment[4];
    ogl::Program program;

    ogl::VertexArray vertex_array_empty;
};

#endif
