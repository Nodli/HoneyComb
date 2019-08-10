#include "Geometry.h"

#include <cassert>

#include "GL.h"

/*
std::vector<float> cube_shape(const float size){

	const float half_size = size / 2.f;

	return {
		// front face
		-half_size,	half_size,	half_size,	0.f, 0.f, 1.f,
		-half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,

		-half_size,	half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	half_size,	half_size,	0.f, 0.f, 1.f,

		// right face
		half_size,	half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	-half_size,	1.f, 0.f, 0.f,

		half_size,	half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	-half_size,	1.f, 0.f, 0.f,
		half_size,	half_size,	-half_size,	1.f, 0.f, 0.f,

		// back face
		half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,
		half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,

		half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,

		// left face
		-half_size,	half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	half_size,	-1.f, 0.f, 0.f,

		-half_size,	half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	half_size,	-1.f, 0.f, 0.f,
		-half_size,	half_size,	half_size,	-1.f, 0.f, 0.f,

		// bottom face
		-half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,
		-half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,

		-half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,

		// top face
		-half_size,	half_size,	-half_size,	0.f, 1.f, 0.f,
		-half_size,	half_size,	half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	half_size,	0.f, 1.f, 0.f,

		-half_size,	half_size,	-half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	-half_size,	0.f, 1.f, 0.f
	};
}

std::vector<float> hexagonal_prism_shape(const float base_height, const float base_width, const float height){

	const float hbh = base_height / 2.f; // half base height
	const float qbh = base_height / 4.f; // quarter base height
	const float hbw = base_width / 2.f; // half base width
	const float hh = height / 2.f; // half height

	const float div = 1.f / (1.f + std::sqrt(3.f));
	const float sqdiv = std::sqrt(3.f) / (1.f + std::sqrt(3.f));

	return {

		// bottom face
		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		hbw, - hh, - qbh, 		0.f, -1.f, 0.f,
		hbw, - hh, qbh,  		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		0.f, - hh, - hbh, 		0.f, -1.f, 0.f,
		hbw, - hh, - qbh, 		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		- hbw, - hh, - qbh, 		0.f, -1.f, 0.f,
		0.f, - hh, - hbh, 		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		- hbw, - hh, qbh,  	0.f, -1.f, 0.f,
		- hbw, - hh, - qbh, 		0.f, -1.f, 0.f,

		// front-right face
		0.f, hh, hbh,			div, 0.f, sqdiv,
		0.f, - hh, hbh,		div, 0.f, sqdiv,
		hbw, - hh, qbh,		div, 0.f, sqdiv,

		0.f, hh, hbh,			div, 0.f, sqdiv,
		hbw, - hh, qbh,		div, 0.f, sqdiv,
		hbw, hh, qbh,			div, 0.f, sqdiv,

		// right face
		hbw, hh, qbh,			1.f, 0.f, 0.f,
		hbw, - hh, qbh,		1.f, 0.f, 0.f,
		hbw, - hh, - qbh, 		1.f, 0.f, 0.f,

		hbw, hh, qbh,			1.f, 0.f, 0.f,
		hbw, - hh, - qbh, 		1.f, 0.f, 0.f,
		hbw, hh, - qbh, 			1.f, 0.f, 0.f,

		// back-right face
		hbw, hh, - qbh,			div, 0.f, - sqdiv,
		hbw, - hh, - qbh, 		div, 0.f, - sqdiv,
		0.f, - hh, - hbh,			div, 0.f, - sqdiv,

		hbw, hh, - qbh,			div, 0.f, - sqdiv,
		0.f, - hh, - hbh,			div, 0.f, - sqdiv,
		0.f, hh, - hbh,			div, 0.f, - sqdiv,

		// back-left face
		0.f, hh, - hbh,			- div, 0.f, - sqdiv,
		0.f, - hh, - hbh, 		- div, 0.f, - sqdiv,
		- hbw, - hh, - qbh,		- div, 0.f, - sqdiv,

		0.f, hh, - hbh,			- div, 0.f, - sqdiv,
		- hbw, - hh, - qbh,		- div, 0.f, - sqdiv,
		- hbw, hh, - qbh,			- div, 0.f, - sqdiv,

		// left face
		- hbw, hh, - qbh,			-1.f, 0.f, 0.f,
		- hbw, - hh, - qbh,		-1.f, 0.f, 0.f,
		- hbw, - hh, qbh,  	-1.f, 0.f, 0.f,

		- hbw, hh, - qbh,			-1.f, 0.f, 0.f,
		- hbw, - hh, qbh,  	-1.f, 0.f, 0.f,
		- hbw, hh, qbh,  		-1.f, 0.f, 0.f,

		// front-left face
		- hbw, hh, qbh,  		- div, 0.f, sqdiv,
		- hbw, - hh, qbh,  	- div, 0.f, sqdiv,
		0.f, - hh, hbh,  		- div, 0.f, sqdiv,

		- hbw, hh, qbh,  		- div, 0.f, sqdiv,
		0.f, - hh, hbh,  		- div, 0.f, sqdiv,
		0.f, hh, hbh,  		- div, 0.f, sqdiv,

		// top face
		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		hbw, hh, qbh,  		0.f, 1.f, 0.f,
		hbw, hh, - qbh, 			0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		hbw, hh, - qbh, 			0.f, 1.f, 0.f,
		0.f, hh, - hbh, 			0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		0.f, hh, - hbh, 			0.f, 1.f, 0.f,
		- hbw, hh, - qbh, 		0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		- hbw, hh, - qbh, 		0.f, 1.f, 0.f,
		- hbw, hh, qbh,  		0.f, 1.f, 0.f
		};
}

void get_shape(const ShapePreset preset,
        std::vector<float>& data, unsigned int& indice_count){

    switch(preset){
        case CUBE :
            data = cube_shape();
            indice_count = 36;
            break;

        case HEXAGONAL_PRISM :
            data = hexagonal_prism_shape();
            indice_count = 60;
            break;
    }
}

Geometry::Geometry(){
	glGenVertexArrays(1, &vao);
}

Geometry::~Geometry(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Geometry::draw(){
	glDrawArrays(GL_TRIANGLES, 0, indice_count);
}

void Geometry::setShape(const ShapePreset preset){
	assert(preset < NUMBER_OF_SHAPE_PRESET);

	std::vector<float> new_data;
	unsigned int new_indice_count;
    get_shape(preset, new_data, new_indice_count);

	if(new_indice_count == indice_count){
		glNamedBufferSubData(vbo, 0, new_data.size() * sizeof(float), new_data.data());

	}else{
		glDeleteBuffers(1, &vbo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, new_data.size() * sizeof(float), new_data.data(), GL_STATIC_DRAW);

		constexpr unsigned int position_component = 3;
		constexpr unsigned int normal_component = 3;
		constexpr unsigned int data_component = position_component + normal_component;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, position_component, GL_FLOAT, GL_FALSE, data_component * sizeof(float), (void*) 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, normal_component, GL_FLOAT, GL_FALSE, data_component * sizeof(float), (void*) (position_component * sizeof(float)));

		glBindVertexArray(0);

		indice_count = new_indice_count;
	}
}

void Geometry::setShape(const std::vector<float>& data){

	constexpr unsigned int data_component = 6;
	assert(data.size() % data_component == 0);

	if(data.size() == (indice_count * data_component)){
		glNamedBufferSubData(vbo, 0, data.size() * sizeof(float), data.data());

	}else{
		glDeleteBuffers(1, &vbo);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

		constexpr unsigned int position_component = 3;
		constexpr unsigned int normal_component = 3;
		constexpr unsigned int data_component = position_component + normal_component;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, position_component, GL_FLOAT, GL_FALSE, data_component * sizeof(float), (void*) 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, normal_component, GL_FLOAT, GL_FALSE, data_component * sizeof(float), (void*) (position_component * sizeof(float)));

		glBindVertexArray(0);

		indice_count = data.size() / data_component;
	}
}

void Geometry::clear(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	indice_count = 0;
	vao = 0;
	vbo = 0;
}

Collection::Collection(){
}

Collection::~Collection(){
	glDeleteBuffers(1, &instance_vbo);
}

void Collection::draw(){
	glDrawArraysInstanced(GL_TRIANGLES, 0, Geometry::indice_count, instance_count);
}

void Collection::setPosition(const std::vector<glm::vec3>& position_data){

	if(position_data.size() == instance_count){
		glNamedBufferSubData(instance_vbo, 0, position_data.size() * sizeof(glm::vec3), position_data.data());

	}else{
		glDeleteBuffers(1, &instance_vbo);
		glGenBuffers(1, &instance_vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
		glBufferData(GL_ARRAY_BUFFER, position_data.size() * sizeof(glm::vec3), position_data.data(), GL_STATIC_DRAW);

		constexpr unsigned int instance_position_component = 3;

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, instance_position_component, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(0);

		instance_count = position_data.size();
	}
}


void Collection::clear(){
	glDeleteBuffers(1, &instance_vbo);

	instance_count = 0;
	instance_vbo = 0;

	Geometry::clear();
}

ValuedCollection::ValuedCollection(){
}

ValuedCollection::~ValuedCollection(){
	glDeleteBuffers(1, &instance_value_vbo);
}

void ValuedCollection::setPosition(const std::vector<glm::vec3>& position_data){
	assert(position_data.size() == instance_count);

	glNamedBufferSubData(instance_vbo, 0, position_data.size() * sizeof(glm::vec3), position_data.data());

}

void ValuedCollection::setValue(const std::vector<float>& value_data){
	assert(value_data.size() == instance_count);

	glNamedBufferSubData(instance_value_vbo, 0, value_data.size() * sizeof(float), value_data.data());
}

void ValuedCollection::setPositionValue(const std::vector<glm::vec3>& position_data, const std::vector<float>& value_data){

	assert(position_data.size() == value_data.size());

	if(value_data.size() == instance_count){
		glNamedBufferSubData(instance_value_vbo, 0, value_data.size() * sizeof(float), value_data.data());

	}else{
		glDeleteBuffers(1, &instance_value_vbo);
		glGenBuffers(1, &instance_value_vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, instance_value_vbo);
		glBufferData(GL_ARRAY_BUFFER, value_data.size() * sizeof(float), value_data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*) 0);
		glVertexAttribDivisor(3, 1);

		glBindVertexArray(0);
	}

	// not updating instance_count now because we need the old value in Collection::SetPosition()

	Collection::setPosition(position_data);
}

void ValuedCollection::clear(){
	glDeleteBuffers(1, &instance_value_vbo);

	instance_value_vbo = 0;

	Collection::clear();
}
*/
