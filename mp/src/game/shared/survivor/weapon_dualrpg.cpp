//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_dualrpg.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "model_types.h"
	#include "beamdraw.h"
	#include "fx_line.h"
	#include "view.h"
#else
	#include "basecombatcharacter.h"
	#include "movie_explosion.h"
	#include "soundent.h"
	#include "player.h"
	#include "rope.h"
	#include "vstdlib/random.h"
	#include "engine/IEngineSound.h"
	#include "explode.h"
	#include "util.h"
	#include "in_buttons.h"
	#include "shake.h"
	#include "te_effect_dispatch.h"
	#include "triggers.h"
	#include "smoke_trail.h"
	#include "collisionutils.h"
	#include "hl2_shareddefs.h"
#endif

#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RPG_SPEED	1500

#define	RPG_BEAM_SPRITE		"effects/laser1.vmt"
#define	RPG_BEAM_SPRITE_NOZ	"effects/laser1_noz.vmt"
#define	RPG_LASER_SPRITE	"sprites/redglow1"

#ifndef CLIENT_DLL
BEGIN_DATADESC(CMissileDualRPG)

DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_flAugerTime, FIELD_TIME),
DEFINE_FIELD(m_flMarkDeadTime, FIELD_TIME),
DEFINE_FIELD(m_flGracePeriodEndsAt, FIELD_TIME),
DEFINE_FIELD(m_flDamage, FIELD_FLOAT),

// Function Pointers
DEFINE_FUNCTION(MissileTouch),
DEFINE_FUNCTION(AccelerateThink),
DEFINE_FUNCTION(AugerThink),
DEFINE_FUNCTION(IgniteThink),

END_DATADESC()

LINK_ENTITY_TO_CLASS(dualrpg_missile, CMissileDualRPG);

class CWeaponRPG;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CMissileDualRPG::CMissileDualRPG()
{
	m_hRocketTrail = NULL;
}

CMissileDualRPG::~CMissileDualRPG()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CMissileDualRPG::Precache(void)
{
	PrecacheModel("models/weapons/w_missile.mdl");
	PrecacheModel("models/weapons/w_missile_launch.mdl");
	PrecacheModel("models/weapons/w_missile_closed.mdl");
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CMissileDualRPG::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);
	SetModel("models/weapons/w_missile_launch.mdl");
	UTIL_SetSize(this, -Vector(4, 4, 4), Vector(4, 4, 4));

	SetTouch(&CMissileDualRPG::MissileTouch);

	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetThink(&CMissileDualRPG::IgniteThink);

	SetNextThink(gpGlobals->curtime + 0.3f);

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 100;
	m_bloodColor = DONT_BLEED;
	m_flGracePeriodEndsAt = 0;

	AddFlag(FL_OBJECT);
}


//---------------------------------------------------------
//---------------------------------------------------------
void CMissileDualRPG::Event_Killed(const CTakeDamageInfo &info)
{
	m_takedamage = DAMAGE_NO;

	ShotDown();
}

unsigned int CMissileDualRPG::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

//---------------------------------------------------------
//---------------------------------------------------------
int CMissileDualRPG::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	if ((info.GetDamageType() & (DMG_MISSILEDEFENSE | DMG_AIRBOAT)) == false)
		return 0;

	bool bIsDamaged;
	if (m_iHealth <= AugerHealth())
	{
		// This missile is already damaged (i.e., already running AugerThink)
		bIsDamaged = true;
	}
	else
	{
		// This missile isn't damaged enough to wobble in flight yet
		bIsDamaged = false;
	}

	int nRetVal = BaseClass::OnTakeDamage_Alive(info);

	if (!bIsDamaged)
	{
		if (m_iHealth <= AugerHealth())
		{
			ShotDown();
		}
	}

	return nRetVal;
}


