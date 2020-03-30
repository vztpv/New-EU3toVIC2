/*Copyright (c) 2014 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/


#include "WinUtils.h"

#include <Windows.h>

#include "Log.h"
#include <filesystem>
namespace fs = std::filesystem;


namespace WinUtils {

bool TryCreateFolder(const std::string& path)
{
	BOOL success = ::CreateDirectoryW(convertUTF8ToUTF16(path).c_str(), NULL);
	if (success || GetLastError() == 183)	// 183 is if the folder already exists
	{
		return true;
	}
	else
	{
		LOG(LogLevel::Warning) << "Could not create folder " << path << " - " << GetLastWindowsError();
		return false;
	}
}

void GetAllFilesInFolder(const std::string& path, std::set<std::string>& fileNames)
{
	if (!exists(fs::u8path(path))) return;
	if (fs::is_empty(fs::u8path(path))) return;
	for (auto& p : fs::directory_iterator(fs::u8path(path)))
	{
		if (!p.is_directory())
		{
			fileNames.insert(p.path().filename().string());
		}

	}
}

bool TryCopyFile(const std::string& sourcePath, const std::string& destPath)
{
	BOOL success = ::CopyFileW(convertUTF8ToUTF16(sourcePath).c_str(), convertUTF8ToUTF16(destPath).c_str(), FALSE);
	if (success)
	{
		return true;
	}
	else
	{
		LOG(LogLevel::Warning) << "Could not copy file " << sourcePath << " to " << destPath;
		return false;
	}

}

bool DoesFileExist(const std::string& path)
{
	const auto tempPath = fs::u8path(path);
	if (exists(tempPath) && !is_directory(tempPath)) return true;
	return false;

}

std::string GetLastWindowsError()
{
	const DWORD errorCode = ::GetLastError();
	const DWORD errorBufferSize = 256;
	wchar_t errorBuffer[errorBufferSize];
	const BOOL success = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorBuffer,
		errorBufferSize - 1,
		NULL);
	if (success)
	{
		return convertUTF16ToUTF8(errorBuffer);
	}
	else
	{
		return "Unknown error";
	}

}

std::wstring convertUTF8ToUTF16(const std::string& UTF8)
{
	const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), -1, NULL, 0);
	if (requiredSize == 0)
	{
		LOG(LogLevel::Error) << "Could not translate string to UTF-16";
	}
	wchar_t* wideKeyArray = new wchar_t[requiredSize];

	if (0 == MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), -1, wideKeyArray, requiredSize))
	{
		LOG(LogLevel::Error) << "Could not translate string to UTF-16";
	}
	std::wstring returnable(wideKeyArray);

	delete[] wideKeyArray;

	return returnable;
}

std::string convertUTF16ToUTF8(const std::wstring& UTF16)
{
	const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, UTF16.c_str(), -1, NULL, 0, NULL, NULL);
	char* utf8array = new char[requiredSize];

	if (0 == WideCharToMultiByte(CP_UTF8, 0, UTF16.c_str(), -1, utf8array, requiredSize, NULL, NULL))
	{
		LOG(LogLevel::Error) << "Could not translate string to UTF-8";
	}
	std::string returnable(utf8array);

	delete[] utf8array;

	return returnable;
}

} // namespace WinUtils
