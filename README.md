---

# An Eulerian 2D fluid simulator

This is a practice project for learning more about real-time simulations and implementation of computational solutions for differential equations, in this case a simplified version of the Navier-Stokes equations.
The visualization of the simulator is done using an OpenGL project i'd developed earlier.

## Here's a short demo
![Demo](Export/EulerianFluidSimulation.gif)

## Features

- **Real-time Fluid Simulation**: Uses an Eulerian grid-based method for simulating fluid motion in real-time.
- **Interactive and Offline Simulation**: Supports both interactive and offline simulations.
- **Boundary Conditions Handling**: Manages boundary conditions for accurate simulation results.
- **Dynamic Rendering**: Visualizes the fluid motion using OpenGL for an engaging experience.

## Installation

To get started with the project, follow these steps:

1. **Clone the repository**:
   ```sh
   git clone https://github.com/EmilKD/EulerianFluidSim.git
   cd Grid-Based-Fluid-Simulator
   ```

2. **Install dependencies**:
   Ensure you have `C++`, `OpenGL`, and `GLFW` installed on your machine.

3. **Build the project**:
   Use your preferred C++ build system. For example, with `CMake`:
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```
4. **Run the simulator**:
   ```sh
   ./fluid_simulator
   ```

## Dependencies
The following libraries and files should be placed in a folder named 'Libraries' in the project path:
GLFW: 
```
./Libraries/lib/glfw3.lib
./Libraries/Include/GLFW/[GLFW built headers]
./Libraries/Include/glm/[glm build headers]
./Libraries/Include/glad[glad build headers]
./Libraries/glad.c
./Libraries/stb_image.h
```

## Usage

### Simulation Parameters

You can configure various simulation parameters such as grid size, time step, and fluid properties directly in the `Grid.cpp` file. Here’s a brief overview:

- **Grid Size**: Determines the resolution of the simulation.
- **Time Step (dt)**: Controls the simulation speed.
- **Fluid Properties**: Adjust properties like density and viscosity to study different fluid behaviors.

### Boundary Conditions

The simulation handles boundary conditions to ensure realistic fluid behavior at the edges of the simulation domain. The boundary cells are assigned special properties to simulate solid walls, inlets, or outlets.

### Visual Rendering

The project utilizes OpenGL for rendering the fluid simulation. The `render` function in `Grid.cpp` is responsible for drawing the grid and the fluid particles. You can modify the rendering parameters to change the appearance of the simulation.

## Code Overview

### `Grid.cpp`

This file contains the main implementation of the fluid simulation, including:
- **Grid Initialization**: Sets up the grid cells and their properties.
- **Projection**: Ensures incompressibility by adjusting the pressure field.
- **Velocity Advection**: Moves the fluid particles based on their velocities.
- **Density Sampling**: Calculates the density of fluid particles for rendering.
- **Extrapolation**: Handles boundary conditions by extrapolating velocity fields.
- **Simulation Loop**: Runs the simulation by updating the grid state over time.

### `EulerianFluidSim.cpp`

This file sets up the window and the main simulation loop, including:
- **Window Initialization**: Initializes the GLFW window and sets up OpenGL context.
- **Input Handling**: Processes user inputs for interaction with the simulation.
- **Rendering Loop**: Continuously updates and renders the fluid simulation.


## Contributions

Contributions are welcome! If you have any ideas for improving the simulator or adding new features, feel free to open an issue or submit a pull request.

## License

This project is completely open-source and free to use and distribute in any way desired.

## Contact

For any questions or further information, please contact [me](emil.ke.200@gmail.com).

---

## refrences:
Matthias Müller's ten minute physics [tutorials](https://matthias-research.github.io/pages/tenMinutePhysics/index.html)<br/>
Cline, David, David L. Cardon, Parris K. Egbert and Brigham Young. “Fluid Flow for the Rest of Us : Tutorial of the Marker and Cell Method in Computer Graphics.” (2005). [link to paper](https://www.semanticscholar.org/paper/Fluid-Flow-for-the-Rest-of-Us-%3A-Tutorial-of-the-and-Cline-Cardon/9d471060d6c48308abcc98dbed850a39dbfea683?p2df)
