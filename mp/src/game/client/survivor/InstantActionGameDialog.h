//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INSTANTACTIONGAMEDIALOG_H
#define INSTANTACTIONGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CInstantActionGameDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CInstantActionGameDialog,  vgui::Frame );

public:
	CInstantActionGameDialog(vgui::Panel *parent);
	~CInstantActionGameDialog();
	
	// returns currently entered information about the server
	void SetMap(const char *name);
	void GetOptions();
	bool IsRandomMapSelected();
	const char *GetMapName();

private:
	virtual void OnCommand( const char *command );
	virtual void OnClose();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	
	void LoadMapList();
	void LoadMaps( const char *pszPathID );

	vgui::ComboBox *m_pMapList;
	vgui::TextEntry *m_maxBots;
	vgui::CheckButton *m_pTeamplay;
	vgui::TextEntry *m_pTimeLimit;
	vgui::CheckButton *m_pRandomizer;

	// for loading/saving game config
	KeyValues *m_pSavedData;
};


#endif
