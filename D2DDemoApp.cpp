//
// D2DDemoApp.cpp : Defines the entry point for the application.
//

#include <math.h>

#include "D2DDemoApp.h"

#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

DemoApp::DemoApp() :
	m_resourcesValid(false),
	m_hwnd(NULL),
	m_pDirect2dFactory(NULL),
	m_pLightSlateGrayBrush(NULL),
	m_pCornflowerBlueBrush(NULL),
	m_pBlackBrush(NULL),
	m_pBitmapBrush(NULL),
	m_pOpacityMaskBitmapBrush(NULL),
	m_pLGBrush(NULL),
	m_pBitmap(NULL),
	m_pOpacityMaskBitmap(NULL),
	m_pWICFactory(NULL),
	m_pPathGeometry(NULL),
	m_swapChain(NULL),
	m_d2dDevice(NULL),
	m_d2dContext(NULL),
	m_pGaussianBlurEffect(NULL),
	m_pColorMatrixEffect(NULL),
	m_pFilledGeometryRealization(NULL),
	m_pStrokedGeometryRealization(NULL)
{
}

DemoApp::~DemoApp()
{
	// device independent resources
	SafeRelease(&m_pPathGeometry);

	DiscardDeviceResources();

	// factories last
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pWICFactory);
}

void DemoApp::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

HRESULT DemoApp::Initialize()
{
	HRESULT hr;

	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = DemoApp::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"D2DDemoApp";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

		// Create the window.
		m_hwnd = CreateWindow(
			L"D2DDemoApp",
			L"Direct2D Demo App",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(WIN_WIDTH * dpiX / 96.f)),
			static_cast<UINT>(ceil(WIN_HEIGHT * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
		);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		&m_pDirect2dFactory
	);

	// Create imaging factory
	if (SUCCEEDED(hr))
	{
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICFactory)
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = CreatePathGeometry();
	}

	return hr;
}

// This method creates resources which are bound to a particular
// Direct3D device. It's all centralized here, in case the resources
// need to be recreated in case of Direct3D device loss (e.g. display
// change, remoting, removal of video card, etc). The resources created
// here can be used by multiple Direct2D device contexts which are created
// from the same Direct2D device.
HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_resourcesValid)
	{
		hr = CreateDeviceContext();

		IDXGISurface* surface = nullptr;
		if (SUCCEEDED(hr))
		{
			// Get a surface from the swap chain.
			hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&surface));
		}
		ID2D1Bitmap1* bitmap = nullptr;
		if (SUCCEEDED(hr))
		{
			FLOAT dpiX, dpiY;
			m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

			// Create a bitmap pointing to the surface.
			D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(
					DXGI_FORMAT_B8G8R8A8_UNORM,
					D2D1_ALPHA_MODE_IGNORE
				),
				dpiX,
				dpiY
			);

			hr = m_d2dContext->CreateBitmapFromDxgiSurface(
				surface,
				&properties,
				&bitmap
			);
		}
		if (SUCCEEDED(hr))
		{
			// Set the bitmap as the target of our device context.
			m_d2dContext->SetTarget(bitmap);
		}

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_d2dContext->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_d2dContext->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a black brush.
			hr = m_d2dContext->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a bitmap by loading it from a file.
			hr = LoadBitmapFromFile(
				m_d2dContext,
				m_pWICFactory,
				L".\\sampleImage.png",
				100,
				0,	// don't change height
				&m_pBitmap
			);
		}
		if (SUCCEEDED(hr))
		{
			// Choose the tiling mode for the bitmap brush.
			D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
				D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP);

			hr = m_d2dContext->CreateBitmapBrush(
				m_pBitmap,
				brushProperties,
				&m_pBitmapBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = CreateOpacityMask();	// creates m_pOpacityMaskBitmap
			// Choose the tiling mode for the opacity mask bitmap brush.
			D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
				D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP);

			hr = m_d2dContext->CreateBitmapBrush(
				m_pOpacityMaskBitmap,
				brushProperties,
				&m_pOpacityMaskBitmapBrush
			);
		}

		if (SUCCEEDED(hr))
		{
			hr = CreateLinearGradientBrush();
		}
		if (SUCCEEDED(hr))
		{
			hr = m_d2dContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pGaussianBlurEffect);
			m_pGaussianBlurEffect->SetInput(0, m_pBitmap);
			hr = m_pGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 6.0f);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_d2dContext->CreateEffect(CLSID_D2D1ColorMatrix, &m_pColorMatrixEffect);
			m_pColorMatrixEffect->SetInput(0, m_pBitmap);

			// swap red and blue channels
			D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
				0, 0, 1, 0,  // R
				0, 1, 0, 0,  // G
				1, 0, 0, 0,  // B
				0, 0, 0, 1,  // A
				0, 0, 0, 0);
			hr = m_pColorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
		}
		if (SUCCEEDED(hr))
		{
			// Create geometry realizations
			// https://msdn.microsoft.com/en-us/library/windows/desktop/dn363632(v=vs.85).aspx#creating_geometry_realizations
			float flatteningTolerance = D2D1::ComputeFlatteningTolerance(D2D1::Matrix3x2F::Identity());

			// Create a realization of the filled interior of the path geometry, using the 
			// recommended flattening tolerance. 
			hr = m_d2dContext->CreateFilledGeometryRealization(m_pPathGeometry, flatteningTolerance, &m_pFilledGeometryRealization);

			// Create a realization of the stroke (outline) of the path geometry, using the 
			// recommended flattening tolerance. 
			hr = m_d2dContext->CreateStrokedGeometryRealization(m_pPathGeometry, flatteningTolerance, 
				1, NULL, &m_pStrokedGeometryRealization);
		}

		SafeRelease(&bitmap);
		SafeRelease(&surface);
	}

	if (FAILED(hr))
	{
		DiscardDeviceResources();
	}
	else
	{
		m_resourcesValid = true;
	}

	return hr;
}

