#include "ColorMap.h"
#include "log.h"

ColorMap::ColorMap(const std::vector<glm::vec3>& ivalues)
: values(ivalues){
}

ColorMap::~ColorMap(){
	if(buffered){
		DeleteTexture();
	}
}

glm::vec3 ColorMap::Sample(float sample_coord) const{
	if(values.size() == 0){
		return {0.f, 0.f, 0.f};

	}else if(values.size() == 1 || sample_coord <= 0.f){
		return values[0];

	}else if(sample_coord >= 1.f){
		return values[values.size() - 1];

	}else{
		float internal_sample_coord = sample_coord * (values.size() - 1);

		unsigned int sample_index = internal_sample_coord;
		float sample_interpolator = internal_sample_coord - sample_index;
		float sample_complement = 1.f - sample_interpolator;

		return values[sample_index] * sample_complement + values[sample_index + 1] * sample_interpolator;
	}
}

void ColorMap::CreateTexture(){

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, values.size() * sizeof(glm::vec3), 0, GL_RGB, GL_FLOAT, values.data());

	buffered = true;
}

void ColorMap::DeleteTexture(){
	if(buffered){
		glDeleteTextures(1, &texture);
		buffered = false;
	}else{
		log("error in ColorMap::delete_texture : texture is not buffered");
	}
}

void ColorMap::Bind(const unsigned int texture_unit){
//https://community.khronos.org/t/when-to-use-glactivetexture/64913

	if(buffered){
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_1D, texture);
	}else{
		log("error in ColorMap::bind : texture is not buffered");
	}
}

void ColorMap::Release(){
	glBindTexture(GL_TEXTURE_1D, GL_TEXTURE0);
}

// float template specialization

