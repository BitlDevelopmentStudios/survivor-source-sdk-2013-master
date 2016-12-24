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
// implementation of CHudScoreRed class
//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
//#include "c_baseplayer.h"
#include "c_team.h"
#include "hl2mp_gamerules.h"
#include <igameresources.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_SCORE	-1

//-----------------------------------------------------------------------------
// Purpose: Displays suit power (armor) on hud
//-----------------------------------------------------------------------------
class CHudScoreRed : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScoreRed, CHudNumericDisplay);

public:
	CHudScoreRed(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iScore;
};

DECLARE_HUDELEMENT(CHudScoreRed);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudScoreRed::CHudScoreRed(const char *pElementName) : BaseClass(NULL, "HudScoreRed"), CHudElement(pElementName)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreRed::Init(void)
{
	Reset();
	m_iScore = INIT_SCORE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreRed::Reset(void)
{
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_RedScore");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"RED SCORE");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreRed::VidInit(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreRed::OnThink(void)
{
	SetVisible(false);

	if (HL2MPRules()->IsTeamplay() == false)
		return;

	Color c = GameResources()->GetTeamColor(TEAM_RED);
	C_Team *pTeam = GetGlobalTeam(TEAM_RED);

	if (pTeam)
	{
		m_iScore = pTeam->Get_Score();
		SetFgColor(c);
		SetDisplayValue(m_iScore);
		SetVisible(true);
	}
}