//
// Create linear gradient brush
//
HRESULT DemoApp::CreateLinearGradientBrush()
{
	ID2D1GradientStopCollection *pGradientStops = NULL;

	// Create a linear gradient.
	static const D2D1_GRADIENT_STOP stops[] =
	{
		{ 0.f,{ 0.f, 1.f, 1.f, 0.25f } },
		{ 1.f,{ 0.f, 0.f, 1.f, 1.f } },
	};

	HRESULT hr = m_d2dContext->CreateGradientStopCollection(
		stops,
		ARRAYSIZE(stops),
		&pGradientStops
	);

	if (SUCCEEDED(hr))
	{
		hr = m_d2dContext->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(100, 0),
				D2D1::Point2F(100, 200)),
			D2D1::BrushProperties(),
			pGradientStops,
			&m_pLGBrush
		);
	}

	SafeRelease(&pGradientStops);

	return hr;
}

//
// Create a path geometry. Device independent resource.
//
HRESULT DemoApp::CreatePathGeometry()
{
	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry(&m_pPathGeometry);

	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;

		// Write to the path geometry using the geometry sink.
		hr = m_pPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(
				D2D1::Point2F(0, 0),
				D2D1_FIGURE_BEGIN_FILLED
			);

			pSink->AddLine(D2D1::Point2F(200, 0));
			pSink->AddBezier(
				D2D1::BezierSegment(
					D2D1::Point2F(150, 50),
					D2D1::Point2F(150, 150),
					D2D1::Point2F(200, 200))
			);
			pSink->AddLine(D2D1::Point2F(0, 200));
			pSink->AddBezier(
				D2D1::BezierSegment(
					D2D1::Point2F(50, 150),
					D2D1::Point2F(50, 50),
					D2D1::Point2F(0, 0))
			);

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = pSink->Close();
		}
		SafeRelease(&pSink);
	}

	return hr;
}

