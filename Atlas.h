#pragma once

//
// A square atlas which contains a series of images.
// All images should be the same size.
// Images are referred to by index; the index starts at 0 as the upper left image, and then increases to the right 
// and then down. 
// ie. 0   1   2   3   4
//     5   6   7   8   9
// ...
//

#include <d2d1_2.h>
#include <cstdint>	// or stdint.h

class Atlas
{
public:
	Atlas();
	~Atlas();

	void Destroy ();	// discard device resources
	HRESULT Create(ID2D1RenderTarget *deviceContext,
		int atlasWidth, int atlasHeight,		// the size of the atlas bitmap
		int imageWidth, int imageHeight);
 
	void AddImage(uint32_t *imageData, int imageindex);	// adds an image to the atlas at the specified index
	void AddPattern(bool *patternData, int imageindex);	// adss a binary pattern at the specified index

	D2D1_VECTOR_4F GetImageRectAsVec4(int index);	// returns the rect of the image given it's index
	D2D1_RECT_F GetImageRect(int index);			// returns the rect of the image given it's index

	ID2D1Bitmap *GetBitmap()	{ return m_pBitmap; }
	const uint32_t *GetImageData() { return m_pImageData;  }

private:
	ID2D1Bitmap		*m_pBitmap;
	uint32_t		*m_pImageData;
	int m_atlasWidth, m_atlasHeight;
	int m_imageHeight, m_imageWidth;	// the width and height of the images
	int m_numImagesPerRow, m_numImagesPerCol;
	int m_maxIndex;
};