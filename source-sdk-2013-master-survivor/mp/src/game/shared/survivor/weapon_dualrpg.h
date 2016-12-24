//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_DUALRPG_H
#define WEAPON_DUALRPG_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp\weapon_rpg.h"

#ifdef CLIENT_DLL

	#include "iviewrender_beams.h"

#endif

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "npcevent.h"
#include "beam_shared.h"

class CWeaponDualRPG;

//###########################################################################
//	>> CMissileDualRPG		(missile launcher class is below this one!)
//###########################################################################
class CMissileDualRPG : public CBaseCombatCharacter
{
	DECLARE_CLASS(CMissileDualRPG, CBaseCombatCharacter);

public:
	CMissileDualRPG();
	~CMissileDualRPG();

#ifdef HL1_DLL
	Class_T Classify(void) { return CLASS_NONE; }
#else
	Class_T Classify(void) { return CLASS_MISSILE; }
#endif

	void	Spawn(void);
	void	Precache(void);
	void	MissileTouch(CBaseEntity *pOther);
	void	Explode(void);
	void	ShotDown(void);
	void	AccelerateThink(void);
	void	AugerThink(void);
	void	IgniteThink(void);
	void	DumbFire(void);
	void	SetGracePeriod(float flGracePeriod);

	int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void	Event_Killed(const CTakeDamageInfo &info);

	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity(void) const;

	CHandle<CWeaponDualRPG>		m_hOwner;

	static CMissileDualRPG *Create(const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner);

protected:
	virtual void DoExplosion();
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	// Creates the smoke trail
	void CreateSmokeTrail(void);

	// Gets the shooting position 

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime;		// Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

private:
	float					m_flGracePeriodEndsAt;

	DECLARE_DATADESC();
};
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponDualRPG C_WeaponDualRPG
#endif

class CWeaponDualRPG : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS(CWeaponDualRPG, CBaseHL2MPCombatWeapon);
public:

	CWeaponDualRPG();
	~CWeaponDualRPG();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	Precache( void );

	void	PrimaryAttack( void );
	void	FireRightGun(void);
	void	FireLeftGun(void);
	void	FireBothGuns(void);
	void	SecondaryAttack(void);
	virtual float GetFireRate( void ) { return 1; };
	void	ItemPostFrame( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	Decrement2Ammo(CBaseCombatCharacter *pOwner);

	bool	Deploy( void );
	bool	Reload( void );
	bool	WeaponShouldBeLowered( void );
	bool	Lower( void );

	bool	CanHolster( void );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

	void	NotifyRocketDied( void );
	void	NotifyRocket2Died(void);

	bool	HasAnyAmmo( void );

	virtual int	 GetWeaponID(void) const			{ return WEAPON_DUALRPG; }

	CBaseEntity *GetMissile( void ) { return m_hMissile; }
	CBaseEntity *GetMissile2(void) { return m_hMissile2; }

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	
protected:

	CNetworkVar( bool, m_bInitialStateUpdate );

	CNetworkHandle( CBaseEntity,	m_hMissile );
	CNetworkHandle(CBaseEntity, m_hMissile2);

	bool bFlip;

private:
	
	CWeaponDualRPG( const CWeaponDualRPG & );
};

#endif // WEAPON_RPG_H
