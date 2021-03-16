#include "cbase.h"
#include "survivor_bot.h"
#include "bot_manager.h"
#include "bots\in_utils.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_survivor_bot_closestdistancetoplayer("sv_survivor_bot_closestdistancetoplayer", "1800", FCVAR_CHEAT | FCVAR_NOTIFY, "");
ConVar sv_survivor_bot_respawn("sv_survivor_bot_respawn", "1", FCVAR_CHEAT | FCVAR_NOTIFY, "");
ConVar sv_survivor_bot_ai("sv_survivor_bot_ai", "1", FCVAR_NOTIFY, "");
ConVar sv_survivor_bot_tag("sv_survivor_bot_tag", "1", FCVAR_NOTIFY, "");

extern void respawn(CBaseEntity* pEdict, bool fCopyCorpse);

CUtlVector<string_t> m_botScriptNames;

void LoadBotNames(void)
{
    m_botScriptNames.RemoveAll();

    KeyValues* pKV = new KeyValues("BotNames");
    if (pKV->LoadFromFile(filesystem, "scripts/bot_names.txt", "GAME"))
    {
        FOR_EACH_VALUE(pKV, pSubData)
        {
            if (FStrEq(pSubData->GetString(), ""))
                continue;

            string_t iName = AllocPooledString(pSubData->GetString());
            if (m_botScriptNames.Find(iName) == m_botScriptNames.InvalidIndex())
                m_botScriptNames[m_botScriptNames.AddToTail()] = iName;
        }
    }

    pKV->deleteThis();
}

const char* NewNameSelection(void)
{
    if (m_botScriptNames.Count() == 0)
        return "MISSINGNO";

    int nPoneNames = m_botScriptNames.Count();
    int randomChoice = rand() % nPoneNames;
    string_t iszName = m_botScriptNames[randomChoice];
    const char* pszName = STRING(iszName);

    return pszName;
}

//================================================================================
// It allows to create a bot with the name and position specified. Uses our custom name algorithm.
//================================================================================
CPlayer* CreateSurvivorBot(const char* pPlayername, const Vector* vecPosition, const QAngle* angles)
{
    LoadBotNames();

    if (!pPlayername) {
        const char* pPlayername1 = NewNameSelection();
        const char* pBotNameFormat = sv_survivor_bot_tag.GetBool() ? "[BOT] %s%s%s" : "%s%s%s";
        char combinedName[MAX_PLAYER_NAME_LENGTH];
        Q_snprintf(combinedName, sizeof(combinedName), pBotNameFormat, pPlayername1);
        pPlayername = NewNameSelection();
    }

    edict_t* pSoul = engine->CreateFakeClient(pPlayername);
    Assert(pSoul);

    if (!pSoul) {
        Warning("There was a problem creating a bot. Maybe there is no more space for players on the server.");
        return NULL;
    }

    CPlayer* pPlayer = (CPlayer*)CBaseEntity::Instance(pSoul);
    Assert(pPlayer);

    pPlayer->ClearFlags();
    pPlayer->AddFlag(FL_CLIENT | FL_FAKECLIENT);

    // This is where we implement the Artificial Intelligence. 
    pPlayer->SetUpBot();
    Assert(pPlayer->GetBotController());

    if (!pPlayer->GetBotController()) {
        Warning("There was a problem creating a bot. The player was created but the controller could not be created.");
        return NULL;
    }

    pPlayer->Spawn();

    if (vecPosition) {
        pPlayer->Teleport(vecPosition, angles, NULL);
    }

    ++g_botID;
    return pPlayer;
}

CSurvivor_Bot::CSurvivor_Bot(CBasePlayer* parent) : BaseClass(parent)
{
}

//================================================================================
// Create the components that the bot will have
//================================================================================
void CSurvivor_Bot::SetUpComponents()
{
    ADD_COMPONENT(CBotVision);
    ADD_COMPONENT(CBotFollow);
    ADD_COMPONENT(CBotLocomotion);
    ADD_COMPONENT(CBotMemory);
    ADD_COMPONENT(CBotAttack);
    ADD_COMPONENT(CBotDecision); // This component is mandatory!
}

//================================================================================
// Create the schedules that the bot will have
//================================================================================
void CSurvivor_Bot::SetUpSchedules()
{
    ADD_COMPONENT(CHuntEnemySchedule);
    ADD_COMPONENT(CReloadSchedule);
    ADD_COMPONENT(CCoverSchedule);
    ADD_COMPONENT(CHideSchedule);
    ADD_COMPONENT(CChangeWeaponSchedule);
    ADD_COMPONENT(CHideAndHealSchedule);
    ADD_COMPONENT(CHideAndReloadSchedule);
    ADD_COMPONENT(CMoveAsideSchedule);
    ADD_COMPONENT(CCallBackupSchedule);
    ADD_COMPONENT(CDefendSpawnSchedule);
}

