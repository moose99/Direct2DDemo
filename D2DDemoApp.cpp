//
// D2DDemoApp.cpp : Defines the entry point for the application.
//

#include <math.h>

#include "D2DDemoApp.h"

#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

DemoApp::DemoApp() :
	m_hwnd(NULL),
	m_pDirect2dFactory(NULL),
	m_pRenderTarget(NULL),
	m_pLightSlateGrayBrush(NULL),
	m_pCornflowerBlueBrush(NULL),
	m_pBlackBrush(NULL),
	m_pBitmapBrush(NULL),
	m_pLGBrush(NULL),
	m_pBitmap(NULL),
	m_pWICFactory(NULL),
	m_pPathGeometry(NULL)
{
}

DemoApp::~DemoApp()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pPathGeometry);
	DiscardDeviceResources();
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
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

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
HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a black brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a bitmap by loading it from a file.
			hr = LoadBitmapFromFile(
				m_pRenderTarget,
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
				D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

			hr = m_pRenderTarget->CreateBitmapBrush(
				m_pBitmap,
				brushProperties,
				&m_pBitmapBrush
			);
		}

		if (SUCCEEDED(hr))
		{
			hr = CreateLinearGradientBrush();
		}
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

	HRESULT hr = m_pRenderTarget->CreateGradientStopCollection(
		stops,
		ARRAYSIZE(stops),
		&pGradientStops
	);

	if (SUCCEEDED(hr))
	{
		hr = m_pRenderTarget->CreateLinearGradientBrush(
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

void DemoApp::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pBitmapBrush);
	SafeRelease(&m_pLGBrush);
	SafeRelease(&m_pBitmap);
}

//
// Draw the hour glass geometry at the upper left corner of the client area.
//
void DemoApp::DrawGeometry()
{
	// Translate drawing by 200 device-independent pixels.
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(200.f, 0.f));

	m_pRenderTarget->DrawGeometry(m_pPathGeometry, m_pBlackBrush, 10.f);
	m_pRenderTarget->FillGeometry(m_pPathGeometry, m_pLGBrush);

	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

//
// Draw a bitmap in the upper-left corner of the window.
//
void DemoApp::DrawBitmap()
{
	D2D1_SIZE_F size = m_pBitmap->GetSize();

	m_pRenderTarget->DrawBitmap(
		m_pBitmap,
		D2D1::RectF(0.0f, 0.0f, size.width, size.height)
	);
}

//
// Draw an ellipse at bottom right
//
void DemoApp::DrawEllipse()
{
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(
		D2D1::Point2F(width - 100.f, height - 100.f),	// center
		75.f,	// radius X
		50.f	// radius Y
	);

	m_pRenderTarget->DrawEllipse(ellipse, m_pBlackBrush, 10.f);
	m_pRenderTarget->FillEllipse(ellipse, m_pBitmapBrush);
}

//
// Draw a grid background.
//
void DemoApp::DrawGrid()
{
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
	int width = static_cast<int>(rtSize.width);
	int height = static_cast<int>(rtSize.height);

	for (int x = 0; x < width; x += 10)
	{
		m_pRenderTarget->DrawLine(
			D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
			D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
			m_pLightSlateGrayBrush,
			0.5f
		);
	}

	for (int y = 0; y < height; y += 10)
	{
		m_pRenderTarget->DrawLine(
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
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

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
	m_pRenderTarget->FillRectangle(&rectangle1, m_pLightSlateGrayBrush);

	// Draw the outline of a rectangle.
	m_pRenderTarget->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush);
}

HRESULT DemoApp::OnRender()
{
	HRESULT hr = S_OK;

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		DrawGrid();
		DrawRectangles();
		DrawEllipse();
		DrawBitmap();
		DrawGeometry();

		hr = m_pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}

void DemoApp::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}


