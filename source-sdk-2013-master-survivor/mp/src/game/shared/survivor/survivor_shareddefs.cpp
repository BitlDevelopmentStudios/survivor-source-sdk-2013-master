#include "cbase.h"
#include "survivor_shareddefs.h"

const char *g_aWeaponNames[] =
{
	"WEAPON_NONE",
	"WEAPON_357",
	"WEAPON_AR2",
	"WEAPON_CROSSBOW",
	"WEAPON_CROWBAR",
	"WEAPON_FRAG",
	"WEAPON_PHYSCANNON",
	"WEAPON_PISTOL",
	"WEAPON_RPG",
	"WEAPON_SHOTGUN",
	"WEAPON_SLAM",
	"WEAPON_SMG1",
	"WEAPON_STUNSTICK",
	"WEAPON_DUALPISTOL",
	"WEAPON_DUALRPG",
	"WEAPON_GATLING",
	"WEAPON_HEALGUN",
	"WEAPON_SHOTGUNDOUBLE",
	"WEAPON_SNIPER_RIFLE",

	"WEAPON_COUNT",
};

const char *g_aClassNames[] =
{
	"Undefined",
	"Ranger",
	"Medic",
	"Soldier",
	"Sniper",
	"Engineer",
};

const char *g_aFactionNames[] =
{
	"Combine",
	"Resistance",
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int GetWeaponId( const char *pszWeaponName )
{
	// if this doesn't match, you need to add missing weapons to the array
	assert( ARRAYSIZE( g_aWeaponNames ) == ( SURVIVOR_WEAPON_COUNT + 1 ) );

	for ( int iWeapon = 0; iWeapon < ARRAYSIZE( g_aWeaponNames ); ++iWeapon )
	{
		if ( !Q_stricmp( pszWeaponName, g_aWeaponNames[iWeapon] ) )
			return iWeapon;
	}

	return WEAPON_NONE;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *WeaponIdToAlias( int iWeapon )
{
	// if this doesn't match, you need to add missing weapons to the array
	assert( ARRAYSIZE( g_aWeaponNames ) == ( SURVIVOR_WEAPON_COUNT + 1 ) );

	if ( ( iWeapon >= ARRAYSIZE( g_aWeaponNames ) ) || ( iWeapon < 0 ) )
		return NULL;

	return g_aWeaponNames[iWeapon];
}

//-----------------------------------------------------------------------------
// Purpose: Entity classnames need to be in lower case. Use this whenever
// you're spawning a weapon.
//-----------------------------------------------------------------------------
const char *WeaponIdToClassname( int iWeapon )
{
	const char *pszWeaponAlias = WeaponIdToAlias( iWeapon );

	if ( pszWeaponAlias == NULL )
		return NULL;

	static char szEntName[256];
	Q_strcpy( szEntName, pszWeaponAlias );
	Q_strlower( szEntName );

	return szEntName;
}