void CSurvivor_Bot::Spawn()
{
    BaseClass::Spawn();
}

void CSurvivor_Bot::Update()
{
    VPROF_BUDGET("CSurvivor_Bot::Update", VPROF_BUDGETGROUP_BOTS);

    //i should have made this a schedule but when i did it just didn't function so i'm doing this.

    if (GetHost()->IsAlive())
    {
        if (sv_survivor_bot_ai.GetBool())
        {
            BaseClass::Update();

            if (TheNavAreas.Count() > 0)
            {
                //wander around.
                Vector vecFrom(GetAbsOrigin());
                float dist = sv_survivor_bot_closestdistancetoplayer.GetFloat();
                CPlayer* closestPlayer = Utils::GetClosestPlayer(GetAbsOrigin(), &dist);

                CNavArea* pArea = closestPlayer->GetLastKnownArea();

                if (pArea == NULL)
                {
                    pArea = TheNavAreas[RandomInt(0, TheNavAreas.Count() - 1)];

                    if (pArea == NULL)
                        return;
                }

                Vector vecGoal(pArea->GetCenter());

                if (!GetLocomotion() || !GetLocomotion()->IsTraversable(vecFrom, vecGoal))
                    return;

                GetLocomotion()->DriveTo("Move to nearest player or randomly roam.", pArea);
            }
            else
            {
                Warning("No nav mesh detected. Kicking %s...\n", GetHost()->GetPlayerName());
                Kick();
                return;
            }
        }
    }
    //respawn if we die.
    else
    {
        if (sv_survivor_bot_respawn.GetBool())
        {
            respawn(GetHost(), !IsObserver());
            Spawn();
        }
    }
}

//we don't use these.... use them.
const char* GetClassNameBot(int iClass)
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

const char* GetFactionNameBot(int iFaction)
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

const char* GetGenderNameBot(int iGender)
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

extern ConVar bot_team;
extern ConVar bot_class;
extern ConVar bot_faction;
extern ConVar bot_gender;

CON_COMMAND_F(bot_add, "Adds a specified number of Survivor bots", FCVAR_SERVER)
{
    SpawnSurvivorBots(Q_atoi(args.Arg(1)));
}

void SpawnSurvivorBots(int botCount)
{
    // Look at -count. Don't include ourselves.
    int count = Clamp(botCount, 1, gpGlobals->maxClients -1);
    // Ok, spawn all the bots.
    while (--count >= 0) {
        CPlayer* pPlayer = CreateSurvivorBot(NULL, NULL, NULL);
        Assert(pPlayer);

        if (!pPlayer || pPlayer == nullptr)
            return;

        if (pPlayer) {
            if (bot_team.GetInt() > 0) 
            {
                pPlayer->ChangeTeam(bot_team.GetInt());
            }
            else
            {
                //force this bot to be on teams
                if (HL2MPRules()->IsTeamplay() == true)
                {
                    if (random->RandomInt(0, 1) == 0)
                    {
                        pPlayer->ChangeTeam(TEAM_RED);
                    }
                    else
                    {
                        pPlayer->ChangeTeam(TEAM_BLUE);
                    }
                }
                else
                {
                    pPlayer->ChangeTeam(TEAM_UNASSIGNED);
                }
            }

            if (bot_class.GetInt() > 0) 
            {
                pPlayer->SetClass(bot_class.GetInt());
            }
            else
            {
                int iRandomClass = random->RandomInt(SURVIVOR_CLASS_RANGER, SURVIVOR_CLASS_ENGINEER);
                pPlayer->SetClass(iRandomClass);
            }

            if (bot_gender.GetInt() > 0)
            {
                pPlayer->SetGender(bot_gender.GetInt());
            }
            else
            {
                int iRandomGender = random->RandomInt(SURVIVOR_GENDER_MALE, SURVIVOR_GENDER_FEMALE);
                pPlayer->SetGender(iRandomGender);
            }

            if (bot_faction.GetInt() > 0)
            {
                pPlayer->SetFaction(bot_faction.GetInt());
            }
            else
            {
                int iRandomFaction = random->RandomInt(SURVIVOR_FACTION_RESISTANCE, SURVIVOR_FACTION_COMBINE);
                pPlayer->SetFaction(iRandomFaction);
            }

            // adding this to bots to make it more believeable. -bitl
            UTIL_ClientPrintAll(HUD_PRINTTALK, "#Player_Changed_FactionClass", pPlayer->GetPlayerName(), pPlayer->GetFactionName(), pPlayer->GetClassName());

            pPlayer->Spawn();

            CCommand args;
            args.Tokenize("joingame");
            pPlayer->ClientCommand(args);
        }
    }
}