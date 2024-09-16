#include<iostream>
#include<random>
#include<functional>
#include "Grid.h"
#include<math.h>
#include<algorithm>

using std::cout, std::endl;

bool randomBool() {
	static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
	return gen();
}

Grid::Grid(int window_res_x, int window_res_y)
{
	gridCount_x = worldSize_x / gridSize;
	gridCount_y = worldSize_y / gridSize;

	std::cout << "grid count x: " << gridCount_x << std::endl;
	std::cout << "grid count y: " << gridCount_y << std::endl;

	cell* thisCell = nullptr;
	cells.resize(gridCount_x);

	cell initialCell;
	initialCell.m = 0.001f;

	for (int i{ 0 }; i < gridCount_x; ++i)
	{
		cells[i].resize(gridCount_y, initialCell);
	}

	for (int i{ 0 }; i < gridCount_x; i++)
	{
		for (int j{ 0 }; j < gridCount_y; j++)
		{
			thisCell = &cells[i][j];
			thisCell->pos = glm::vec3(
				worldSize_x*(i + 0.5f)/gridCount_x,
				worldSize_y*(j + 0.5f)/gridCount_y,
				0.0f
			);

			thisCell->s = 1;
			// Boundary conditions
			if (i == 0 || j == 0 || i==gridCount_x-1 || j==gridCount_y-1)
			{
				thisCell->s = 0;
			}
			else {
				thisCell->right = &cells[i + 1][j];
				thisCell->left = &cells[i - 1][j];
				thisCell->up = &cells[i][j + 1];
				thisCell->down = &cells[i][j - 1];
			}

			/*if (i == 1)
			{
				thisCell->u = 2.0f;
			}*/

			if (i == 0 && (j > std::ceil(2 * gridCount_y / 5.0f) && j <= std::ceil(3 * gridCount_y / 5.0f)))
			{
				thisCell->m = 0.0f;
			}

			if ((i < std::ceil(gridCount_x/2.0f) + 5 && i > std::floor(gridCount_x/2.0f) - 5) && (j < std::ceil(gridCount_y / 2.0f) + 5 && j > std::floor(gridCount_y / 2.0f) - 5))
			{
				thisCell->v = 2.0f;
				thisCell->m = 1.0;
			}
			cellPtrs.push_back(thisCell);
		}
	}
}

void Grid::project(double dt) {
	double d{ 0.0 }, p{0};
	int s{ 0 };
	cell* thisCell{ nullptr };

	float cp = this->density * gridSize / dt;

	for (int n = 0; n < substeps; n++)
	{
		for (int i = 1; i < gridCount_x - 1; i++)
		{
			for (int j = 1; j < gridCount_y - 1; j++)
			{
				thisCell = &cells[i][j];
				if (thisCell->s == 0)
					continue;

				d = thisCell->right->u - thisCell->u + thisCell->up->v - thisCell->v;
				s = thisCell->down->s + thisCell->up->s + thisCell->left->s + thisCell->right->s;

				if (s == 0)
					continue;

				p = -d / s;
				p *= overRelaxation;

				thisCell->p += p * cp;

				thisCell->u -= p * thisCell->left->s;
				thisCell->right->u += p * thisCell->right->s;
				thisCell->v -= p * thisCell->down->s;
				thisCell->up->v += p * thisCell->up->s;
			}
		}
	}
}

void Grid::advectVelocity(double dt) 
{
	glm::vec2 samplePos{ glm::vec2(0.0f, 0.0f) }, sampleVels{ glm::vec2(0.0f, 0.0f) };

	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			thisCell = &cells[i][j];
			if (thisCell->s == 1 && thisCell->left->s == 1 && i < gridCount_x - 2)
			{
				avgV = (thisCell->v + thisCell->left->v + thisCell->up->v + cells[i - 1][j + 1].v) / 4.0f;

				samplePos.x = thisCell->pos.x - gridSize / 2.0f - dt * thisCell->u;
				samplePos.y = thisCell->pos.y - dt * avgV;

				thisCell->newU = sampleVelocity(samplePos).x;
			}
			if (thisCell->s == 1 && thisCell->down->s == 1 && j < gridCount_y - 2)
			{
				avgU = (thisCell->u + thisCell->right->u + thisCell->down->u + cells[i + 1][j - 1].u) / 4.0f;

				samplePos.x = thisCell->pos.x - dt * avgU;
				samplePos.y = thisCell->pos.y - gridSize / 2.0f - dt * thisCell->v;

				thisCell->newV = sampleVelocity(samplePos).y;
			}
			if (thisCell->s == 1) {
				avgU = (thisCell->u + thisCell->right->u) / 2.0f;
				avgV = (thisCell->v + thisCell->up->v) / 2.0f;

				samplePos.x = thisCell->pos.x - avgU * dt;
				samplePos.y = thisCell->pos.y - avgV * dt;

				thisCell->newM = sampleDensity(samplePos);
			}
		}
	}
	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			thisCell = &cells[i][j];
			thisCell->u = thisCell->newU;
			thisCell->v = thisCell->newV;
			thisCell->m = thisCell->newM;

			if ((i < std::ceil(gridCount_x / 2.0f) + 5 && i > std::floor(gridCount_x / 2.0f) - 5) && (j < std::ceil(gridCount_y / 2.0f) + 5 && j > std::floor(gridCount_y / 2.0f) - 5))
			{
				thisCell->v = 2.0f;
				if (simulationTime < 0.5f)
				{
					thisCell->m = 1;
				}
			}
		}
	}
}


