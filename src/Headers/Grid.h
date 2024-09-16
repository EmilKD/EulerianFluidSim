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
	float u{ 0.0f };
	float v{ 0.0f};
	float newU{ 0.0f };
	float newV{ 0.0f };
	float m{ 0.0f };
	float newM{ 0.0f };
	int s{ 1 };
	double p{ 0.0f };
};

class Grid
{
	
public:
	Grid(int window_res_x, int window_res_y);
	array<int, 2> getSize();
	vector<vector<cell>> getCells();
	void render(GraphicalObj* gobj, float &scale_x, float &scale_y);
	cell* getCellByID(int id);
	void project(double dt);
	void advectVelocity(double dt);
	void advectSmoke(double dt);
	void simulate(double dt);
	void extrapolate();

	glm::vec2 sampleVelocity(glm::vec2 &samplePos);
	float sampleDensity(glm::vec2& samplePos);

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
	vector<cell*> cellPtrs;
	float simgridCount_x, simgridCount_y;
	Colors color;
	int substeps{ 40 };
	double ndt{ 0 };
	float de{1.0};
	float density{ 1000.0f };
	float overRelaxation{ 1.9f };
	float simulationTime{ 0.0f };
};

