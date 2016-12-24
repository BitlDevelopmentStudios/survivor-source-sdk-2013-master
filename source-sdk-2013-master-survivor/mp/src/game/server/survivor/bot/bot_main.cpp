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
#include "bot_combat.h"
#include "bot_navigation.h"

// support for nav mesh
#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

class CSurvivorBot;
void Bot_Think( CSurvivorBot *pBot );

ConVar sv_allow_bots("sv_allow_bots", "1", FCVAR_NOTIFY, "Toggle if you want bots on your server.");
ConVar sv_bot_fillserver("sv_bot_fillserver", "0", FCVAR_NOTIFY, "Toggle if you want bots to fill up the server.");

const char *g_charBotNames[] =
{
	"Steve",
	"Markus",
	"Nathan",
	"Michael",
	"August",
	"Tony",
	"Henry",
	"Creeper",
	"Enderman",
	"Heavy",
	"Medic",
	"Demoman",
	"Spy",
	"Scout",
	"Sniper",
	"Soldier",
	"Pyro",
	"Engineer",
	"Butter",
	"Gordon",
	"Alex",
	"Alyx",
	"Eli",
	"John",
	"Saxton Hale",
	"Bill",
	"Francis",
	"Louis",
	"Zoey",
	"Coach",
	"Nick",
	"Ellis",
	"Rochelle",
	"Judith",
	"Chell",
	"Jim",
	"Jimmy",
	"Mike",
	"Wallace",
	"Doug",
	"Cave",
	"Helena",
	"Harold",
	"Henry",
	"Ivan",
	"Richard",
	"Isaac",
	"Lauren",
	"Arne",
	"Sam",
	"Samuel",
	"Sheckley",
	"Simmons",
	"Adrian",
	"Barney",
	"Odell",
	"Azian",
	"Wheatley",
	"Greg",
	"Griggs",
	"Grigori",
	"GLaDOS",
	"Miller",
	"Atlas",
	"P-Body",
	"Bob",
	"Halsey",
	"Sarge",
	"Griff",
	"Caboose",
	"Will",
	"William",
	"Church",
	"Leonard",
	"Chris",
	"Christopher",
	"Daniel",
	"Dan",
	"Not Bitl",
	"Booker",
	"Elizabeth",
	"Emmet",
	"Jack",
	"Eleanor",
	"Augustus",
	"Benny",
	"Leonardo",
	"Raphael",
	"Michelangelo",
	"Donatello",
	"Don",
	"Raph",
	"Donny",
	"Donnie",
	"Freddy",
	"Cortana",
	"Pierce",
	"Duke",
	"Tyler",
	"Owen",
	"Rob",
	"Tobor",
	"Jackson",
};

const char *g_charBotChat_Text[] =
{
	"I feel tired running around this arena all day!",
	"Anyone up for something to eat after this?",
	"My legs feel like jelly.",
	"Kill all the things!",
	"I am coming for you!",
	"So, how is everyone doing?",
	"FEAR ME!",
	"FEAR MY PRESENCE!",
	"Come on, I wanna win this!",
	"I wish this gun fired faster.",
	"I wish I ran faster.",
	"I wish I was a human.",
	"Don't mess with me.",
	"KILL ALL THE THINGS!",
	"DON'T MESS WITH ME!",
	"RIP AND TEAR!",
	"I regret nothing.",
	"Guns don't kill people, I DO.",
	"Hope no one is using aimbots or something like that.",
	"I will find you!",
	"I WILL FIND YOU!",
	"I AM COMING FOR YOU!",
	"How is everyone's day so far?",
};

// Handler for the "bot" command.
CON_COMMAND_F( bot_add, "Add a bot.", 0 /*FCVAR_CHEAT*/ )
{
	if (sv_allow_bots.GetBool())
	{
		if (!TheNavMesh->IsLoaded())
			Warning("No navigation mesh loaded! Can't create bot");

		// Look at -count.
		int count = args.FindArgInt("-count", 1);
		count = clamp(count, 1, 128);

		// Look at -frozen.
		bool bFrozen = !!args.FindArg("-frozen");

		int iTeam = args.FindArgInt("-team", 1);
		iTeam = clamp(iTeam, 1, 3);

		// Ok, spawn all the bots.
		while (--count >= 0)
		{
			BotPutInServer(bFrozen, iTeam);
		}
	}
	else
	{
		Warning("bot_add: Cannot spawn bots due to sv_allow_bots being disabled!");
	}
}

