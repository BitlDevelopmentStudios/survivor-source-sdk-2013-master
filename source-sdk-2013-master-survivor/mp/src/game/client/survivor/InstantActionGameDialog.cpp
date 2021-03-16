//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "InstantActionGameDialog.h"

#include <stdio.h>

using namespace vgui;

#include <vgui/ILocalize.h>

#include "FileSystem.h"
#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/convar.h"

// for SRC
#include <vstdlib/random.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define RANDOM_MAP "#GameUI_RandomMap"

void LoadCommand(void)
{
	CInstantActionGameDialog* pCInstantActionGameDialog = new CInstantActionGameDialog(NULL);
	pCInstantActionGameDialog->Activate();
}

ConCommand instantactiondialog("instantactiondialog", LoadCommand, "", FCVAR_HIDDEN);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CInstantActionGameDialog::CInstantActionGameDialog(vgui::Panel *parent) : BaseClass(NULL, "InstantActionGameDialog")
{
	SetSize(348, 460);
	//SetOKButtonText("#GameUI_Start");
	
	m_pMapList = new ComboBox(this, "MapList", 12, false);
	m_maxBots = new TextEntry( this, "MaxBots" );
	m_maxBots->SetText("16");
	m_pTeamplay = new CheckButton(this, "Teamplay", "#Valve_Teamplay");
	m_pTeamplay->SetSelected(1);
	m_pTimeLimit = new TextEntry(this, "TimeLimit");
	m_pTimeLimit->SetText("5");
	m_pRandomizer = new CheckButton(this, "Randomizer", "#SURVIVOR_Randomizer_InstantAction");
	m_pRandomizer->SetSelected(0);

	LoadMapList();

	// create KeyValues object to load/save config options
	m_pSavedData = new KeyValues("InstantActionConfig");

	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile(g_pFullFileSystem, "InstantActionConfig.vdf", "GAME"); // this is game-specific data, so it should live in GAME, not CONFIG
		GetOptions();
	}

	LoadControlSettings("Resource/InstantActionGameDialog.res");

	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CInstantActionGameDialog::~CInstantActionGameDialog()
{
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInstantActionGameDialog::OnClose()
{
	BaseClass::OnClose();
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CInstantActionGameDialog::OnCommand(const char *command)
{
	if ( !stricmp( command, "Ok" ) )
	{
		char szMapName[64];
		Q_strncpy(szMapName, GetMapName(), sizeof( szMapName ));
		
		char szMaxBots[256];
		m_maxBots->GetText( szMaxBots, sizeof( szMaxBots ) );

		//this teamplay thing is hacky but it works. -Bitl
		const char *teamplayValue = "";

		if (m_pTeamplay->IsSelected())
		{
			teamplayValue = "1";
		}
		else
		{
			teamplayValue = "0";
		}

		char szTeamplay[256];

		Q_strncpy(szTeamplay, teamplayValue, sizeof(szTeamplay));

		char szTimeLimit[256];
		m_pTimeLimit->GetText(szTimeLimit, sizeof(szTimeLimit));

		//this teamplay thing is hacky but it works. -Bitl
		const char *randomizerValue = "";

		if (m_pRandomizer->IsSelected())
		{
			randomizerValue = "1";
		}
		else
		{
			randomizerValue = "0";
		}

		char szRandomizer[256];

		Q_strncpy(szRandomizer, randomizerValue, sizeof(szRandomizer));

		// save the config data
		if (m_pSavedData)
		{
			if (IsRandomMapSelected())
			{
				// it's set to random map, just save an
				m_pSavedData->SetString("map", "");
			}
			else
			{
				m_pSavedData->SetString("map", szMapName);
			}

			m_pSavedData->SetString("maxBots", szMaxBots);
			m_pSavedData->SetBool("teamplay", m_pTeamplay->IsSelected());
			m_pSavedData->SetString("timelimit", szTimeLimit);
			m_pSavedData->SetBool("randomizer", m_pRandomizer->IsSelected());

			// save config to a file
			m_pSavedData->SaveToFile(g_pFullFileSystem, "InstantActionConfig.vdf", "GAME");
		}

		char szMapCommand[1024];

		// create the command to execute
		Q_snprintf(szMapCommand, sizeof(szMapCommand), "disconnect\nmaxplayers %s\nmp_teamplay %s\nmp_timelimit %s\nsurvivor_randomizer %s\nsv_lan 1\nsv_survivor_bot_fillserver 1\nprogress_enable\nmap %s\n", szMaxBots, szTeamplay, szTimeLimit, szRandomizer, szMapName);

		// exec
		engine->ClientCmd_Unrestricted(szMapCommand);
		OnClose();
		return;
	}

	BaseClass::OnCommand( command );
}

void CInstantActionGameDialog::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CInstantActionGameDialog::LoadMaps(const char *pszPathID)
{
	FileFindHandle_t findHandle = NULL;

	const char *pszFilename = g_pFullFileSystem->FindFirst( "maps/*.bsp", &findHandle );
	while ( pszFilename )
	{
		char mapname[256];

		// FindFirst ignores the pszPathID, so check it here
		// TODO: this doesn't find maps in fallback dirs
		Q_snprintf( mapname, sizeof(mapname), "maps/%s", pszFilename );
		if ( !g_pFullFileSystem->FileExists( mapname, pszPathID ) )
		{
			goto nextFile;
		}

		// remove the text 'maps/' and '.bsp' from the file name to get the map name
		
		const char *str = Q_strstr( pszFilename, "maps" );
		if ( str )
		{
			Q_strncpy( mapname, str + 5, sizeof(mapname) - 1 );	// maps + \\ = 5
		}
		else
		{
			Q_strncpy( mapname, pszFilename, sizeof(mapname) - 1 );
		}
		char *ext = Q_strstr( mapname, ".bsp" );
		if ( ext )
		{
			*ext = 0;
		}

		// add to the map list
		m_pMapList->AddItem( mapname, new KeyValues( "data", "mapname", mapname ) );

		// get the next file
	nextFile:
		pszFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );
}



//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CInstantActionGameDialog::LoadMapList()
{
	// clear the current list (if any)
	m_pMapList->DeleteAllItems();

	// add special "name" to represent loading a randomly selected map
	m_pMapList->AddItem( RANDOM_MAP, new KeyValues( "data", "mapname", RANDOM_MAP ) );

	// iterate the filesystem getting the list of all the files
	// UNDONE: steam wants this done in a special way, need to support that
	const char *pathID = "MOD";

	// Load the GameDir maps
	LoadMaps( pathID ); 

	// set the first item to be selected
	m_pMapList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CInstantActionGameDialog::IsRandomMapSelected()
{
	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!stricmp( mapname, RANDOM_MAP ))
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CInstantActionGameDialog::GetMapName()
{
	int count = m_pMapList->GetItemCount();

	// if there is only one entry it's the special "select random map" entry
	if( count <= 1 )
		return NULL;

	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!strcmp( mapname, RANDOM_MAP ))
	{
		int which = RandomInt( 1, count - 1 );
		mapname = m_pMapList->GetItemUserData( which )->GetString("mapname");
	}

	return mapname;
}

//-----------------------------------------------------------------------------
// Purpose: Sets currently selected map in the map combobox
//-----------------------------------------------------------------------------
void CInstantActionGameDialog::SetMap(const char *mapName)
{
	for (int i = 0; i < m_pMapList->GetItemCount(); i++)
	{
		if (!m_pMapList->IsItemIDValid(i))
			continue;

		if (!stricmp(m_pMapList->GetItemUserData(i)->GetString("mapname"), mapName))
		{
			m_pMapList->ActivateItem(i);
			break;
		}
	}
}

void CInstantActionGameDialog::GetOptions()
{
	const char *startMap = m_pSavedData->GetString("map", "");
	if (startMap[0])
	{
		SetMap(startMap);
	}

	const char *startMax = m_pSavedData->GetString("maxBots", "");
	if (startMax[0])
	{
		m_maxBots->SetText(startMax);
	}

	bool startTeams = m_pSavedData->GetBool("teamplay", true);
	if (startTeams)
	{
		m_pTeamplay->SetSelected(1);
	}
	else
	{
		m_pTeamplay->SetSelected(0);
	}

	const char *startTime = m_pSavedData->GetString("timelimit", "");
	if (startTime[0])
	{
		m_pTimeLimit->SetText(startTime);
	}

	bool startRand = m_pSavedData->GetBool("randomizer", false);
	if (startRand)
	{
		m_pRandomizer->SetSelected(1);
	}
	else
	{
		m_pRandomizer->SetSelected(0);
	}
}
