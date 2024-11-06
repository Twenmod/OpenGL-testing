#pragma once
#include "level.hpp"

class mainLevel : public Level
{
public:
	mainLevel();
	void Init(GLFWwindow* window, mainSettings* mainSettings);
	void Shutdown();
	void Tick(float deltaTime);
	void Draw(unsigned int screenWidth, unsigned int screenHeigth);
	void processInput();
	void onKeyDown(int key, int scancode, int action, int mods);
	void processMouse(double xpos, double ypos, float deltaX, float deltaY);
	void processScroll(double xoffset, double yoffset);

private:
	float deltaTime;
	bool wireframe = false;
	bool lockedMouse;
	Camera mainCamera;
	Shader baseShader;
	Shader skyboxShader;
	Model cube;
	Model skyboxModel;
	TextureObject skyBoxTexture;
	GLFWwindow* window;
	mainSettings* settings;
};