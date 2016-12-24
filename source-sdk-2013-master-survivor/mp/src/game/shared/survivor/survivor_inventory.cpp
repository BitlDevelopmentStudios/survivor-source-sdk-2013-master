//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple Inventory
// by MrModez modified by Bitl
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "survivor_shareddefs.h"
#include "survivor_inventory.h"

CSURVIVORInventory *pSURVIVORInventory = NULL;

CSURVIVORInventory *GetSURVIVORInventory()
{
	if (NULL == pSURVIVORInventory)
	{
		pSURVIVORInventory = new CSURVIVORInventory();
	}
	return pSURVIVORInventory;
}

CSURVIVORInventory::CSURVIVORInventory()
{
};

int CSURVIVORInventory::GetWeapon(int iClass, int iFaction, int iSlot, int iNum)
{
	return Weapons[iClass][iFaction][iSlot][iNum];
};

bool CSURVIVORInventory::CheckValidSlot(int iClass, int iFaction, int iSlot, bool HudCheck)
{
	if (iClass < SURVIVOR_CLASS_UNDEFINED || iClass >= SURVIVOR_CLASS_COUNT)
		return false;
	int iCount = (HudCheck ? INVENTORY_ROWNUM : INVENTORY_WEAPONS);
	if (iSlot >= iCount || iSlot < 0)
		return false;
	bool bWeapon = false;
	for (int i = 0; i < iCount; i++) //if there's at least one weapon in slot
	{
		if (Weapons[iClass][iFaction][iSlot][i])
		{
			bWeapon = true;
			break;
		}
	}
	return bWeapon;
};

bool CSURVIVORInventory::CheckValidWeapon(int iClass, int iFaction, int iSlot, int iWeapon, bool HudCheck)
{
	if (iClass < SURVIVOR_CLASS_UNDEFINED || iClass >= SURVIVOR_CLASS_COUNT)
		return false;
	int iCount = (HudCheck ? INVENTORY_ROWNUM : INVENTORY_WEAPONS);
	if (iWeapon >= iCount || iWeapon < 0)
		return false;
	if (!Weapons[iClass][iFaction][iSlot][iWeapon])
		return false;
	return true;
};

//commenting this sout since we want the server to use this too.
//#if defined( CLIENT_DLL )
const char* CSURVIVORInventory::GetSlotName(int iSlot)
{
	return g_aPlayerSlotNames[iSlot];
};

KeyValues* CSURVIVORInventory::GetInventory(IBaseFileSystem *pFileSystem)
{
	KeyValues *pInv = new KeyValues("Loadout");
	pInv->LoadFromFile(pFileSystem, "scripts/loadout.txt");
	return pInv;
};

void CSURVIVORInventory::SetInventory(IBaseFileSystem *pFileSystem, KeyValues* pInventory)
{
	pInventory->SaveToFile(pFileSystem, "scripts/loadout.txt");
};

int CSURVIVORInventory::GetLocalPreset(KeyValues* pInventory, int iClass, int iFaction, int iSlot)
{
	KeyValues *pSub = pInventory->FindKey(g_aClassNames[iClass]);
	if (!pSub)
		return 0;
	KeyValues *pSub2 = pSub->FindKey(g_aFactionNames[iFaction]);
	if (!pSub2)
		return 0;
	const int iPreset = pSub2->GetInt(g_aPlayerSlotNames[iSlot], 0);
	return iPreset;
};

int CSURVIVORInventory::GetWeaponPreset(IBaseFileSystem *pFileSystem, int iClass, int iFaction, int iSlot)
{
	return GetLocalPreset(GetInventory(pFileSystem), iClass, iFaction, iSlot);
};
//#endif

const char *CSURVIVORInventory::g_aPlayerSlotNames[INVENTORY_SLOTS] =
{
	"Primary",
	"Secondary",
	"Melee",
};

const int CSURVIVORInventory::Weapons[SURVIVOR_CLASS_COUNT][SURVIVOR_FACTION_COUNT][INVENTORY_SLOTS][INVENTORY_WEAPONS] =
{
	{

	},
	//ranger
	{
		{
			{
				WEAPON_AR2
			},
			{
				WEAPON_DUALPISTOL
			},
			{
				WEAPON_STUNSTICK
			}
		},
		{
			{
				WEAPON_SMG1
			},
			{
				WEAPON_SHOTGUN
			},
			{
				WEAPON_CROWBAR
			}
		}
	},
	//medic
	{
		{
			{
				WEAPON_AR2
			},
			{
				WEAPON_HEALGUN
			},
			{
				WEAPON_STUNSTICK
			}
		},
		{
			{
				WEAPON_SMG1
			},
			{
				WEAPON_HEALGUN
			},
			{
				WEAPON_CROWBAR
			}
		}
	},
	//soldier
	{
		{
			{
				WEAPON_AR2
			},
			{
				WEAPON_GATLING
			},
			{
				WEAPON_STUNSTICK
			}
		},
		{
			{
				WEAPON_SMG1
			},
			{
				WEAPON_RPG
			},
			{
				WEAPON_CROWBAR
			}
		}
	},
	//sniper
	{
		{
			{
				WEAPON_AR2
			},
			{
				WEAPON_SNIPER_RIFLE
			},
			{
				WEAPON_STUNSTICK
			}
		},
		{
			{
				WEAPON_SMG1
			},
			{
				WEAPON_SNIPER_RIFLE
			},
			{
				WEAPON_CROWBAR
			}
		}
	},
	//engineer
	{
		{
			{
				WEAPON_AR2
			},
			{
				WEAPON_PHYSCANNON
			},
			{
				WEAPON_STUNSTICK
			}
		},
		{
			{
				WEAPON_SMG1
			},
			{
				WEAPON_PHYSCANNON
			},
			{
				WEAPON_CROWBAR
			}
		}
	},
};