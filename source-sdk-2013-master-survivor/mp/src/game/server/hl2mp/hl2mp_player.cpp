//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "survivor/survivor_shareddefs.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	SendPropInt(SENDINFO(m_iClass), 2),
	SendPropInt(SENDINFO(m_iFaction), 2),
	SendPropInt(SENDINFO(m_iGender), 2),
	SendPropInt(SENDINFO(m_WeaponPresetPrimary), 2),
	SendPropInt(SENDINFO(m_WeaponPresetSecondary), 2),
	SendPropInt(SENDINFO(m_WeaponPresetMelee), 2),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	m_iClass = 0;
	m_iFaction = -1;
	m_iGender = SURVIVOR_GENDER_MALE;

//	BaseClass::ChangeTeam( 0 );
	
//	UseClientSideAnimation();

	m_WeaponPresetPrimary = 0;
	m_WeaponPresetSecondary = 0;
	m_WeaponPresetMelee = 0;
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	PrecacheModel("models/player/combine/ranger/combine_normal_male.mdl");
	PrecacheModel("models/player/combine/engineer/combine_engineer_male.mdl");
	PrecacheModel("models/player/combine/medic/combine_medic_male.mdl");
	PrecacheModel("models/player/combine/sniper/combine_sniper_male.mdl");
	PrecacheModel("models/player/combine/soldier/combine_soldier_male.mdl");
	PrecacheModel("models/player/resistance/ranger/resis_normal_male.mdl");
	PrecacheModel("models/player/resistance/engineer/resis_engineer_male.mdl");
	PrecacheModel("models/player/resistance/medic/resis_medic_male.mdl");
	PrecacheModel("models/player/resistance/sniper/resis_sniper_male.mdl");
	PrecacheModel("models/player/resistance/soldier/resis_soldier_male.mdl");
	PrecacheModel("models/player/combine/ranger/combine_normal_female.mdl");
	PrecacheModel("models/player/combine/engineer/combine_engineer_female.mdl");
	PrecacheModel("models/player/combine/medic/combine_medic_female.mdl");
	PrecacheModel("models/player/combine/sniper/combine_sniper_female.mdl");
	PrecacheModel("models/player/combine/soldier/combine_soldier_female.mdl");
	PrecacheModel("models/player/resistance/ranger/resis_normal_female.mdl");
	PrecacheModel("models/player/resistance/engineer/resis_engineer_female.mdl");
	PrecacheModel("models/player/resistance/medic/resis_medic_female.mdl");
	PrecacheModel("models/player/resistance/sniper/resis_sniper_female.mdl");
	PrecacheModel("models/player/resistance/soldier/resis_soldier_female.mdl");

	PrecacheFootStepSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
	PrecacheScriptSound("NPC_Citizen_Female.die");
	PrecacheScriptSound("Player.Death");
}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo(999, "Pistol");
	CBasePlayer::GiveAmmo(999, "AR2");
	CBasePlayer::GiveAmmo(999, "AR2AltFire");
	CBasePlayer::GiveAmmo(999, "SMG1");
	CBasePlayer::GiveAmmo(999, "smg1_grenade");
	CBasePlayer::GiveAmmo(999, "Buckshot");
	CBasePlayer::GiveAmmo(999, "357");
	CBasePlayer::GiveAmmo(999, "rpg_round");
	CBasePlayer::GiveAmmo(999, "grenade");
	CBasePlayer::GiveAmmo(999, "slam");
	CBasePlayer::GiveAmmo(999, "Gatling");
	CBasePlayer::GiveAmmo(999, "Sniper");
	CBasePlayer::GiveAmmo(999, "Recover");
	CBasePlayer::GiveAmmo(999, "XBowBolt");

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );
	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_ar2" );
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	GiveNamedItem( "weapon_crossbow" );
	GiveNamedItem( "weapon_rpg" );
	GiveNamedItem( "weapon_slam" );
	GiveNamedItem( "weapon_dualrpg" );
	GiveNamedItem( "weapon_dualpistol" );
	GiveNamedItem("weapon_shotgundouble");
	GiveNamedItem( "weapon_gatling");
	GiveNamedItem("weapon_sniper_rifle");
	GiveNamedItem( "weapon_healgun");
	GiveNamedItem( "weapon_physcannon" );
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	EquipSuit();

	int iFaction = GetFaction();
	int iClass = GetClass();

	if (!survivor_randomizer.GetBool())
	{
		if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			GiveNamedItem(WeaponIdToClassname(WEAPON_STUNSTICK));
			if (iClass == SURVIVOR_CLASS_RANGER)
			{
				CBasePlayer::GiveAmmo(175, "AR2");
				CBasePlayer::GiveAmmo(2, "AR2AltFire");
				CBasePlayer::GiveAmmo(72, "Pistol");
				GiveNamedItem(WeaponIdToClassname(WEAPON_AR2));
				GiveNamedItem(WeaponIdToClassname(WEAPON_DUALPISTOL));
			}
			else if (iClass == SURVIVOR_CLASS_MEDIC)
			{
				CBasePlayer::GiveAmmo(210, "AR2");
				CBasePlayer::GiveAmmo(2, "AR2AltFire");
				GiveNamedItem(WeaponIdToClassname(WEAPON_AR2));
				GiveNamedItem(WeaponIdToClassname(WEAPON_HEALGUN));
			}
			else if (iClass == SURVIVOR_CLASS_SOLDIER)
			{
				CBasePlayer::GiveAmmo(210, "AR2");
				CBasePlayer::GiveAmmo(2, "AR2AltFire");
				GiveNamedItem(WeaponIdToClassname(WEAPON_AR2));
				GiveNamedItem(WeaponIdToClassname(WEAPON_GATLING));
			}
			else if (iClass == SURVIVOR_CLASS_SNIPER)
			{
				CBasePlayer::GiveAmmo(140, "AR2");
				CBasePlayer::GiveAmmo(2, "AR2AltFire");
				GiveNamedItem(WeaponIdToClassname(WEAPON_AR2));
				GiveNamedItem(WeaponIdToClassname(WEAPON_SNIPER_RIFLE));
			}
			else if (iClass == SURVIVOR_CLASS_ENGINEER)
			{
				CBasePlayer::GiveAmmo(140, "AR2");
				CBasePlayer::GiveAmmo(2, "AR2AltFire");
				GiveNamedItem(WeaponIdToClassname(WEAPON_AR2));
				GiveNamedItem(WeaponIdToClassname(WEAPON_PHYSCANNON));
			}
		}
		else if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			GiveNamedItem(WeaponIdToClassname(WEAPON_CROWBAR));
			if (iClass == SURVIVOR_CLASS_RANGER)
			{
				CBasePlayer::GiveAmmo(180, "SMG1");
				CBasePlayer::GiveAmmo(2, "smg1_grenade");
				CBasePlayer::GiveAmmo(30, "Buckshot");
				GiveNamedItem(WeaponIdToClassname(WEAPON_SMG1));
				GiveNamedItem(WeaponIdToClassname(WEAPON_SHOTGUN));
			}
			else if (iClass == SURVIVOR_CLASS_MEDIC)
			{
				CBasePlayer::GiveAmmo(225, "SMG1");
				CBasePlayer::GiveAmmo(2, "smg1_grenade");
				GiveNamedItem(WeaponIdToClassname(WEAPON_SMG1));
				GiveNamedItem(WeaponIdToClassname(WEAPON_HEALGUN));
			}
			else if (iClass == SURVIVOR_CLASS_SOLDIER)
			{
				CBasePlayer::GiveAmmo(225, "SMG1");
				CBasePlayer::GiveAmmo(2, "smg1_grenade");
				CBasePlayer::GiveAmmo(2, "rpg_round");
				GiveNamedItem(WeaponIdToClassname(WEAPON_SMG1));
				GiveNamedItem(WeaponIdToClassname(WEAPON_RPG));
			}
			else if (iClass == SURVIVOR_CLASS_SNIPER)
			{
				CBasePlayer::GiveAmmo(135, "SMG1");
				CBasePlayer::GiveAmmo(2, "smg1_grenade");
				GiveNamedItem(WeaponIdToClassname(WEAPON_SMG1));
				GiveNamedItem(WeaponIdToClassname(WEAPON_SNIPER_RIFLE));
			}
			else if (iClass == SURVIVOR_CLASS_ENGINEER)
			{
				CBasePlayer::GiveAmmo(135, "SMG1");
				CBasePlayer::GiveAmmo(2, "smg1_grenade");
				GiveNamedItem(WeaponIdToClassname(WEAPON_SMG1));
				GiveNamedItem(WeaponIdToClassname(WEAPON_PHYSCANNON));
			}
		}
	}
	else
	{
		ManageRandomWeapons();
	}

	const char *szDefaultWeaponName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_defaultweapon" );

	CBaseCombatWeapon *pDefaultWeapon = Weapon_OwnsThisType( szDefaultWeaponName );

	if ( pDefaultWeapon )
	{
		Weapon_Switch( pDefaultWeapon );
	}
	else
	{
		if (!survivor_randomizer.GetBool())
		{
			if (iFaction == SURVIVOR_FACTION_COMBINE)
			{
				if (iClass == SURVIVOR_CLASS_RANGER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_dualpistol"));
				}
				else if (iClass == SURVIVOR_CLASS_MEDIC)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_ar2"));
				}
				else if (iClass == SURVIVOR_CLASS_SOLDIER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_gatling"));
				}
				else if (iClass == SURVIVOR_CLASS_SNIPER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_sniper_rifle"));
				}
				else if (iClass == SURVIVOR_CLASS_ENGINEER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_ar2"));
				}
			}
			else if (iFaction == SURVIVOR_FACTION_RESISTANCE)
			{
				if (iClass == SURVIVOR_CLASS_RANGER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_shotgun"));
				}
				else if (iClass == SURVIVOR_CLASS_MEDIC)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_smg1"));
				}
				else if (iClass == SURVIVOR_CLASS_SOLDIER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_rpg"));
				}
				else if (iClass == SURVIVOR_CLASS_SNIPER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_sniper_rifle"));
				}
				else if (iClass == SURVIVOR_CLASS_ENGINEER)
				{
					Weapon_Switch(Weapon_OwnsThisType("weapon_smg1"));
				}
			}
		}
	}
}

