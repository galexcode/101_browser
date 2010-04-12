#include "gui/Texture.h"
// for GL_CLAMP_TO_EDGE
#include "GL/glext.h"

size_t colorModePixelBytesCount(ColorMode colorMode)
{
	switch (colorMode)
	{
	case ColorModeInvalid:
		return 0;
	case ColorModeRGB:
		return 3;
	case ColorModeRGBA:
		return 4;
	}
}

size_t textureLineBytesCount(const Texture* in_pTexture)
{
	return (in_pTexture->width * colorModePixelBytesCount(in_pTexture->colorMode)*4+3)/4;
}

size_t textureBytesCount(const Texture* in_pTexture)
{
	return in_pTexture->height * textureLineBytesCount(in_pTexture);
}

void allocateTextureMemory(Texture* in_pTexture)
{
	in_pTexture->data = (unsigned char*) malloc(textureBytesCount(in_pTexture));
}

void freeTextureMemory(Texture* in_pTexture)
{
	free(in_pTexture->data);
	in_pTexture->data = NULL;
}

GLenum colorModeTextureFormat(ColorMode colorMode)
{
	switch (colorMode)
	{
	case ColorModeInvalid:
		return 0;
	case ColorModeRGB:
		return GL_RGB;
	case ColorModeRGBA:
		return GL_RGBA;
	}
}

void createOpenGLTexture(Texture* in_pTexture)
{
	glGenTextures(1, &in_pTexture->textureID);
	glBindTexture(GL_TEXTURE_2D, in_pTexture->textureID);
	glTexImage2D(GL_TEXTURE_2D,	                          // type of texture
		0,                                                // the texture level
		colorModePixelBytesCount(in_pTexture->colorMode), // number of bytes per pixel
		in_pTexture->width,                               // width
		in_pTexture->height,                              // height
		0,                                                // border size
		colorModeTextureFormat(in_pTexture->colorMode),   // the format of the colors
		                                                  // GL_RGB, GL_RGBA etc.
		GL_UNSIGNED_BYTE,                                 // the type of each component
		in_pTexture->data
		);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
}