// discard resources on device loss
void DemoApp::DiscardDeviceResources()
{
	SafeRelease(&m_d2dContext);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pBitmapBrush);
	SafeRelease(&m_pOpacityMaskBitmapBrush);
	SafeRelease(&m_pLGBrush);
	SafeRelease(&m_pBitmap);
	SafeRelease(&m_pOpacityMaskBitmap);
	SafeRelease(&m_pGaussianBlurEffect);
	SafeRelease(&m_pColorMatrixEffect);
	SafeRelease(&m_pFilledGeometryRealization);
	SafeRelease(&m_pStrokedGeometryRealization);
	SafeRelease(&m_swapChain);
	SafeRelease(&m_d2dDevice);
	SafeRelease(&m_d2dContext);

	m_resourcesValid = false;
}

//
// Draw the hour glass geometry at the upper left corner of the client area.
//
void DemoApp::DrawGeometry()
{
	// Translate drawing by 200 device-independent pixels.
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(200.f, 0.f));

	m_d2dContext->DrawGeometry(m_pPathGeometry, m_pBlackBrush);
	m_d2dContext->FillGeometry(m_pPathGeometry, m_pLGBrush);

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw the hour glass geometry realizations
//
void DemoApp::DrawGeometryRealizations()
{
	// Translate drawing by 400 device-independent pixels.
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(400.f, 0.f));

	m_d2dContext->DrawGeometryRealization(m_pStrokedGeometryRealization, m_pBlackBrush);
	m_d2dContext->DrawGeometryRealization(m_pFilledGeometryRealization, m_pLGBrush);

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw a bitmap in the upper-left corner of the window.
//
void DemoApp::DrawBitmap()
{
	D2D1_SIZE_F size = m_pBitmap->GetSize();

	m_d2dContext->DrawBitmap(m_pBitmap, D2D1::RectF(0.0f, 0.0f, size.width, size.height));

	// draw blurred image effect
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, size.height));
	m_d2dContext->DrawImage(m_pGaussianBlurEffect);
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw a blurred image effect
//
void DemoApp::DrawEffectBlur()
{
	D2D1_SIZE_F size = m_pBitmap->GetSize();

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, size.height));
	m_d2dContext->DrawImage(m_pGaussianBlurEffect);
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw a color matrix image effect
//
void DemoApp::DrawEffectColorMatrix()
{
	D2D1_SIZE_F size = m_pBitmap->GetSize();

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, size.height*2));
	m_d2dContext->DrawImage(m_pColorMatrixEffect);
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw an ellipse at bottom right
//
void DemoApp::DrawEllipse()
{
	D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(
		D2D1::Point2F(width - 100.f, height - 100.f),	// center
		75.f,	// radius X
		50.f	// radius Y
	);

	m_d2dContext->DrawEllipse(ellipse, m_pBlackBrush, 10.f);
	m_d2dContext->FillEllipse(ellipse, m_pBitmapBrush);
}

//
// Draw a grid background.
//
void DemoApp::DrawGrid()
{
	D2D1_SIZE_F rtSize = m_d2dContext->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);

	for (int x = 0; x < width; x += 10)
	{
		m_d2dContext->DrawLine(
			D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
			D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
			m_pLightSlateGrayBrush,
			0.5f
		);
	}

	for (int y = 0; y < height; y += 10)
	{
		m_d2dContext->DrawLine(
			D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
			D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
			m_pLightSlateGrayBrush,
			0.5f
		);
	}
}

//
// Draw two rectangles.
//
void DemoApp::DrawRectangles()
{
	D2D1_SIZE_F rtSize = m_d2dContext->GetSize();

	D2D1_RECT_F rectangle1 = D2D1::RectF(
		rtSize.width / 2 - 50.0f,
		rtSize.height / 2 - 50.0f,
		rtSize.width / 2 + 50.0f,
		rtSize.height / 2 + 50.0f
	);

	D2D1_RECT_F rectangle2 = D2D1::RectF(
		rtSize.width / 2 - 100.0f,
		rtSize.height / 2 - 100.0f,
		rtSize.width / 2 + 100.0f,
		rtSize.height / 2 + 100.0f
	);

	// Draw a filled rectangle.
	m_d2dContext->FillRectangle(&rectangle1, m_pLightSlateGrayBrush);

	// Draw the outline of a rectangle.
	m_d2dContext->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush);
}

