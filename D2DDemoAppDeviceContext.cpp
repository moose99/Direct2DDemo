//
// Device Context creation code
//
#include "D2DDemoApp.h"

//
// See here on the reasons to use a DeviceContext for rendering, instead of hwnd render target
//		https://msdn.microsoft.com/en-us/library/windows/desktop/hh780339(v=vs.85).aspx
// Create a Direct3D11 device, get the associated DXGI device, create a Direct2D device, 
// and then finally create the Direct2D device context for rendering.
//
// Create D2D context for display (Direct3D) device
HRESULT DemoApp::CreateDeviceContext()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(
		rc.right - rc.left,
		rc.bottom - rc.top
	);

	// Create a D3D device and a swap chain sized to the child window.
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
	};
	UINT countOfDriverTypes = ARRAYSIZE(driverTypes);

	DXGI_SWAP_CHAIN_DESC swapDescription;
	ZeroMemory(&swapDescription, sizeof(swapDescription));
	swapDescription.BufferDesc.Width = size.width;
	swapDescription.BufferDesc.Height = size.height;
	swapDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDescription.BufferDesc.RefreshRate.Numerator = 60;
	swapDescription.BufferDesc.RefreshRate.Denominator = 1;
	swapDescription.SampleDesc.Count = 1;
	swapDescription.SampleDesc.Quality = 0;
	swapDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDescription.BufferCount = 1;
	swapDescription.OutputWindow = m_hwnd;
	swapDescription.Windowed = TRUE;

	ID3D11Device* d3dDevice = nullptr;
	for (UINT driverTypeIndex = 0; driverTypeIndex < countOfDriverTypes; driverTypeIndex++)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,       // use default adapter
			driverTypes[driverTypeIndex],
			nullptr,       // no external software rasterizer
			createDeviceFlags,
			nullptr,       // use default set of feature levels
			0,
			D3D11_SDK_VERSION,
			&swapDescription,
			&m_swapChain,
			&d3dDevice,
			nullptr,       // do not care about what feature level is chosen
			nullptr        // do not retain D3D device context
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	IDXGIDevice* dxgiDevice = nullptr;
	if (SUCCEEDED(hr))
	{
		// Get a DXGI device interface from the D3D device.
		hr = d3dDevice->QueryInterface(&dxgiDevice);
	}
	if (SUCCEEDED(hr))
	{
		// Create a D2D device from the DXGI device.
		hr = m_pDirect2dFactory->CreateDevice(
			dxgiDevice,
			&m_d2dDevice
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a device context from the D2D device.
		hr = m_d2dDevice->CreateDeviceContext(
			/* D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTI_THREADED_OPTIMIZATIONS */
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
		);
	}

	SafeRelease(&dxgiDevice);
	SafeRelease(&d3dDevice);
	return hr;
}