void CHL2MP_Player::Regenerate(void)
{
	int iFaction = GetFaction();
	int iClass = GetClass();

	if (!survivor_randomizer.GetBool())
	{
		if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iClass == SURVIVOR_CLASS_RANGER)
			{
				CBasePlayer::GiveAmmo(175, "AR2");
				CBasePlayer::GiveAmmo(72, "Pistol");
			}
			else if (iClass == SURVIVOR_CLASS_MEDIC)
			{
				CBasePlayer::GiveAmmo(210, "AR2");
			}
			else if (iClass == SURVIVOR_CLASS_SOLDIER)
			{
				CBasePlayer::GiveAmmo(210, "AR2");
			}
			else if (iClass == SURVIVOR_CLASS_SNIPER)
			{
				CBasePlayer::GiveAmmo(140, "AR2");
			}
			else if (iClass == SURVIVOR_CLASS_ENGINEER)
			{
				CBasePlayer::GiveAmmo(140, "AR2");
			}
		}
		else if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			GiveNamedItem(WeaponIdToClassname(WEAPON_CROWBAR));
			if (iClass == SURVIVOR_CLASS_RANGER)
			{
				CBasePlayer::GiveAmmo(180, "SMG1");
				CBasePlayer::GiveAmmo(30, "Buckshot");
			}
			else if (iClass == SURVIVOR_CLASS_MEDIC)
			{
				CBasePlayer::GiveAmmo(225, "SMG1");
			}
			else if (iClass == SURVIVOR_CLASS_SOLDIER)
			{
				CBasePlayer::GiveAmmo(225, "SMG1");
			}
			else if (iClass == SURVIVOR_CLASS_SNIPER)
			{
				CBasePlayer::GiveAmmo(135, "SMG1");
			}
			else if (iClass == SURVIVOR_CLASS_ENGINEER)
			{
				CBasePlayer::GiveAmmo(135, "SMG1");
			}
		}
	}
	else
	{
		CBasePlayer::GiveAmmo(225, "SMG1");
		CBasePlayer::GiveAmmo(30, "Buckshot");
		CBasePlayer::GiveAmmo(210, "AR2");
		CBasePlayer::GiveAmmo(72, "Pistol");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MP_Player::ManageRandomWeapons(void)
{
	for (int iWeapon = 0; iWeapon < 5; ++iWeapon)
	{
		int iWeaponID = RandomInt(WEAPON_NONE + 1, WEAPON_COUNT - 1);
		const char *pszWeaponName = WeaponIdToClassname(iWeaponID);

		GiveNamedItem(pszWeaponName);
	}
}

int CHL2MP_Player::GetWeaponPreset(int iSlotNum)
{
	if (iSlotNum == 0)
	{
		return m_WeaponPresetPrimary;
	}
	else if (iSlotNum == 1)
	{
		return m_WeaponPresetSecondary;
	}
	else if (iSlotNum == 2)
	{
		return m_WeaponPresetMelee;
	}

	return 0;
}

void CHL2MP_Player::SetWeaponPreset(int iSlotNum, int iPresetNum)
{
	if (iSlotNum == 0)
	{
		m_WeaponPresetPrimary = iPresetNum;
	}
	else if (iSlotNum == 1)
	{
		m_WeaponPresetSecondary = iPresetNum;
	}
	else if (iSlotNum == 2)
	{
		m_WeaponPresetMelee = iPresetNum;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	m_flNextTeamChangeTime = 0.0f;

	GetPlayerSpawnSettings();
	SetPlayerModel();

	BaseClass::Spawn();
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
		
		GiveDefaultItems();
	}

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	if (GetTeamNumber() != TEAM_SPECTATOR)
	{
		StopObserverMode();
	}
	else
	{ 
		StartObserverMode(OBS_MODE_ROAMING);
	}
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

void CHL2MP_Player::SetPlayerModel( void )
{
	if ( GetTeamNumber() == TEAM_BLUE )
	{
		PlayerGameSettings(GetClass(), GetFaction(), GetGender());
		m_nSkin = 2;
		m_iModelType = TEAM_BLUE;
	}
	else if (GetTeamNumber() == TEAM_RED)
	{
		PlayerGameSettings(GetClass(), GetFaction(), GetGender());
		m_nSkin = 1;
		m_iModelType = TEAM_RED;
	}
	else
	{
		if (random->RandomInt(0, 1) == 0)
		{
			PlayerGameSettings(GetClass(), GetFaction(), GetGender());
			m_iModelType = TEAM_BLUE;
		}
		else
		{
			PlayerGameSettings(GetClass(), GetFaction(), GetGender());
			m_iModelType = TEAM_RED;
		}
	}
}

void CHL2MP_Player::PlayerGameSettings(int iClass, int iFaction, int iGender)
{
	if (iClass == SURVIVOR_CLASS_RANGER)
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/ranger/resis_normal_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/ranger/resis_normal_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/ranger/combine_normal_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_METROPOLICE;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/ranger/combine_normal_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_METROPOLICE;
			}
		}
	}
	else if (iClass == SURVIVOR_CLASS_MEDIC)
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/medic/resis_medic_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/medic/resis_medic_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/medic/combine_medic_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/medic/combine_medic_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
		}
	}
	else if (iClass == SURVIVOR_CLASS_SOLDIER)
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/soldier/resis_soldier_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/soldier/resis_soldier_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/soldier/combine_soldier_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/soldier/combine_soldier_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
		}
	}
	else if (iClass == SURVIVOR_CLASS_SNIPER)
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/sniper/resis_sniper_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/sniper/resis_sniper_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/sniper/combine_sniper_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/sniper/combine_sniper_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
		}
	}
	else if (iClass == SURVIVOR_CLASS_ENGINEER)
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/engineer/resis_engineer_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/engineer/resis_engineer_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/engineer/combine_engineer_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/engineer/combine_engineer_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_COMBINESOLDIER;
			}
		}
	}
	else
	{
		if (iFaction == SURVIVOR_FACTION_RESISTANCE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/resistance/ranger/resis_normal_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/resistance/ranger/resis_normal_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_CITIZEN_FEMALE;
			}
		}
		else if (iFaction == SURVIVOR_FACTION_COMBINE)
		{
			if (iGender == SURVIVOR_GENDER_MALE)
			{
				SetModel("models/player/combine/ranger/combine_normal_male.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_METROPOLICE;
			}
			else if (iGender == SURVIVOR_GENDER_FEMALE)
			{
				SetModel("models/player/combine/ranger/combine_normal_female.mdl");
				m_iPlayerSoundType = PLAYER_SOUNDS_METROPOLICE;
			}
		}
	}
}

