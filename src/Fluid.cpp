#include<iostream>
#include<random>
#include<functional>
#include "Fluid.h"
#include<math.h>
#include<algorithm>
#include<execution>

bool randomBool() {
	static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
	return gen();
}

Fluid::Fluid(const float& Size_x, const float& Size_y) : 
	gridCount_x{ static_cast<int>(Size_x / gridSize) }, 
	gridCount_y{ static_cast<int>(Size_y / gridSize) }, 
	worldSize_x{ Size_x }, worldSize_y{ Size_y }
{
	cell initialCell;
	initialCell.m = 0.001f;
	initialCell.u = 2.0f;
	initialCell.v = 0;
	
	cells.resize(gridCount_x * gridCount_y, initialCell);

	for (int i{ 0 }; i < gridCount_x; i++)
	{
		for (int j{ 0 }; j < gridCount_y; j++)
		{
			cell* const thisCell = &cells[i + j * gridCount_x];
			thisCell->pos = glm::vec3(
				worldSize_x*(i + 0.5f)/gridCount_x,
				worldSize_y*(j + 0.5f)/gridCount_y,
				0.0f
			);

			if (i != 0 && i != gridCount_x - 1 && j != 0 && j != gridCount_y - 1)
			{
				thisCell->id_right = i + 1 + j * gridCount_x;
				thisCell->id_left = i - 1 + j * gridCount_x;
				thisCell->id_up = i + (j + 1) * gridCount_x;
				thisCell->id_down = i + (j - 1) * gridCount_x;
			}

			thisCell->s = 1;

			// Boundary conditions
			if (i == 0 || i == gridCount_x - 1 || j == 0 || j == gridCount_y - 1) {
				thisCell->s = 0;
				thisCell->u = 0;
				thisCell->v = 0;
			}
			/*else if (i >= gridCount_x * 2 / 5 && i <= gridCount_x * 3 / 5 && j >= gridCount_y * 2 / 5 && j <= gridCount_y * 3 / 5) { 
				thisCell->s = 0;
			}*/

			/*if (i==0)
			{
				thisCell->u = InletVel[0];
				thisCell->v = InletVel[1];
			}*/
			cellPtrs.push_back(thisCell);
		}
	}

	ParticleInit();

	IterWidth.resize(gridCount_x-2);
	for (size_t i = 1; i < gridCount_x-1; i++)
		IterWidth.at(i-1) = i;

	IterHeight.resize(gridCount_y-2);
	for (size_t i = 1; i < gridCount_y-1; i++)
		IterHeight.at(i-1) = i;

	IterSubSteps.resize(substeps);
	for (size_t i = 1; i < substeps; i++)
		IterSubSteps.at(i) = i;

	const int spacing = 4;
	IterIndices.resize((gridCount_x - 2) * (gridCount_y - 2));
	for (size_t s = 0; s < spacing; s++)
		for (size_t p = 0; p < spacing; p++)
			for (size_t j = 1 + s; j < gridCount_y - 1; j += spacing)
				for (size_t i = 1 + p; i < gridCount_x - 1; i += spacing)
					IterIndices[i - 1 + (j - 1) * (gridCount_x - 2)] = i + j * gridCount_x;
				
	if ((gridCount_x - 2) * (gridCount_y - 2) != IterIndices.size())
	{
		printf("Iterator size: %i\n", int(IterIndices.size()));
		printf("correct size: %i\n", (gridCount_x - 2) * (gridCount_y - 2));
		throw std::invalid_argument("iterator construction failed");
	}
	/*else
	{
		for (size_t i : IterIndices)
		{
			printf("%i\n", i);
		}
	}*/
}

