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
		m_atlas.AddPattern(pattern1, 0, PATTERN_SIZE, PATTERN_SIZE );
		m_atlas.AddPattern(pattern2, 1, PATTERN_SIZE, PATTERN_SIZE );
		m_atlas.AddPattern(pattern3, 2, PATTERN_SIZE, PATTERN_SIZE );

		// init bitmap from image data
		hr = m_atlas.GetBitmap()->CopyFromMemory(nullptr, m_atlas.GetImageData(), ATLAS_WIDTH * 4);
		assert(hr == S_OK);

		m_pAtlasEffect->SetInput( 0, m_atlas.GetBitmap() );
		assert(hr == S_OK);

		// ATLAS PATTERN -> COLORMATRIX -> TILE
		m_pColorMatrixEffect->SetInputEffect(0, m_pAtlasEffect);	
		m_pTileEffect->SetInputEffect( 0, m_pColorMatrixEffect );
		m_pTileEffect->SetValue( D2D1_TILE_PROP_RECT, D2D1::RectF( 0.0f, 0.0f, PATTERN_SIZE, PATTERN_SIZE ) );
	}

	return hr;
}

void DemoApp::DrawPatterns()
{
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(200.f, 200.f));

	HRESULT hr=S_OK;

	D2D1_RECT_F inputRect;
	m_atlas.GetImageRect( 2, &inputRect );
	hr = m_pAtlasEffect->SetValue(D2D1_ATLAS_PROP_INPUT_RECT, inputRect);

	// change fg color of pattern image to blue
	D2D_COLOR_F blue = { 0,0,1,1 };
	D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
		blue.r, 0,		0,		0,		// R
		0,		blue.g, 0,		0,		// G
		0,		0,		blue.b, 0,		// B
		0,		0,		0,		blue.a, // A
		0,		0,		0,		0);
	hr = m_pColorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
	assert(hr == S_OK);

	// STROKE
	m_d2dContext->DrawGeometry(m_pPathGeometry, m_pBlackBrush);

	// FILL
	m_d2dContext->PushLayer( D2D1::LayerParameters( D2D1::InfiniteRect(), m_pPathGeometry ), NULL );
	D2D_COLOR_F black= { 0,0,0,0 };	// bg color
	D2D_COLOR_F red = { 1,0,0,1 };	// bg color
	m_d2dContext->Clear( red ); //(in background color or transparent black depending on opaque or transparent fill style)
	m_d2dContext->DrawImage( m_pTileEffect );
	m_d2dContext->PopLayer();

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

