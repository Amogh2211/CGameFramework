#include <Windows.h>
#include <stdlib.h>
#include <gl/GLU.h>
#include <assert.h>
#include "baseTypes.h"
#include "levelmgr.h"
#include "field.h"
#include "ball.h"
#include "face.h"
#include "objmgr.h"
#include "SOIL.h"
#include "sound.h"

typedef struct level_t
{
    const LevelDef* def;

    Field* field;
    Ball** balls;
    Face** faces;
} Level;

static int32_t _soundId = SOUND_NOSOUND;

static void _levelMgrPlaySound(Ball* ball);

/// @brief Initialize the level manager
void levelMgrInit()
{
    faceInitTextures();

    _soundId = soundLoad("asset/beep.wav");
    ballSetCollideCB(_levelMgrPlaySound);
}

/// @brief Shutdown the level manager
void levelMgrShutdown()
{
    ballClearCollideCB();
    soundUnload(_soundId);
}

/// @brief Loads the level and all required objects/assets
/// @param levelDef 
/// @return 
Level* levelMgrLoad(const LevelDef* levelDef)
{
    Level* level = malloc(sizeof(Level));
    if (level != NULL)
    {
        level->def = levelDef;

        // the field provides the boundaries of the scene & encloses the faces & balls
        level->field = fieldNew(levelDef->fieldBounds, levelDef->fieldColor);

        // initialize a bunch of balls to bounce around the scene
        level->balls = malloc(levelDef->numBalls * sizeof(Ball*));
        if (level->balls != NULL)
        {
            for (uint32_t i = 0; i < levelDef->numBalls; ++i)
            {
                level->balls[i] = ballNew(levelDef->fieldBounds);
            }
        }

        // initialize a 2d grid of faces for the background
        level->faces = malloc(levelDef->numFaces * levelDef->numFaces * sizeof(Face*));
        if (level->faces != NULL)
        {
            Coord2D faceSize = boundsGetDimensions(&levelDef->fieldBounds);
            faceSize.x /= levelDef->numFaces;
            faceSize.y /= levelDef->numFaces;

            for (uint32_t y = 0; y < levelDef->numFaces; ++y)
            {
                float yPos = levelDef->fieldBounds.topLeft.y + (y * faceSize.y);

                for (uint32_t x = 0; x < levelDef->numFaces; ++x)
                {
                    float xPos = levelDef->fieldBounds.topLeft.x + (x * faceSize.x);

                    uint32_t faceId = y * levelDef->numFaces + x;
                    Bounds2D faceBounds = {
                        {xPos, yPos},
                        {xPos + faceSize.x, yPos + faceSize.y }
                    };
                    level->faces[faceId] = faceNew(faceBounds);
                }
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
        for (uint32_t i = 0; i < level->def->numFaces * level->def->numFaces; ++i)
        {
            faceDelete(level->faces[i]);
        }
        free(level->faces);

        for (uint32_t i = 0; i < level->def->numBalls; ++i)
        {
            ballDelete(level->balls[i]);
        }
        free(level->balls);

        fieldDelete(level->field);
    }
    free(level);
}

static void _levelMgrPlaySound(Ball* ball)
{
    soundPlay(_soundId);
}
