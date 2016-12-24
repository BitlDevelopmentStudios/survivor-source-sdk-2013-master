//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp_player.h"
#include "hl2mp_gamerules.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar survivor_teamplay_dogtagscore("survivor_teamplay_dogtagscore", "500");
ConVar survivor_teamplay_dogtagplayerscore("survivor_teamplay_dogtagplayerscore", "10");
ConVar survivor_ffa_dogtagplayerscore("survivor_ffa_dogtagplayerscore", "30");

class CItemDogTagRed : public CItem
{
public:
	DECLARE_CLASS(CItemDogTagRed, CItem);

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/dogtag/dtag_r.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/dogtag/dtag_r.mdl");
		PrecacheScriptSound("ItemDogtag.Touch");
		PrecacheScriptSound("ItemDogtag.Lost");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		CHL2MP_Player *pHL2Player = dynamic_cast<CHL2MP_Player *>( pPlayer );
		if (pHL2Player->GetTeamNumber() == TEAM_BLUE)
		{
			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			WRITE_STRING(GetClassname());
			MessageEnd();

			pHL2Player->AddPointsToTeam(survivor_teamplay_dogtagscore.GetInt(), false);
			pHL2Player->AddPoints(survivor_teamplay_dogtagplayerscore.GetInt(), false);
			CPASAttenuationFilter filter(pPlayer, "ItemDogtag.Touch");
			EmitSound(filter, pPlayer->entindex(), "ItemDogtag.Touch");
			if (g_pGameRules->ItemShouldRespawn(this))
			{
				Respawn();
			}
			else
			{
				UTIL_Remove(this);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
};

LINK_ENTITY_TO_CLASS(item_dogtag_red, CItemDogTagRed);
PRECACHE_REGISTER(item_dogtag_red);

class CItemDogTagBlue : public CItem
{
public:
	DECLARE_CLASS(CItemDogTagBlue, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/dogtag/dtag_b.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/dogtag/dtag_b.mdl");
		PrecacheScriptSound("ItemDogtag.Touch");
		PrecacheScriptSound("ItemDogtag.Lost");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CHL2MP_Player *pHL2Player = dynamic_cast<CHL2MP_Player *>(pPlayer);
		if (pHL2Player->GetTeamNumber() == TEAM_RED)
		{
			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			WRITE_STRING(GetClassname());
			MessageEnd();

			pHL2Player->AddPointsToTeam(survivor_teamplay_dogtagscore.GetInt(), false);
			pHL2Player->AddPoints(survivor_teamplay_dogtagplayerscore.GetInt(), false);
			CPASAttenuationFilter filter(pPlayer, "ItemDogtag.Touch");
			EmitSound(filter, pPlayer->entindex(), "ItemDogtag.Touch");
			if (g_pGameRules->ItemShouldRespawn(this))
			{
				Respawn();
			}
			else
			{
				UTIL_Remove(this);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
};

LINK_ENTITY_TO_CLASS(item_dogtag_blue, CItemDogTagBlue);
PRECACHE_REGISTER(item_dogtag_blue);

class CItemDogTagFFA : public CItem
{
public:
	DECLARE_CLASS(CItemDogTagFFA, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/dogtag/dtag_g.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/dogtag/dtag_g.mdl");
		PrecacheScriptSound("ItemDogtag.Touch");
		PrecacheScriptSound("ItemDogtag.Lost");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING(GetClassname());
		MessageEnd();

		CHL2MP_Player *pHL2Player = dynamic_cast<CHL2MP_Player *>(pPlayer);
		pHL2Player->AddPoints(survivor_ffa_dogtagplayerscore.GetInt(), false);
		CPASAttenuationFilter filter(pPlayer, "ItemDogtag.Touch");
		EmitSound(filter, pPlayer->entindex(), "ItemDogtag.Touch");
		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_dogtag, CItemDogTagFFA);
PRECACHE_REGISTER(item_dogtag);

