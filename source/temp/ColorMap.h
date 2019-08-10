#ifndef H_COLORMAP
#define H_COLORMAP

#include "GL/gl3w.h"
#include "glm/vec3.hpp"
#include <vector>

struct ColorMap{
	ColorMap(const std::vector<glm::vec3>& ivalues);
	~ColorMap();

	// ----------------------------------------------

	// sample the colormap at /sample_coor/ in [0; 1]
	glm::vec3 Sample(float sample_coord) const;

	// creates a 1D texture and buffers the colormap data
	void CreateTexture();
	// deletes the texture
	void DeleteTexture();

	// bind the colormap texture to the given /texture_unit/
	void Bind(const unsigned int texture_unit);
	// binds the default texture to the given /texture_unit/
	void Release();

	// ----------------------------------------------

	std::vector<glm::vec3> values;

	bool buffered = false;
	GLuint texture;
};

namespace Palette{
	ColorMap Viridis();
};

#endif
