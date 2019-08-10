#include "HexGrid.h"

bool glm::operator<(const glm::ivec3& lhs, const glm::ivec3& rhs){
	return (lhs.x < rhs.x) || (lhs.x == rhs.x && (lhs.y < rhs.y || (lhs.y == rhs.y && lhs.z < rhs.z)));
}
