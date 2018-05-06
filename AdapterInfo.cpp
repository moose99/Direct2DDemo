//
// Use Microsoft DirectX Graphics Infrastructure (DXGI) to enumerate the available graphics adapters on a computer. 
// Direct3D 10 and 11 can use DXGI to enumerate adapters.
//

#include "AdapterInfo.h"
#include <dxgi.h>
#include <assert.h>

std::vector <IDXGIAdapter*> 
AdapterInfo::EnumerateAdapters ()
{
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	IDXGIFactory* pFactory = NULL;

	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory( __uuidof(IDXGIFactory), (void**)&pFactory )))
	{
		return vAdapters;
	}

	for (UINT i = 0;
		pFactory->EnumAdapters( i, &pAdapter ) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		vAdapters.push_back( pAdapter );
	}

	if (pFactory)
	{
		pFactory->Release();
	}

	return vAdapters;
}

std::wstring 
AdapterInfo::GetAdapterInfo ()
{
	std::wstring out;
	HRESULT hr = S_OK;
	UINT i = 0;
	std::vector <IDXGIAdapter*> adapters = EnumerateAdapters();
	for (IDXGIAdapter *pAdapter : adapters)
	{
		if (pAdapter != nullptr)
		{
			wchar_t buff[256];
			DXGI_ADAPTER_DESC adapterDesc;
			hr = pAdapter->GetDesc( &adapterDesc );
			assert( SUCCEEDED(hr) );
			if ( FAILED(hr) )
			{
				continue;
			}

			swprintf(buff, sizeof(buff), L"\nDesc: %ls\n", adapterDesc.Description);
			out += buff;
			
			swprintf(buff, sizeof(buff), L"VendorId: %lu\n", adapterDesc.VendorId);
			out += buff;

			swprintf(buff, sizeof(buff), L"DeviceId: %lu\n", adapterDesc.DeviceId);
			out += buff;

			swprintf(buff, sizeof(buff), L"SubSysId: %lu\n", adapterDesc.SubSysId);
			out += buff;

			swprintf(buff, sizeof(buff), L"Revision: %lu\n", adapterDesc.Revision);
			out += buff;

			swprintf(buff, sizeof(buff), L"Dedicated VideoMem: %lu mb\n", adapterDesc.DedicatedVideoMemory / (1024 * 1024));
			out += buff;

			swprintf(buff, sizeof(buff), L"Dedicated SystemMem: %lu mb\n", adapterDesc.DedicatedSystemMemory / (1024 * 1024));
			out += buff;

			swprintf(buff, sizeof(buff), L"Shared SystemMem: %lu mb\n", adapterDesc.SharedSystemMemory / (1024 * 1024));
			out += buff;

			swprintf(buff, sizeof(buff), L"UID: (%lu, %lu)\n", adapterDesc.AdapterLuid.LowPart, adapterDesc.AdapterLuid.HighPart);
			out += buff;

			// ENUM OUTPUTS
			IDXGIOutput * pOutput;
			while ( pAdapter->EnumOutputs(i++, &pOutput) != DXGI_ERROR_NOT_FOUND )
			{
				DXGI_OUTPUT_DESC outputDesc;
				hr = pOutput->GetDesc( &outputDesc );
				assert( SUCCEEDED(hr) );
				if ( FAILED(hr) )
				{
					continue;
				}

				swprintf(buff, sizeof(buff), L"\tOutput Device: %s, Attached to desktop:%d\n", 
					outputDesc.DeviceName, outputDesc.AttachedToDesktop);
				out += buff;
			}
		}
	}
	return out;
}
