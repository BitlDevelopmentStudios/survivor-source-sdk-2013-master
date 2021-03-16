#ifndef Survivor_BOT_H
#define Survivor_BOT_H

#ifdef _WIN32
#pragma once
#endif

#include "bots\bot.h"
#include "ai_hint.h"

//================================================================================
// Artificial intelligence for Survivor
//================================================================================
class CSurvivor_Bot : public CBot
{
public:
    DECLARE_CLASS_GAMEROOT(CSurvivor_Bot, CBot);

    CSurvivor_Bot(CBasePlayer* parent);

public:
    virtual void Spawn();
    virtual void Update();
    virtual void SetUpComponents();
    virtual void SetUpSchedules();
};

extern CPlayer* CreateSurvivorBot(const char* pPlayername, const Vector* vecPosition, const QAngle* angles);
void SpawnSurvivorBots(int botCount);

#endif // Survivor_BOT_H