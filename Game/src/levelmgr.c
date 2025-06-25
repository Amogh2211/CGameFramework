#include <Windows.h>
#include <stdlib.h>
#include <gl/GLU.h>
#include <assert.h>
#include "baseTypes.h"


#include "levelmgr.h"
#include "utils/jsonPaths.h"
#include "field.h"
#include "ball.h"
#include "face.h"
#include "player.h"
#include "objmgr.h"
#include "SOIL.h"
#include "sound.h"

typedef struct level_t
{
    const LevelDef* def;

    Player* player;
    Field* field;
    Ball** enemies;
} Level;

static int32_t _soundId = SOUND_NOSOUND;

static void _levelMgrPlaySound(Ball* ball);

/// @brief Initialize the level manager
void levelMgrInit()
{
    faceInitTextures();

    // inside a sounds.c/h
    // have a player_sound.h
    // have a enemy_sound.h
    // have a level_sound.h
    // have other ui_sounds.h
    _soundId = soundLoad("asset/beep.wav");
    ballSetCollideCB(_levelMgrPlaySound);
}

/// @brief Shutdown the level manager
void levelMgrShutdown()
{
    soundUnload(_soundId);
    ballClearCollideCB();
}

/// @brief Loads the level and all required objects/assets
/// @param levelDef 
/// @return 
Level* levelMgrLoad(const LevelDef* levelDef)
{
    // Have something that would read a json or something to populate player stats and enemy data
    Level* level = malloc(sizeof(Level));
    if (level != NULL)
    {
        level->def = levelDef;

        // the field provides the boundaries of the scene & encloses the faces & balls
        level->field = fieldNew(levelDef->fieldBounds, levelDef->fieldColor);

        Bounds2D bounds = { 100.0f, 100.0f };
        level->player = playerNew(bounds, playerJsonPath);
        // initialize a bunch of balls to bounce around the scene
        level->enemies = malloc(levelDef->numEnemies * sizeof(Ball*));
        if (level->enemies != NULL)
        {
            for (uint32_t i = 0; i < levelDef->numEnemies; ++i)
            {
                level->enemies[i] = ballNew(levelDef->fieldBounds);
            }
        }
    }
    return level;
}

/// @brief Unloads the level and frees up any assets associated
/// @param level 
void levelMgrUnload(Level* level)
{
    if (level != NULL) 
    {
        for (uint32_t i = 0; i < level->def->numPlayers; ++i)
        {
            playerDelete(level->player);
        }
        free(level->player);
        for (uint32_t i = 0; i < level->def->numEnemies; ++i)
        {
            ballDelete(level->enemies[i]);
        }
        free(level->enemies);

        fieldDelete(level->field);
    }
    free(level);
}

static void _levelMgrPlaySound(Ball* ball)
{
    soundPlay(_soundId);
}
