#include "stdafx.h"
#include "constants.h"
#include "char.h"
#include "desc.h"
#include "desc_manager.h"
#include "packet.h"
#include "item.h"

/////////////////////////////////////////////////////////////////////////////
// KEYBOARD SYSTEM - VERSION 0.0.1
/////////////////////////////////////////////////////////////////////////////
bool CHARACTER::IsValidCellForQuickSlot(BYTE bOldPos, BYTE bNewPos)
{
	if (m_quickslot[bNewPos].pos == m_newquickslot[bNewPos].pos)
	{
		return false;
	}
	
	return true;
}

bool GetRequirementsLevel(LPCHARACTER ch, BYTE srcCell)
{
	struct SValidPosition
	{
		int pos;
		int level;
	};

	SValidPosition m_configuration[QUICKSLOT_MAX_NUM / 4 - 1] =
	{
		{	1,	 1	},
		{	2, 	45	},
		{	3, 	55	},
		{	4, 	65	},
		{	5, 	75	},
		{	6, 	85	},
		{	7, 	95	},
		{	8, 	105	}
	};
	
	struct SValidPosition * m2_user = m_configuration;

	if (srcCell == m2_user[srcCell].pos - 1 && ch->GetLevel() < m2_user[srcCell].level)
	{
		return false;
	}
	
	return true;
}

void CHARACTER::SyncNewQuickslot(BYTE bType, BYTE bOldPos, BYTE bNewPos)
{
	if (bOldPos == bNewPos)
		return;

	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_newquickslot[i].type == bType && m_newquickslot[i].pos == bOldPos)
		{
			if (bNewPos == 255)
			{
				DelNewQuickslot(i);
			}
			else
			{
				TNewQuickslot slot;

				slot.type = bType;
				slot.pos = bNewPos;

				SetNewQuickslot(i, slot);
			}
		}
	}
}

bool CHARACTER::GetNewQuickslot(BYTE pos, TNewQuickslot ** ppSlot)
{
	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	*ppSlot = &m_newquickslot[pos];
	return true;
}

bool CHARACTER::SetNewQuickslot(BYTE pos, TNewQuickslot & rSlot)
{
	struct packet_newquickslot_add pack_newquickslot_add;

	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	if (rSlot.type >= QUICKSLOT_TYPE_MAX_NUM)
		return false;

	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (rSlot.type == 0)
			continue;

		else if (m_newquickslot[i].type == rSlot.type && m_newquickslot[i].pos == rSlot.pos)
			DelNewQuickslot(i);
	}

	TItemPos srcCell(INVENTORY, rSlot.pos);

	switch (rSlot.type)
	{
		case QUICKSLOT_TYPE_ITEM:
			if (false == srcCell.IsDefaultInventoryPosition() && false == srcCell.IsBeltInventoryPosition())
				return false;

			break;

		case QUICKSLOT_TYPE_SKILL:
			if ((int) rSlot.pos >= SKILL_MAX_NUM)
				return false;

			break;

		case QUICKSLOT_TYPE_COMMAND:
			break;

		default:
			return false;
	}

	m_newquickslot[pos] = rSlot;

	if (GetDesc())
	{
		pack_newquickslot_add.header	= HEADER_GC_NEWQUICKSLOT_ADD;
		pack_newquickslot_add.pos		= pos;
		pack_newquickslot_add.slot		= m_newquickslot[pos];

		GetDesc()->Packet(&pack_newquickslot_add, sizeof(pack_newquickslot_add));
	}

	return true;
}

bool CHARACTER::DelNewQuickslot(BYTE pos)
{
	struct packet_newquickslot_del pack_newquickslot_del;

	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	memset(&m_newquickslot[pos], 0, sizeof(TNewQuickslot));

	pack_newquickslot_del.header	= HEADER_GC_NEWQUICKSLOT_DEL;
	pack_newquickslot_del.pos	= pos;

	GetDesc()->Packet(&pack_newquickslot_del, sizeof(pack_newquickslot_del));
	return true;
}

bool CHARACTER::SwapNewQuickslot(BYTE a, BYTE b)
{
	struct packet_newquickslot_swap pack_newquickslot_swap;
	TNewQuickslot quickslot;

	if (a >= QUICKSLOT_MAX_NUM || b >= QUICKSLOT_MAX_NUM)
		return false;
	
	if (!GetRequirementsLevel(this, a) || !GetRequirementsLevel(this, b))
		return false;

	quickslot = m_newquickslot[a];

	m_newquickslot[a] = m_newquickslot[b];
	m_newquickslot[b] = quickslot;

	pack_newquickslot_swap.header	= HEADER_GC_NEWQUICKSLOT_SWAP;
	pack_newquickslot_swap.pos	= a;
	pack_newquickslot_swap.pos_to	= b;

	GetDesc()->Packet(&pack_newquickslot_swap, sizeof(pack_newquickslot_swap));
	return true;
}

void CHARACTER::ChainNewQuickslotItem(LPITEM pItem, BYTE bType, BYTE bOldPos)
{
	if (pItem->IsDragonSoul())
		return;
	
	if (!GetRequirementsLevel(this, bOldPos))
		return;

	for ( int i=0; i < QUICKSLOT_MAX_NUM; ++i )
	{
		if ( m_newquickslot[i].type == bType && m_newquickslot[i].pos == bOldPos )
		{
			TNewQuickslot slot;
			slot.type = bType;
			slot.pos = pItem->GetCell();
			SetNewQuickslot(i, slot);
			break;
		}
	}
}