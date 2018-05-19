#ifndef __INC_METIN_II_GAME_BANK_SYSTEM_H__
#define __INC_METIN_II_GAME_BANK_SYSTEM_H__
/*********************************************************************
* date        : 2016.09.21
* file        : bank.h
* author      : VegaS
* version	  : 0.0.4
*/
#pragma once
class BankManager : public singleton<BankManager>
{
	public:
		BankManager();
		~BankManager();

	bool	Send_Create(LPCHARACTER ch, const char* mNewAccount, const char* mNewPassword);			
	bool	Send_Open(LPCHARACTER ch, const char* mAccount, const char* mPassword);	
	bool	Send_AddMoney(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mAmount);
	bool	Send_WithdrawMoney(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mAmount);
	bool	Send_Money(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mTargetAccount, const char* mAmount);
	bool	Send_ChangePassword(LPCHARACTER ch, const char* mAccount, const char* mPassword, const char* mNewPassword, const char* mNewPasswordRepeat);
};
#endif