void Fluid::InitializeGraphics(const Shader& shader)
{
	// Buffers
	PositionBuffer.resize(ArraySize * 2);
	ColorBuffer.resize(ArraySize * 3);

	for (size_t i = 0; i < ArraySize / 2; i++)
		UpdatePosBuffer(i);
	
	for (size_t i = 0; i < ArraySize / 2; i++)
		UpdateColorBuffer(i);
	
	// Create and bind VAO
	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	// Position VBO
	glGenBuffers(1, &PositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
	glBufferData(GL_ARRAY_BUFFER, PositionBuffer.size() * sizeof(float), PositionBuffer.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	// Color VBO
	glGenBuffers(1, &ColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ColorVBO);
	glBufferData(GL_ARRAY_BUFFER, ColorBuffer.size() * sizeof(float), ColorBuffer.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	// Unbind VAO
	glBindVertexArray(0);

	shader.setFloat("pointSize", 5.f);
}

void Fluid::UpdatePosBuffer(int idx)
{
	const particle& p = particles[idx];

	PositionBuffer[idx * 2] = xC(p.pos.x);
	PositionBuffer[idx * 2 + 1] = yC(p.pos.y);
}

void Fluid::UpdateColorBuffer(int idx)
{
	const particle& p = particles[idx];

	ColorBuffer[idx * 3] = p.color.x;
	ColorBuffer[idx * 3 + 1] = p.color.y;
	ColorBuffer[idx * 3 + 2] = p.color.z;

	/*ColorBuffer[idx * 3] = 0.f;
	ColorBuffer[idx * 3 + 1] = abs(p.vel.y * 10);
	ColorBuffer[idx * 3 + 2] = abs(p.vel.x * 10);*/
}

void Fluid::AddObstacle(CircularObj* obj)
{
	this->Obstacles.push_back(obj);
	UpdateObstacle(Obstacles.size() - 1);
}

void Fluid::UpdateObstacle(int id)
{
	const CircularObj* obstacle = Obstacles.at(id);
	for (cell* c : obstacle->cells)
	{
		c->s = 0;
		c->u = sin((c->pos.x - obstacle->x) / obstacle->radius) * obstacle->u;
		c->v = cos((c->pos.y - obstacle->y) / obstacle->radius) * obstacle->v;
		c->m = 0;
	}
}

void Fluid::project(double dt) {
	cp = this->density * gridSize / dt;

	std::for_each(std::execution::par_unseq, IterSubSteps.begin(), IterSubSteps.end(), [this](uint8_t n) 
	{
		std::for_each(std::execution::par_unseq, IterIndices.begin(), IterIndices.end(), [this](uint32_t i)
		{
			double p{ 0 };
			cell* const thisCell = &cells[i];

			if (thisCell->s == 0)
				return;

			const double d = cells[thisCell->id_right].u - thisCell->u + cells[thisCell->id_up].v - thisCell->v;
			const int s = cells[thisCell->id_down].s + cells[thisCell->id_up].s + cells[thisCell->id_left].s + cells[thisCell->id_right].s;

			if (s == 0) // Obstacle velocity should be added
				return;

			p = -d / s;
			p *= overRelaxation;

			thisCell->p += p * cp;

			thisCell->u -= p * cells[thisCell->id_left].s;
			cells[thisCell->id_right].u += p * cells[thisCell->id_right].s;
			thisCell->v -= p * cells[thisCell->id_down].s;
			cells[thisCell->id_up].v += p * cells[thisCell->id_up].s;
		});
	});
}

void Fluid::advectVelocity(double dt) 
{
	std::for_each(std::execution::par_unseq, IterHeight.begin(), IterHeight.end(), [this, &dt](uint32_t j)
	{
		std::for_each(std::execution::par_unseq, IterWidth.begin(), IterWidth.end(), [this, &dt, &j](uint32_t i)
		{
			cell* const thisCell = &cells[i + j * gridCount_x];
			if (thisCell->s == 0)
			{
				return;
			}
			else
			{
				if (cells[thisCell->id_left].s == 1 && i < gridCount_x - 2)
				{
					avgV = (thisCell->v + cells[thisCell->id_left].v + cells[thisCell->id_up].v + cells[i - 1 + (j + 1) * gridCount_x].v) / 4.0f;

					const glm::vec2 samplePos{ thisCell->pos.x - gridSize / 2.0f - dt * thisCell->u, thisCell->pos.y - dt * avgV };
					thisCell->newU = sampleVelocity(samplePos).x;
				}
				if (cells[thisCell->id_down].s == 1 && j < gridCount_y - 2)
				{
					avgU = (thisCell->u + cells[thisCell->id_right].u + cells[thisCell->id_down].u + cells[i + 1 + (j - 1) * gridCount_x].u) / 4.0f;

					const glm::vec2 samplePos{ thisCell->pos.x - dt * avgU, thisCell->pos.y - gridSize / 2.0f - dt * thisCell->v };
					thisCell->newV = sampleVelocity(samplePos).y;
				}

				avgU = (thisCell->u + cells[thisCell->id_right].u) / 2.0f;
				avgV = (thisCell->v + cells[thisCell->id_up].v) / 2.0f;

				const glm::vec2 samplePos{ thisCell->pos.x - avgU * dt, thisCell->pos.y - avgV * dt };
				thisCell->newM = sampleDensity(samplePos);
			}
		});
	});

	std::for_each(std::execution::par_unseq, IterHeight.begin(), IterHeight.end(), [this](uint32_t j) 
	{
		std::for_each(std::execution::par_unseq, IterWidth.begin(), IterWidth.end(), [this, &j](uint32_t i)
		{
			cell& thisCell = cells[i + j * gridCount_x];
			
			thisCell.u = thisCell.newU;
			thisCell.v = thisCell.newV;
			thisCell.m = thisCell.newM;

			//UpdateColorBuffer(i + j * gridCount_x);
		});
	});
}


glm::vec2 Fluid::sampleVelocity(const glm::vec2 &samplePos) 
{
	const int sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 2);
	const int sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 2);

	const cell* sampleCell = &cells[sampleCelli + sampleCellj * gridCount_x];
	const cell* c0, * c1, * c2, * c3;
	// sampling v
	if (samplePos.x < sampleCell->pos.x)
	{
		c0 = &cells[sampleCelli - 1 + sampleCellj * gridCount_x];
		c1 = sampleCell;
		c2 = &cells[sampleCelli - 1 + (sampleCellj + 1) * gridCount_x];
		c3 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
	}
	else
	{
		c0 = sampleCell;
		c1 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
		c2 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
		c3 = &cells[sampleCelli + 1 + (sampleCellj + 1) * gridCount_x];
	}

	const float x = samplePos.x - c0->pos.x;
	const float y = samplePos.y - (c0->pos.y - gridSize / 2.0f);

	const float w00 = 1 - x / gridSize;
	const float w01 = x / gridSize;
	const float w10 = 1 - y / gridSize;
	const float w11 = y / gridSize;

	const float sample_v = w00 * w10 * c0->v + w01 * w10 * c1->v + w00 * w11 * c2->v + w01 * w11 * c3->v;

	// sampling u
	if (samplePos.y > cells[sampleCelli + sampleCellj * gridCount_x].pos.y)
	{
		c0 = sampleCell;
		c1 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
		c2 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
		c3 = &cells[sampleCelli + 1 + (sampleCellj + 1) * gridCount_x];
	}
	else
	{
		c0 = &cells[sampleCelli + (sampleCellj - 1) * gridCount_x];
		c1 = &cells[sampleCelli + 1 + (sampleCellj - 1) * gridCount_x];
		c2 = sampleCell;
		c3 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
	}

	const float x_u = samplePos.x - (c0->pos.x - gridSize / 2.0f);
	const float y_u = samplePos.y - c0->pos.y;

	const float w00_u = 1 - x_u / gridSize;
	const float w01_u = x_u / gridSize;
	const float w10_u = 1 - y_u / gridSize;
	const float w11_u = y_u / gridSize;

	const float sample_u = w00_u * w10_u * c0->u + w01_u * w10_u * c1->u + w00_u * w11_u * c2->u + w01_u * w11_u * c3->u;

	return glm::vec2(sample_u, sample_v);
}

float Fluid::sampleDensity(const glm::vec2& samplePos)
{
	float samplede{ 0.0f };

	const int sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 2);
	const int sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 2);
	
	cell* sampleCell = &cells[sampleCelli + sampleCellj * gridCount_x];

	const cell* c0 = &cells[sampleCelli - 1 + (sampleCellj - 1) * gridCount_x];
	const cell* c1 = &cells[sampleCelli - 1 + sampleCellj * gridCount_x];
	const cell* c2 = &cells[sampleCelli - 1 + (sampleCellj + 1) * gridCount_x];
	const cell* c3 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
	const cell* c4 = &cells[sampleCelli + 1 + (sampleCellj - 1) * gridCount_x];
	const cell* c5 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
	const cell* c6 = &cells[sampleCelli + 1 + (sampleCellj - 1) * gridCount_x];
	const cell* c7 = &cells[sampleCelli + (sampleCellj - 1) * gridCount_x];
	
	if (samplePos.x <= sampleCell->pos.x && samplePos.y >= sampleCell->pos.y)
	{
		c0 = &cells[sampleCelli - 1 + sampleCellj * gridCount_x];
		c1 = sampleCell;
		c2 = &cells[sampleCelli - 1 + (sampleCellj + 1) * gridCount_x];
		c3 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
	}
	else if (samplePos.x <= sampleCell->pos.x && samplePos.y < sampleCell->pos.y)
	{
		c0 = &cells[sampleCelli - 1 + (sampleCellj - 1) * gridCount_x];
		c1 = &cells[sampleCelli + (sampleCellj - 1) * gridCount_x];
		c2 = &cells[sampleCelli - 1 + sampleCellj * gridCount_x];
		c3 = sampleCell;	
	}
	else if (samplePos.x > sampleCell->pos.x && samplePos.y >= sampleCell->pos.y)
	{
		c0 = sampleCell;
		c1 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
		c2 = &cells[sampleCelli + (sampleCellj + 1) * gridCount_x];
		c3 = &cells[sampleCelli + 1 + (sampleCellj + 1) * gridCount_x];
	}
	else if (samplePos.x > sampleCell->pos.x && samplePos.y < sampleCell->pos.y)
	{
		c0 = &cells[sampleCelli + (sampleCellj - 1) * gridCount_x];
		c1 = &cells[sampleCelli + 1 + (sampleCellj - 1) * gridCount_x];
		c2 = sampleCell;
		c3 = &cells[sampleCelli + 1 + sampleCellj * gridCount_x];
	}
	else if (samplePos.x == sampleCell->pos.x && samplePos.y == sampleCell->pos.y)
	{
		return sampleCell->m;
	}

	const float x = samplePos.x - c0->pos.x;
	const float y = samplePos.y - c0->pos.y;

	const float w00 = 1 - x / gridSize;
	const float w01 = x / gridSize;
	const float w10 = 1 - y / gridSize;
	const float w11 = y / gridSize;

	return w00 * w10 * c0->m + w01 * w10 * c1->m + w00 * w11 * c2->m + w01 * w11 * c3->m;
}