void CHL2MP_Player::GetPlayerSpawnSettings(void)
{
	if (!IsBot())
	{
		int iGender = Q_atoi(engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_preferredgender"));
		SetGender(iGender);
	}
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_BLUE )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	//bool bKill = false;

	if (HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR)
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if (GetClass() == 0)
	{
		int iRandomClass = random->RandomInt(SURVIVOR_CLASS_RANGER, SURVIVOR_CLASS_ENGINEER);
		SetClass(iRandomClass);
	}

	if (GetFaction() < 0)
	{
		int iRandomFaction = random->RandomInt(SURVIVOR_FACTION_RESISTANCE, SURVIVOR_FACTION_COMBINE);
		SetFaction(iRandomFaction);
	}

	/*
	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}
	*/

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	SetPlayerModel();

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	/*
	if ( bKill == true )
	{
		CommitSuicide();
	}
	*/
}

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );
	ForceRespawn(false);

	return true;
}

bool CHL2MP_Player::HandleCommand_JoinClass(int iClass)
{
	if (iClass == 0)
	{
		Warning("HandleCommand_JoinClass( %d ) - invalid class index.\n", iClass);
		return false;
	}
	
	SetClass(iClass);

	UTIL_ClientPrintAll(HUD_PRINTTALK, "#Player_Changed_Class", GetPlayerName(), GetClassName());

	if (GetTeamNumber() != TEAM_SPECTATOR && !IsDead())
	{
		m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work
		CommitSuicide();
		// add 1 to frags to balance out the 1 subtracted for killing yourself
		IncrementFragCount(1);
		ForceRespawn(false);
	}

	return true;
}

