//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// battery.cpp
//
// implementation of CHudScoreFFA class
//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
#include "c_baseplayer.h"
#include "c_playerresource.h"
//#include "c_team.h"
#include "hl2mp_gamerules.h"
#include <igameresources.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_SCORE	-1

//-----------------------------------------------------------------------------
// Purpose: Displays suit power (armor) on hud
//-----------------------------------------------------------------------------
class CHudScoreFFA : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScoreFFA, CHudNumericDisplay);

public:
	CHudScoreFFA(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iScore;
};

DECLARE_HUDELEMENT(CHudScoreFFA);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudScoreFFA::CHudScoreFFA(const char *pElementName) : BaseClass(NULL, "HudScoreFFA"), CHudElement(pElementName)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreFFA::Init(void)
{
	Reset();
	m_iScore = INIT_SCORE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreFFA::Reset(void)
{
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_YourScore");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"YOUR SCORE");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreFFA::VidInit(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreFFA::OnThink(void)
{
	SetVisible(false);

	if (HL2MPRules()->IsTeamplay() == true)
		return;
	
	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !g_PR )
		return;

	int localPlayerIndex = pPlayer->entindex();

	bool shouldShow = g_PR->IsConnected(localPlayerIndex);
	
	if ( shouldShow )
	{
		m_iScore = g_PR->GetPlayerScore(localPlayerIndex);
		SetDisplayValue(m_iScore);
		SetVisible(true);
	}
}