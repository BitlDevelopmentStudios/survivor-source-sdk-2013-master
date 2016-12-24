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

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
#define CWeaponGatling C_WeaponGatling
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GATLING_DMG 6.0f
#define GATLING_DMG_NUMSHOTS 30

ConVar	survivor_gatling_damageplayer("survivor_gatling_damageplayer", "1", FCVAR_NOTIFY);

class CWeaponGatling : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS( CWeaponGatling, CHL2MPMachineGun );

	CWeaponGatling();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	void	AddViewKick( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );
	void	DecrementAmmo(CBaseCombatCharacter *pOwner);
	void	PrimaryAttack(void);
	virtual void	ItemPostFrame(void);

	float	GetFireRate( void ) { return 0.105f; }	// 13.3hz
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_6DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	virtual int	 GetWeaponID(void) const			{ return WEAPON_GATLING; }

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	
private:

	CNetworkVar(int, m_nNumShotsFiredDamage);

	CWeaponGatling( const CWeaponGatling & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGatling, DT_WeaponGatling )

BEGIN_NETWORK_TABLE( CWeaponGatling, DT_WeaponGatling )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_nNumShotsFiredDamage ) ),
#else
	SendPropInt(SENDINFO( m_nNumShotsFiredDamage )),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponGatling )
	DEFINE_PRED_FIELD(m_nNumShotsFiredDamage, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_gatling, CWeaponGatling );
PRECACHE_WEAPON_REGISTER(weapon_gatling);

#ifndef CLIENT_DLL
acttable_t	CWeaponGatling::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2AC_IDLE_GATLING,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2AC_RUN_GATLING,						false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2AC_CROUCH_GATLING,					false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2AC_WALK_CROUCH_GATLING,				false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2AC_GESTURE_RANGE_ATTACK01_GATLING,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_HL2MP_JUMP,					ACT_HL2AC_JUMP_GATLING,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SMG1,					false },
};

IMPLEMENT_ACTTABLE(CWeaponGatling);
#endif

//=========================================================
CWeaponGatling::CWeaponGatling( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 8192;

	m_nNumShotsFiredDamage = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponGatling::Equip( CBaseCombatCharacter *pOwner )
{
	m_fMaxRange1 = 8192;

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponGatling::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

void CWeaponGatling::PrimaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

#ifndef CLIENT_DLL
	CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer(pOwner);

	if (!pHL2MPPlayer->IsWalking() || !pHL2MPPlayer->IsSprinting())
	{
		pHL2MPPlayer->SetMaxSpeed(150);
	}
#endif

	BaseClass::PrimaryAttack();
	DecrementAmmo(pOwner);

	m_nNumShotsFiredDamage++;

	if (survivor_gatling_damageplayer.GetBool())
	{
		if (m_nNumShotsFiredDamage == GATLING_DMG_NUMSHOTS)
		{
#ifndef CLIENT_DLL
			CTakeDamageInfo info(this, this, GATLING_DMG, DMG_DIRECT);
			pOwner->TakeDamage(info);
#endif
			m_nNumShotsFiredDamage = 1;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponGatling::Reload( void )
{
	//This is a MINIGUN! Don't reload!
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGatling::AddViewKick( void )
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle	viewPunch;

	viewPunch.x = SharedRandomFloat("gatlingpax", 0.1f, 2.0f);
	viewPunch.y = SharedRandomFloat("gatlingpay", 0.1f, 2.0f);
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponGatling::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
	// Take away our primary ammo type
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

void CWeaponGatling::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		SendWeaponAnim(ACT_VM_IDLE);

#ifndef CLIENT_DLL
		CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer(pOwner);

		if (!pHL2MPPlayer->IsWalking() || !pHL2MPPlayer->IsSprinting())
		{
			pHL2MPPlayer->SetMaxSpeed(190);
		}
#endif
	}

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 || HL2MPRules()->IsIntermission())
	{
		SendWeaponAnim(ACT_VM_IDLE);
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponGatling::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
