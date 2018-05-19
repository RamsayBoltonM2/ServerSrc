#include "stdafx.h"
#include "constants.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "desc.h"
#include "item_manager.h"

const int MAX_NORM_ATTR_NUM = ITEM_MANAGER::MAX_NORM_ATTR_NUM;
const int MAX_RARE_ATTR_NUM = ITEM_MANAGER::MAX_RARE_ATTR_NUM;

int CItem::GetAttributeSetIndex()
{
	if (GetType() == ITEM_WEAPON)
	{
		if (GetSubType() == WEAPON_ARROW)
			return -1;

		return ATTRIBUTE_SET_WEAPON;
	}

	if (GetType() == ITEM_ARMOR || GetType() == ITEM_COSTUME)
	{
		switch (GetSubType())
		{
			case ARMOR_BODY:
				return ATTRIBUTE_SET_BODY;

			case ARMOR_WRIST:
				return ATTRIBUTE_SET_WRIST;

			case ARMOR_FOOTS:
				return ATTRIBUTE_SET_FOOTS;

			case ARMOR_NECK:
				return ATTRIBUTE_SET_NECK;

			case ARMOR_HEAD:
				return ATTRIBUTE_SET_HEAD;

			case ARMOR_SHIELD:
				return ATTRIBUTE_SET_SHIELD;

			case ARMOR_EAR:
				return ATTRIBUTE_SET_EAR;
		}
	}

	return -1;
}

bool CItem::HasAttr(BYTE bApply)
{
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		if (m_pProto->aApplies[i].bType == bApply)
			return true;

	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
		if (GetAttributeType(i) == bApply)
			return true;

	return false;
}

bool CItem::HasApply(BYTE bApply)
{
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		if (m_pProto->aApplies[i].bType == bApply)
			return true;
		
	return false;
}

bool CItem::HasOnlyAttr(BYTE bApply)
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
		if (GetAttributeType(i) == bApply)
			return true;
	
	return false;
}

bool CItem::HasRareAttr(BYTE bApply)
{
	for (int i = 0; i < MAX_RARE_ATTR_NUM; ++i)
		if (GetAttributeType(i + 5) == bApply)
			return true;

	return false;
}

