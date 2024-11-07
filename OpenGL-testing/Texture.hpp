#pragma once
#include <vector>

class TextureObject
{
public:
	TextureObject() { ID = -1; };
	TextureObject(const char* file, bool flip = true);
	TextureObject(std::vector<std::string> faces, bool flip = true);
	TextureObject(unsigned int id) { ID = id; }
	unsigned int ID;

private:
};