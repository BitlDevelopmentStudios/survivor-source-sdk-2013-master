//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#include "cbase.h"
#include "player.h"
#include "hl2mp_player.h"
#include "in_buttons.h"
#include "movehelper_server.h"
#include "gameinterface.h"
#include "bot_main.h"

bool AcquireEnemy( CSurvivorBot *pBot )
{
	float minDist = FLT_MAX;
	bool Success = false;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		if (pPlayer && pPlayer != NULL && pPlayer->IsAlive() && pPlayer != pBot && pPlayer->GetTeamNumber() != TEAM_SPECTATOR)
		{
			if (HL2MPRules()->IsTeamplay())
			{
				if (pPlayer->GetTeamNumber() != pBot->GetTeamNumber())
				{
					float dist = (pBot->GetLocalOrigin() - pPlayer->GetLocalOrigin()).Length();

					if (dist < minDist)
					{
						minDist = dist;
						pBot->hEnemy.Set(pPlayer);
						Success = true;
					}
				}
			}
			else
			{
				float dist = (pBot->GetLocalOrigin() - pPlayer->GetLocalOrigin()).Length();

				if (dist < minDist)
				{
					minDist = dist;
					pBot->hEnemy.Set(pPlayer);
					Success = true;
				}
			}
		}
	}

	return Success;
}

void SwitchtoMelee(CSurvivorBot *pBot)
{
	if (!survivor_randomizer.GetBool())
	{
		int iFaction = pBot->GetFaction();

		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			pBot->Weapon_Switch(pBot->Weapon_OwnsThisType("weapon_crowbar"));
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			pBot->Weapon_Switch(pBot->Weapon_OwnsThisType("weapon_stunstick"));
		}
	}
	else
	{
		CBaseCombatWeapon *pCheck;

		for (int i = 0; i < pBot->WeaponCount(); ++i)
		{
			pCheck = pBot->GetWeapon(i);
			if (!pCheck)
				continue;

			if (BotWeaponRangeDetermine(pCheck) == SKILL_MELEE_RANGE && pCheck->GetWeight() > 0)
			{
				pBot->Weapon_Switch(pCheck);
			}
		}
	}
}

CBaseCombatWeapon *GetNextBestWeaponBot(CSurvivorBot *pPlayer, CBaseCombatWeapon *pCurrentWeapon)
{
	CBaseCombatWeapon *pCheck;
	CBaseCombatWeapon *pBest;// this will be used in the event that we don't find a weapon in the same category.

	int iCurrentWeight = -1;
	int iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	if (pCurrentWeapon)
	{
		iCurrentWeight = pCurrentWeapon->GetWeight();
	}

	for (int i = 0; i < pPlayer->WeaponCount(); ++i)
	{
		pCheck = pPlayer->GetWeapon(i);
		if (!pCheck)
			continue;

		if (pCheck->GetWeight() > iBestWeight && pCheck != pCurrentWeapon)// don't reselect the weapon we're trying to get rid of
		{
			//Msg( "Considering %s\n", STRING( pCheck->GetClassname() );
			// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
			// that the player was using. This will end up leaving the player with his heaviest-weighted 
			// weapon. 
			if (pCheck->HasAnyAmmo())
			{
				// if this weapon is useable, flag it as the best
				iBestWeight = pCheck->GetWeight();
				pBest = pCheck;
			}
		}

		if (!pCheck->HasAnyAmmo())
		{
			SwitchtoMelee(pPlayer);
			CBaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();
			return pActiveWeapon;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 

	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	return pBest;
}

bool SwitchToNextBestWeaponBot(CSurvivorBot *pPlayer, CBaseCombatWeapon *pCurrent)
{
	CBaseCombatWeapon *pNewWeapon = GetNextBestWeaponBot(pPlayer, pCurrent);

	if ((pNewWeapon != NULL) && (pNewWeapon != pCurrent))
	{
		return pPlayer->Weapon_Switch(pNewWeapon);
	}

	return false;
}

void BotAttack( CSurvivorBot *pBot, CUserCmd &cmd )
{
	// EXCEPTIONS
	if (!pBot->m_bEnemyOnSights || !pBot->m_bInRangeToAttack || pBot->m_flNextBotAttack > gpGlobals->curtime)
		return;

	CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();

	if (!pActiveWeapon->HasAnyAmmo())
	{
		SwitchToNextBestWeaponBot(pBot, pActiveWeapon);
	}

	if (pBot->GetWaterLevel() == 3)
	{
		SwitchtoMelee(pBot);
	}
	else
	{
		SwitchToNextBestWeaponBot(pBot, pActiveWeapon);
	}

	CBaseCombatWeapon *pAR2 = pBot->Weapon_OwnsThisType("weapon_ar2");
	CBaseCombatWeapon *pSMG1 = pBot->Weapon_OwnsThisType("weapon_smg1");
	CBaseCombatWeapon *pGatling = pBot->Weapon_OwnsThisType("weapon_gatling");
	CBaseCombatWeapon *pDualPistol = pBot->Weapon_OwnsThisType("weapon_dualpistol");
	CBaseCombatWeapon *pHealgun = pBot->Weapon_OwnsThisType("weapon_healgun");
	CBaseCombatWeapon *pPhyscannon = pBot->Weapon_OwnsThisType("weapon_physcannon");
	
	if (pActiveWeapon == pAR2 || pActiveWeapon == pSMG1)
	{
		if (random->RandomInt(0, 4) > 0)
		{
			cmd.buttons |= IN_ATTACK;
		}
		else
		{
			cmd.buttons |= IN_ATTACK2;
		}
		pBot->m_flNextBotAttack = gpGlobals->curtime + 0.1f;
	}
	else if (pActiveWeapon == pGatling)
	{
		cmd.buttons |= IN_ATTACK;
		pBot->m_flNextBotAttack = gpGlobals->curtime + 0.1f;
	}
	else if (pActiveWeapon == pDualPistol)
	{
		cmd.buttons |= IN_ATTACK;
		pBot->m_flNextBotAttack = gpGlobals->curtime + 0.2f;
	}
	else if ((pActiveWeapon == pHealgun && survivor_randomizer.GetBool()) || (pActiveWeapon == pPhyscannon && survivor_randomizer.GetBool()))
	{
		SwitchToNextBestWeaponBot(pBot, pActiveWeapon);
	}
	else
	{
		cmd.buttons |= IN_ATTACK;
		pBot->m_flNextBotAttack = gpGlobals->curtime + 0.75f;
	}
}


