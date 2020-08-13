#include <iostream>
#include <Windows.h>
#include <fstream>
#include <Shlobj_core.h>
#include <string>
#include <cstring>
#include <locale>
#include <io.h>
#include "lumi-decrypt.h"
using namespace std;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	if (!lumi::Inirialization())
	{
		wcout << L"初始化失败，退出" << endl;
		return -1;
	}
	if (argc == 1)
		lumi::ShowHelp();
	else if (strcmp(argv[1], "--decrypt") == 0 || strcmp(argv[1], "-d") == 0)
		lumi::Decrypt(L"");
	else
		lumi::ShowHelp();
}


bool lumi::Inirialization()
{
	TCHAR N0vaPathTemp[MAX_PATH]{}; //where is lumi
	if (!GetPrivateProfileString(L"lumi-decrypt", L"N0vaPath", L"", N0vaPathTemp, MAX_PATH, L".\\lumi-decrypt.ini"))
	{
		if (GetLastError() == 0x2) //没有ini文件
		{
			TCHAR ProgramPath[MAX_PATH]{}; //N0va路径
			if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, ProgramPath))
				return false;
			wcscat_s(ProgramPath, MAX_PATH, L"\\N0vaDesktop\\N0vaDesktopCache");
			if (!WritePrivateProfileString(L"lumi-decrypt", L"N0vaPath", ProgramPath, L".\\lumi-decrypt.ini"))
				return false;

			TCHAR CurrentDir[MAX_PATH]{}; //文件保存路径
			GetCurrentDirectory(MAX_PATH, CurrentDir);
			wcscat_s(CurrentDir, MAX_PATH, L"\\lumi");
			WritePrivateProfileString(L"lumi-decrypt", L"DecryptPath", CurrentDir, L".\\lumi-decrypt.ini");

			N0vaPath = ProgramPath;
			DecryptPath = CurrentDir;
			return true;
		}
		else
			return false;
	}
	N0vaPath = N0vaPathTemp;

	TCHAR DecryptPathTemp[MAX_PATH]{};
	GetPrivateProfileString(L"lumi-decrypt", L"DecryptPath", L"", DecryptPathTemp, MAX_PATH, L".\\lumi-decrypt.ini");
	DecryptPath = DecryptPathTemp;
	return true;
}

void lumi::ShowHelp()
{
	wcout << LR"(无参数 : 初始化，如果没有配置文件则创建
--decrypt -d : 提取lumi动作)";
}

void lumi::Decrypt(const wstring& RelativePath) //相对路径
{
	WIN32_FIND_DATA FileData;
	HANDLE hFile = FindFirstFile((N0vaPath + RelativePath + L"\\*.ndf").c_str(), &FileData); //遍历文件
	if (hFile != INVALID_HANDLE_VALUE)
		CreateMultiFolder(DecryptPath + RelativePath);
	while (hFile != INVALID_HANDLE_VALUE)
	{
		wcout << L"File: " << RelativePath + L"\\" + FileData.cFileName;
		if (CheckDecryptType(RelativePath + L"\\" + FileData.cFileName))
			DecryptCover(RelativePath + L"\\" + FileData.cFileName);
		else
			DecryptVideo(RelativePath + L"\\" + FileData.cFileName);
		if (!FindNextFile(hFile, &FileData))
			break;
	}
	FindClose(hFile);

	hFile = FindFirstFile((N0vaPath + RelativePath + L"\\*").c_str(), &FileData); //遍历文件夹
	while (hFile != INVALID_HANDLE_VALUE)
	{
		wcout << L"Folder: " << RelativePath + L"\\" + FileData.cFileName << endl;
		if (wcscmp(FileData.cFileName, L".") == 0 || wcscmp(FileData.cFileName, L"..") == 0)
		{
			FindNextFile(hFile, &FileData);
			continue;
		}
		Decrypt(RelativePath + L"\\" + FileData.cFileName);
		if (!FindNextFile(hFile, &FileData))
			break;
	}
	FindClose(hFile);
}

bool lumi::CheckDecryptType(const wstring& RelativePath)
{
	fstream File;
	File.open(N0vaPath + RelativePath, ios::in | ios::binary);
	char FileHead[4]{};
	File.read(FileHead, sizeof(char) * 4);
	File.close();
	if (FileHead[0] == -119 && FileHead[1] == 0x50 && FileHead[2] == 0x4E && FileHead[3] == 0x47) //-119 = 0xfffffffffffffffffff89 PNG头
	{
		wcout << L"  ---  cover" << endl;
		return 1;
	}
	else
	{
		wcout << L"  ---  video" << endl;
		return 0;
	}
}

void lumi::DecryptVideo(const wstring& RelativePath)
{
	fstream ndfFile, decFile;
	ndfFile.open(N0vaPath + RelativePath, ios::in | ios::binary);
	decFile.open(DecryptPath + RelativePath.substr(0, RelativePath.length() - 4) + L".mp4", ios::out | ios::binary | ios::trunc);
	char Temp{};
	ndfFile.read(&Temp, sizeof(char));
	ndfFile.read(&Temp, sizeof(char));

	ndfFile.read(&Temp, sizeof(char)); //本次读取将被保存到mp4
	while (ndfFile)
	{
		decFile.write(&Temp, sizeof(char));
		ndfFile.read(&Temp, sizeof(char));
	}
}

void lumi::DecryptCover(const wstring& RelativePath)
{
	CopyFile((N0vaPath + RelativePath).c_str(), (DecryptPath + RelativePath.substr(0, RelativePath.length() - 4) + L".png").c_str(), FALSE);
}

void lumi::CreateMultiFolder(const wstring& Folder)
{
	wstring PathTemp, Path = Folder;
	size_t n = 0;
	if ((n = Path.find(L"\\")) != wstring::npos)
	{
		PathTemp = Path.substr(0, n);
		Path = Path.substr(n + 1);
		if (_waccess(PathTemp.c_str(), 0) == -1)
		{
			CreateDirectory(PathTemp.c_str(), NULL);
			wcout << L"CreateDirectory: " << PathTemp << endl;
		}
	}
	while ((n = Path.find(L"\\")) != wstring::npos)
	{
		PathTemp += L'\\' + Path.substr(0, n);
		Path = Path.substr(n + 1);
		if (_waccess(PathTemp.c_str(), 0) == -1)
		{
			CreateDirectory(PathTemp.c_str(), NULL);
			wcout << L"CreateDirectory: " << PathTemp << endl;
		}
	}
	PathTemp += L"\\" + Path;
	if (_waccess(PathTemp.c_str(), 0) == -1)
	{
		CreateDirectory(PathTemp.c_str(), NULL);
		wcout << L"CreateDirectory: " << PathTemp << endl;
	}
}