// Not being used for now
void Fluid::advectSmoke(double dt) {
	float avgU{ 0.0f }, avgV{ 0.0f };
	glm::vec2 samplePos{ glm::vec2(0.0f, 0.0f) };

	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			cell* const thisCell = &cells[i + j * gridCount_x];
			if (thisCell->s != 0) {
				avgU = (thisCell->u + cells[thisCell->id_right].u) / 2.0f;
				avgV = (thisCell->v + cells[thisCell->id_right].v) / 2.0f;

				samplePos.x = thisCell->pos.x - avgU * dt;
				samplePos.y = thisCell->pos.y - avgV * dt;
				
				//cout << avgU * dt << endl;

				int sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 1);
				int sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 1);
			
				thisCell->newM = cells[sampleCelli + sampleCellj * gridCount_x].m;
			}
			
		}
	}
	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			cell* const thisCell = &cells[i + j * gridCount_x];
			if (thisCell->s != 0)
			{
				thisCell->m = thisCell->newM;
			}
		}
	}
}

void Fluid::extrapolate() 
{
	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			cell* const thisCell = &cells[i + j * gridCount_x];

			if (j==0)
			{
				thisCell->v = cells[thisCell->id_up].v;
			}
			else if (j==gridCount_y-1)
			{
				thisCell->v = cells[thisCell->id_down].v;
			}
			if (i==0)
			{
				thisCell->u = 2.0f;
			}
			else if (i == gridCount_x - 1)
			{
				thisCell->u = cells[thisCell->id_left].u;
			}
			
		}
	}
}

