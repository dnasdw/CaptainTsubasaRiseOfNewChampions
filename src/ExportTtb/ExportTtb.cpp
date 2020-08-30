#include <sdw.h>

#include SDW_MSC_PUSH_PACKED
struct SHeader
{
	u32 Signature;
	u32 Size;
	u32 Section0Count;
	u32 LanguageCount;
	u32 TextCount;
	u32 Unknown0x14;
	u32 Section0Offset;
	u32 Section1Offset;
	u32 Section2Offset;
	u32 TextOffset;
	u32 Unknown0x28[34];
} SDW_GNUC_PACKED;

struct SSection0
{
	u32 Hash;
	u32 Unknown0x4;
	u32 Count;
	u32 Section1Offset;
} SDW_GNUC_PACKED;

struct SSection1
{
	u32 Hash;
	u32 Section2Index;
} SDW_GNUC_PACKED;

struct SSection2
{
	u32 TextOffset;
	u32 TextSize;
	u32 Unknown0x8;
	u32 Unknown0xC;
} SDW_GNUC_PACKED;
#include SDW_MSC_POP_PACKED

int UMain(int argc, UChar* argv[])
{
	u32 uOriginalTextLanguageIndex = 0;
	string sOriginalTextLanguageName;
	if (argc != 3)
	{
		if (argc != 4)
		{
			return 1;
		}
		sOriginalTextLanguageName = UToU8(argv[3]);
	}
	FILE* fp = UFopen(argv[1], USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	u32 uTtbSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	u8* pTtb = new u8[uTtbSize];
	fread(pTtb, 1, uTtbSize, fp);
	fclose(fp);
	SHeader* pHeader = reinterpret_cast<SHeader*>(pTtb);
	if (pHeader->Signature != SDW_CONVERT_ENDIAN32('TTB0'))
	{
		delete[] pTtb;
		return 1;
	}
	if (pHeader->Size != sizeof(SHeader))
	{
		delete[] pTtb;
		return 1;
	}
	if (pHeader->Section0Offset != pHeader->Size)
	{
		delete[] pTtb;
		return 1;
	}
	if (pHeader->Section1Offset != pHeader->Section0Offset + pHeader->Section0Count * sizeof(SSection0))
	{
		delete[] pTtb;
		return 1;
	}
	if (pHeader->Section2Offset != pHeader->Section1Offset + pHeader->TextCount * sizeof(SSection1))
	{
		delete[] pTtb;
		return 1;
	}
	if (pHeader->TextOffset != pHeader->Section2Offset + pHeader->LanguageCount * (4 + pHeader->TextCount * sizeof(SSection2)))
	{
		delete[] pTtb;
		return 1;
	}
	map<u32, string> mLanguage;
	for (u32 i = 0; i < pHeader->LanguageCount; i++)
	{
		u32 uLanguageOffset = pHeader->Section2Offset + i * (4 + pHeader->TextCount * sizeof(SSection2));
		string sLanguage(reinterpret_cast<char*>(pTtb + uLanguageOffset), 4);
		mLanguage.insert(make_pair(i, sLanguage));
	}
	if (!sOriginalTextLanguageName.empty())
	{
		bool bMatch = false;
		for (map<u32, string>::const_iterator it = mLanguage.begin(); it != mLanguage.end(); ++it)
		{
			if (it->second == sOriginalTextLanguageName)
			{
				uOriginalTextLanguageIndex = it->first;
				bMatch = true;
				break;
			}
		}
		if (!bMatch)
		{
			uOriginalTextLanguageIndex = SToU32(sOriginalTextLanguageName);
			if (uOriginalTextLanguageIndex >= pHeader->LanguageCount)
			{
				uOriginalTextLanguageIndex = 0;
			}
		}
	}
	sOriginalTextLanguageName = mLanguage[uOriginalTextLanguageIndex];
	char* pText = reinterpret_cast<char*>(pTtb + pHeader->TextOffset);
	for (u32 i = 0; i < pHeader->LanguageCount; i++)
	{
		u32 uSection2Offset0 = pHeader->Section2Offset + uOriginalTextLanguageIndex * (4 + pHeader->TextCount * sizeof(SSection2)) + 4;
		u32 uSection2Offset1 = pHeader->Section2Offset + i * (4 + pHeader->TextCount * sizeof(SSection2)) + 4;
		SSection2* pSection20 = reinterpret_cast<SSection2*>(pTtb + uSection2Offset0);
		SSection2* pSection21 = reinterpret_cast<SSection2*>(pTtb + uSection2Offset1);
		string sLanguage = mLanguage[i];
		UString sTxt = argv[2];
		sTxt += USTR("_") + U8ToU(sLanguage) + USTR(".txt");
		FILE* fpTxt = UFopen(sTxt.c_str(), USTR("wb"), false);
		if (fpTxt == nullptr)
		{
			delete[] pTtb;
			return 1;
		}
		fwrite("\xFF\xFE", 2, 1, fpTxt);
		for (u32 j = 0; j < pHeader->TextCount; j++)
		{
			string sText0U8(pText + pSection20[j].TextOffset, pSection20[j].TextSize);
			string sText1U8(pText + pSection21[j].TextOffset, pSection21[j].TextSize);
			wstring sText0 = U8ToW(sText0U8);
			wstring sText1 = U8ToW(sText1U8);
			wstring::size_type uPos0 = 0;
			uPos0 = sText0.find(L"[No].");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText0.find(L"[--------------------------------------]");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText0.find(L"[======================================]");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText0.find(L"<r>");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText1.find(L"[No].");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText1.find(L"[--------------------------------------]");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText1.find(L"[======================================]");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			uPos0 = sText1.find(L"<r>");
			if (uPos0 != wstring::npos)
			{
				fclose(fpTxt);
				delete[] pTtb;
				return 1;
			}
			sText0 = Replace(sText0, L"No.", L"[No].");
			sText0 = Replace(sText0, L"--------------------------------------", L"[--------------------------------------]");
			sText0 = Replace(sText0, L"======================================", L"[======================================]");
			sText0 = Replace(sText0, L"\r\n", L"\n");
			sText0 = Replace(sText0, L'\r', L"<r>");
			sText0 = Replace(sText0, L'\n', L"\r\n");
			sText1 = Replace(sText1, L"No.", L"[No].");
			sText1 = Replace(sText1, L"--------------------------------------", L"[--------------------------------------]");
			sText1 = Replace(sText1, L"======================================", L"[======================================]");
			sText1 = Replace(sText1, L"\r\n", L"\n");
			sText1 = Replace(sText1, L'\r', L"<r>");
			sText1 = Replace(sText1, L'\n', L"\r\n");
			if (ftell(fpTxt) != 2)
			{
				fu16printf(fpTxt, L"\r\n\r\n");
			}
			fu16printf(fpTxt, L"No.%d\r\n", j);
			fu16printf(fpTxt, L"--------------------------------------\r\n");
			fu16printf(fpTxt, L"%ls\r\n", sText0.c_str());
			fu16printf(fpTxt, L"======================================\r\n");
			fu16printf(fpTxt, L"%ls\r\n", sText1.c_str());
			fu16printf(fpTxt, L"--------------------------------------\r\n");
		}
		fclose(fpTxt);
	}
	delete[] pTtb;
	return 0;
}
