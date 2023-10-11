//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudTimer class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_basetimer.h"
#include "hl2mp/hl2mp_gamerules.h"

#include "convar.h"

#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_TIMER -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudTimer : public CHudElement, public CHudBaseTimer
{
	DECLARE_CLASS_SIMPLE( CHudTimer, CHudBaseTimer );

public:
	CHudTimer( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

private:
	// old variables
};	

DECLARE_HUDELEMENT( CHudTimer );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudTimer::CHudTimer( const char *pElementName ) : CHudElement( pElementName ), CHudBaseTimer(NULL, "HudTimer")
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::Init()
{
	Reset();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerInit");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::Reset()
{
	SetMinutes(INIT_TIMER);
	SetSeconds(INIT_TIMER);
	SetAlpha(255);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::OnThink()
{
	if (HL2MPRules()->GetMapRemainingTime() > 0)
	{
		SetAlpha(255);
		int iRemain = (int)HL2MPRules()->GetMapRemainingTime();
		int iMinutes, iSeconds;
		iMinutes = iRemain / 60;
		iSeconds = iRemain % 60;
		SetMinutes(iMinutes);
		SetSeconds(iSeconds);

		if (iMinutes == 0 && iSeconds < 30)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerBelow30");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerAbove30");
		}
	}
	else
	{
		SetAlpha(0);
	}
}