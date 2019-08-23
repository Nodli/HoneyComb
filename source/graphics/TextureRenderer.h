#ifndef H_TEXTURE_RENDERER
#define H_TEXTURE_RENDERER

#include "GL.h"
#include "OpenGLWindow.h"

struct TextureRenderer{

    enum RenderMode {R = 0, RG = 1, RGB = 2, RGBA = 3};

    TextureRenderer(RenderMode mode);
    TextureRenderer(const char* const fragment_code);
    ~TextureRenderer();

    void render(const OpenGLWindow& window, const ogl::Texture& texture);

    ogl::Shader vertex;
    ogl::Shader fragment;
    ogl::Program program;

    ogl::VertexArray vertex_array_empty;
};

#endif
