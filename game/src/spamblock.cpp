#include "fstream"
#include "string"
#include "sstream"
#include "stdafx.h"
#include "../../common/length.h"

std::vector<std::string> SpamBlockListArray;
std::vector<std::string> SpamBannListArray;

void LoadBlockSpamList()
{
	std::string TempBlockList;
	std::ifstream File("chat/blockspamlist.lst");

	if (!File.is_open())
	{
		sys_log(0, "ERRO: Nao foi possivel abrir chat/blockspamlist.lst");
		return;
	}

	SpamBlockListArray.clear();

	while (!File.eof())
	{
		File >> TempBlockList;
		SpamBlockListArray.push_back(TempBlockList);
	}

	File.close();
}

void LoadBannSpamList()
{
	std::string TempBannList;
	std::ifstream File("chat/bannspamlist.lst");

	if (!File.is_open())
	{
		sys_log(0, "ERRO: Nao foi possivel abrir chat/bannspamlist.lst");
		return;
	}

	SpamBannListArray.clear();

	while (!File.eof())
	{
		File >> TempBannList;
		SpamBannListArray.push_back(TempBannList);
	}

	File.close();
}

