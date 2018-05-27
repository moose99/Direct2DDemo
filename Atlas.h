/******************************************************************************
 * Atlas.h: Manage a bitmap atlas (a single bitmap containing other bitmaps).
 *
 * Copyright (C) 2018 SL Corporation.  All Rights Reserved.
 * May 23, 2018
 *****************************************************************************/

#pragma once

//
// A square atlas which contains a series of images.
// All images do not have to be the same size, but they will occupy the size of the largest image dimensions
// Images are referred to by index; the index starts at 0 as the upper left image, and then increases to the right 
// and then down. 
// ie. 0   1   2   3   4
//     5   6   7   8   9
// ...
//

#include <d2d1_2.h>
#include <cstdint>	// or stdint.h
#include <vector>


class Atlas
{
private:
	std::vector<D2D1_RECT_F> m_imageRects;	// the rect of each image by index
	ID2D1Bitmap *m_pBitmap;
	uint32_t *m_pImageData;
	int m_atlasWidth, m_atlasHeight;	// the width and height of the atlas
	int m_maxImageHeight, m_maxImageWidth;	// the max width and height of the images
	int m_numImagesPerRow, m_numImagesPerCol;
	int m_maxIndex;

public:
	Atlas();
	~Atlas();

	void Destroy();	// discard device resources
	HRESULT Create( ID2D1RenderTarget *deviceContext,
		int atlasWidth, int atlasHeight,		// the size of the atlas bitmap
		int maxImageWidth, int maxImageHeight );

	void AddImage( uint32_t *imageData, int imageindex, int imageWidth, int imageHeight );	// adds an image to the atlas at the specified index
	void AddPattern( bool *patternData, int imageindex, int imageWidth, int imageHeight );	// adds a binary pattern at the specified index

	void GetImageRect( int index, D2D1_RECT_F *rect );		// returns the rect of the image given it's index

	ID2D1Bitmap *GetBitmap() { return m_pBitmap; }
	const uint32_t *GetImageData() { return m_pImageData; }

	int GetAtlasWidth() const { return m_atlasWidth;  }
	int GetAtlasHeight() const { return m_atlasHeight;  }
};