bool CHL2MP_Player::HandleCommand_JoinFaction(int iFaction)
{
	if (iFaction < 0)
	{
		Warning("HandleCommand_JoinFaction( %d ) - invalid faction index.\n", iFaction);
		return false;
	}

	SetFaction(iFaction);

	UTIL_ClientPrintAll(HUD_PRINTTALK, "#Player_Changed_Faction", GetPlayerName(), GetFactionName());

	if (GetTeamNumber() != TEAM_SPECTATOR && !IsDead())
	{
		m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work
		CommitSuicide();
		// add 1 to frags to balance out the 1 subtracted for killing yourself
		IncrementFragCount(1);
		ForceRespawn(false);
	}

	return true;
}

bool CHL2MP_Player::HandleCommand_JoinFactionClass(int iFaction, int iClass)
{
	if (iFaction < 0)
	{
		Warning("HandleCommand_JoinFactionClass( %d ) - invalid faction index.\n", iFaction);
		return false;
	}

	if (iClass == 0)
	{
		Warning("HandleCommand_JoinFactionClass( %d ) - invalid class index.\n", iClass);
		return false;
	}

	SetFaction(iFaction);
	SetClass(iClass);

	UTIL_ClientPrintAll(HUD_PRINTTALK, "#Player_Changed_FactionClass", GetPlayerName(), GetFactionName(), GetClassName());

	if (GetTeamNumber() != TEAM_SPECTATOR && !IsDead())
	{
		m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work
		CommitSuicide();
		// add 1 to frags to balance out the 1 subtracted for killing yourself
		IncrementFragCount(1);
		ForceRespawn(false);
	}

	return true;
}

