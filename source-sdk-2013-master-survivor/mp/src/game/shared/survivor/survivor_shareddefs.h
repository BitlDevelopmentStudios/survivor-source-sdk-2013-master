//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SURVIVOR_SHAREDDEFS_H
#define SURVIVOR_SHAREDDEFS_H

#ifdef _WIN32
#pragma once
#endif

#include "const.h"

//--------------
// SURVIVOR SPECIFIC
//--------------
#define DMG_SNIPER			(DMG_LASTGENERICFLAG<<1)	// This is sniper damage
#define DMG_MINIGUN			(DMG_LASTGENERICFLAG<<2)	// Minigun Knockback Damage
#define DMG_HEALGUN			(DMG_LASTGENERICFLAG<<3)	// Healgun Healing

#define CRIT_DMG	15

//weapons

enum
{
	WEAPON_NONE = 0,
	WEAPON_357,
	WEAPON_AR2,
	WEAPON_CROSSBOW,
	WEAPON_CROWBAR,
	WEAPON_FRAG,
	WEAPON_PHYSCANNON,
	WEAPON_PISTOL,
	WEAPON_RPG,
	WEAPON_SHOTGUN,
	WEAPON_SLAM,
	WEAPON_SMG1,
	WEAPON_STUNSTICK,
	WEAPON_DUALPISTOL,
	WEAPON_DUALRPG,
	WEAPON_GATLING,
	WEAPON_HEALGUN,
	WEAPON_SHOTGUNDOUBLE,
	WEAPON_SNIPER_RIFLE,

	WEAPON_COUNT
};

extern const char *g_aWeaponNames[];
extern const char *g_aClassNames[];
extern const char *g_aFactionNames[];
int GetWeaponId(const char *pszWeaponName);
const char *WeaponIdToAlias(int iWeapon);
const char *WeaponIdToClassname(int iWeapon);


#endif // SURVIVOR_SHAREDDEFS_H