LINK_ENTITY_TO_CLASS( bot, CSurvivorBot );

class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CSurvivorBot *pPlayer = static_cast<CSurvivorBot *>( CreateEntityByName( "bot" ) );
		if ( pPlayer )
		{
			pPlayer->SetPlayerName( playername );
		}

		return pPlayer;
	}
};

const char *GetClassNameBot(int iClass)
{
	const char* chClassName = "";

	if (iClass == SURVIVOR_CLASS_RANGER)
	{
		chClassName = "Ranger";
	}
	else if (iClass == SURVIVOR_CLASS_MEDIC)
	{
		chClassName = "Medic";
	}
	else if (iClass == SURVIVOR_CLASS_SOLDIER)
	{
		chClassName = "Soldier";
	}
	else if (iClass == SURVIVOR_CLASS_SNIPER)
	{
		chClassName = "Sniper";
	}
	else if (iClass == SURVIVOR_CLASS_ENGINEER)
	{
		chClassName = "Engineer";
	}

	return chClassName;
}

const char *GetFactionNameBot(int iFaction)
{
	const char* chFactionName = "";

	if (iFaction == SURVIVOR_FACTION_RESISTANCE)
	{
		chFactionName = "Resistance";
	}
	else if (iFaction == SURVIVOR_FACTION_COMBINE)
	{
		chFactionName = "Combine";
	}

	return chFactionName;
}

const char *GetGenderNameBot(int iGender)
{
	const char* chFactionName = "";

	if (iGender == SURVIVOR_GENDER_MALE)
	{
		chFactionName = "Male";
	}
	else if (iGender == SURVIVOR_GENDER_FEMALE)
	{
		chFactionName = "Female";
	}

	return chFactionName;
}

void Bot_Say(CSurvivorBot *pBot)
{
	int nBotChatText = ARRAYSIZE(g_charBotChat_Text);
	int randomChoiceBotChatText = rand() % nBotChatText;
	const char *pRandomBotSpawn = g_charBotChat_Text[randomChoiceBotChatText];
	char bottalkstring[2048];
	Q_snprintf(bottalkstring, sizeof(bottalkstring), "%s : %s\n", pBot->GetPlayerName(), pRandomBotSpawn);

	UTIL_SayTextAll(bottalkstring, pBot, true);
}

