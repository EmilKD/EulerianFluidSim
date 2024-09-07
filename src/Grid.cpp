#include<iostream>
#include<random>
#include<functional>
#include "Grid.h"
#include<math.h>
#include<algorithm>

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

	int counter{ 0 };
	cell* thisCell = nullptr;
	cells.resize(gridCount_x);

	cell initialCell;
	initialCell.density = 1.0f;

	for (int i{ 0 }; i < gridCount_x; ++i)
	{
		cells[i].resize(gridCount_y, initialCell);
	}

	u.resize(gridCount_x + 1);
	for (int i{ 0 }; i < gridCount_x; ++i) 
	{
		u[i].resize(gridCount_y);
	}

	v.resize(gridCount_x);
	for (int i{ 0 }; i < gridCount_x; ++i)
	{
		v[i].resize(gridCount_y + 1);
	}

	for (int i{ 0 }; i < gridCount_x; i++)
	{
		for (int j{ 0 }; j < gridCount_y; j++)
		{
			thisCell = &cells[i][j];
			//std::cout << "x: " << j / 20.0f + 1.0f / 20.0f / 2.0f << " y: " << i / 10.0f + 1.0f / 10.0f / 2.0f << std::endl;
			thisCell->pos = glm::vec3(
				worldSize_x*(i + 0.5f)/gridCount_x,
				worldSize_y*(j + 0.5f)/gridCount_y,
				0.0f
			);

			thisCell->id = counter;
			if (i == 0 )//|| y == 0 || x==gridCount_x-1 || y==gridCount_y-1)
			{
				thisCell->s = 0;
				//thisCell->vel = glm::vec2(1.0f, 0.0f);
				u[i][j] = 1.0f;
				v[i][j] = 0.0f;
			}
			else
			{
				thisCell->s = 1;
				u[i][j] = 0.0f;
				v[i][j] = 0.0f;
			}
			
			counter++;

			cellPtrs.push_back(thisCell);
		}
	}
}

vector<vector<cell>> Grid::getCells()
{
	return cells;
}

array<int, 2> Grid::getSize()
{
	return { gridCount_x, gridCount_y };
}

cell* Grid::getCellByID(int id)
{
	return cellPtrs[id];
	std::cout << "cell with specified id not found";
	return nullptr;
}

void Grid::Project(int &cellx, int &celly, double &dt) {
	cell* thisCell = &cells[cellx][celly];
	cell* top, * bottom, * left, * right;
	
	double d{ 0.0f };
	int s{ 0 };

	top = &cells[cellx][celly - 1];
	bottom = &cells[cellx][celly + 1];
	left = &cells[cellx - 1][celly];
	right = &cells[cellx + 1][celly];
	/*
	d = 1.9f*(right->vel.x - left->vel.x + top->vel.y - bottom->vel.y);
	s = right->s + bottom->s + left->s + right->s;
	
	top->vel.y = top->vel.y - d * float(top->s / s);
	bottom->vel.y = bottom->vel.y + d * float(bottom->s / s);
	left->vel.x = left->vel.x + d * float(left->s / s);
	right->vel.x = right->vel.x - d * float(right->s / s);*/

	/*thisCell->vel = glm::vec2(
		right->vel.x - d * float(right->s / s) + left->vel.x + d * float(left->s / s),
		top->vel.y - d * float(top->s / s) + bottom->vel.y + d * float(bottom->s / s)
	);*/
	for (int i = 0; i < 40; i++)
	{
		d = 1.9f * (u[cellx + 1][celly] - u[cellx][celly] + v[cellx][celly + 1] - v[cellx][celly]);
		s = top->s + bottom->s + left->s + right->s;

		u[cellx][celly] += d * cells[cellx - 1][celly].s / s;
		u[cellx + 1][celly] -= d * cells[cellx + 1][celly].s / s;
		v[cellx][celly] += d * cells[cellx][celly - 1].s / s;
		v[cellx][celly + 1] -= d * cells[cellx][celly + 1].s / s;

		thisCell->p += gridSize * d / (s * dt);
	}
	std::cout << thisCell->p << std::endl;
}

float Grid::sampleGrid(float x, float y) 
{
	int x = std::max(std::floor(x / worldSize_x * gridCount_x), std::ceil(x / worldSize_x * gridCount_x));
	int y = std::max(std::floor(y / worldSize_y * gridCount_y), std::ceil(y / worldSize_y * gridCount_y));

	float u1 = u[x][y];
	float u2 = u[x+1][y];
	float v1 = v[x][y];
	float v2 = v[x][y+1];

	cells[x][y].pos 

}

void Grid::simulate(GraphicalObj* gobj, float &scale_x, float &scale_y, double dt)
{
	cell* thisCell = nullptr;
	for (int i{ 0 }; i < gridCount_x; i++)
	{
		for (int j{ 0 }; j < gridCount_y; j++) {
			thisCell = &cells[i][j];
			// Rendering
			
			//thisCell->vel.y += -9.81 * dt;
			

			gobj->transform(glm::vec3(scale_x, scale_y, 0.0f), glm::vec3(xC(thisCell->pos.x), yC(thisCell->pos.y), 0.0f));
			
			if (i>0 && j>0 && i<gridCount_x-1 && j<gridCount_y-1)
			{
				v[i][j] += -9.81 * dt;
				Project(i, j, dt);
			}
			//std::cout << dt << std::endl;

			if (thisCell->s)
				gobj->DrawShape(glm::vec3(abs(thisCell->p), abs(thisCell->p), abs(thisCell->p)));
			else 
			gobj->DrawShape(glm::vec3(1.0f, 1.0f, 1.0f));
		}
	}
}

