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

	int id, s{ 1 };

	float u{ 0.0f };
	float v{ 0.0f};
	float newU{ 0.0f };
	float newV{ 0.0f };
	float m{ 0.0f };
	float newM{ 0.0f };

	double p{ 0.0f };

	int id_right{ 0 }, id_left{ 0 }, id_up{ 0 }, id_down{ 0 };
	vector<int>cellParticleIds;
};

struct particle
{
	glm::vec2 pos;
	glm::vec2 vel;
	glm::vec3 color;
	int cellid;
};

struct CircularObj;

class Fluid
{
	
public:
	Fluid(const float& Size_x, const float& Size_y);	
	void render(GraphicalObj* gobj, const float &renderScale_x, const float &renderScale_y);
	void project(double dt);
	void advectVelocity(double dt);
	void advectSmoke(double dt);
	void simulate(double dt);
	void extrapolate();
	void AddObstacle(CircularObj* obj);
	void UpdateObstacle(int id);
	void UpdatePosBuffer(int idx);
	void UpdateColorBuffer(int idx);
	void InitializeGraphics(const Shader& shader);
	
	void ParticleInit();
	void SimulateParticles(const float& dt);

	glm::vec2 sampleVelocity(const glm::vec2 &samplePos);
	float sampleDensity(const glm::vec2& samplePos);

	float xC(float worldx) {
		return 2 * (worldx / worldSize_x - 0.5);
	}

	float yC(float worldy) {
		return 2 * (worldy / worldSize_y - 0.5);
	}

public:
	static constexpr float gridSize = 0.003; // meters
	const int gridCount_x;
	const int gridCount_y;
	vector<cell> cells;
	float InletVel[2]{ 0.2f, 0.f };

private:
	const float worldSize_x;
	const float worldSize_y;
	vector<cell*> cellPtrs;
	vector<CircularObj*> Obstacles{};
	vector<uint32_t> IterHeight, IterWidth, IterIndices, IterSubSteps;
	vector<particle> particles;
	vector<int> cellParticleIds;
	const float particleSize{ 0.1f };
	int particleCount{ 0 };

	// Creating the grid graphical object
	const int ArraySize{ gridCount_x * gridCount_y };
	vector<float> PositionBuffer;
	vector<float> ColorBuffer;
	GLuint PositionVBO, ColorVBO, VAO;

	Colors color;
	int substeps{ 50 };
	double ndt{ 0 }, d;
	float de{1.0}, cp;
	float density{ 1000.0f };
	float overRelaxation{ 1.9f };
	float simulationTime{ 0.0f };
	float avgV{ 0.0f }, avgU{ 0.0f };
};

struct CircularObj
{
	float x{ 0.0f }, y{ 0.0f };
	float radius{ 0.05f };
	float u{ 0.f }, v{ 0.f };
	vector<cell*> cells{};
	Fluid* FluidGrid{ nullptr };

	CircularObj(const float x, const float y, const float radius, Fluid* grid)
	{
		FluidGrid = grid;
		this->radius = radius; this->x = x; this->y = y;

		for (int i = 1; i < grid->gridCount_x - 1; i++)
		{
			for (int j = 1; j < grid->gridCount_y - 1; j++)
			{
				if (std::sqrt(std::pow(grid->cells[i + j * grid->gridCount_x].pos.x - x, 2) + std::pow(grid->cells[i + j*grid->gridCount_x].pos.y - y, 2)) <= radius)
				{
					this->cells.push_back(&grid->cells[i + j * grid->gridCount_x]);
				}
			}
		}
	}

	void Update(const float x, const float y, const float radius, const float u, const float v)
	{
		this->radius = radius; this->x = x; this->y = y;
		for (cell* c : cells)
			c->s = 1;
		
		cells.clear();
		for (int i = int((x - radius) / FluidGrid->gridSize); i < int((x + radius) / FluidGrid->gridSize); i++)
		{
			for (int j = int((y - radius) / FluidGrid->gridSize); j < int((y + radius) / FluidGrid->gridSize); j++)
			{
				const int& idx = i + j * FluidGrid->gridCount_x;
				if (std::sqrt(std::pow(FluidGrid->cells[idx].pos.x - x, 2) + std::pow(FluidGrid->cells[idx].pos.y - y, 2)) <= radius)
				{
					this->cells.push_back(&FluidGrid->cells[idx]);
				}
			}
		}
	}

};