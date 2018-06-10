/******************************************************************************
 * Atlas.cpp: Atlas class method implementation.
 *
 * Copyright (C) 2018 SL Corporation.  All Rights Reserved.
 * May 23, 2018
 *****************************************************************************/

#include <assert.h>
#include "Atlas.h"


Atlas::Atlas () :
	m_pImageData( nullptr ),
	m_pBitmap( nullptr ),
	m_atlasWidth( 0 ),
	m_atlasHeight( 0 ),
	m_maxImageHeight( 0 ),
	m_maxImageWidth( 0 ),
	m_numImagesPerRow( 0 ),
	m_numImagesPerCol( 0 ),
	m_maxIndex( 0 )
{

}

Atlas::~Atlas ()
{
	Destroy();
}

//
// Free bitmap and image data (device dependent resources)
//
void 
Atlas::Destroy ()
{
	if (m_pBitmap != nullptr)
	{
		m_pBitmap->Release();
		m_pBitmap = nullptr;
	}
	delete[] m_pImageData;
	m_pImageData = nullptr;

	m_atlasWidth = 0;
	m_atlasHeight = 0;
	m_maxIndex = 0;
}

//
// Create bitmap and imaga data from size (device dependent resources)
//
HRESULT 
Atlas::Create ( ID2D1RenderTarget *deviceContext, int atlasWidth, int atlasHeight, int maxImageWidth, int maxImageHeight )
{
	/// create bitmap
	HRESULT hr = deviceContext->CreateBitmap( D2D1::SizeU(atlasWidth, atlasHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_pBitmap );
	if (FAILED(hr))
	{
		return hr;
	}

	// create image data
	m_pImageData = new uint32_t[atlasWidth * atlasHeight];
	memset(m_pImageData, 0, atlasWidth * atlasHeight * sizeof(uint32_t));

	// store the width/height of the atlas
	m_atlasWidth = atlasWidth;
	m_atlasHeight = atlasHeight;

	// store the width/height of the images in the atlas
	m_maxImageWidth = maxImageWidth;
	m_maxImageHeight = maxImageHeight;

	// calculate the max number of images allowed per row and col
	m_numImagesPerRow = int((float)atlasWidth / maxImageWidth);
	m_numImagesPerCol = int((float)atlasHeight / maxImageHeight);
	m_maxIndex = m_numImagesPerCol * m_numImagesPerRow - 1;

	m_imageRects.resize( m_maxIndex );
	return hr;
}

//
// Adds a binary pattern to the atlas at the specified index
// Unset pixels are transparent BLACK.  Set pixels are opaque WHITE.
//
void 
Atlas::AddPattern ( bool *patternData, int imageIndex, int imageWidth, int imageHeight )
{
	const uint32_t setColor = 0xffffffff;	// WHITE ABGR

	// convert binary pattern to image data
	uint32_t *imageData = new uint32_t[imageHeight * imageWidth];
	int cnt = 0;
	for (int i = 0; i < imageHeight; i++)
	{
		for (int j = 0; j < imageWidth; j++)
		{
			imageData[cnt] = patternData[cnt] == true ? setColor : 0x00000000;
			cnt++;
		}
	}

	AddImage( imageData, imageIndex, imageWidth, imageHeight );
	delete[] imageData;
}

//
// adds an image to the atlas at the specified index
//
void 
Atlas::AddImage ( uint32_t *imageData, int imageIndex, int imageWidth, int imageHeight )
{
	assert(imageIndex >= 0 && imageIndex <= m_maxIndex);
	assert( m_pImageData );
	if (m_pImageData == nullptr)
		return;

	// compute image row and column from index
	int imageCol = imageIndex % m_numImagesPerRow;
	int imageRow = int((float)imageIndex / m_numImagesPerRow);

	// compute atlas starting index
	int atlasIndex = m_atlasWidth * m_maxImageHeight * imageRow + imageCol * m_maxImageWidth;
	assert(atlasIndex < m_atlasWidth * m_atlasHeight);

	m_imageRects[imageIndex] = D2D1::RectF(
		(float)imageCol * m_maxImageWidth,			// left
		(float)imageRow * m_maxImageHeight,			// top
		(float)imageWidth + imageCol * m_maxImageWidth,		// right
		(float)imageHeight + imageRow * m_maxImageHeight	// bot		
	);

	// copy image data into atlas at the correct spot
	for (int i = 0; i < imageHeight; i++)
	{
		for (int j = 0; j < imageWidth; j++)
		{
			assert(atlasIndex < m_atlasWidth * m_atlasHeight);
			m_pImageData[atlasIndex++] = *(imageData++);
		}
		atlasIndex += m_atlasWidth - imageWidth;	// go to start of image at next row
	}
}

//
// returns the rect of the image given it's index
//
void 
Atlas::GetImageRect ( int imageIndex, D2D1_RECT_F *rect )
{
	assert( m_pImageData );
	*rect = m_imageRects[imageIndex];
}

