#ifndef __TEXTURE_LOADER__
#define __TEXTURE_LOADER__

#include<iostream>
#include<vector>

std::string createTexture2D(unsigned* texture, std::string texturePath);
std::string loadTextureData(std::string texturePath, unsigned char** data, int* width, int* height);

#endif