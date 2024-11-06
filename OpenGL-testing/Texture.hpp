#pragma once

class TextureObject
{
public:
	TextureObject() { ID = -1; };
	TextureObject(const char* file);
	unsigned int ID;

private:
};