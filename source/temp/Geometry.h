#ifndef H_GEOMETRY
#define H_GEOMETRY

#include "GL/gl3w.h"
#include "glm/vec3.hpp"

#include <vector>
#include <cmath>

/*
std::vector<float> cube_shape(const float size = 1.f);
std::vector<float> hexagonal_prism_shape(const float base_height = 1.f, const float base_width = std::sqrt(3.f) / 2.f, const float height = 1.f);

struct Geometry{

	Geometry();
	~Geometry();

	void draw();

	void setShape(const std::vector<float>& data);

	void clear();

	unsigned int indice_count = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
};

struct Collection : Geometry{

	Collection();
	~Collection();

	void draw();
	void setPosition(const std::vector<glm::vec3>& position_data);

	void clear();

	unsigned int instance_count = 0;
	GLuint instance_vbo = 0;
};

struct ValuedCollection : Collection{

	ValuedCollection();
	~ValuedCollection();

	void setPosition(const std::vector<glm::vec3>& position_data);
	void setValue(const std::vector<float>& value_data);
	void setPositionValue(const std::vector<glm::vec3>& position_data,
            const std::vector<float>& value_data);
	void clear();

	GLuint instance_value_vbo = 0;
};
*/

#endif
