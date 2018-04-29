#include <assert.h>
#include "Atlas.h"
#include "D2DDemoApp.h"

Atlas::Atlas() :
	m_pImageData(nullptr),
	m_pBitmap(nullptr),
	m_atlasWidth(0),
	m_atlasHeight(0),
	m_imageHeight(0),
	m_imageWidth(0),
	m_numImagesPerRow(0),
	m_numImagesPerCol(0)
{

}

Atlas::~Atlas()
{
	Destroy();
}

//
// Free bitmap and image data (device dependent resources)
//
void Atlas::Destroy()
{
	SafeRelease(&m_pBitmap);
	delete[] m_pImageData;
	m_pImageData = nullptr;
}

//
// Create bitmap and imaga data from size (device dependent resources)
//
HRESULT Atlas::Create(ID2D1RenderTarget *deviceContext, int atlasWidth, int atlasHeight, int imageWidth, int imageHeight)
{
	/// create bitmap
	HRESULT hr = deviceContext->CreateBitmap(D2D1::SizeU(atlasWidth, atlasHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_pBitmap);
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
	m_imageWidth = imageWidth;
	m_imageHeight = imageHeight;

	// calculate the max number of images allowed per row and col
	m_numImagesPerRow = int((float)atlasWidth / imageWidth);
	m_numImagesPerCol = int((float)atlasHeight / imageHeight);
	m_maxIndex = m_numImagesPerCol * m_numImagesPerRow - 1;
	return hr;
}

//
// adds a binary pattern to the atlas at the specified index
//
void Atlas::AddPattern(bool *patternData, int imageIndex)
{
	// convert binary pattern to image data
	uint32_t *imageData = new uint32_t[m_imageHeight * m_imageWidth];
	int cnt = 0;
	for (int i = 0; i < m_imageHeight; i++)
	{
		for (int j = 0; j < m_imageWidth; j++)
		{
			imageData[cnt] = patternData[cnt] == true ? 0xffffffff : 0x00000000;
			cnt++;
		}
	}

	AddImage(imageData, imageIndex);
	delete[] imageData;
}

//
// adds an image to the atlas at the specified index
//
void Atlas::AddImage(uint32_t *imageData, int imageIndex)
{
	assert(imageIndex >= 0 && imageIndex <= m_maxIndex);
	assert(m_pImageData);
	if (m_pImageData == nullptr)
		return;

	// compute image row and column from index
	int imageCol = imageIndex % m_numImagesPerRow;
	int imageRow = int((float)imageIndex / m_numImagesPerRow);

	// compute atlas starting index
	int atlasIndex = m_imageWidth * m_imageHeight * m_numImagesPerRow * imageRow + imageCol * m_imageWidth;
	assert(atlasIndex < m_atlasWidth * m_atlasHeight);

	// copy image data into atlas at the correct spot
	for (int i = 0; i < m_imageHeight; i++)
	{
		for (int j = 0; j < m_imageWidth; j++)
		{
			assert(atlasIndex < m_atlasWidth * m_atlasHeight);
			m_pImageData[atlasIndex++] = *(imageData++);
		}
		atlasIndex += (m_numImagesPerRow - 1) * m_imageWidth;	// go to start of image at next row
	}
}

//
// returns the rect of the image as vec4, given it's index
//
D2D1_VECTOR_4F Atlas::GetImageRectAsVec4(int imageIndex)
{
	assert(m_pImageData);

	// compute image row and column from index
	int imageCol = imageIndex % m_numImagesPerRow;
	int imageRow = int((float)imageIndex / m_numImagesPerRow);

	return D2D1::Vector4F((float)imageCol * m_imageWidth,	// left
		(float)imageRow * m_atlasHeight,			// top
		(float)m_imageWidth,						// width
		(float)m_imageHeight);						// height		
}

//
// returns the rect of the image given it's index
//
D2D1_RECT_F Atlas::GetImageRect(int imageIndex)
{
	D2D1_VECTOR_4F vec4 = GetImageRectAsVec4(imageIndex);
	D2D1_RECT_F rect = { vec4.x /*left*/, vec4.y/*top*/, 
		vec4.x + vec4.z/*right*/, vec4.y + vec4.w/*bot*/ };
	return rect;
}