void CItem::AddNewAttr(BYTE index, BYTE bApply, BYTE bLevel)
{
	if (HasApply(bApply))
		return;

	if (bLevel <= 0)
		return;
	
	int iAttributeSet = GetAttributeSetIndex();
	
	const TItemAttrTable & r = g_map_itemAttr[bApply];
	if (!r.bMaxLevelBySet[iAttributeSet])
		return;

	int i = GetAttributeCount();

	if (i == 3)
	{
		RemoveAttributeAt(index);
		const TItemAttrTable & r = g_map_itemAttr[bApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetAttribute(index, bApply, lVal);		
	}
	else
	{
		const TItemAttrTable & r = g_map_itemAttr[bApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetAttribute(index, bApply, lVal);
	}
}

void CItem::AddAttribute(BYTE bApply, short sValue)
{
	if (HasAttr(bApply))
		return;

	int i = GetAttributeCount();

	if (i >= MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		if (sValue)
			SetAttribute(i, bApply, sValue);
	}
}

void CItem::AddAttr(BYTE bApply, BYTE bLevel)
{
	if (HasAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int i = GetAttributeCount();

	if (i == MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		const TItemAttrTable & r = g_map_itemAttr[bApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetAttribute(i, bApply, lVal);
	}
}

void CItem::PutAttributeWithLevel(BYTE bLevel)
{
	int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	// 붙일 수 있는 속성 배열을 구축
	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable & r = g_map_itemAttr[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasAttr(i))
		{
			avail.push_back(i);
			total += r.dwProb;
		}
	}

	// 구축된 배열로 확률 계산을 통해 붙일 속성 선정
	unsigned int prob = number(1, total);
	int attr_idx = APPLY_NONE;

	for (DWORD i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable & r = g_map_itemAttr[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable & r = g_map_itemAttr[attr_idx];

	// 종류별 속성 레벨 최대값 제한
	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddAttr(attr_idx, bLevel);
}

void CItem::PutAttribute(const int * aiAttrPercentTable)
{
	int iAttrLevelPercent = number(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutAttributeWithLevel(i + 1);
}

void CItem::ChangeAttribute(const int* aiChangeProb)
{
	int iAttributeCount = GetAttributeCount();

	ClearAttribute();

	if (iAttributeCount == 0)
		return;

	TItemTable const * pProto = GetProto();

	if (pProto && pProto->sAddonType)
	{
		ApplyAddon(pProto->sAddonType);
	}

	static const int tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		0, 10, 40, 35, 15,
	};

	for (int i = GetAttributeCount(); i < iAttributeCount; ++i)
	{
		if (aiChangeProb == NULL)
		{
			PutAttribute(tmpChangeProb);
		}
		else
		{
			PutAttribute(aiChangeProb);
		}
	}
}

#ifdef __NEW_ENCHANT_ATTR__
void CItem::PrepareAttribute()
{
	int iAttributeCount = GetAttributeCount();
	if (iAttributeCount == 0)
		return;
	
	TItemTable const * pProto = GetProto();
	if (pProto && pProto->sAddonType)
		ApplyAddon(pProto->sAddonType);
	
	static const int tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {0, 10, 40, 35, 15,};
	for (int i = 0; i < iAttributeCount; ++i)
	{
		int iAttrLevelPercent = number(1, 100);
		int c;
		for (c = 0; c < ITEM_ATTRIBUTE_MAX_LEVEL; ++c)
		{
			if (iAttrLevelPercent <= tmpChangeProb[c])
				break;
			
			iAttrLevelPercent -= tmpChangeProb[c];
		}
		
		BYTE bLevel = c + 1;
		int iAttributeSet = GetAttributeSetIndex();
		if (iAttributeSet < 0)
			break;
		
		if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
			break;
		
		int total = 0;
		std::vector<int> avail;
		for (int b = 0; b < MAX_APPLY_NUM; ++b)
		{
			const TItemAttrTable & r = g_map_itemAttr[b];
			if (r.bMaxLevelBySet[iAttributeSet] && !HasAttr(b))
			{
				if (GetOwner()->GetEnchantType1() != b && GetOwner()->GetEnchantType2() != b && GetOwner()->GetEnchantType3() != b && GetOwner()->GetEnchantType4() != b && GetOwner()->GetEnchantType5() != b)
				{
					avail.push_back(b);
					total += r.dwProb;
				}
			}
		}
		
		unsigned int prob = number(1, total);
		int attr_idx = APPLY_NONE;
		for (DWORD j = 0; j < avail.size(); ++j)
		{
			const TItemAttrTable & r = g_map_itemAttr[avail[j]];
			if (prob <= r.dwProb)
			{
				attr_idx = avail[j];
				break;
			}
			
			prob -= r.dwProb;
		}
		
		if (!attr_idx)
		{
			sys_err("Cannot prepare item attribute: %d %d.", iAttributeSet, bLevel);
			break;
		}
		
		const TItemAttrTable & r = g_map_itemAttr[attr_idx];
		if (bLevel > r.bMaxLevelBySet[iAttributeSet])
			bLevel = r.bMaxLevelBySet[iAttributeSet];
		
		long lVal = r.lValues[MIN(4, bLevel - 1)];
		if (!lVal)
			lVal = 0;
		
		GetOwner()->PrepareEnchantAttr(i, attr_idx, lVal);
	}
}
#endif

void CItem::AddAttribute()
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		40, 50, 10, 0, 0
	};

	if (GetAttributeCount() < MAX_NORM_ATTR_NUM)
		PutAttribute(aiItemAddAttributePercent);
}

void CItem::ClearAttribute()
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		m_aAttr[i].bType = 0;
		m_aAttr[i].sValue = 0;
	}
}

int CItem::GetAttributeCount()
{
	int i;

	for (i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == 0)
			break;
	}

	return i;
}

int CItem::GetAcceAttributeCount()
{
	int i;
	int c = 0;
	for (i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		if (GetAttributeType(i) != 0)
			c = c + 1;
	}
	
	return c;
}

int CItem::FindAttribute(BYTE bType)
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == bType)
			return i;
	}

	return -1;
}

