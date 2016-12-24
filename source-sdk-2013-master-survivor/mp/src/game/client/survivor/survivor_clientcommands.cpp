#include "cbase.h"
#include "game/client/iviewport.h"
#include "hl2mp/hl2mp_gamerules.h"

CON_COMMAND(chooseteam, "Opens a menu for teams")
{
	if (!gViewPortInterface)
		return;
	if (HL2MPRules()->IsTeamplay() == true)
	{
		gViewPortInterface->ShowPanel("team", true);
	}
	else
	{
		gViewPortInterface->ShowPanel("team_ffa", true);
	}
}

CON_COMMAND(chooseclass, "Opens a menu for classes")
{
	if (!gViewPortInterface)
		return;
	gViewPortInterface->ShowPanel("class", true);
}

CON_COMMAND(choosefaction, "Opens a menu for factions")
{
	if (!gViewPortInterface)
		return;
	gViewPortInterface->ShowPanel("faction", true);
}

CON_COMMAND(choosefactionclass, "Opens a menu that allows the player to pick a class and a faction at the same time")
{
	if (!gViewPortInterface)
		return;
	gViewPortInterface->ShowPanel("factionclass", true);
}