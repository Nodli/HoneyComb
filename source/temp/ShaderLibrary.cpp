#include "ShaderLibrary.h"

const std::string Header::Version450(){
	return R"(
	#version 450
	)";
}

const std::string Vertex::PositionNormal(){
	return R"(

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;

	layout(location = 0) uniform mat4 u_matrix_mvp;

	out vec3 interp_color;

	void main(){
		gl_Position = u_matrix_mvp * vec4(in_position, 1.);
		interp_color = abs(in_normal);
	}

	)";
}

const std::string Vertex::InstancePositionNormal(){
	return R"(

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec3 in_instance_position;

	layout(location = 0) uniform mat4 u_matrix_mvp;

	out vec3 interp_color;

	void main(){
		gl_Position = u_matrix_mvp * vec4(in_position + in_instance_position, 1.);
		interp_color = abs(in_normal);
	}

	)";
}

const std::string Vertex::InstancePositionNormalValue(){
	return R"(

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec3 in_instance_position;
	layout(location = 3) in float in_instance_value;

	layout(location = 0) uniform mat4 u_matrix_mvp;

	out vec3 interp_color;

	void main(){

		vec4 valued_position = vec4(in_position + in_instance_position, 1.);
		valued_position.y *= in_instance_value;

		gl_Position = u_matrix_mvp * valued_position;

		//interp_color = abs(in_normal);
        interp_color = abs(in_normal) * dot(vec3(1., 1., 1.), in_normal);
	}

	)";
}

const std::string Vertex::ScreenSquare(){
    return R"(

    out vec2 texture_coord;

    void main(){

        // 0, 1, 2, 3, 4, 5 -> 0, 1, 2, 3, 2, 1
        int square_vertex = gl_VertexID - (gl_VertexID / 3) * (2 * gl_VertexID - 6);

        gl_Position = vec4(-1.0 + (square_vertex & 1) * 2.0,
                            -1.0 + (square_vertex & 2) * 2.0,
                            0.0,
                            1.0);
        texture_coord = vec2(0.0 + float(square_vertex & 1),
                            0.0 + float(square_vertex & 2));
    }

    )";
}

const std::string Fragment::Color(){
	return R"(

	in vec3 interp_color;

	out vec4 out_color;

	void main(){

		out_color = vec4(interp_color, 0.25);
	}

	)";
}

const std::string Fragment::TextureSample(){
	return R"(

    layout(location = 0) uniform sampler2D texture_data;

	in vec2 texture_coord;

	out vec4 out_color;

	void main(){

		out_color = vec4(texture(texture_data, texture_coord));
	}

	)";
}

const std::string Fragment::HorizontalBloomTexture(){
	return R"(

    layout(location = 0) uniform sampler2D texture_data;
    uniform float bloom_weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

	in vec2 texture_coord;

	out vec4 out_color;


	void main(){

        float texel_size = 1.0 / textureSize(texture_data, 0).x;

		out_color = vec4(texture(texture_data, texture_coord) * bloom_weights[0]);

        for(uint isample = 1; isample != 5; ++isample){
            out_color += vec4(texture(texture_data, vec2(texture_coord.x + isample * texel_size,
                                                        texture_coord.y))

                                * bloom_weights[isample]);
            out_color += vec4(texture(texture_data, vec2(texture_coord.x - isample * texel_size,
                                                        texture_coord.y))

                                * bloom_weights[isample]);
        }

	}

	)";

}

const std::string Fragment::VerticalBloomTexture(){
	return R"(

    layout(location = 0) uniform sampler2D texture_data;
    uniform float bloom_weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

	in vec2 texture_coord;

	out vec4 out_color;


	void main(){

        float texel_size = 1.0 / textureSize(texture_data, 0).y;

		out_color = vec4(texture(texture_data, texture_coord) * bloom_weights[0]);

        for(uint isample = 1; isample != 5; ++isample){
            out_color += vec4(texture(texture_data, vec2(texture_coord.x,
                                                        texture_coord.y + isample * texel_size))

                                * bloom_weights[isample]);
            out_color += vec4(texture(texture_data, vec2(texture_coord.x,
                                                        texture_coord.y - isample * texel_size))

                                * bloom_weights[isample]);
        }

	}

	)";
}

void GetShaderPreset(const ProgramPreset preset,
		std::string& header_code, std::string& vertex_code, std::string& fragment_code){

	switch(preset){
		case MESH :
			header_code = Header::Version450();
			vertex_code = Vertex::PositionNormal();
			fragment_code = Fragment::Color();
			break;

		case MESH_INSTANCE :
			header_code = Header::Version450();
			vertex_code = Vertex::InstancePositionNormal();
			fragment_code = Fragment::Color();
			break;

		case MESH_INSTANCE_VALUE :
			header_code = Header::Version450();
			vertex_code = Vertex::InstancePositionNormalValue();
			fragment_code = Fragment::Color();
			break;
	}
}
