
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "beam_shared.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponHealgun C_WeaponHealgun
#endif

#define GAUSS_BEAM_SPRITE "sprites/lgtning.vmt"

//-----------------------------------------------------------------------------
// CWeaponHealgun
//-----------------------------------------------------------------------------

class CWeaponHealgun : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponHealgun, CBaseHL2MPCombatWeapon );
public:

	CWeaponHealgun( void );

	void	Precache(void);
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	PrimaryAttackTeamplay(void);
	void	PrimaryAttackFFA(void);
	void	ItemPostFrame(void);
	void 	HealPlayer( void );
	void	DrawBeam(const Vector& startPos, const Vector& endPos);
	void 	HealSelfPlayer( void );
	bool	Deploy(void);
	bool	Reload(void);
	virtual int	 GetWeaponID(void) const			{ return WEAPON_HEALGUN; }
	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_1DEGREES;
		return cone;
	}
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

private:
	CBeam* m_pBeam;
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
void CWeaponHealgun::Precache(void)
{
	// Laser dot...
	PrecacheModel(GAUSS_BEAM_SPRITE);

	BaseClass::Precache();
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

void CWeaponHealgun::HealPlayer( void )
{
   CBasePlayer *pOwner = ToBasePlayer(GetOwner());
 
   if (!pOwner)
      return;
 
   Vector	startPos = pOwner->Weapon_ShootPosition();
   Vector	aimDir = pOwner->GetAutoaimVector(AUTOAIM_5DEGREES);

   Vector vecUp, vecRight;
   VectorVectors(aimDir, vecRight, vecUp);

   float x, y, z;

   //Gassian spread
   do {
	   x = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
	   y = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
	   z = x * x + y * y;
   } while (z > 1);

   aimDir = aimDir + x * GetBulletSpread().x * vecRight + y * GetBulletSpread().y * vecUp;

   Vector	endPos = startPos + (aimDir * MAX_TRACE_LENGTH);

   //Shoot a shot straight out
   trace_t	tr;
   UTIL_TraceLine(startPos, endPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

   CBaseEntity* pEntity = tr.m_pEnt;
   if (pEntity)
   {
	   if (pEntity->IsPlayer())
	   {
		   if ((pEntity->GetHealth() < pEntity->GetMaxHealth()) && (pEntity->GetTeamNumber() == pOwner->GetTeamNumber()))
		   {
#ifndef CLIENT_DLL
			   pEntity->TakeHealth(20, DMG_GENERIC);
#endif
		   }
	   }
   }

   float hitAngle = -DotProduct(tr.plane.normal, aimDir);

   Vector vReflection;

   vReflection = 2.0 * tr.plane.normal * hitAngle + aimDir;

   startPos = tr.endpos;
   endPos = startPos + (vReflection * MAX_TRACE_LENGTH);

   //Draw beam to reflection point
   DrawBeam(tr.startpos, tr.endpos);

#ifndef CLIENT_DLL
   // Register a muzzleflash for the AI
   pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
#endif 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHealgun::DrawBeam(const Vector& startPos, const Vector& endPos)
{
#ifndef CLIENT_DLL

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	//Draw the main beam shaft
	m_pBeam = CBeam::BeamCreate(GAUSS_BEAM_SPRITE, 20.0f);
	m_pBeam->SetStartPos(startPos);
	m_pBeam->PointEntInit(endPos, this);
	m_pBeam->SetEndAttachment(LookupAttachment("muzzle"));
	m_pBeam->SetColor(94, 116, 196);
	m_pBeam->SetEndWidth(m_pBeam->GetWidth());
	m_pBeam->SetBrightness(200);
	m_pBeam->RelinkBeam();
	m_pBeam->LiveForTime(0.05f);
#endif
}

void CWeaponHealgun::HealSelfPlayer( void )
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
 
	if (!pOwner)
		return;
 
	if (pOwner->GetHealth() < pOwner->GetMaxHealth())
	{
#ifndef CLIENT_DLL
		pOwner->TakeHealth( 20, DMG_GENERIC );
#endif
    }
}