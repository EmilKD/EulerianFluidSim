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
	cell* right{ nullptr }, * left{ nullptr }, * up{ nullptr }, * down{ nullptr };
};

class Grid
{
	
public:
	Grid(int window_res_x, int window_res_y);
	array<int, 2> getSize();
	//vector<vector<cell>>* getCells();
	vector<vector<cell>> cells;
	void render(GraphicalObj* gobj, float &scale_x, float &scale_y);
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
	const float gridSize = 0.002;
	vector<cell*> cellPtrs;
	float simgridCount_x, simgridCount_y;
	Colors color;
	int substeps{ 40 };
	int sampleCelli, sampleCellj;
	double ndt{ 0 }, d;
	float de{1.0}, cp;
	float density{ 1000.0f };
	float overRelaxation{ 1.9f };
	float simulationTime{ 0.0f };
	float w00, w10, w01, w11, x, y;
	float sample_u{ 0.0f }, sample_v{ 0.0f };
	float avgV{ 0.0f }, avgU{ 0.0f };
	cell* thisCell{ nullptr };
	cell* c0 = nullptr, * c1 = nullptr, * c2 = nullptr, * c3 = nullptr, * c4 = nullptr, * c5 = nullptr, * c6 = nullptr, * c7 = nullptr;
	glm::vec2 samplePos{ glm::vec2(0.0f, 0.0f) }, sampleVels{ glm::vec2(0.0f, 0.0f) };
};

struct circularObj
{
	float x, y;
	float radius;
	vector<cell*> cells;
	\
	circularObj(float radius, float x, float y, Grid* grid)
	{
		this->radius = radius; this->x = x; this->y = y;

		for (int i = 0; i < grid->gridCount_x - 1; i++)
		{
			for (int j = 0; j < grid->gridCount_y - 1; j++)
			{
				if (std::sqrt(std::pow(grid->cells[i][j].pos.x - x, 2) + std::pow(grid->cells[i][j].pos.y - y, 2)) <= radius)
				{
					this->cells.push_back(&grid->cells[i][j]);
				}
			}
		}
	}

};