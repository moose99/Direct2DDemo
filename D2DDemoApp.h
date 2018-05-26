#pragma once

#include <windows.h>
#include <stdlib.h>

#include <d2d1_2.h>
#include <d2d1helper.h>
#include <d3d11.h>

#include <wincodec.h>

#include "Atlas.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = nullptr;
	}
}

class DemoApp
{
public:
	DemoApp();
	~DemoApp();

	// Register the window class and call methods for instantiating drawing resources
	HRESULT Initialize();

	// Process and dispatch messages
	void RunMessageLoop();

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceResources();

	// Release device-dependent resource.
	void DiscardDeviceResources();

	HRESULT CreateDeviceContext();
	HRESULT CreatePathGeometry();
	HRESULT CreateLinearGradientBrush();
	HRESULT LoadBitmapFromFile(
		ID2D1DeviceContext* d2dContext, 
		IWICImagingFactory *pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
	);

	// Draw content.
	HRESULT OnRender();
	void DrawGrid();
	void DrawRectangles();
	void DrawEllipse();
	void DrawBitmap();
	void DrawEffectBlur();
	void DrawEffectColorMatrix();
	void DrawGeometry();
	void DrawGeometryRealizations();
	void DrawTransparentOpacityMap();
	void DrawOpaqueOpacityMap();
	void DrawPatterns();

	HRESULT CreatePatterns();
	HRESULT CreateOpacityMasks();
	HRESULT CreateColoredBitmap(unsigned long color, ID2D1Bitmap **bitmap);

	// Resize the render target.
	void OnResize(UINT width, UINT height);

	// The windows procedure.
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

private:
	bool m_resourcesValid;                      // Whether or not the device-dependent resources are ready to use.

	HWND m_hwnd;

	/* device independent resources */
	ID2D1Factory2* m_pDirect2dFactory;
	IWICImagingFactory *m_pWICFactory;
	ID2D1PathGeometry *m_pPathGeometry;

	/* device dependent resources */
	IDXGISwapChain* m_swapChain;
	ID2D1Device1* m_d2dDevice;
	ID2D1DeviceContext1* m_d2dContext;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
	ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
	ID2D1SolidColorBrush* m_pBlackBrush;
	ID2D1BitmapBrush *m_pBitmapBrush;
	ID2D1BitmapBrush *m_pFgndColorBitmapBrush;
	ID2D1BitmapBrush *m_pBgndColorBitmapBrush;
	ID2D1BitmapBrush *m_pOpacityMaskBitmapBrush;
	ID2D1BitmapBrush *m_pOpacityMaskBitmapBrushInv;
	ID2D1LinearGradientBrush *m_pLGBrush;
	ID2D1Bitmap *m_pBitmap;
	ID2D1Bitmap *m_pBgndColorBitmap;
	ID2D1Bitmap *m_pFgndColorBitmap;
	ID2D1Bitmap *m_pTransparentBitmap;
	ID2D1Bitmap *m_pOpacityMaskBitmap;
	ID2D1Bitmap *m_pOpacityMaskBitmapInv;
	ID2D1Effect *m_pGaussianBlurEffect;
	ID2D1Effect *m_pColorMatrixEffect;
	ID2D1Effect *m_pTileEffect;
	ID2D1GeometryRealization *m_pFilledGeometryRealization;
	ID2D1GeometryRealization *m_pStrokedGeometryRealization;
	ID2D1ImageBrush *m_pPatternImageBrush;
	ID2D1Effect *m_pAtlasEffect;
	Atlas m_atlas;
};