//-----------------------------------------------------------------------------
// Purpose: Stops any kind of tracking and shoots dumb
//-----------------------------------------------------------------------------
void CMissileDualRPG::DumbFire(void)
{
	SetThink(NULL);
	SetMoveType(MOVETYPE_FLY);

	SetModel("models/weapons/w_missile.mdl");
	UTIL_SetSize(this, vec3_origin, vec3_origin);

	EmitSound("Missile.Ignite");

	// Smoke trail.
	CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMissileDualRPG::SetGracePeriod(float flGracePeriod)
{
	m_flGracePeriodEndsAt = gpGlobals->curtime + flGracePeriod;

	// Go non-solid until the grace period ends
	AddSolidFlags(FSOLID_NOT_SOLID);
}

//---------------------------------------------------------
//---------------------------------------------------------
void CMissileDualRPG::AccelerateThink(void)
{
	Vector vecForward;

	// !!!UNDONE - make this work exactly the same as HL1 RPG, lest we have looping sound bugs again!
	EmitSound("Missile.Accelerate");

	// SetEffects( EF_LIGHT );

	AngleVectors(GetLocalAngles(), &vecForward);
	SetAbsVelocity(vecForward * RPG_SPEED);
}

#define AUGER_YDEVIANCE 20.0f
#define AUGER_XDEVIANCEUP 8.0f
#define AUGER_XDEVIANCEDOWN 1.0f

//---------------------------------------------------------
//---------------------------------------------------------
void CMissileDualRPG::AugerThink(void)
{
	// If we've augered long enough, then just explode
	if (m_flAugerTime < gpGlobals->curtime)
	{
		Explode();
		return;
	}

	if (m_flMarkDeadTime < gpGlobals->curtime)
	{
		m_lifeState = LIFE_DYING;
	}

	QAngle angles = GetLocalAngles();

	angles.y += random->RandomFloat(-AUGER_YDEVIANCE, AUGER_YDEVIANCE);
	angles.x += random->RandomFloat(-AUGER_XDEVIANCEDOWN, AUGER_XDEVIANCEUP);

	SetLocalAngles(angles);

	Vector vecForward;

	AngleVectors(GetLocalAngles(), &vecForward);

	SetAbsVelocity(vecForward * 1000.0f);

	SetNextThink(gpGlobals->curtime + 0.05f);
}

//-----------------------------------------------------------------------------
// Purpose: Causes the missile to spiral to the ground and explode, due to damage
//-----------------------------------------------------------------------------
void CMissileDualRPG::ShotDown(void)
{
	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();

	DispatchEffect("RPGShotDown", data);

	if (m_hRocketTrail != NULL)
	{
		m_hRocketTrail->m_bDamaged = true;
	}

	SetThink(&CMissileDualRPG::AugerThink);
	SetNextThink(gpGlobals->curtime);
	m_flAugerTime = gpGlobals->curtime + 1.5f;
	m_flMarkDeadTime = gpGlobals->curtime + 0.75;

	// Let the RPG start reloading immediately
	if (m_hOwner != NULL)
	{
		m_hOwner->NotifyRocketDied();
		m_hOwner = NULL;
	}
}


//-----------------------------------------------------------------------------
// The actual explosion 
//-----------------------------------------------------------------------------
void CMissileDualRPG::DoExplosion(void)
{
	// Explode
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), GetDamage(), GetDamage() * 2,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMissileDualRPG::Explode(void)
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	Vector forward;

	GetVectors(&forward, NULL, NULL);

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	m_takedamage = DAMAGE_NO;
	SetSolid(SOLID_NONE);
	if (tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY))
	{
		DoExplosion();
	}

	if (m_hRocketTrail)
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	if (m_hOwner != NULL)
	{
		m_hOwner->NotifyRocketDied();
		m_hOwner = NULL;
	}

	StopSound("Missile.Ignite");
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CMissileDualRPG::MissileTouch(CBaseEntity *pOther)
{
	Assert(pOther);

	// Don't touch triggers (but DO hit weapons)
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON)
		return;

	Explode();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMissileDualRPG::CreateSmokeTrail(void)
{
	if (m_hRocketTrail)
		return;

	// Smoke trail.
	if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f, 0.65f);
		m_hRocketTrail->m_EndColor.Init(0.0, 0.0, 0.0);
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 32;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;

		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMissileDualRPG::IgniteThink(void)
{
	SetMoveType(MOVETYPE_FLY);
	SetModel("models/weapons/w_missile.mdl");
	UTIL_SetSize(this, vec3_origin, vec3_origin);
	RemoveSolidFlags(FSOLID_NOT_SOLID);

	//TODO: Play opening sound

	Vector vecForward;

	EmitSound("Missile.Ignite");

	AngleVectors(GetLocalAngles(), &vecForward);
	SetAbsVelocity(vecForward * RPG_SPEED);

	if (m_hOwner && m_hOwner->GetOwner())
	{
		CBasePlayer *pPlayer = ToBasePlayer(m_hOwner->GetOwner());

		color32 white = { 255, 225, 205, 64 };
		UTIL_ScreenFade(pPlayer, white, 0.1f, 0.0f, FFADE_IN);
	}

	CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CMissileDualRPG
//-----------------------------------------------------------------------------
CMissileDualRPG *CMissileDualRPG::Create(const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner = NULL)
{
	//CMissileDualRPG *pMissile = (CMissileDualRPG *)CreateEntityByName("dualrpg_missile" );
	CMissileDualRPG *pMissile = (CMissileDualRPG *)CBaseEntity::Create("dualrpg_missile", vecOrigin, vecAngles, CBaseEntity::Instance(pentOwner));
	pMissile->SetOwnerEntity(Instance(pentOwner));
	pMissile->Spawn();
	pMissile->AddEffects(EF_NOSHADOW);

	Vector vecForward;
	AngleVectors(vecAngles, &vecForward);

	pMissile->SetAbsVelocity(vecForward * 300 + Vector(0, 0, 128));

	return pMissile;
}
#endif

//=============================================================================
// RPG
//=============================================================================

LINK_ENTITY_TO_CLASS( weapon_dualrpg, CWeaponDualRPG );
PRECACHE_WEAPON_REGISTER(weapon_dualrpg);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDualRPG, DT_WeaponDualRPG )

