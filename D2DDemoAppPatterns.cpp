//
// Code for drawing patterns using an atlas
//

#include <assert.h>
#include "D2DDemoApp.h"

#define ATLAS_HEIGHT 128
#define ATLAS_WIDTH 128
#define PATTERN_SIZE 8

bool pattern4[PATTERN_SIZE * PATTERN_SIZE] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

bool pattern1[PATTERN_SIZE * PATTERN_SIZE] = {
	1, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 1
};
bool pattern2[PATTERN_SIZE * PATTERN_SIZE] = {
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0
};

bool pattern3[PATTERN_SIZE * PATTERN_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

//
// create atlas and add patterns to the atlas
//
HRESULT DemoApp::CreatePatterns()
{
	// create atlas and bitmap
	HRESULT hr = S_OK;
	hr = m_atlas.Create(m_d2dContext, ATLAS_WIDTH, ATLAS_HEIGHT, PATTERN_SIZE, PATTERN_SIZE);

	if (hr == S_OK)
	{
		// add pattern images to atlas
		m_atlas.AddPattern(pattern1, 0);
		m_atlas.AddPattern(pattern2, 1);
		m_atlas.AddPattern(pattern3, 2);

		// init bitmap from image data
		hr = m_atlas.GetBitmap()->CopyFromMemory(nullptr, m_atlas.GetImageData(), ATLAS_WIDTH * 4);
		assert(hr == S_OK);

		// create image brush from atlas
		hr = m_d2dContext->CreateImageBrush(m_atlas.GetBitmap(),
			D2D1::ImageBrushProperties(
				D2D1::RectF(0, 0, ATLAS_WIDTH, ATLAS_HEIGHT),
				D2D1_EXTEND_MODE_WRAP,
				D2D1_EXTEND_MODE_WRAP,
				D2D1_INTERPOLATION_MODE_LINEAR
			),
			&m_pPatternImageBrush);
		assert(hr == S_OK);
	}

	return hr;
}

void DemoApp::DrawPatterns()
{
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(200.f, 200.f));

#if 0
	// I think effects only work with DrawImage
	D2D1_VECTOR_4F inputRect = m_atlas.GetImageRectAsVec4(0);
	HRESULT hr = m_pAtlasEffect->SetValue(D2D1_ATLAS_PROP_INPUT_RECT, inputRect);
	assert(hr == S_OK);
	ID2D1Image *pImage=nullptr;
	m_pAtlasEffect->GetOutput(&pImage);			// this doesn't seem to respect the rectangle
	m_pPatternImageBrush->SetImage(pImage);
	//m_d2dContext->DrawImage(m_pAtlasEffect);	// this seems to work
#else
	D2D1_RECT_F rect = m_atlas.GetImageRect(2);
	m_pPatternImageBrush->SetSourceRectangle(&rect);
#endif

	m_d2dContext->DrawGeometry(m_pPathGeometry, m_pBlackBrush);	// stroke
	m_d2dContext->FillGeometry(m_pPathGeometry, m_pPatternImageBrush);
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