glm::vec2 Grid::sampleVelocity(glm::vec2 &samplePos) 
{
	sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 1);
	sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 1);

	cell* localThisCell = &cells[sampleCelli][sampleCellj];
	// sampling v
	if (samplePos.x < localThisCell->pos.x)
	{
		c0 = &cells[sampleCelli - 1][sampleCellj];
		c1 = localThisCell;
		c2 = &cells[sampleCelli - 1][sampleCellj + 1];
		c3 = &cells[sampleCelli][sampleCellj + 1];
	}
	else
	{
		c0 = localThisCell;
		c1 = &cells[sampleCelli + 1][sampleCellj];
		c2 = &cells[sampleCelli][sampleCellj + 1];
		c3 = &cells[sampleCelli + 1][sampleCellj + 1];
	}

	x = samplePos.x - c0->pos.x;
	y = samplePos.y - (c0->pos.y - gridSize / 2.0f);

	w00 = 1 - x / gridSize;
	w01 = x / gridSize;
	w10 = 1 - y / gridSize;
	w11 = y / gridSize;

	sample_v = w00 * w10 * c0->v + w01 * w10 * c1->v + w00 * w11 * c2->v + w01 * w11 * c3->v;

	// sampling u
	if (samplePos.y > cells[sampleCelli][sampleCellj].pos.y)
	{
		c0 = localThisCell;
		c1 = &cells[sampleCelli+1][sampleCellj];
		c2 = &cells[sampleCelli][sampleCellj + 1];
		c3 = &cells[sampleCelli + 1][sampleCellj + 1];
	}
	else
	{
		c0 = &cells[sampleCelli][sampleCellj - 1];
		c1 = &cells[sampleCelli + 1][sampleCellj - 1];
		c2 = localThisCell;
		c3 = &cells[sampleCelli + 1][sampleCellj];
	}

	x = samplePos.x - (c0->pos.x - gridSize / 2.0f);
	y = samplePos.y - c0->pos.y;

	w00 = 1 - x / gridSize;
	w01 = x / gridSize;
	w10 = 1 - y / gridSize;
	w11 = y / gridSize;

	sample_u = w00 * w10 * c0->u + w01 * w10 * c1->u + w00 * w11 * c2->u + w01 * w11 * c3->u;

	return glm::vec2(sample_u, sample_v);
}

float Grid::sampleDensity(glm::vec2& samplePos)
{
	float samplede{ 0.0f };

	sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 1);
	sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 1);
	
	cell* localThisCell = &cells[sampleCelli][sampleCellj];

	c0 = &cells[sampleCelli - 1][sampleCellj - 1];
	c1 = &cells[sampleCelli - 1][sampleCellj];
	c2 = &cells[sampleCelli - 1][sampleCellj + 1];
	c3 = &cells[sampleCelli][sampleCellj + 1];
	c4 = &cells[sampleCelli + 1][sampleCellj - 1];
	c5 = &cells[sampleCelli + 1][sampleCellj];
	c6 = &cells[sampleCelli + 1][sampleCellj - 1];
	c7 = &cells[sampleCelli][sampleCellj - 1];
	
	if (samplePos.x <= localThisCell->pos.x && samplePos.y >= localThisCell->pos.y)
	{
		c0 = &cells[sampleCelli - 1][sampleCellj];
		c1 = localThisCell;
		c2 = &cells[sampleCelli - 1][sampleCellj + 1];
		c3 = &cells[sampleCelli][sampleCellj + 1];
	}
	else if (samplePos.x <= localThisCell->pos.x && samplePos.y < localThisCell->pos.y)
	{
		c0 = &cells[sampleCelli - 1][sampleCellj - 1];
		c1 = &cells[sampleCelli][sampleCellj - 1];
		c2 = &cells[sampleCelli - 1][sampleCellj];
		c3 = localThisCell;	
	}
	else if (samplePos.x > localThisCell->pos.x && samplePos.y >= localThisCell->pos.y)
	{
		c0 = localThisCell;
		c1 = &cells[sampleCelli + 1][sampleCellj];
		c2 = &cells[sampleCelli][sampleCellj + 1];
		c3 = &cells[sampleCelli + 1][sampleCellj + 1];
	}
	else if (samplePos.x > localThisCell->pos.x && samplePos.y < localThisCell->pos.y)
	{
		c0 = &cells[sampleCelli][sampleCellj - 1];
		c1 = &cells[sampleCelli + 1][sampleCellj - 1];
		c2 = localThisCell;
		c3 = &cells[sampleCelli + 1][sampleCellj];
	}
	else if (samplePos.x == localThisCell->pos.x && samplePos.y == localThisCell->pos.y)
	{
		return localThisCell->m;
	}

	float x = samplePos.x - c0->pos.x;
	float y = samplePos.y - c0->pos.y;

	w00 = 1 - x / gridSize;
	w01 = x / gridSize;
	w10 = 1 - y / gridSize;
	w11 = y / gridSize;

	return w00 * w10 * c0->m + w01 * w10 * c1->m + w00 * w11 * c2->m + w01 * w11 * c3->m;
}

