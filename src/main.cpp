#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Component.h"

using namespace std;

GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Shape> shape;

//Initialize Components
Component* Torso = new Component(2.2, 3, 1.6, 0, -2.5, 0, 0, 0, 0);
Component* Head = new Component(1.2, 1.2, 1.2, 0, 1.5, 0, 0, 0.5, 0, Torso, Torso);
Component* Upper_Right_Arm = new Component(2.5, 0.9, 1.0, -1.15, 1, 0, -1.2, 0, 0, Torso, Head);
Component* Lower_Right_Arm = new Component(2.0, 0.6, 0.7, -2.2, 0, 0, -1, 0, 0, Upper_Right_Arm, Upper_Right_Arm);
Component* Upper_Left_Arm = new Component(2.5, 0.9, 1.0, 1.15, 1, 0, 1.2, 0, 0, Torso, Lower_Right_Arm);
Component* Lower_Left_Arm = new Component(2, 0.6, 0.7, 2.2, 0, 0, 1, 0, 0, Upper_Left_Arm, Upper_Left_Arm);
Component* Upper_Right_Leg = new Component(0.9, 2.7, 1, -0.5, -1.3, 0, 0, -1.2, 0, Torso, Lower_Left_Arm);
Component* Lower_Right_Leg = new Component(0.75, 2, 0.7, 0, -2.3, 0, 0, -1.2, 0, Upper_Right_Leg, Upper_Right_Leg);
Component* Upper_Left_Leg = new Component(0.9, 2.7, 1, 0.6, -1.3, 0, 0, -1.2, 0, Torso, Lower_Right_Leg);
Component* Lower_Left_Leg = new Component(0.75, 2, 0.7, 0, -2.3, 0, 0, -1.2, 0, Upper_Left_Leg, Upper_Left_Leg);
//Initialize Current Position
Component* Position = Torso;


static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}
//if a certain key is pushed on the keyboard then it does a certain command
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (codepoint == 120)	//if 'x' is pushed
	{
		Position->Adjust_X_Angle(0.1);
	}
	else if (codepoint == 88)	//if 'X' is pushed
	{
		Position->Adjust_X_Angle(-0.1);
	}
	else if (codepoint == 121)	//if 'y' is pushed
	{
		Position->Adjust_Y_Angle(0.1);
	}
	else if (codepoint == 89)	//if 'Y' is pushed
	{
		Position->Adjust_Y_Angle(-0.1);
	}
	else if (codepoint == 122)	//if 'z' is pushed
	{
		Position->Adjust_Z_Angle(0.1);
	}
	else if (codepoint == 90)	//if 'Z' is pushed
	{
		Position->Adjust_Z_Angle(-0.1);
	}
	else if (codepoint == 46)	//if '.' is pushed
	{
		if (Position->get_Next())
		{
			Position = Position->get_Next();
			Position->Adjust_Scale(0.2);
			Position->get_Prev()->Adjust_Scale(-0.2);
		}
	}
	else if (codepoint == 44)	//if ',' is pushed
	{
		if (Position->get_Prev())
		{
			Position = Position->get_Prev();
			Position->Adjust_Scale(0.2);
			Position->get_Next()->Adjust_Scale(-0.2);
		}
	}
	else
	{
		//do nothing
	}
}

static void init()
{
	GLSL::checkVersion();

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RES_DIR + "cube.obj");
	shape->init();
	
	// Initialize the GLSL program.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "vert.glsl", RES_DIR + "frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	//Current positon scale is increased
	Position->Adjust_Scale(0.2);
	//set all the next pointers for each component
	Torso->set_Next(Head);
	Head->set_Next(Upper_Right_Arm);
	Upper_Right_Arm->set_Next(Lower_Right_Arm);
	Lower_Right_Arm->set_Next(Upper_Left_Arm);
	Upper_Left_Arm->set_Next(Lower_Left_Arm);
	Lower_Left_Arm->set_Next(Upper_Right_Leg);
	Upper_Right_Leg->set_Next(Lower_Right_Leg);
	Lower_Right_Leg->set_Next(Upper_Left_Leg);
	Upper_Left_Leg->set_Next(Lower_Left_Leg);
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(0, 4, -15);
	//Draw the Robot
	Torso->Draw(MV, P, prog, shape);

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, character_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
