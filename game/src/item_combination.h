#ifndef _item_combination_h_
#define _item_combination_h_
#define COMB_MAX_NUM	10
#define COMB_MAX_DISTANCE	1000

void Comb_open_costume(LPCHARACTER ch);
void Comb_open_skillbook(LPCHARACTER ch);
void Comb_add_item(LPCHARACTER ch, int combination_index, int inven_index);
void Comb_add_item2(LPCHARACTER ch, int combination_index, int inven_index);
void Comb_del_item(LPCHARACTER ch, int combination_index);
bool Comb_make_cotume(LPCHARACTER ch);
bool Comb_make_skillbook(LPCHARACTER ch);
void Comb_close(LPCHARACTER ch);
void Comb_clean_item(LPCHARACTER ch);
void Comb_init();

#endif

