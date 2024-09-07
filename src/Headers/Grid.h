#pragma once

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<vector>
#include<array>
#include "Graphics.h"

using std::vector, std::array;

struct cell
{
	glm::vec3 pos{0.0f, 0.0f, 0.0f};
	int id;
	glm::vec2 vel{ glm::vec2(0.0f, 0.0f) };
	float density{1.0f};
	int s{ 1 };
	double p{ 0.0f };
};

class Grid
{
	
public:
	Grid(int window_res_x, int window_res_y);
	array<int, 2> getSize();
	vector<vector<cell>> getCells();
	void simulate(GraphicalObj* gobj, float &scale_x, float &scale_y, double dt);
	cell* getCellByID(int id);
	void Project(int &cellx, int &celly, double &dt);
	float sampleGrid(float x, float y);

	float xC(float worldx) {
		return 2 * (worldx / worldSize_x - 0.5);
	}

	float yC(float worldy) {
		return 2 * (worldy / worldSize_y - 0.5);
	}

	int gridCount_x;
	int gridCount_y;

private:
	float worldSize_x = 0.4; //m
	float worldSize_y = 0.3;
	const float gridSize = 0.004;
	vector<vector<cell>> cells;
	vector<vector<float>> u, v;
	vector<cell*> cellPtrs;
	float simgridCount_x, simgridCount_y;
	Colors color;
};

