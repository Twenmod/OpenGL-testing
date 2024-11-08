#include "mainlevel.hpp"


mainLevel::mainLevel() : Level(),
particleSystem(0.0001,0.5,glm::vec3(0,15,0))
{
	deltaTime = 0;
	baseShader = Shader("shaders/baseVertShader.glsl", "shaders/baseFragShader.glsl");
	skyboxShader = Shader("shaders/skyboxVert.glsl", "shaders/skyboxFrag.glsl");
	particleShader = Shader("shaders/particleLitVert.glsl", "shaders/particleLitFrag.glsl");


	cube = Model(Primitives::PRIMITIVE_CUBE, TextureObject("assets/diffuse.jpg"), TextureObject("assets/specular.jpg"));
	sphere = Model("assets/sphere.obj");

	lockedMouse = true;

	std::vector<std::string> faces
	{
		"assets/skybox/right.jpg",
		"assets/skybox/left.jpg",
		"assets/skybox/top.jpg",
		"assets/skybox/bottom.jpg",
		"assets/skybox/front.jpg",
		"assets/skybox/back.jpg"
	};
	skyBoxTexture = TextureObject(faces, false);
}

void mainLevel::Init(GLFWwindow* _window, mainSettings* _mainSettings)
{
	settings = _mainSettings;
	window = _window;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	skyboxModel = Model(Primitives::PRIMITIVE_CUBE, skyBoxTexture);
	
	cube.LoadCubeMap(skyBoxTexture);

	ComputeShader computeShader = ComputeShader("shaders/testcompute.glsl");
	particleSystem.Init(computeShader);
}

void mainLevel::Tick(float _deltaTime)
{
	deltaTime = _deltaTime;
	particleSystem.Tick(deltaTime);
}

void mainLevel::Shutdown()
{

}

void mainLevel::Draw(unsigned int screenWidth, unsigned int screenHeigth)
{
	//Clear screen
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Gui
	ImGui::Begin("Performance counter", NULL, ImGuiWindowFlags_MenuBar);
	ImGui::Text("FPS: %.f (%.2f ms)", ImGui::GetIO().Framerate, ImGui::GetIO().DeltaTime*1000);
	framerates.push_back(1/ImGui::GetIO().DeltaTime);
	if (framerates.size() > 5000)
		framerates.erase(framerates.begin());
	float tot = 0;
	for (float f : framerates)
		tot += f;
	float avarage = tot / framerates.size();
	ImGui::Text("Avarage: %.f (%.2f ms)", avarage, (1 / avarage)*1000);
	ImGui::Checkbox("PostProcessing", &settings->postProcessingEnabled);
	ImGui::Checkbox("Wireframe", &wireframe);

	ImGui::End();

	//Level

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//Camera
	glm::mat4 view;
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(mainCamera.Zoom), (float)screenWidth / (float)screenHeigth, 0.1f, 100.0f);
	view = mainCamera.GetViewMatrix();
	baseShader.use();
	baseShader.setMat4("projection", projection);
	baseShader.setMat4("view", view);
	baseShader.setVec3("viewPos", mainCamera.Position);

	//particles
	particleShader.use();
	particleShader.setMat4("projection", projection);
	particleShader.setMat4("view", view);
	particleShader.setVec3("viewPos", mainCamera.Position);
	particleShader.setVec3("fogColor", glm::vec3(0.1f, 0.1f, 0.1f));

	particleShader.setVec3("dirLight.direction", 0.4f, -1.0f, 0.3f);
	particleShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
	particleShader.setVec3("dirLight.diffuse", 0.7f, 0.7f, 0.7f);
	particleShader.setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);

	baseShader.use();
	baseShader.setInt("skybox", skyBoxTexture.ID);

	// Lights
	baseShader.setVec3("dirLight.direction", 0.4f, -1.0f, 0.3f);
	baseShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
	baseShader.setVec3("dirLight.diffuse", 0.7f, 0.7f, 0.7f);
	baseShader.setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);

	baseShader.setVec3("fogColor", glm::vec3(0.1f, 0.1f, 0.1f));


	//Material
	// textures
	baseShader.setInt("material.diffuse", 0);
	baseShader.setInt("material.specular", 1);
	baseShader.setInt("material.emission", 2);
	baseShader.setBool("hasEmission", false);
	// settings
	baseShader.setFloat("material.shininess", 32.0f);

	//Skybox
	glCullFace(GL_FRONT);
	//glDepthMask(GL_FALSE);

	skyboxShader.use();
	skyboxShader.setMat4("projection", projection);
	glm::mat4 skyView = glm::mat4(glm::mat3(mainCamera.GetViewMatrix())); // Removes translation part of the matrix
	skyboxShader.setMat4("view", skyView);
	skyboxModel.Draw(skyboxShader);

	//Models/objects
	//glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);

	particleSystem.Draw(sphere, particleShader);

	baseShader.use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(8.0f, 0.5f, 8.0f));
	model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));

	baseShader.setMat4("model", model);
	cube.Draw(baseShader);

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 3.0f));
	baseShader.setMat4("model", model);
	cube.Draw(baseShader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, 1.0f, 3.0f));
	model = glm::rotate(model, static_cast<float>(glfwGetTime()), glm::vec3(1, 1, 1));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	baseShader.setMat4("model", model);
	cube.Draw(baseShader);

}

void mainLevel::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		mainCamera.MovementSpeed = 7.5f;
	}


	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		mainCamera.MovementSpeed = 7.5f;
	}
	else
	{
		mainCamera.MovementSpeed = 2.5f;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(RIGHT, deltaTime);
}

void mainLevel::onKeyDown(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		//flashlight = !flashlight;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		lockedMouse = !lockedMouse;
		if (lockedMouse)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

}

void mainLevel::processMouse(double xpos, double ypos, float deltaX, float deltaY)
{
	if (lockedMouse)
		mainCamera.ProcessMouseMovement(deltaX, deltaY, true);
}

void mainLevel::processScroll(double xoffset, double yoffset)
{
	if (lockedMouse)
		mainCamera.ProcessMouseScroll((float)yoffset);
}