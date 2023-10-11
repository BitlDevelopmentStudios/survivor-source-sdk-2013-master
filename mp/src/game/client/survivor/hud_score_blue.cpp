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
// implementation of CHudScoreBlue class
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
class CHudScoreBlue : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScoreBlue, CHudNumericDisplay);

public:
	CHudScoreBlue(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iScore;
};

DECLARE_HUDELEMENT(CHudScoreBlue);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudScoreBlue::CHudScoreBlue(const char *pElementName) : BaseClass(NULL, "HudScoreBlue"), CHudElement(pElementName)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreBlue::Init(void)
{
	Reset();
	m_iScore = INIT_SCORE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreBlue::Reset(void)
{
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_BlueScore");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"BLUE SCORE");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreBlue::VidInit(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScoreBlue::OnThink(void)
{
	SetVisible(false);

	if (HL2MPRules()->IsTeamplay() == false)
		return;

	Color c = GameResources()->GetTeamColor(TEAM_BLUE);
	C_Team *pTeam = GetGlobalTeam(TEAM_BLUE);

	if (pTeam)
	{
		m_iScore = pTeam->Get_Score();
		SetFgColor(c);
		SetDisplayValue(m_iScore);
		SetVisible(true);
	}
}