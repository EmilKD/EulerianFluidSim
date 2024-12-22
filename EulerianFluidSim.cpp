#include"Graphics.h"
#include"Fluid.h"
#include<chrono>
#include<thread>

using std::cout, std::endl, std::vector, std::array;
using namespace std::chrono;

// Global Variables and Objects declaration
bool left_mouse_button;
bool right_mouse_button;

float gc_x, gc_y, wc_x, wc_y, prevc_x, prevc_y;

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	static constexpr float worldSize_x = 0.4f;
	static constexpr float worldSize_y = 0.3f;

	float simulationTime{ 0.0f };
	float g{ -9.8 }; // m.s^-2
	
	double dt{ 0.f };
	double dtmin; // CFL condition, minimum time step size C_max * gridSize / V_max
	
	bool paused{ false };

	float xC(float worldx) {
		return 2 * (worldx / worldSize_x - 0.5);
	}

	float yC(float worldy) {
		return 2 * (worldy / worldSize_y - 0.5);
	}

private:
	
};

PhysicsWorld PhysWorld;
Fluid fluid(PhysWorld.worldSize_x, PhysWorld.worldSize_y);
CircularObj circle(PhysWorld.worldSize_x / 2.0f - 0.05, PhysWorld.worldSize_y / 2.0f, 0.03f, &fluid);

// Rendering
constexpr float windowScale = 800.0f / PhysWorld.worldSize_x;
constexpr int windowSize[2]{ 800, PhysWorld.worldSize_y * windowScale };
const float renderScale_x = float(1.0f / fluid.gridCount_x);
const float renderScale_y = float(1.0f / fluid.gridCount_y);

// CallBacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
void cursor_pos_callBack(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_clicked(GLFWwindow* window, int button, int action, int mod);

// Main 
int main()
{
	// GLFW initialization
	glfwInit();
	

	// setting window hints aka OpenGL version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// setting up the window and error handling
	GLFWwindow* window = glfwCreateWindow(windowSize[0], windowSize[1], "EulerianFluid", NULL, NULL);//glfwGetPrimaryMonitor(), NULL);
	if (window == NULL)
	{
		std::cout << "window failed to Initialize";
		return -1;
	}

	// setting the window as OpendGl's current context
	glfwMakeContextCurrent(window);

	//Turning VSync Off! :/
	glfwSwapInterval(0);

	// glad loading error handling
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// CallBacks 
	// updating viewport size if window size is changed CallBack
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callBack);
	glfwSetMouseButtonCallback(window, mouse_clicked);
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_PROGRAM_POINT_SIZE);

	// Shader Compilation 
	Shader MainShader;

	// Graphical Objects Declaration 
	GraphicalObj RenderGrid(MainShader);
	RenderGrid.getShader().use();
	Colors color;

	
	// Program Loop 
	int fpsLimit{ 20000 };
	int timer{ 0 };
	auto lastTime = high_resolution_clock::now();

	fluid.AddObstacle(&circle);
	fluid.InitializeGraphics(MainShader);

	PhysWorld.dtmin = 0.5f * fluid.gridSize / fluid.InletVel[0];
	printf("world dt_min: %f", PhysWorld.dtmin);

	while (!glfwWindowShouldClose(window))
	{
		auto currentTime = high_resolution_clock::now();
		const duration<double> elapsedTime = duration_cast<milliseconds>(currentTime - lastTime);
		// input
		process_input(window);
		glfwPollEvents();
		if (!PhysWorld.paused)
		{
			timer = 0;
			// rendering commands
			glfwSwapBuffers(window);
			glfwPollEvents();
			glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		
			const double dt = std::min(PhysWorld.dtmin, elapsedTime.count());
			printf("real-time factor: %f\n", PhysWorld.dtmin / elapsedTime.count());
			// Fluid Sim
			//std::printf("%f\n", dt);
			fluid.simulate(dt);
			
			// Rendering
			fluid.render(&RenderGrid, renderScale_x, renderScale_y);
		}
		lastTime = currentTime;
			
	}

	// Unbinding and closing all glfw windows and clearing opbjects
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glfwTerminate();
	return 0;
}

PhysicsWorld::PhysicsWorld()
{

}

PhysicsWorld::~PhysicsWorld()
{
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void cursor_pos_callBack(GLFWwindow* window, double xpos, double ypos)
{
	gc_x = xpos;
	gc_y = ypos;
	wc_x = gc_x / windowSize[0] * PhysWorld.worldSize_x;
	wc_y = (1 - gc_y / windowSize[1]) * PhysWorld.worldSize_y;
	const float u = (wc_x - prevc_x) / PhysWorld.dt;
	const float v = (wc_y - prevc_y) / PhysWorld.dt;

	if (left_mouse_button)
	{
		circle.Update(wc_x, wc_y, 0.03f, u, v);
		fluid.UpdateObstacle(0);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		PhysWorld.paused = !PhysWorld.paused;
	}
}

void mouse_clicked(GLFWwindow* window, int button, int action, int mod)
{
	// Dropping Particles
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
	{

	}

	left_mouse_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
	right_mouse_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
}