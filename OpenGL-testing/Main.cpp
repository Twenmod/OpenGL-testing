#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

#include "level.hpp"
#include "mainlevel.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void init();
void shutdown();

Camera mainCamera(glm::vec3(0, 0, 3));

GLFWwindow* window;
int width, height;

float deltaTime = 0.0f;	// Time between current frame and last frame

unsigned int postProcessingBuffer;
unsigned int postProcessingTexture;

Shader screenShader;


mainSettings settings;

Level* currentLevel;

int main()
{
	init();

	currentLevel = new mainLevel();

	currentLevel->Init(window, &settings);

	float lastFrame = 0.0f; // Time of last frame

	bool firstFrame = true;

	Model screenQuad(Primitives::PRIMITIVE_PLANE);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float currentFrame = (float)glfwGetTime();
		deltaTime = std::min(currentFrame - lastFrame,1.f);
		lastFrame = currentFrame;

		if (firstFrame)
		{
			firstFrame = false;
			deltaTime = 0;
		}

		//Input
		processInput(window);

		currentLevel->Tick(deltaTime);

		//Rendering

		//Set up Imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// first pass	
		if (settings.postProcessingEnabled)
			glBindFramebuffer(GL_FRAMEBUFFER, postProcessingBuffer); // Bind buffer
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // draw to default buffer aka window
		}
		glEnable(GL_DEPTH_TEST);
		//Draw level
		currentLevel->Draw(width,height, postProcessingBuffer);

		if (settings.postProcessingEnabled)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, postProcessingBuffer); // Bind buffer
			// postprocessing pass
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glBindFramebuffer(GL_FRAMEBUFFER, 0); // draw to default buffer aka window
			//Clear screen
			glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			screenShader.use();
			screenShader.setInt("screenTexture", 0);
			glDisable(GL_DEPTH_TEST);
			glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
			screenQuad.Draw(screenShader);
		}
		//Render Imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//Swap the buffers to display
		glfwSwapBuffers(window);
	}

	delete currentLevel;

	shutdown();

	return 0;
}


void init()
{

#pragma region initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // FOR MAC

	//Create a window
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "Super cool window", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		abort();
	}
	glfwMakeContextCurrent(window);
	width = SCREEN_WIDTH;
	height = SCREEN_HEIGTH;

	glfwSwapInterval(0);

#pragma endregion

	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		abort();
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGTH);

	//Set callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glEnable(GL_STENCIL_TEST);
	//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	stbi_set_flip_vertically_on_load(true);

	//Set up framebuffer for post procesing
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//Create a texture
	unsigned int frameTexture;
	glGenTextures(1, &frameTexture);
	glBindTexture(GL_TEXTURE_2D, frameTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Bind the texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture, 0);
	//Set up render buffer for depth and stencil
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	//Error testing
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	
	postProcessingBuffer = fbo;
	postProcessingTexture = frameTexture;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	screenShader = Shader("shaders/kuwaharaVert.glsl", "shaders/kuwaharaFrag.glsl");

#pragma region Initialize ImGui
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

#pragma endregion


}

void shutdown()
{
	//Shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//Free glfw
	glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
	width = _width;
	height = _height;
}

void processInput(GLFWwindow* window)
{
	currentLevel->processInput();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	currentLevel->onKeyDown(key, scancode, action, mods);
}

float lastX, lastY;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // initially set to true
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates range from bottom to top
	lastX = (float)xpos;
	lastY = (float)ypos;

	const float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	currentLevel->processMouse(xpos, ypos, xoffset, yoffset);

}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	currentLevel->processScroll(xoffset, yoffset);
}