bool CItem::RemoveAttributeAt(int index)
{
	if (GetAttributeCount() <= index)
		return false;

	for (int i = index; i < MAX_NORM_ATTR_NUM - 1; ++i)
	{
		SetAttribute(i, GetAttributeType(i + 1), GetAttributeValue(i + 1));
	}

	SetAttribute(MAX_NORM_ATTR_NUM - 1, APPLY_NONE, 0);
	return true;
}

bool CItem::RemoveAttributeType(BYTE bType)
{
	int index = FindAttribute(bType);
	return index != -1 && RemoveAttributeType(index);
}

void CItem::SetAttributes(const TPlayerItemAttribute* c_pAttribute)
{
	thecore_memcpy(m_aAttr, c_pAttribute, sizeof(m_aAttr));
	Save();
}

void CItem::SetAttribute(int i, BYTE bType, short sValue)
{
	assert(i < MAX_NORM_ATTR_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();

	if (bType)
	{
		const char * pszIP = NULL;

		if (GetOwner() && GetOwner()->GetDesc())
			pszIP = GetOwner()->GetDesc()->GetHostName();

		LogManager::instance().ItemLog(i, bType, sValue, GetID(), "SET_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum());
	}
}

void CItem::SetAttribute2(int i, BYTE bType, short sValue)
{
	if (i == 15)
		i = 6;
	else if (i == 14)
		i = 5;
	else
	{
		assert(i < MAX_NORM_ATTR_NUM);
	}

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}

void CItem::SetForceAttribute(int i, BYTE bType, short sValue)
{
	assert(i < ITEM_ATTRIBUTE_MAX_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();

	if (bType)
	{
		const char * pszIP = NULL;

		if (GetOwner() && GetOwner()->GetDesc())
			pszIP = GetOwner()->GetDesc()->GetHostName();

		LogManager::instance().ItemLog(i, bType, sValue, GetID(), "SET_FORCE_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum());
	}
}


void CItem::CopyAttributeTo(LPITEM pItem)
{
	pItem->SetAttributes(m_aAttr);
}

int CItem::GetRareAttrCount()
{
	int ret = 0;

	if (m_aAttr[5].bType != 0)
		ret++;

	if (m_aAttr[6].bType != 0)
		ret++;

	return ret;
}

bool CItem::ChangeRareAttribute()
{
	if (GetRareAttrCount() == 0)
		return false;

	int cnt = GetRareAttrCount();

	for (int i = 0; i < cnt; ++i)
	{
		m_aAttr[i + 5].bType = 0;
		m_aAttr[i + 5].sValue = 0;
	}

	if (GetOwner() && GetOwner()->GetDesc())
		LogManager::instance().ItemLog(GetOwner(), this, "SET_RARE_CHANGE", "");
	else
		LogManager::instance().ItemLog(0, 0, 0, GetID(), "SET_RARE_CHANGE", "", "", GetOriginalVnum());

	for (int i = 0; i < cnt; ++i)
	{
		AddRareAttribute();
	}

	return true;
}

bool CItem::AddRareAttribute()
{
	int count = GetRareAttrCount();

	if (count >= 2)
		return false;

	int pos = count + 5;
	TPlayerItemAttribute & attr = m_aAttr[pos];

	int nAttrSet = GetAttributeSetIndex();
	std::vector<int> avail;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable & r = g_map_itemRare[i];

		if (r.dwApplyIndex != 0 && r.bMaxLevelBySet[nAttrSet] > 0 && HasRareAttr(i) != true)
		{
			avail.push_back(i);
		}
	}

	const TItemAttrTable& r = g_map_itemRare[avail[number(0, avail.size() - 1)]];
	int nAttrLevel = 5;

	if (nAttrLevel > r.bMaxLevelBySet[nAttrSet])
		nAttrLevel = r.bMaxLevelBySet[nAttrSet];

	attr.bType = r.dwApplyIndex;
	attr.sValue = r.lValues[nAttrLevel - 1];

	UpdatePacket();

	Save();

	const char * pszIP = NULL;

	if (GetOwner() && GetOwner()->GetDesc())
		pszIP = GetOwner()->GetDesc()->GetHostName();

	LogManager::instance().ItemLog(pos, attr.bType, attr.sValue, GetID(), "SET_RARE", "", pszIP ? pszIP : "", GetOriginalVnum());
	return true;
}

