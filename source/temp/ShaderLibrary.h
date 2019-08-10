#ifndef H_SHADER_LIBRARY
#define H_SHADER_LIBRARY

#include <string>

enum ProgramPreset {MESH, MESH_INSTANCE, MESH_INSTANCE_VALUE, NUMBER_OF_PROGRAM_PRESET};

namespace Header{
	const std::string Version450();
}

namespace Vertex{
	const std::string PositionNormal();
	const std::string InstancePositionNormal();
	const std::string InstancePositionNormalValue();
    const std::string ScreenSquare();
}

namespace Fragment{
	const std::string Color();
    const std::string TextureSample();
    const std::string HorizontalBloomTexture();
    const std::string VerticalBloomTexture();
}

void GetShaderPreset(const ProgramPreset preset,
    std::string& header_code, std::string& vertex_code, std::string& fragment_code);


#endif
