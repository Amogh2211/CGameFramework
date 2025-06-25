#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief 
typedef struct leveldef_t {
    Bounds2D fieldBounds;
    uint32_t fieldColor;
    uint32_t numEnemies;
    uint32_t numPlayers;
} LevelDef;

typedef struct level_t Level;

void levelMgrInit();
void levelMgrShutdown();
Level *levelMgrLoad(const LevelDef* levelDef);
void levelMgrUnload(Level* level);

#ifdef __cplusplus
}
#endif