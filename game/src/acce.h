#ifndef _acce_h_
#define _acce_h_
#define ACCE_MAX_NUM	2
#define ACCE_MAX_DISTANCE	1000

struct ACCE_VALUE
{
	DWORD	vnum;
	int		count;
	bool operator == (const ACCE_VALUE& b)
	{
		return (this->count == b.count) && (this->vnum == b.vnum);
	}
};

struct ACCE_DATA
{
	std::vector<WORD>	npc_vnum;
	std::vector<ACCE_VALUE>	item;
	std::vector<ACCE_VALUE>	reward;
	int	percent;
	int	gold;
	int	abs_chance_min;
	int	abs_chance_max;
	
	ACCE_DATA();
	
	bool		can_make_item (LPITEM *items, WORD npc_vnum);
	ACCE_VALUE*	reward_value ();
	void		remove_material(LPCHARACTER ch, int result);
};

void Acce_init();
bool Acce_load(const char *file);
bool Acce_make(LPCHARACTER ch);
bool Acce_absorption_make(LPCHARACTER ch);
void Acce_clean_item(LPCHARACTER ch);
void Acce_open(LPCHARACTER ch);
void Acce_absorption_open(LPCHARACTER ch);
void Acce_close(LPCHARACTER ch);
void Acce_show_list(LPCHARACTER ch);
void Acce_add_item(LPCHARACTER ch, int acce_index, int inven_index);
void Acce_delete_item(LPCHARACTER ch, int acce_index);
void Acce_request_result_list(LPCHARACTER ch);
void Acce_request_material_info(LPCHARACTER ch, int request_start_index, int request_count = 1);
void Acce_print();
bool Acce_InformationInitialize();
#endif

