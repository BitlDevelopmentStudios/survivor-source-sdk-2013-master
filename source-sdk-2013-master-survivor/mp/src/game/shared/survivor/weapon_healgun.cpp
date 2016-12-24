
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponHealgun C_WeaponHealgun
#endif

//-----------------------------------------------------------------------------
// CWeaponHealgun
//-----------------------------------------------------------------------------

class CWeaponHealgun : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponHealgun, CBaseHL2MPCombatWeapon );
public:

	CWeaponHealgun( void );

	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	PrimaryAttackTeamplay(void);
	void	PrimaryAttackFFA(void);
	void	ItemPostFrame(void);
	bool 	HealPlayer( void );
	bool 	HealSelfPlayer( void );
	bool	Deploy(void);
	bool	Reload(void);
	virtual int	 GetWeaponID(void) const			{ return WEAPON_HEALGUN; }
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

private:

	CNetworkVar(bool, m_bIsReloading);
	
	CWeaponHealgun( const CWeaponHealgun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHealgun, DT_WeaponHealgun )

BEGIN_NETWORK_TABLE( CWeaponHealgun, DT_WeaponHealgun )
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bIsReloading)),
#else
SendPropBool(SENDINFO(m_bIsReloading)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponHealgun )
DEFINE_PRED_FIELD(m_bIsReloading, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_healgun, CWeaponHealgun );
PRECACHE_WEAPON_REGISTER( weapon_healgun );


#ifndef CLIENT_DLL
acttable_t CWeaponHealgun::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2AC_IDLE_INJECTION,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2AC_RUN_INJECTION,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2AC_CROUCH_INJECTION,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2AC_WALK_CROUCH_INJECTION,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2AC_GESTURE_RANGE_ATTACK_INJECTION,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2AC_JUMP_INJECTION,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL,				false },
};



IMPLEMENT_ACTTABLE( CWeaponHealgun );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponHealgun::CWeaponHealgun( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_bIsReloading		= false;
}

bool CWeaponHealgun::Deploy(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		return false;
	}

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	pViewModel->m_nSkin = 1;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHealgun::PrimaryAttack( void )
{
	if (HL2MPRules()->IsTeamplay())
	{
		PrimaryAttackTeamplay();
	}
	else
	{
		PrimaryAttackFFA();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHealgun::SecondaryAttack( void ) 
{
	PrimaryAttackFFA();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHealgun::PrimaryAttackTeamplay(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		return;
	}

	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
			m_flNextSecondaryAttack = 0.15;
		}

		return;
	}

	WeaponSound(SINGLE);
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pOwner->SetAnimation(PLAYER_ATTACK1);
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.75;
	m_iClip1--;
	HealPlayer();
	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHealgun::PrimaryAttackFFA(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		return;
	}

	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
			m_flNextSecondaryAttack = 0.15;
		}

		return;
	}

	WeaponSound(WPN_DOUBLE);
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	pOwner->SetAnimation(PLAYER_ATTACK1);
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.75;
	m_iClip1--;
	HealSelfPlayer();
	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}

bool CWeaponHealgun::Reload(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		return false;
	}

	CBaseViewModel *pViewModel = pOwner->GetViewModel();
	pViewModel->m_nSkin = 0;
	m_bIsReloading = true;
	return BaseClass::Reload();
}

void CWeaponHealgun::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		return;
	}

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (!m_bInReload)
	{
		m_bIsReloading = false;
	}

	if (!m_bIsReloading)
	{
		pViewModel->m_nSkin = 1;
	}
	
	BaseClass::ItemPostFrame();
}

bool CWeaponHealgun::HealPlayer( void )
{
   CBasePlayer *pOwner = ToBasePlayer(GetOwner());
 
   if (!pOwner)
   {
      return false;
   }
 
   Vector vecSrc, vecAiming;
   
   vecSrc = pOwner->EyePosition();
 
   QAngle angles = pOwner->GetLocalAngles();
 
   AngleVectors( angles, &vecAiming );
 
   trace_t tr;
 
   Vector   vecEnd = vecSrc + (vecAiming * 42);
   UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
 
   if (tr.fraction < 1.0)
   {
      if (tr.m_pEnt)
      {
         CBaseEntity *pEntity = tr.m_pEnt;
         if (pEntity->IsPlayer())
         {
            if (pEntity->GetHealth()<pEntity->GetMaxHealth() && pEntity->GetTeamNumber() == pOwner->GetTeamNumber())
            {
#ifndef CLIENT_DLL
               CBasePlayer *pPlayer = ToBasePlayer(pEntity);
 
               CPASAttenuationFilter filter( pPlayer, "HealthVial.Touch" );
               EmitSound( filter, pPlayer->entindex(), "HealthVial.Touch" );
               pEntity->TakeHealth( 20, DMG_GENERIC );
#endif
               return true;
            }
			else
			{
				return false;
			}
         }
      }
      return false;
   }
   else
   {
      return false;
   }
}

bool CWeaponHealgun::HealSelfPlayer( void )
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
 
	if (!pOwner)
	{
		return false;
	}
 
	if (pOwner->GetHealth()<pOwner->GetMaxHealth())
	{
#ifndef CLIENT_DLL
 
		CPASAttenuationFilter filter( pOwner, "HealthVial.Touch" );
		EmitSound( filter, pOwner->entindex(), "HealthVial.Touch" );
		pOwner->TakeHealth( 20, DMG_GENERIC );
#endif
		return true;
    }
	else
	{
		return false;
	}
}