void Fluid::ParticleInit()
{
	for (size_t i = 0; i < int(ArraySize / 2.f); i++)
	{
		particles.push_back(particle(cells[2*i].pos, glm::vec2(0.f), glm::vec3(0.f), i));
		cells[i].cellParticleIds.push_back(i);
	}
	particleCount = particles.size();
}

void Fluid::SimulateParticles(const float& dt) 
{
	
	for (size_t i = 0; i < particleCount; i++)
	{
		const particle& p = particles[i];
		const int& idx = int(p.pos.x / gridSize) + int(p.pos.y / gridSize) * gridCount_x;
		cells[idx].cellParticleIds.push_back(i);
	}
	for (size_t i = 0; i < particleCount; i++)
	{
		particle& p = particles[i];
		p.vel.y += -9.87 * dt;
		p.pos += p.vel * dt;

		const vector<int>& CollisionIds = cells[p.cellid].cellParticleIds;

		const int& idx = int(p.pos.x / gridSize) + int(p.pos.y / gridSize) * gridCount_x;
		p.vel.x = cells[idx].u;
		p.vel.y = cells[idx].v;

		if (p.pos.y < 0)
		{
			p.pos.y = 0;
			p.vel.y *= -1;
		}
		else if (p.pos.y > worldSize_y)
		{
			p.pos.y = worldSize_y;
			p.vel.y *= -1;
		}
		if (p.pos.x < 0)
		{
			p.pos.x = 0;
			p.vel.x *= -1;
		}
		else if (p.pos.x > worldSize_x)
		{
			p.pos.x = worldSize_x;
			p.vel.x *= -1;
		}

		if (CollisionIds.size() > 1)
		{
			for (size_t cpi : cells[p.cellid].cellParticleIds) // cpi: collision particle id
			{
				if (cpi == i)
					continue;
				else if ((particles[cpi].pos - p.pos).length() < particleSize)
				{
					printf("collision");
					particle& cp = particles[cpi];
					cp.pos += (cp.pos - p.pos) / 2.f;
					p.pos -= (cp.pos - p.pos) / 2.f;
					cp.vel *= -1.f;
					p.vel *= -1.f;
					p.color = glm::vec3(1.f, 0.8f, 0.f);
					cp.color = glm::vec3(1.f, 0.8f, 0.f);
				}
			}
		}


		UpdatePosBuffer(i);
		UpdateColorBuffer(i);
	}
	
}

void Fluid::simulate(double dt) {
	ndt = dt / substeps;

#define GRAVITY 0
#if GRAVITY
	for (int i = 1; i < gridCount_x - 1; i++) 
	{
		for (int j = 1; j < gridCount_y - 1; j++) 
		{
			cell* const thisCell = &cells[i + j * gridCount_x];
			if (thisCell->s == 1 && cells[thisCell->id_down].s == 1)
			{
				thisCell->v += -9.81 * dt;
			}
		}
	}
#endif // GRACITY

	project(ndt);
	extrapolate();
	advectVelocity(dt);
	SimulateParticles(dt);
	simulationTime += dt;
}

void Fluid::render(GraphicalObj* gobj, const float &renderScale_x, const float &renderScale_y)
{
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, PositionVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, PositionBuffer.size() * sizeof(float), PositionBuffer.data());

	glBindBuffer(GL_ARRAY_BUFFER, ColorVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ColorBuffer.size() * sizeof(float), ColorBuffer.data());

	// Render all cells
	glDrawArrays(GL_POINTS, 0, ArraySize);

	// Unbind VAO
	glBindVertexArray(0);
}

//vector<vector<cell>>* Grid::getCells() {
//	return &cells;
//}