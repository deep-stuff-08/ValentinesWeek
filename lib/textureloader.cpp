#include<iostream>
#include<sstream>
#include<GL/glew.h>
#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include<FreeImage.h>

using namespace std;

string loadTextureData(string texturePath, unsigned char** data, int* width, int* height) {
	FREE_IMAGE_FORMAT fif;
	fif = FreeImage_GetFileType(texturePath.c_str());
	if(fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(texturePath.c_str());
		if(fif == FIF_UNKNOWN) {
			return "Error : Unknown or Unsupported Texture Format\n";
		}
	}

	FIBITMAP* dib = FreeImage_Load(fif, texturePath.c_str());
	FIBITMAP* tmp = dib;
	dib = FreeImage_ConvertTo32Bits(dib);
	FreeImage_Unload(tmp);
	*width = FreeImage_GetWidth(dib);
	*height = FreeImage_GetHeight(dib);
	*data = FreeImage_GetBits(dib);
	if(*width == 0 || *height == 0 || *data == NULL) {
		return "Error : Could not load texture at '" + texturePath + "'\n";
	}
	return "";
}

string createTexture2D(unsigned* texture, string texturePath) {
	stringstream errString;
	unsigned char* data;
	int w, h;
	
	errString<<loadTextureData(texturePath, &data, &w, &h);
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return errString.str();
}
