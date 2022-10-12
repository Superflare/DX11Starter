
#include <Windows.h>
#include <codecvt>
#include <locale>
#include <DirectXMath.h>

#include "Helpers.h"


// --------------------------------------------------------------------------
// Gets the actual path to this executable as a wide character string (wstring)
//
// - As it turns out, the relative path for a program is different when 
//    running through VS and when running the .exe directly, which makes 
//    it a pain to properly load external files (like textures & shaders)
//    - Running through VS: Current Dir is the *project folder*
//    - Running from .exe:  Current Dir is the .exe's folder
// - This has nothing to do with DEBUG and RELEASE modes - it's purely a 
//    Visual Studio "thing", and isn't obvious unless you know to look 
//    for it.  In fact, it could be fixed by changing a setting in VS, but
//    that option is stored in a user file (.suo), which is ignored by most
//    version control packages by default.  Meaning: the option must be
//    changed on every PC.  Ugh.  So instead, here's a helper.
// --------------------------------------------------------------------------
std::wstring GetExePath()
{
	// Assume the path is just the "current directory" for now
	std::wstring path = L".\\";

	// Get the real, full path to this executable
	wchar_t currentDir[1024] = {};
	GetModuleFileName(0, currentDir, 1024);

	// Find the location of the last slash charaacter
	wchar_t* lastSlash = wcsrchr(currentDir, '\\');
	if (lastSlash)
	{
		// End the string at the last slash character, essentially
		// chopping off the exe's file name.  Remember, c-strings
		// are null-terminated, so putting a "zero" character in 
		// there simply denotes the end of the string.
		*lastSlash = 0;

		// Set the remainder as the path
		path = currentDir;
	}

	// Toss back whatever we've found
	return path;
}


// ----------------------------------------------------
//  Fixes a relative path so that it is consistently
//  evaluated from the executable's actual directory
//  instead of the app's current working directory.
// 
//  See the comments of GetExePath() for more details.
// 
//  Note that this uses wide character strings (wstring)
//  instead of standard strings, as most windows API
//  calls require wide character strings.
// ----------------------------------------------------
std::wstring FixPath(const std::wstring& relativeFilePath)
{
	return GetExePath() + L"\\" + relativeFilePath;
}


// ----------------------------------------------------
//  Helper function for converting a wide character 
//  string to a standard ("narrow") character string
// ----------------------------------------------------
std::string WideToNarrow(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
}


// ----------------------------------------------------
//  Helper function for converting a standard ("narrow") 
//  string to a wide character string
// ----------------------------------------------------
std::wstring NarrowToWide(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

float Deg2Rad(float deg)
{
	return deg * (DirectX::XM_PI / 180.0f);
}

DirectX::XMFLOAT3 Deg2RadFromVector(DirectX::XMFLOAT3 degV)
{
	return DirectX::XMFLOAT3(Deg2Rad(degV.x), Deg2Rad(degV.y), Deg2Rad(degV.z));
}

float Rad2Deg(float rad)
{
	return rad * (180.0f / DirectX::XM_PI);
}

DirectX::XMFLOAT3 Rad2DegFromVector(DirectX::XMFLOAT3 radV)
{
	return DirectX::XMFLOAT3(Rad2Deg(radV.x), Rad2Deg(radV.y), Rad2Deg(radV.z));
}
