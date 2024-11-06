#pragma once

class TextureObject
{
public:
	TextureObject() { ID = -1; };
	TextureObject(const char* file, bool flip = true);
	TextureObject(std::vector<std::string> faces, bool flip = true);
	unsigned int ID;

private:
};