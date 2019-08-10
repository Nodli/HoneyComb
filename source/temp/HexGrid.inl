#ifndef INL_HONEYCOMB
#define INL_HONEYCOMB

#include <cmath>

glm::ivec2 cube_to_square(const glm::ivec3 hex_coord){
	return {hex_coord.x + (hex_coord.z - (hex_coord.z & 1)) / 2, hex_coord.z};
}

glm::ivec3 square_to_cube(const glm::ivec2 square_coord){
	glm::ivec3 hex;
	hex.x = square_coord.x - (square_coord.y - (square_coord.y & 1)) / 2;
	hex.y = square_coord.y;
	hex.z = - hex.x - hex.y;

	return hex;
}

glm::vec2 cube_to_world(const glm::ivec3 cube_coord, const float width, const float height){
    const float delta_vertical = 0.75 * height;
    const float delta_horizontal = std::sqrt(3.f) / 2.f * width;
    const float delta_diagonal = delta_horizontal / 2.f;

    const glm::ivec2 plane_coord = cube_to_square(cube_coord);

    return {plane_coord.x * delta_horizontal + (int)(plane_coord.y % 2 != 0) * delta_diagonal,
            plane_coord.y * delta_vertical};
}

unsigned int cube_manhattan_distance(const glm::ivec3 A, const glm::ivec3 B){
    return (std::abs(A.x - B.x) + std::abs(A.y - B.y) + std::abs(A.z - B.z)) / 2;
    //return std::max(std::max(std::abs(A.x - B.x), std::abs(A.y - B.y)), std::abs(A.z - B.z));
}

template<typename T>
HexGrid<T>::HexGrid(){
}

template<typename T>
HexGrid<T>::~HexGrid(){
}

template<typename T>
void HexGrid<T>::buildLayered(unsigned int number_of_layers){

    // compute the number of cells
	number_of_cells = 0;
	for(unsigned int layer = 0; layer != number_of_layers; ++layer){
		number_of_cells += (layer + 1);
	}
	number_of_cells = 1 + 6 * number_of_cells;

    // free old memory
    if(grid_cells){
        delete[] grid_cells;
    }
    if(grid_values){
        delete[] grid_values;
    }

    // allocate new memory
    grid_cells = new HexCell<T>[number_of_cells];
    grid_values = new T[number_of_cells];

	// update the cells coordinates
	grid_cells[0].cube_coord = {0, 0, 0};
    grid_cells[0].value = grid_values;
	unsigned int cell_offset = 1;

	for(unsigned int layer = 1; layer != number_of_layers + 1; ++layer){
		glm::ivec3 layer_coord = {layer, 0, -layer};

		// top-right to right
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.y -= 1;
			layer_coord.z += 1;

			++cell_offset;
		}

		// right to bottom-right
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.x -= 1;
			layer_coord.z += 1;

			++cell_offset;
		}

		// bottom-right to bottom-left
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.x -= 1;
			layer_coord.y += 1;

			++cell_offset;
		}

		// bottom-left to left
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.y += 1;
			layer_coord.z -= 1;

			++cell_offset;
		}

		// left to top-left
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.x += 1;
			layer_coord.z -= 1;

			++cell_offset;
		}

		// top-left to top-right
		for(unsigned int side_cell = 0; side_cell != layer; ++side_cell){
			grid_cells[cell_offset].cube_coord = layer_coord;
            grid_cells[cell_offset].value = grid_values + cell_offset;

			layer_coord.x += 1;
			layer_coord.y -= 1;

			++cell_offset;
		}
	}


    updateNeighbors();
}

template<typename T>
void HexGrid<T>::buildGridScanline(unsigned int width, unsigned int height){

	// update the allocation
	number_of_cells = width * height;

    // deallocate old memory
    if(grid_cells){
        delete[] grid_cells;
    }
    if(grid_values){
        delete[] grid_values;
    }

    // allocate new memory
    grid_cells = new HexCell<T>[number_of_cells];
    grid_values = new T[number_of_cells];

    // update the cells coordinates
	// cube offset of a square row = (row / 2) * (1, 1, -2) + (row % 2) * (1, 0, -1);
	// cube offset of a square column = column * (1, -1, 0)
	for(unsigned int row = 0; row != height; ++row){
		unsigned int double_step = row / 2;
		unsigned int odd_row = row % 2;

		for(unsigned int column = 0; column != width; ++column){
            const unsigned int cell_offset = row * width + column;

			grid_cells[cell_offset].cube_coord.x = double_step + odd_row + column;
			grid_cells[cell_offset].cube_coord.y = double_step - column;
			grid_cells[cell_offset].cube_coord.z = - 2 * double_step - odd_row;

            grid_cells[cell_offset].value = grid_values[cell_offset];
		}
	}

    updateNeighbors();
}

template<typename T>
void HexGrid<T>::updateNeighbors(){

	// adding cells to the map
	for(unsigned int icell = 0; icell != number_of_cells; ++icell){
		coord_to_cell.emplace(grid_cells[icell].cube_coord, &grid_cells[icell]);
	}

	// updating neighbors
	for(unsigned int icell = 0; icell != number_of_cells; ++icell){

		// top-right
        glm::ivec3 neighbor = {grid_cells[icell].cube_coord.x + 1,
            grid_cells[icell].cube_coord.y,
            grid_cells[icell].cube_coord.z - 1};
        typename std::map<glm::ivec3, HexCell<T>*>::iterator neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[0] = neighbor_iterator->second;
		}

		// right
		neighbor.y -= 1;
		neighbor.z += 1;
		neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[1] = neighbor_iterator->second;
		}

		// bottom-right
		neighbor.x -= 1;
		neighbor.z += 1;
		neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[2] = neighbor_iterator->second;
		}

		// bottom-left
		neighbor.x -= 1;
		neighbor.y += 1;
		neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[3] = neighbor_iterator->second;
		}

		// left
		neighbor.y += 1;
		neighbor.z -= 1;
		neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[4] = neighbor_iterator->second;
		}

		// top-left
		neighbor.x += 1;
		neighbor.z -= 1;
		neighbor_iterator = coord_to_cell.find(neighbor);
		if(neighbor_iterator != coord_to_cell.end()){
			grid_cells[icell].neighbors[5] = neighbor_iterator->second;
		}
	}
}

template<typename T>
HexCell<T>* HexGrid<T>::searchCell(const glm::ivec3 cube_coord){
    typename std::map<glm::ivec3, HexCell<T>*>::iterator cell_iterator = coord_to_cell.find(cube_coord);

    if(cell_iterator != coord_to_cell.end()){
        return cell_iterator->second;
    }else{
        return nullptr;
    }
}

template<typename T>
HexCell<T>* HexGrid<T>::searchCell(const int x, const int y, const int z){
    glm::ivec3 cube_coord(x, y, z);
    searchCell(cube_coord);
}

#endif
