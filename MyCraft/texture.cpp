
#include <Windows.h>
#include "texture.h"
#include <stdio.h>

#include "World.h"
#include "SOIL.h"

TextureMgr::TextureMgr() {
}

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader) {
	FILE *filePtr;							// the file pointer
	BITMAPFILEHEADER	bitmapFileHeader;		// bitmap file header
	unsigned char		*bitmapImage;			// bitmap image data
	int					imageIdx = 0;		// image index counter
	unsigned char		tempRGB;				// swap variable

	fopen_s(&filePtr, filename, "rb");
	if (filePtr == NULL)
		return NULL;

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	
	if (bitmapFileHeader.bfType != 0x4D42) {
		fclose(filePtr);
		return NULL;
	}

	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	int size=bitmapInfoHeader->biHeight*bitmapInfoHeader->biWidth*3;
	bitmapImage = new unsigned char[size];

	if (!bitmapImage) {
		delete [] bitmapImage;
		fclose(filePtr);
		return NULL;
	}

	fread(bitmapImage, 1, size, filePtr);
	
	if (bitmapImage == NULL) {
		fclose(filePtr);	
		return NULL;
	}

	// swap the R and B values to get RGB since the bitmap color format is in BGR
	for (imageIdx = 0; imageIdx < size; imageIdx+=3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}

	fclose(filePtr);

	return bitmapImage;
}

int TextureMgr::LoadBitmap(LPTSTR szFileName, GLuint &texid)
{
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char *bitmapData = 0;
	if ((bitmapData = LoadBitmapFile(szFileName, &bitmapInfoHeader)) == 0)
		return 0;

	glGenTextures(1, &texid);											// Create The Texture

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 
		0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);
	
	delete[] bitmapData;

	return 1;														// Loading Was Successful
}

int TextureMgr::LoadAllTextures() {
	block_texture = SOIL_load_OGL_texture("Textures/terrain.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// Linear Mag Filter

	return 2;
}