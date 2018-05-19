#include "stdafx.h"
#include "empire_text_convert.h"

namespace 
{
	struct STextConvertTable 
	{
		char acUpper[26];
		char acLower[26];
		BYTE aacHan[5000][2];
		BYTE aacJaum[50][2];
		BYTE aacMoum[50][2];
	} g_aTextConvTable[3];
}

bool LoadEmpireTextConvertTable(DWORD dwEmpireID, const char* c_szFileName)
{
	if (dwEmpireID < 1 || dwEmpireID > 3)
		return false;

	FILE * fp = fopen(c_szFileName, "rb");

	if (!fp)
		return false;

	DWORD dwEngCount = 26;
	DWORD dwHanCount = (0xC8 - 0xB0+1) * (0xFE - 0xA1+1);
	DWORD dwHanSize = dwHanCount * 2;

	STextConvertTable& rkTextConvTable=g_aTextConvTable[dwEmpireID-1];

	fread(rkTextConvTable.acUpper, 1, dwEngCount, fp);
	fread(rkTextConvTable.acLower, 1, dwEngCount, fp);
	fread(rkTextConvTable.aacHan, 1, dwHanSize, fp);

	fread(rkTextConvTable.aacJaum, 1, 60, fp);
	fread(rkTextConvTable.aacMoum, 1, 42, fp);

	fclose(fp);

	return true;
}

void ConvertEmpireText(DWORD dwEmpireID, char* szText, size_t len, int iPct)
{
    szText;
}

