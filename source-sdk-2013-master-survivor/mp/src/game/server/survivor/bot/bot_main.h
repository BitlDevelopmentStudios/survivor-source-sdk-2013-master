//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#ifndef BOT_MAIN_H
#define BOT_MAIN_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player.h"
#include "hl2mp_player.h"

#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

// how often bot checks for obstacles (phys props, doors,etc) and deals with them
#define BOT_NEXT_OBSTACLE_CHECK 5.0f

// how often bot checks if got stuck
#define STUCK_SAMPLES 5.0f

// how quick bot turns 
#define SKILL_MIN_YAW_RATE 7.0f
#define SKILL_MAX_YAW_RATE 10.0f

// how fast bot runs
#define SKILL_MIN_SPEED 190.0f
#define SKILL_MAX_SPEED 320.0f

// how often bot strafes
#define SKILL_MIN_STRAFE 3.0f
#define SKILL_MAX_STRAFE 5.0f // set this to 0 to disable strafe skill at all

//ranges
#define SKILL_MELEE_RANGE 60.0f
#define SKILL_CLOSE_RANGE 100.0f
#define SKILL_MID_RANGE 150.0f
#define SKILL_LONG_RANGE 250.0f
#define SKILL_MAX_RANGE FLT_MAX

// spawn points are used as hide place references for bots
#define SPAWN_POINT_NAME "info_player_deathmatch"

// ladder's top dismount point gets added some artificial distance to allow bot more room to complete the operation
#define LADDER_EXTRA_HEIGHT_VEC Vector(0,0,25)

enum BotGeneralStates_t
{
	BOT_NAVIG_IDLE, // bot just awaits
	BOT_NAVIG_PATH, // assigned a route to somewhere
	BOT_NAVIG_PATH_ENFORCED, // same but cannot be aborted by direct navigation
	BOT_NAVIG_DIRECT, // bot can see enemy and no obstacles in between detected, no need to ask for waypoints (less expensive)
	BOT_NAVIG_UNSTUCK, // bot is headed to a waypoint to get unstuck, cannot be interrupted no matter what
};

enum BotSchedules_t
{
	BOT_SCHED_COMBAT, 
	BOT_SCHED_HIDE,	
	BOT_SCHED_FOLLOW_TEAM_MEMBERS,
};

// these are the basic bot parameters, defined at creation time only
enum BotSkill_t
{
	BOT_SKILL_SPEED,
	BOT_SKILL_YAW_RATE,
	BOT_SKILL_STRAFE,

	BOT_SKILL_MAX_TOKENS,
};

// structure to store waypoints
struct NavAreaData_t
{
	Vector Center;
	NavTraverseType TransientType;
	int AttributeType;
	int Id;
};

// intermediate sized hull used when we want to test something smaller than the full bot hull, and bigger than a simple trace line
static Vector BotTestHull = Vector(5,5,5);

// This is our bot class.
class CSurvivorBot : public CHL2MP_Player
{
public:

	float			m_flNextStrafeTime;
	float			m_flStrafeSkillRelatedTimer;
	float			m_flSideMove;

	QAngle			m_ForwardAngle;
	QAngle			m_LastAngles;

	// behaviour - capabilities
	int m_nBotState;
	int m_nBotSchedule;
	float m_flSkill[BOT_SKILL_MAX_TOKENS];

	// navigation
	float m_flNextJump;
	bool m_bIsOnLadder;
	CUtlVector <NavAreaData_t> m_Waypoints;
	float m_flNextPathCheck;
	float m_flDontUseDirectNav;
	float m_flTimeToRecheckStuck;
	float m_flDesiredYaw;
	float m_flNextDealObstacles;
	int m_nIsHiding;
	CUtlVector<int> m_AlreadyCheckedHideSpots;
	float m_flCreateRandomPathCoolDown;
	float m_flNextProximityCheck;
	float m_flBotToEnemyDist;
	float m_flDistTraveled;
	float m_flHeightDifToEnemy;

	// combat
	EHANDLE hEnemy;
	float m_flTimeToRecheckEnemy;
	bool m_bEnemyOnSights;	
	bool m_bInRangeToAttack;
	float m_flNextBotAttack;
	float m_flMinRangeAttack;
	bool m_bInCombat;

	bool HasEnemy() { return (hEnemy.Get() != NULL && hEnemy.Get()->IsAlive()); }
	bool RecheckEnemy() { return m_flTimeToRecheckEnemy < gpGlobals->curtime; }

	CBasePlayer *GetEnemy() { return dynamic_cast<CBasePlayer *>(hEnemy.Get()); }
		
	void Spawn()
	{
		BaseClass::Spawn();

		hEnemy.Set(NULL);
		ResetNavigationParams();	
		m_AlreadyCheckedHideSpots.RemoveAll();
		m_flNextDealObstacles = 0;
		m_flCreateRandomPathCoolDown = 0;
		m_flNextProximityCheck = 0;		
		m_flDistTraveled = 0; // distance this bot has traveled recently, since last stuck check
		m_flMinRangeAttack = SKILL_MELEE_RANGE;
		m_bInRangeToAttack = false;
		m_flNextAttack = gpGlobals->curtime;
		m_flNextStrafeTime = 0;
		m_flStrafeSkillRelatedTimer = 0;

		// normal hl2mp spawn() stuff -Bitl
		CHL2MP_Player::Spawn();
	}

	void ResetNavigationParams()
	{
		m_Waypoints.RemoveAll();
		m_flNextJump = 0;
		m_bIsOnLadder = false;
		m_flNextPathCheck = 0;
		m_flDontUseDirectNav = 0;
	}

	void AddWaypoint( Vector center, NavTraverseType transient, int attribute, int id, bool AddToTail = false )
	{  
		NavAreaData_t data;
		VectorCopy( center, data.Center );
		data.TransientType = transient;
		data.AttributeType = attribute;
		data.Id = id;		

		if( AddToTail )
			m_Waypoints.AddToTail(data); 
		else
			m_Waypoints.AddToHead(data); 	
	}

	void ResetWaypoints( void ) { m_Waypoints.RemoveAll(); }
};


CBasePlayer *BotPutInServer(bool bFrozen, int iTeam);

bool CreatePath( CSurvivorBot *pBot, CBasePlayer *pPlayer, Vector OptionalOrg = vec3_origin );

float BotWeaponRangeDetermine(CBaseCombatWeapon *pActiveWeapon);

void Bot_RunAll();

void Bot_Say(CSurvivorBot *pBot);

#endif // BOT_MAIN_H
