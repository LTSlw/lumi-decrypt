#pragma once
#include <string>

namespace lumi
{
	bool Inirialization(); //³õÊ¼»¯
	void ShowHelp();
	void Decrypt(const std::wstring&);
	void DecryptVideo(const std::wstring&);
	void DecryptCover(const std::wstring&);
	bool CheckDecryptType(const std::wstring&); //0 -> video, 1 -> cover
	void CreateMultiFolder(const std::wstring&);
	std::wstring N0vaPath{}, DecryptPath{};
}