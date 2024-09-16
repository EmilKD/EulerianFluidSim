#include"Graphics.h"
#include"Grid.h"
#include<chrono>
#include<thread>

using std::cout, std::endl, std::vector, std::array;


//=====================================================================================================================
// Variables and Objects declaration-----------------------------------------------------------------------------------
//=====================================================================================================================

bool left_mouse_button;
bool right_mouse_button;


float g_xpos, g_ypos;
float wc_x;
float wc_y;

int cols = 100;
int rows = 56;

float worldSize_x = 0.4; //m
float worldSize_y = 0.3;

float windowScale = 800 / worldSize_x;
int windowSize[2]{ 800, worldSize_y * windowScale };

Grid grid(windowSize[0], windowSize[1]);

float global_scale = 1.0f;
float scale_x = float(1.0f / grid.gridCount_x) * global_scale;
float scale_y = float(1.0f / grid.gridCount_y) * global_scale;

bool paused{ false };


//=====================================================================================================================
// CallBacks ----------------------------------------------------------------------------------------------------------
//=====================================================================================================================
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
	g_xpos = xpos;
	g_ypos = ypos;
	wc_x = (2 * (xpos / windowSize[0]) - 1) / scale_x;
	wc_y = (-2 * (ypos / windowSize[1]) + 1) / scale_y;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		paused = !paused;
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

	if (right_mouse_button)
	{

	}
}


//=====================================================================================================================
// Main ---------------------------------------------------------------------------------------------------------------
//=====================================================================================================================
int main()
{
	using namespace std::this_thread;
	using namespace std::chrono;

	//=================================================================================================================
	// GLFW initialization --------------------------------------------------------------------------------------------
	//=================================================================================================================
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

	//=================================================================================================================
	// CallBacks ------------------------------------------------------------------------------------------------------
	//=================================================================================================================
	// updating viewport size if window size is changed CallBack
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callBack);
	glfwSetMouseButtonCallback(window, mouse_clicked);
	glfwSetKeyCallback(window, key_callback);

	//=================================================================================================================
	// Shader Compilation ---------------------------------------------------------------------------------------------
	//=================================================================================================================
	Shader MainShader;

	//=================================================================================================================
	// Graphical Objects Declaration ----------------------------------------------------------------------------------
	//=================================================================================================================
	GraphicalObj rectangle(MainShader, "./Textures/GlowDotFilled.png");
	rectangle.getShader().use();
	Colors color;

	bool start_flag{ true };

	//=================================================================================================================
	// Program Loop ---------------------------------------------------------------------------------------------------
	//=================================================================================================================


	int fpsLimit{ 20000 };
	int timer{ 0 };
	double dt{ 0 };
	auto lastTime = high_resolution_clock::now();

	while (!glfwWindowShouldClose(window))
	{
		auto currentTime = high_resolution_clock::now();
		duration<double> elapsedTime = duration_cast<microseconds>(currentTime - lastTime);
		// input
		process_input(window);
		glfwPollEvents();
		if (!paused)
		{
			timer = 0;
			// rendering commands
			glfwSwapBuffers(window);
			glfwPollEvents();
			glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		
			dt = elapsedTime.count() / 1000000;
			// Fluid Sim
			grid.simulate(1.0f/512.f);
			
			// Rendering
			grid.render(&rectangle, scale_x, scale_y);
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