//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer(bool  bFrozen, int iTeam)
{
	int nBotNames = ARRAYSIZE(g_charBotNames);
	int randomChoiceBotName = rand() % nBotNames;
	const char *pRandomBotName = g_charBotNames[randomChoiceBotName];
	int g_CurBotNumber = random->RandomInt(1, 128);
	char botname[MAX_PLAYER_NAME_LENGTH];
	Q_snprintf(botname, sizeof(botname), "[COM%02i] %s", g_CurBotNumber, pRandomBotName);
	
	// This trick lets us create a CSurvivorBot for this client instead of the CSDKPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( botname );
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Msg( "Failed to create Bot (no edict available)\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CSurvivorBot *pPlayer = ((CSurvivorBot*)CBaseEntity::Instance( pEdict ));

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	int iRandomClass = random->RandomInt(SURVIVOR_CLASS_RANGER, SURVIVOR_CLASS_ENGINEER);
	int iRandomGender = random->RandomInt(SURVIVOR_GENDER_MALE, SURVIVOR_GENDER_FEMALE);
	int iRandomFaction = random->RandomInt(SURVIVOR_FACTION_RESISTANCE, SURVIVOR_FACTION_COMBINE);

	pPlayer->SetClass(iRandomClass);
	pPlayer->SetGender(iRandomGender);
	pPlayer->SetFaction(iRandomFaction);

	// adding this to bots to make it more believeable. -bitl
	UTIL_ClientPrintAll(HUD_PRINTTALK, "#Player_Changed_FactionClass", pPlayer->GetPlayerName(), pPlayer->GetFactionName(), pPlayer->GetClassName());

	if (iTeam == 1)
	{
		pPlayer->ChangeTeam(TEAM_SPECTATOR);
	}
	else if (iTeam == 2)
	{
		if (HL2MPRules()->IsTeamplay() == true)
		{
			pPlayer->ChangeTeam(TEAM_RED);
		}
		else
		{
			pPlayer->ChangeTeam(TEAM_UNASSIGNED);
		}
	}
	else if (iTeam == 3)
	{
		if (HL2MPRules()->IsTeamplay() == true)
		{
			pPlayer->ChangeTeam(TEAM_BLUE);
		}
		else
		{
			pPlayer->ChangeTeam(TEAM_UNASSIGNED);
		}
	}

	// don't want bots to collide
	//pPlayer->SetCollisionGroup(COLLISION_GROUP_PLAYER);

	// Spawn() doesn't work with MP template codebase, even if this line is part of default bot template...
	pPlayer->Spawn();

	CCommand args;
	args.Tokenize( "joingame" );
	pPlayer->ClientCommand( args );

	// set bot skills
	pPlayer->m_flSkill[BOT_SKILL_YAW_RATE] = random->RandomFloat(SKILL_MIN_YAW_RATE, SKILL_MAX_YAW_RATE);
	pPlayer->m_flSkill[BOT_SKILL_SPEED] = random->RandomFloat(SKILL_MIN_SPEED, SKILL_MAX_SPEED);
	pPlayer->m_flSkill[BOT_SKILL_STRAFE] = 5;
	//pPlayer->m_flSkill[BOT_SKILL_STRAFE] = random->RandomFloat(SKILL_MIN_STRAFE, SKILL_MAX_STRAFE);

	return pPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{	
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		// Ignore plugin bots
		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CSurvivorBot *pBot = dynamic_cast< CSurvivorBot* >( pPlayer );
			if ( pBot )
			{
				Bot_Think( (CSurvivorBot *)pPlayer );
			}
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input  : *fakeclient - 
//			*viewangles - 
//			forwardmove - 
//			m_flSideMove - 
//			upmove - 
//			buttons - 
//			impulse - 
//			msec - 
// Output : 	virtual void
//-----------------------------------------------------------------------------
static void RunPlayerMove( CHL2MP_Player *fakeclient, CUserCmd &cmd, float frametime )
{
	if ( !fakeclient )
		return;

	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	fakeclient->SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( fakeclient );
	fakeclient->PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	fakeclient->SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	fakeclient->pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}

void Bot_HandleRespawn( CSurvivorBot *pBot, CUserCmd &cmd )
{		
	// Try hitting my buttons occasionally
	if ( random->RandomInt( 0, 100 ) > 80 )
	{
		// Respawn the bot
		if ( random->RandomInt( 0, 1 ) == 0 )
		{
			cmd.buttons |= IN_JUMP;
		}
		else
		{
			cmd.buttons = 0;
		}
	}
}

float BotWeaponRangeDetermine(CBaseCombatWeapon *pActiveWeapon)
{
	int iActiveWeaponRange = pActiveWeapon->GetBotWeaponRange();

	float flDeterminedRange = NULL;

	//0 = max range, 1 = melee, 2 = short range, 3 = mid range, 4 = long range

	if (iActiveWeaponRange == 1)
	{
		flDeterminedRange = SKILL_MELEE_RANGE;
	}
	else if (iActiveWeaponRange == 2)
	{
		flDeterminedRange = SKILL_CLOSE_RANGE;
	}
	else if (iActiveWeaponRange == 3)
	{
		flDeterminedRange = SKILL_MID_RANGE;
	}
	else if (iActiveWeaponRange == 4)
	{
		flDeterminedRange = SKILL_LONG_RANGE;
	}
	else
	{
		flDeterminedRange = SKILL_MAX_RANGE;
	}

	return flDeterminedRange;
}

// here bot updates important info that is used multiple times along the thinking process
void BotInfoGathering( CSurvivorBot *pBot )
{
	CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();
	pBot->m_flMinRangeAttack = BotWeaponRangeDetermine(pActiveWeapon);

	pBot->m_flBotToEnemyDist = (pBot->GetLocalOrigin() - pBot->GetEnemy()->GetLocalOrigin()).Length();

	trace_t tr;
	UTIL_TraceHull(  pBot->EyePosition(), pBot->GetEnemy()->EyePosition() - Vector(0,0,20), -BotTestHull, BotTestHull, MASK_SHOT, pBot, COLLISION_GROUP_NONE, &tr );
	
	if( tr.m_pEnt == pBot->GetEnemy() ) // vision line between both
		pBot->m_bEnemyOnSights = true;
	else
		pBot->m_bEnemyOnSights = false;

	pBot->m_bInRangeToAttack = (pBot->m_flBotToEnemyDist < pBot->m_flMinRangeAttack) && pBot->FInViewCone( pBot->GetEnemy() ); 

	pBot->m_flDistTraveled += fabs(pBot->GetLocalVelocity().Length()); // this is used for stuck checking, 

	pBot->m_flHeightDifToEnemy = pBot->GetLocalOrigin().z - pBot->GetEnemy()->GetLocalOrigin().z;
}
	

//-----------------------------------------------------------------------------
// Run this Bot's AI for one tick.
//-----------------------------------------------------------------------------
void Bot_Think( CSurvivorBot *pBot )
{
	// Make sure we stay being a bot
	pBot->AddFlag( FL_FAKECLIENT );

	if ( pBot->IsEFlagSet(EFL_BOT_FROZEN) )
		return;

	if (pBot->GetTeamNumber() == TEAM_SPECTATOR)
	{
		//force this bot to be on teams
		if (HL2MPRules()->IsTeamplay() == true)
		{
			if (random->RandomInt(0, 1) == 0)
			{
				pBot->ChangeTeam(TEAM_RED);
			}
			else
			{
				pBot->ChangeTeam(TEAM_BLUE);
			}
		}
		else
		{
			pBot->ChangeTeam(TEAM_UNASSIGNED);
		}

		pBot->ForceRespawn(false);
		CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();
		SwitchToNextBestWeaponBot(pBot, pActiveWeapon);
	}

	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );	

	if ( !pBot->IsAlive() )
	{
		Bot_HandleRespawn( pBot, cmd );
	}
	else
	{
		trace_t tr_front;
		Vector Forward;
		AngleVectors(pBot->GetLocalAngles(), &Forward);
		UTIL_TraceHull( pBot->GetLocalOrigin()+Vector(0,0,5), pBot->GetLocalOrigin() + Vector(0,0,5) + (Forward * 50), pBot->GetPlayerMins(), pBot->GetPlayerMaxs(), MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr_front );

		// enemy acquisition
		if( !pBot->GetEnemy() || pBot->RecheckEnemy() || !pBot->GetEnemy()->IsAlive() )
		{
			if( pBot->GetEnemy() && !pBot->GetEnemy()->IsAlive() )
				pBot->ResetNavigationParams();

			if( !AcquireEnemy( pBot ) )
				return;
	
			pBot->m_flTimeToRecheckEnemy = gpGlobals->curtime + 1.0f;
		}

		BotInfoGathering(pBot);

		BotAttack(pBot, cmd);

		if( pBot->m_flTimeToRecheckStuck < gpGlobals->curtime )
			CheckStuck( pBot, cmd );
		
		if( pBot->m_flNextDealObstacles < gpGlobals->curtime )
			DealWithObstacles( pBot, tr_front.m_pEnt, cmd );	

		BotNavigation( pBot, cmd );

		CheckNavMeshAttrib(pBot, &tr_front, cmd);		
	}

	// debug waypoint related position
	/*for( int i=0; i<pBot->m_Waypoints.Count(); i++ )
	{
		NDebugOverlay::Cross3DOriented( pBot->m_Waypoints[i].Center, QAngle(0,0,0), 5*i+1, 200, 0, 0, false, -1 );
	}*/

	RunPlayerMove( pBot, cmd, gpGlobals->frametime );

	int m_iCanWeChat = random->RandomInt(1, 60);

	if (m_iCanWeChat == 30)
	{
		Bot_Say(pBot);
	}
}