void Grid::advectSmoke(double dt) {
	float avgU{ 0.0f }, avgV{ 0.0f };
	glm::vec2 samplePos{ glm::vec2(0.0f, 0.0f) };

	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			thisCell = &cells[i][j];
			if (thisCell->s != 0) {
				avgU = (thisCell->u + thisCell->right->u) / 2.0f;
				avgV = (thisCell->v + thisCell->right->v) / 2.0f;

				samplePos.x = thisCell->pos.x - avgU * dt;
				samplePos.y = thisCell->pos.y - avgV * dt;
				
				//cout << avgU * dt << endl;

				int sampleCelli = std::min(std::max(int(samplePos.x / worldSize_x * gridCount_x), 1), gridCount_x - 1);
				int sampleCellj = std::min(std::max(int(samplePos.y / worldSize_y * gridCount_y), 1), gridCount_y - 1);
			
				thisCell->newM = cells[sampleCelli][sampleCellj].m;
			}
			
		}
	}
	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			thisCell = &cells[i][j];
			if (thisCell->s != 0)
			{
				thisCell->m = thisCell->newM;
			}
		}
	}
}

void Grid::extrapolate() 
{
	for (int i = 1; i < gridCount_x - 1; i++)
	{
		for (int j = 1; j < gridCount_y - 1; j++)
		{
			thisCell = &cells[i][j];

			if (i==0)
			{
				thisCell->u = thisCell->up->u;
			}
			
		}
	}
}

void Grid::simulate(double dt) {
	ndt = dt / substeps;

	// Gravity 
	for (int i = 1; i < gridCount_x - 1; i++) 
	{
		for (int j = 1; j < gridCount_y - 1; j++) 
		{
			thisCell = &cells[i][j];
			if (thisCell->s == 1 && thisCell->down->s == 1)
			{
				thisCell->v += -9.81 * dt;
			}
			
		}
	}
	
	project(ndt);
	//advectSmoke(dt);
	//extrapolate();
	advectVelocity(dt);
	simulationTime += dt;
}

void Grid::render(GraphicalObj* gobj, float &scale_x, float &scale_y)
{
	cell* thisCell = nullptr;
	glm::vec2 testPos = glm::vec2(0, 0.0f);

	for (int i{ 0 }; i < gridCount_x; i++)
	{
		for (int j{ 0 }; j < gridCount_y; j++) 
		{
			thisCell = &cells[i][j];
			// Rendering
			gobj->transform(glm::vec3(scale_x, scale_y, 0.0f), glm::vec3(xC(thisCell->pos.x), yC(thisCell->pos.y), 0.0f));

			if (thisCell->s)
			{
				// Pressure

				gobj->DrawShape(glm::vec3(thisCell->u, 0.0f, thisCell->v));
				
				if (thisCell->m!=0)
				{
					gobj->DrawShape(glm::vec3(1.0f, 1.0f, 1.0f)*thisCell->m);
				}
				// Density
				//gobj->DrawShape(glm::vec3(1.0f, 1.0f, 1.0f) * thisCell->m*100.0f);
				//cout << thisCell->m << endl;
			}
			
			else 
				gobj->DrawShape(glm::vec3(0.0f, 0.5f, 0.0f));
		}
	}
	//cout << "pressure: " << cells[gridCount_x/2][1].p << endl;
}

