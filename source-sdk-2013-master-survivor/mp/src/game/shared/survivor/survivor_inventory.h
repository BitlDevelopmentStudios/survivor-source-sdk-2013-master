//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple Inventory
// by MrModez modified by Bitl
// $NoKeywords: $
//=============================================================================//
#ifndef SURVIVOR_INVENTORY_H
#define SURVIVOR_INVENTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
//#include "server_class.h"
#include "survivor_shareddefs.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "filesystem.h" 
#if defined( CLIENT_DLL )
#include "hl2mp/c_hl2mp_player.h"
#endif

#define INVENTORY_SLOTS			3
#define INVENTORY_WEAPONS		5
#define INVENTORY_WEAPONS_COUNT	1500
#define INVENTORY_COLNUM		5
#define INVENTORY_ROWNUM		3
#define INVENTORY_VECTOR_NUM	INVENTORY_COLNUM * INVENTORY_ROWNUM

class CSURVIVORInventory
{
public:
	CSURVIVORInventory();

	int GetWeapon(int iClass, int iFaction, int iSlot, int iNum);
	bool CheckValidSlot(int iClass, int iFaction, int iSlot, bool HudCheck);
	bool CheckValidWeapon(int iClass, int iFaction, int iSlot, int iWeapon, bool HudCheck);

//#if defined( CLIENT_DLL )
	KeyValues* GetInventory(IBaseFileSystem *pFileSystem);
	void SetInventory(IBaseFileSystem *pFileSystem, KeyValues* pInventory);
	int GetLocalPreset(KeyValues* pInventory, int iClass, int iFaction, int iSlot);
	int GetWeaponPreset(IBaseFileSystem *pFileSystem, int iClass, int iFaction, int iSlot);
	const char* GetSlotName(int iSlot);
//#endif


private:
	static const int Weapons[SURVIVOR_CLASS_COUNT][SURVIVOR_FACTION_COUNT][INVENTORY_SLOTS][INVENTORY_WEAPONS];
	static const char *g_aPlayerSlotNames[INVENTORY_SLOTS];
};

CSURVIVORInventory *GetSURVIVORInventory();

#endif // TF_INVENTORY_H