HRESULT DemoApp::CreateOpacityMask()
{
	const unsigned size = 200U;		// Note: The geometry this is used on is 200x200
	unsigned long memory[size * size];
	m_d2dContext->CreateBitmap(D2D1::SizeU(size, size),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_pOpacityMaskBitmap);
	
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (j % 4 == 0)
			{
				memory[i*size+j] = 0x00000000;
			}
			else
			{
				memory[i*size+j] = 0xFFFFFFFF;
			}
		}
	}

	HRESULT hr = m_pOpacityMaskBitmap->CopyFromMemory(nullptr, (byte*)memory, size * 4);
	return hr;
}

//
// Draw opacity map test
//
void DemoApp::DrawOpacityMap()
{
	// Translate drawing by 200 device-independent pixels.
	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Translation(400.f, 0.f));

//	m_d2dContext->DrawGeometry(m_pPathGeometry, m_pBlackBrush);	// stroke
	m_d2dContext->FillGeometry(m_pPathGeometry, m_pBitmapBrush, m_pOpacityMaskBitmapBrush);

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

HRESULT DemoApp::OnRender()
{
	HRESULT hr = S_OK;

	if (!m_resourcesValid)
	{
		hr = CreateDeviceResources();
	}

	if (SUCCEEDED(hr))
	{
		m_d2dContext->BeginDraw();

		m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
		m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White));

		DrawGrid();
		DrawRectangles();
		DrawEllipse();
		DrawBitmap();
		DrawEffectBlur();
		DrawEffectColorMatrix();
		DrawGeometry();
		DrawGeometryRealizations();
		DrawOpacityMap();

		hr = m_d2dContext->EndDraw();
	}

	if (SUCCEEDED(hr))
	{
		// Present the swap chain immediately
		hr = m_swapChain->Present(0, 0);
	}

	if (hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		hr = S_OK;
		// Recreate device resources then force repaint.
		DiscardDeviceResources();
		InvalidateRect(m_hwnd, nullptr, FALSE);
	}

	return hr;
}

// Called whenever the application window is resized. Recreates
// the swap chain with buffers sized to the new window.
void DemoApp::OnResize(UINT width, UINT height)
{
	if (m_d2dContext)
	{
		HRESULT hr = S_OK;
		// Remove the bitmap from rendering device context.
		m_d2dContext->SetTarget(nullptr);

		// Resize the swap chain.
		if (SUCCEEDED(hr))
		{
			hr = m_swapChain->ResizeBuffers(
				0,
				width,
				height,
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
			);
		}

		// Get a surface from the swap chain.
		IDXGISurface* surface = nullptr;
		if (SUCCEEDED(hr))
		{
			hr = m_swapChain->GetBuffer(
				0,
				IID_PPV_ARGS(&surface)
			);
		}

		// Create a bitmap pointing to the surface.
		ID2D1Bitmap1* bitmap = nullptr;
		if (SUCCEEDED(hr))
		{
			FLOAT dpiX, dpiY;
			m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);
			D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(
					DXGI_FORMAT_B8G8R8A8_UNORM,
					D2D1_ALPHA_MODE_IGNORE
				),
				dpiX,
				dpiY
			);
			hr = m_d2dContext->CreateBitmapFromDxgiSurface(
				surface,
				&properties,
				&bitmap
			);
		}

		// Set bitmap back onto device context.
		if (SUCCEEDED(hr))
		{
			m_d2dContext->SetTarget(bitmap);
		}

		SafeRelease(&bitmap);
		SafeRelease(&surface);

		// Force a repaint.
		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
}