const char *CHL2MP_Player::GetClassName(void)
{
	int iClass = GetClass();
	const char* chClassName = "";

	if (iClass == SURVIVOR_CLASS_RANGER)
	{
		chClassName = "#SURVIVOR_Class_Ranger";
	}
	else if (iClass == SURVIVOR_CLASS_MEDIC)
	{
		chClassName = "#SURVIVOR_Class_Medic";
	}
	else if (iClass == SURVIVOR_CLASS_SOLDIER)
	{
		chClassName = "#SURVIVOR_Class_Soldier";
	}
	else if (iClass == SURVIVOR_CLASS_SNIPER)
	{
		chClassName = "#SURVIVOR_Class_Sniper";
	}
	else if (iClass == SURVIVOR_CLASS_ENGINEER)
	{
		chClassName = "#SURVIVOR_Class_Engineer";
	}

	return chClassName;
}

const char *CHL2MP_Player::GetFactionName(void)
{
	int iFaction = GetFaction();
	const char* chFactionName = "";

	if (iFaction == SURVIVOR_FACTION_RESISTANCE)
	{
		chFactionName = "#SURVIVOR_Faction_Resistance";
	}
	else if (iFaction == SURVIVOR_FACTION_COMBINE)
	{
		chFactionName = "#SURVIVOR_Faction_Combine";
	}

	return chFactionName;
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else if (FStrEq(args[0], "joinclass"))
	{
		if (args.ArgC() < 2)
		{
			Warning("Player sent bad joinclass syntax\n");
		}

		if (ShouldRunRateLimitedCommand(args))
		{
			int iClass = atoi(args[1]);
			HandleCommand_JoinClass(iClass);
		}
		return true;
	}
	else if (FStrEq(args[0], "joinfaction"))
	{
		if (args.ArgC() < 2)
		{
			Warning("Player sent bad joinfaction syntax\n");
		}

		if (ShouldRunRateLimitedCommand(args))
		{
			int iFaction = atoi(args[1]);
			HandleCommand_JoinFaction(iFaction);
		}
		return true;
	}
	else if (FStrEq(args[0], "joinfactionclass"))
	{
		if (args.ArgC() < 2)
		{
			Warning("Player sent bad joinfactionclass syntax\n");
		}

		if (ShouldRunRateLimitedCommand(args))
		{
			int iFaction = atoi(args[1]);
			int iClass = atoi(args[2]);
			HandleCommand_JoinFactionClass(iFaction, iClass);
		}
		return true;
	}
	else if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
	CNetworkVar(int, m_iTeam);
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),
	SendPropInt(SENDINFO(m_iTeam), 3, SPROP_UNSIGNED)
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
		pRagdoll->m_iTeam = GetTeamNumber();
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	if (GetTeamNumber() != TEAM_SPECTATOR)
	{
		CreateRagdollEntity();
	}

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, inputInfo );

	CBaseEntity *pInflictor = inputInfo.GetInflictor();

	if (pInflictor && (inputInfo.GetDamageType() != DMG_DIRECT))
	{
		int iScoreToAdd = 1;

		if (HL2MPRules()->IsTeamplay() == true)
		{
			GetGlobalTeam(pInflictor->GetTeamNumber())->AddScore(iScoreToAdd);
		}
		else
		{
			pInflictor->AddPoints(iScoreToAdd, false);
		}
	}

	return BaseClass::OnTakeDamage( inputInfo );
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if (GetTeamNumber() == TEAM_SPECTATOR)
		return;

	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
	EmitSound("Player.Death");
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	//edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_BLUE )
		{
			pSpawnpointName = "info_player_combine";
			pLastSpawnPoint = g_pLastCombineSpawn;
		}
		else if (GetTeamNumber() == TEAM_RED)
		{
			pSpawnpointName = "info_player_rebel";
			pLastSpawnPoint = g_pLastRebelSpawn;
		}

		if ( gEntList.FindEntityByClassname( NULL, pSpawnpointName ) == NULL )
		{
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
		}
	}
	else
	{
		int iRandomSpawn = random->RandomInt(0, 1);
		if (GetTeamNumber() != TEAM_SPECTATOR)
		{
			if (iRandomSpawn == 0)
			{
				pSpawnpointName = "info_player_combine";
				pLastSpawnPoint = g_pLastSpawn;
			}
			else
			{
				pSpawnpointName = "info_player_rebel";
				pLastSpawnPoint = g_pLastSpawn;
			}
		}

		if (gEntList.FindEntityByClassname(NULL, pSpawnpointName) == NULL)
		{
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	/*
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}
	*/

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_BLUE )
		{
			g_pLastCombineSpawn = pSpot;
		}
		else if (GetTeamNumber() == TEAM_RED)
		{
			g_pLastRebelSpawn = pSpot;
		}
	}

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}

CON_COMMAND(showposition, "prints the time remaining in the match")
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());

	Vector vecOrigin = pPlayer->GetAbsOrigin() + Vector(0, 0, 10);
	Msg("Player position XYZ Coords: X: %f Y: %f Z: %f\n", vecOrigin.x, vecOrigin.y, vecOrigin.z);
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}
