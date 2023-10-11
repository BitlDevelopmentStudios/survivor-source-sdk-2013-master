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
	#include "particles_new.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponSniperRifle C_WeaponSniperRifle
#endif

//-----------------------------------------------------------------------------
// CWeaponSniperRifle
//-----------------------------------------------------------------------------

class CWeaponSniperRifle : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponSniperRifle, CBaseHL2MPCombatWeapon );
public:

	CWeaponSniperRifle( void );

	bool	Deploy(void);
	virtual void	Precache(void);
	virtual void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);
	virtual void	ItemPostFrame(void);
	virtual void	ItemBusyFrame(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	virtual int	 GetWeaponID(void) const			{ return WEAPON_SNIPER_RIFLE; }
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

private:

	void	ToggleZoom(void);
	void	CheckZoomToggle(void);

	CNetworkVar(bool, m_bInZoom);
	
	CWeaponSniperRifle( const CWeaponSniperRifle & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSniperRifle, DT_WeaponSniperRifle )

BEGIN_NETWORK_TABLE( CWeaponSniperRifle, DT_WeaponSniperRifle )
#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bInZoom)),
#else
	SendPropBool(SENDINFO(m_bInZoom)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSniperRifle)
DEFINE_PRED_FIELD(m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sniper_rifle, CWeaponSniperRifle );
PRECACHE_WEAPON_REGISTER( weapon_sniper_rifle );


#ifndef CLIENT_DLL
acttable_t CWeaponSniperRifle::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2AC_IDLE_SNIPER, false },
	{ ACT_HL2MP_RUN,					ACT_HL2AC_RUN_SNIPER, false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2AC_CROUCH_SNIPER, false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2AC_WALK_CROUCH_SNIPER, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER, false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP,					ACT_HL2AC_JUMP_SNIPER, false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL, false },
};



IMPLEMENT_ACTTABLE( CWeaponSniperRifle );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSniperRifle::CWeaponSniperRifle( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_bInZoom			= false;
}

bool CWeaponSniperRifle::Deploy(void)
{
/*
#if CLIENT_DLL
	C_BasePlayer *pPlayer = ToBasePlayer(GetOwner());
	CParticleProperty * pProp = ParticleProp();
	const char *pszEffectName;

	switch (pPlayer->GetTeamNumber())
	{
	case TEAM_BLUE:
		pszEffectName = "sniper_laser_blue";
		break;
	case TEAM_RED:
		pszEffectName = "sniper_laser_red";
		break;
	default:
		pszEffectName = "sniper_laser_red";
		break;
	}

	if (pProp->FindEffect(pszEffectName))
	{
		pProp->StopParticlesNamed(pszEffectName);
	}

	CNewParticleEffect *pEffect = pProp->Create(pszEffectName, PATTACH_POINT_FOLLOW, "laser");
	pProp->AddControlPoint(pEffect, 1, this, PATTACH_ABSORIGIN_FOLLOW, "laser_end");
#endif
*/

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::Precache(void)
{
	//PrecacheParticleSystem("sniper_laser_red");
	//PrecacheParticleSystem("sniper_laser_blue");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.75;

	m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt( -1, 1 );
	angles.y += random->RandomInt( -1, 1 );
	angles.z = 0;

#ifndef CLIENT_DLL
	pPlayer->SnapEyeAngles( angles );
#endif

	pPlayer->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}
}

void CWeaponSniperRifle::SecondaryAttack(void)
{
	//NOTENOTE: The zooming is handled by the post/busy frames
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	CBaseViewModel *pViewModel = pPlayer->GetViewModel();

	if (pViewModel == NULL)
		return;

	const char *vmmodel = "";

	if (pPlayer->GetTeamNumber() == TEAM_BLUE)
	{
		vmmodel = GetViewModelBlue();
	}
	else if (pPlayer->GetTeamNumber() == TEAM_RED)
	{
		vmmodel = GetViewModelRed();
	}
	else
	{
		vmmodel = GetViewModel();
	}

	if (m_bInZoom)
	{
		pViewModel->SetWeaponModel(NULL, this);
	}
	else
	{
		pViewModel->SetWeaponModel(vmmodel, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::ItemPostFrame(void)
{
	// Allow zoom toggling
	CheckZoomToggle();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	CBaseViewModel *pViewModel = pPlayer->GetViewModel();

	if (pViewModel == NULL)
		return;

	const char *vmmodel = "";

	if (pPlayer->GetTeamNumber() == TEAM_BLUE)
	{
		vmmodel = GetViewModelBlue();
	}
	else if (pPlayer->GetTeamNumber() == TEAM_RED)
	{
		vmmodel = GetViewModelRed();
	}
	else
	{
		vmmodel = GetViewModel();
	}

	if (m_bInZoom)
	{
		pViewModel->SetWeaponModel(NULL, this);
	}
	else
	{
		pViewModel->SetWeaponModel(vmmodel, this);
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

#ifndef CLIENT_DLL

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			WeaponSound(SPECIAL1);
			m_bInZoom = false;
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			WeaponSound(SPECIAL1);
			m_bInZoom = true;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponSniperRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}

	return BaseClass::Holster(pSwitchingTo);
}