#ifdef CLIENT_DLL
void RecvProxy_MissileDiedDual( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CWeaponDualRPG *pRPG = ((CWeaponDualRPG*)pStruct);

	RecvProxy_IntToEHandle( pData, pStruct, pOut );

	CBaseEntity *pNewMissile = pRPG->GetMissile();

	if ( pNewMissile == NULL )
	{
		if ( pRPG->GetOwner() && pRPG->GetOwner()->GetActiveWeapon() == pRPG )
		{
			pRPG->NotifyRocketDied();
		}
	}
}

void RecvProxy_MissileDiedDual2(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CWeaponDualRPG *pRPG = ((CWeaponDualRPG*)pStruct);

	RecvProxy_IntToEHandle( pData, pStruct, pOut );

	CBaseEntity *pNewMissile = pRPG->GetMissile2();

	if (pNewMissile == NULL)
	{
		if (pRPG->GetOwner() && pRPG->GetOwner()->GetActiveWeapon() == pRPG)
		{
			pRPG->NotifyRocket2Died();
		}
	}
}

#endif

BEGIN_NETWORK_TABLE( CWeaponDualRPG, DT_WeaponDualRPG )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bInitialStateUpdate ) ),
	RecvPropEHandle( RECVINFO( m_hMissile ), RecvProxy_MissileDiedDual ),
	RecvPropEHandle( RECVINFO( m_hMissile2 ), RecvProxy_MissileDiedDual2 ),
#else
	SendPropBool( SENDINFO( m_bInitialStateUpdate ) ),
	SendPropEHandle( SENDINFO( m_hMissile ) ),
	SendPropEHandle( SENDINFO( m_hMissile2 ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponDualRPG )
	DEFINE_PRED_FIELD( m_bInitialStateUpdate, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

#endif

#ifndef CLIENT_DLL
acttable_t	CWeaponDualRPG::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_RPG2,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_RPG2,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_RPG2,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_RPG2,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG, false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_RPG2,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_RPG,				false },
};

