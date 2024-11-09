#pragma once

#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <vector>

#include "common.h"
#include "Shader.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "Model.hpp"

#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

class Level
{
public:
	Level() = default;
	virtual ~Level();
	virtual void Init(GLFWwindow* window, mainSettings* settings) = 0;
	virtual void Shutdown() {};
	virtual void Tick(float deltaTime) = 0;
	virtual void Draw(unsigned int screenWidth, unsigned int screenHeigth, unsigned int frameBuffer) = 0;
	virtual void processInput() {};
	virtual void onKeyDown(int key, int scancode, int action, int mods) {};
	virtual void processMouse(double xpos, double ypos, float deltaX, float deltaY) {};
	virtual void processScroll(double xoffset, double yoffset) {};
private:
};