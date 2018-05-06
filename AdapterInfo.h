#pragma once

//
// Use Microsoft DirectX Graphics Infrastructure (DXGI) to enumerate the available graphics adapters on a computer. 
// Direct3D 10 and 11 can use DXGI to enumerate adapters.
//

#include <vector>
#include <string>

struct IDXGIAdapter;
class AdapterInfo
{
private:
	std::vector <IDXGIAdapter*> EnumerateAdapters();

public:
	std::wstring GetAdapterInfo();		// returns a string with the adapter decription
};