IMPLEMENT_ACTTABLE(CWeaponDualRPG);

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponDualRPG::CWeaponDualRPG()
{
	m_bReloadsSingly = true;
	m_bInitialStateUpdate= false;
	bFlip = false;

	m_fMinRange1 = m_fMinRange2 = 40*12;
	m_fMaxRange1 = m_fMaxRange2 = 500*12;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponDualRPG::~CWeaponDualRPG()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDualRPG::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Missile.Ignite" );
	PrecacheScriptSound( "Missile.Accelerate" );

	// Laser dot...
	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( RPG_LASER_SPRITE );
	PrecacheModel( RPG_BEAM_SPRITE );
	PrecacheModel( RPG_BEAM_SPRITE_NOZ );

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "rpg2_missile" );
#endif

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponDualRPG::HasAnyAmmo( void )
{
	if ( m_hMissile != NULL )
		return true;

	if (m_hMissile2 != NULL)
		return true;

	return BaseClass::HasAnyAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDualRPG::WeaponShouldBeLowered( void )
{
	// Lower us if we're out of ammo
	if ( !HasAnyAmmo() )
		return true;
	
	return BaseClass::WeaponShouldBeLowered();
}

void CWeaponDualRPG::PrimaryAttack(void)
{
	if (!bFlip)
	{
		FireRightGun();
		bFlip = true;
	}
	else
	{
		FireLeftGun();
		bFlip = false;
	}
}

void CWeaponDualRPG::SecondaryAttack(void)
{
	FireBothGuns();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDualRPG::FireRightGun( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	// Can't have an active missile out
	if (m_hMissile2 != NULL)
		return;

	if (m_hMissile != NULL)
		return;

	// Can't be reloading
	if ( GetActivity() == ACT_VM_RELOAD )
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.5f;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors( &vForward, &vRight, &vUp );

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;

#ifndef CLIENT_DLL
	QAngle vecAngles;
	VectorAngles( vForward, vecAngles );

	CMissileDualRPG *pMissile = CMissileDualRPG::Create( muzzlePoint, vecAngles, GetOwner()->edict() );
	pMissile->m_hOwner = this;

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine( vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction == 1.0 )
	{
		pMissile->SetGracePeriod( 0.3 );
	}

	pMissile->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );

	m_hMissile = pMissile;
#endif

	DecrementAmmo( GetOwner() );
	SendWeaponAnim(ACT_VM_PRIMARYATTACK_R);
	WeaponSound( SINGLE );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

void CWeaponDualRPG::FireLeftGun(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	// Can't have an active missile out
	if (m_hMissile2 != NULL)
		return;

	if (m_hMissile != NULL)
		return;

	// Can't be reloading
	if (GetActivity() == ACT_VM_RELOAD)
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint2 = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * -6.0f + vUp * 3.0f;

#ifndef CLIENT_DLL
	QAngle vecAngles;
	VectorAngles(vForward, vecAngles);

	CMissileDualRPG *pMissile2 = CMissileDualRPG::Create(muzzlePoint2, vecAngles, GetOwner()->edict());
	pMissile2->m_hOwner = this;

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction == 1.0)
	{
		pMissile2->SetGracePeriod(0.3);
	}

	pMissile2->SetDamage(GetHL2MPWpnData().m_iPlayerDamage);

	m_hMissile2 = pMissile2;
#endif

	DecrementAmmo(GetOwner());
	SendWeaponAnim(ACT_VM_PRIMARYATTACK_L);
	WeaponSound(SINGLE);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CWeaponDualRPG::FireBothGuns(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) > 1)
	{
		// Can't have an active missile out
		if (m_hMissile2 != NULL)
			return;

		if (m_hMissile != NULL)
			return;

		// Can't be reloading
		if (GetActivity() == ACT_VM_RELOAD)
			return;

		Vector vecOrigin;
		Vector vecForward;

		m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 1.5f;

		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		Vector	vForward, vRight, vUp;

		pOwner->EyeVectors(&vForward, &vRight, &vUp);

		Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
		Vector	muzzlePoint2 = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * -6.0f + vUp * 3.0f;

#ifndef CLIENT_DLL
		QAngle vecAngles;
		VectorAngles(vForward, vecAngles);

		CMissileDualRPG *pMissile = CMissileDualRPG::Create(muzzlePoint, vecAngles, GetOwner()->edict());
		pMissile->m_hOwner = this;

		// If the shot is clear to the player, give the missile a grace period
		trace_t	tr;
		Vector vecEye = pOwner->EyePosition();
		UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction == 1.0)
		{
			pMissile->SetGracePeriod(0.3);
		}

		pMissile->SetDamage(GetHL2MPWpnData().m_iPlayerDamage);

		m_hMissile = pMissile;

		CMissileDualRPG *pMissile2 = CMissileDualRPG::Create(muzzlePoint2, vecAngles, GetOwner()->edict());
		pMissile2->m_hOwner = this;

		// If the shot is clear to the player, give the missile a grace period
		UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction == 1.0)
		{
			pMissile2->SetGracePeriod(0.3);
		}

		pMissile2->SetDamage(GetHL2MPWpnData().m_iPlayerDamage);

		m_hMissile2 = pMissile2;
#endif

		Decrement2Ammo(GetOwner());
		SendWeaponAnim(ACT_VM_PRIMARYATTACK_RL);
		WeaponSound(SINGLE);

		// player "shoot" animation
		pPlayer->SetAnimation(PLAYER_ATTACK1);
	}
	else
	{
		if (!bFlip)
		{
			FireRightGun();
			bFlip = true;
		}
		else
		{
			FireLeftGun();
			bFlip = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponDualRPG::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	// Take away our primary ammo type
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

void CWeaponDualRPG::Decrement2Ammo(CBaseCombatCharacter *pOwner)
{
	// Take away our primary ammo type
	pOwner->RemoveAmmo(2, m_iPrimaryAmmoType);
}

//-----------------------------------------------------------------------------
// Purpose: Override this if we're guiding a missile currently
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDualRPG::Lower( void )
{
	if ( m_hMissile != NULL )
		return false;

	if (m_hMissile2 != NULL)
		return false;

	return BaseClass::Lower();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDualRPG::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDualRPG::Deploy( void )
{
	m_bInitialStateUpdate = true;

	return BaseClass::Deploy();
}

bool CWeaponDualRPG::CanHolster( void )
{
	//Can't have an active missile out
	if ( m_hMissile != NULL )
		return false;

	if (m_hMissile2 != NULL)
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDualRPG::NotifyRocketDied( void )
{
	m_hMissile = NULL;

	if ( GetActivity() == ACT_VM_RELOAD )
		return;

	Reload();
}

void CWeaponDualRPG::NotifyRocket2Died(void)
{
	m_hMissile2 = NULL;

	if (GetActivity() == ACT_VM_RELOAD)
		return;

	Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponDualRPG::Reload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		return false;

	WeaponSound( RELOAD );
	
	//SendWeaponAnim( ACT_VM_RELOAD );

	return true;
}
