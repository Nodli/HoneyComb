#ifndef H_HONEYCOMB
#define H_HONEYCOMB

#include <map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

// Requierement to use an std::map<glm::ivec3>
namespace glm{
	bool operator<(const glm::ivec3& lhs, const glm::ivec3& rhs);
}

// Constants defined for easy access to neighbor cells
enum HexNeighbor {TOP_RIGHT, RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, LEFT, TOP_LEFT};
constexpr glm::ivec3 neighbor_coord[6] = {{1, 0, -1}, // top-right
    {1, -1, 0}, // right
    {0, -1, 1}, // bottom-right
    {-1, 0, 1}, // bottom-left
    {-1, 1, 0}, // left
    {0, 1, -1}}; // top-left
constexpr glm::ivec3 side_coord[6] = {{0, -1, 1}, // top-right -> right
    {-1, 0, 1}, // right -> bottom-right
    {-1, 1, 0}, // bottom-right -> bottom-left
    {0, 1, -1}, // bottom-left -> left
    {1, 0, -1}, // left -> top-left
    {1, -1, 0}}; // top-left -> top-right

// Converts {x, y, z} hexagonal cube grid coordinates to {x, y} hexagonal
// square grid coordinates and vice-versa
inline glm::ivec2 cube_to_square(const glm::ivec3 cube_coord);
inline glm::ivec3 square_to_cube(const glm::ivec2 square_coord);

// Computes the 2D world coordinates of the center of the hexagonal cell at
// given {x, y, z} hexagonal cube coordinates
inline glm::vec2 cube_to_world(const glm::ivec3 cube_coord, const float width, const float height);

// Manhattan distance on the Hexgrid between two coordinates
inline unsigned int cube_manhattan_distance(const glm::ivec3 A, const glm::ivec3 B);

template<typename T>
struct HexCell{
	glm::ivec3 cube_coord = {0, 0, 0};
	HexCell* neighbors[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    T* value = nullptr;
};

template<typename T>
struct HexGrid{

    // ---- constructor ---- //

	HexGrid();
	~HexGrid();

    // ---- storage related ---- //

    // Layers of cells around a central cell at {0, 0, 0}
    // The allocation follows a clockwise spiral with smaller layers allocated
    // first and starting from the top right cell of each layer
	void buildLayered(unsigned int number_of_layers);

    // A sheared 2D grid with the origin cell at the bottom left with coordinates {0, 0, 0}
    // The grid allocates cells in scanlines from left to right contiguously and
    // from bottom to top
	void buildGridScanline(unsigned int width, unsigned int height);

    // Update the /neighbors/ of each HexCell in the HexGrid
    void updateNeighbors();

    // ---- accessors ---- //

    // Returns the HexCell at /cube_coord/ if it exists or nullptr
    HexCell<T>* searchCell(const glm::ivec3 cube_coord);
    inline HexCell<T>* searchCell(const int x, const int y, const int z);

    // ---- data ---- //

    unsigned int number_of_cells = 0;
    HexCell<T>* grid_cells = nullptr;
    T* grid_values = nullptr;

	std::map<glm::ivec3, HexCell<T>*> coord_to_cell;
};

#include "HexGrid.inl"

#endif
