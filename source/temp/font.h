#include <ft2build.h>
#include <freetype_header.h>

#include "print.h"

// allocate FreeType ressources

FT_Library library;
if(FT_Init_FreeType(&library)){
    print("Could not initialize the FreeType library");
}

constexpr char* face_filepath[] = "";
constexpr unsigned int face_index = 0;
constexpr unsigned int face_height = 48;

FT_Face face;
if(FT_New_Face(library, face_filepath, face_index, &face)){
    print("Could not import the FreeType font at", face_filepath);
}
FT_Set_Pixel_Sizes(&face, 0, face_height); // face width computed with /face_height/

struct Glyph{
    GLuint texture;

    unsigned int width;
    unsigned int height;

    unsigned int bearing_width;
    unsigned int bearing_height;

    GLuint offset;
};


// caching all the ASCII GLchar glyphs
Glyph glyph_cache[128];
GLuint glyph_textures[128];
glGenTextures(128, &glyph_textures);

glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
for(unsigned char ichar = 0; ichar != 128; ++ichar){

    // creates a grayscale bitmap image of the glyph
    if(FT_Load_Char(face, ichar, FT_LOAD_RENDER)){
        print("Could not load glyph", ichar);
    }

    glBindTexture(GL_TEXTURE_2D, glyph_textures[(size_t)(ichar)]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glyph_cache[(size_t)(ichar)].texture = glyph_textures[(size_t)(ichar)];
    glyph_cache[(size_t)(ichar)].width = face->glyph->bitmap.width;
    glyph_cache[(size_t)(ichar)].height = face->glyph->bitmap.rows;
    glyph_cache[(size_t)(ichar)].bearing_width = face->glyph->bitmap_left;
    glyph_cache[(size_t)(ichar)].bearing_height = face->glyph->bitmap_top;
    glyph_cache[(size_t)(ichar)].offset = face->glyph->advance.x;
}

// restore pixel storage alignment
glPixelStorei(GL_UNPACK_ALIGNMENT);

// free FreeType ressources
FT_Done_Face(face);
FT_Done_FreeType(library);

const char* const glyph_header = R"(
#version 450
)";
const char* const glyph_vertex = R"(
layout(location = 0) in vec4 vertex; // (posX, posY, texcoordX, texcoordY)
out vec2 texcoord;

layout(location = 0) uniform mat4 projection;

void main(){
    gl_Position = projection * vec4(vertex.xy, 0., 1.);
    texcoord = vertex.zw;
}
)";

const char* const glyph_fragment = R"(
in vec2 texcoord;
out vec4 color;

layout(location = 1) uniform sampler2D glyph_texture;
layout(location = 2) uniform vec2 glyph_color;

void main(){
    color = vec4(glyph_color.rgb, texture(glyph_texture, texcoord));
}
)";

Shader glyph_shader;
glyph_shader.set(glyph_header, glyph_vertex, glyph_fragment);

GLuint vao;
GLuint vbo;

glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);

constexpr unsigned int buffer_char_count = 1;
glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * buffer_char_count, NULL, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float) * buffer_char_count, 0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);

// to render with origin (0, 0) at bottom left
const float x;
const float y;
const float scale;
const float r;
const float g;
const float b;

glUseProgram(glyph_shader.program);
constexpr unsigned int glyph_color_location = 2;
glUniform3f(glyph_color_location, 1.0, 1.0, 1.0);
glActiveTexture(GL_TEXTURE0);
glBindVertexArray(vao);

glBindBuffer(GL_ARRAY_BUFFER, vbo);

const std::string text_to_render = "text_to_render";
for(size_t ichar = 0; ichar != text_to_render.size(); ++ichar){
    Glyph* glyph = glyph_cache + (size_t)(text_to_render[ichar]);

    float render_posx = x + glyph->bearing_width * scale;
    float render_posy = y - (glyph->height - glyph->bearing_height) * scale;
    float render_width = glyph->width * scale;
    float render_height = glyph->height * scale;

    constexpr unsigned int float_count = 24;
    float render_vertices[vertices_count] = {
        render_posx, render_posy + render_height, 0., 1.,
        render_posx, render_posy, 0., 1.,
        render_posx + render_width, render_posy, 1., 1.,
        render_posx, render_posy + render_height, 0., 1.,
        render_posx + render_width, render_posy, 1., 1.,
        render_posx + render_width, render_posy + render_height, 1., 0.
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, float_count * sizeof(float), render_vertices);

    x += (glyph->offset >> 6) * scale; // bitshift gets the value in pixels because advance is 1 / 64 pixels (do this when creating the glyph)
}

glDrawArrays(GL_TRIANGLES, 0, 6 * text_to_render.size());

glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, 0);


