#include <stdlib.h>
#include "stdbool.h"
#include <stdio.h>
#include <assert.h>
#include "string.h"
#include "baseTypes.h"
#include <gl/GLU.h>
#include "SOIL.h"

#include "utils/cJSON.h"
#include "utils/utils.h"
#include "utils/drawDefines.h"
#include "Object.h"
#include "input.h"

#include "player.h"

#define MAX_SPRITESHEETS 10
#define MAX_DIRECTIONS 8

typedef struct playerStats_t
{
	int32_t health;
	int32_t attack;
	int32_t defense;
	int32_t speed;
} PlayerStats;


typedef struct frameDims_t
{
	float width;
	float height;
} FrameDims;

typedef struct {
	char* name;      // e.g., "north"
	int yOffset;
} SpriteDirection;

typedef struct {
	char* name;
	char* spriteSheetPath; // for each state
	SpriteDirection directions[MAX_DIRECTIONS];
	int numDirections;

	int frameWidth;
	int frameHeight;
	GLuint textureHandle; // store OpenGL texture handle 
	int textureWidth;
	int textureHeight;
	int numFramesPerRow;
} SpriteSheet;

typedef struct AnimationState_t
{
	int currentFrame;    // which frame in the row
	float frameTimer;    // time accumulator
	float frameDuration; // time per frame (1 / fps)
} AnimationState;
void updateAnimation(AnimationState* animationState, int maxFrames, uint32_t deltaTime);


typedef struct player_t
{
	Object obj;

	SpriteSheet spriteSheets[MAX_SPRITESHEETS];
	int numSpriteSheets;

	AnimationState animState;
	int currDir;
	int currState;

	PlayerStats* stats;
} Player;

// spriteData
static uint32_t _playerSpriteTexture = 0;

#define NUM_FRAMES_PER_STATE 8;

// player states
typedef enum playerStates_t
{
	STATE_IDLE,
	STATE_WALK,
	STATE_ALIVE,
	STATE_FAINT,
	STATE_COUNT
} State;

typedef enum playerDirections
{
	DIR_SOUTH,
	DIR_SOUTH_EAST,
	DIR_EAST,
	DIR_NORTH_EAST,
	DIR_NORTH,
	DIR_NORTH_WEST,
	DIR_WEST,
	DIR_SOUTH_WEST,
	DIR_COUNT
} Direction;

// Default to idle in the start
int currentState = STATE_IDLE;
int currentFrame = 0;
// default to south in the start
int currentDirection = DIR_SOUTH;


// player related callbacks
static PlayerCollideCB _playerCollideCB = NULL;

// player update and draw pre-defs
static void _playerUpdate(Object* obj, uint32_t milliseconds);
static void _playerDraw(Object* obj);
static void _playerFixedUpdate(Object* obj, uint32_t milliseconds);
static ObjVtable _playerVtable = {
	_playerDraw,
	_playerUpdate,
	_playerFixedUpdate
};

// player class private functions


// load player data into memory

/// @brief create a player based on what data is given in the json
/// @param jsonData 
/// @return 
Player* createPlayerWithData(const char* jsonData);
bool initPlayerTextures(Player* player);

void playerSetCollideCB(PlayerCollideCB cb)
{
	_playerCollideCB = cb;
}

void playerClearCollideCB()
{
	_playerCollideCB = NULL;
}

/// @brief Constructor
/// @param bounds 
/// @param jsonPath 
/// @return 
Player* playerNew(Bounds2D bounds, const char* jsonPath)
{
	const char* jsonData = readFileIntoString(jsonPath);

	Player* player = createPlayerWithData(jsonData);
	assert(player);
	if (!player)
		return NULL;

	// update the direction and state of player
	player->currDir = DIR_SOUTH;
	player->currState = STATE_IDLE;

	// also initialize textures for the player
	initPlayerTextures(player);
	Coord2D pos = boundsGetCenter(&bounds);
	Coord2D vel = { 0,0 };
	objInit(&player->obj, &_playerVtable, pos, vel);
	return player;
}

void playerDelete(Player* player)
{
	objDeinit(&player->obj);
	free(player->stats);
}

void updateAnimation(AnimationState* animationState, int maxFrames, uint32_t deltaTime)
{
	animationState->frameTimer += deltaTime;

	if (animationState->frameTimer >= animationState->frameDuration)
	{
		animationState->currentFrame += 1;
		animationState->frameTimer -= animationState->frameDuration;

		if (animationState->currentFrame >= maxFrames)
		{
			animationState->currentFrame = 0; // loop
		}
	}
}
/// @brief for all visual things/ inputs are not to be polled here
/// @param obj 
/// @param milliseconds 
void _playerUpdate(Object* obj, uint32_t milliseconds)
{
	// Cast down to player
	Player* player = (Player*)obj;
	// handle input callbacks, any player component updates and so on
	// update direction and state here, along with animation
#pragma region
	// Update animation logic
	SpriteSheet* sheet = &player->spriteSheets[player->currState];
	int framesInRow = sheet->numFramesPerRow;

	updateAnimation(&player->animState, framesInRow, milliseconds);
#pragma endregion
	// At this point, the states have to be updated based on some frame time so that annoying input polling doesn't occur
	// however that is being done based on frame times for the animation
}

void _playerDraw(Object* obj)
{
	Player* player = (Player*)obj; // cast to Player

	// Pick which sheet & direction to use:
	SpriteSheet* sheet = &player->spriteSheets[currentState];
	SpriteDirection* dir = &sheet->directions[currentDirection];

	// Get texture handle (however you manage textures)
	// If you're using SOIL or stb_image, you'd load the texture once and store GLuint handle
	GLuint textureHandle = sheet->textureHandle;

	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBegin(GL_TRIANGLE_STRIP);


	// calculate the bounding box
	GLfloat xPositionLeft = (obj->position.x - sheet->frameWidth / 2);
	GLfloat xPositionRight = (obj->position.x + sheet->frameWidth / 2);
	GLfloat yPositionTop = (obj->position.y - sheet->frameHeight / 2);
	GLfloat yPositionBottom = (obj->position.y + sheet->frameHeight / 2);

	// calculate UVs
	GLfloat uPerFrame = (GLfloat)sheet->frameWidth / (GLfloat)sheet->textureWidth;
	GLfloat vPerRow = (GLfloat)sheet->frameHeight / (GLfloat)sheet->textureHeight;

	// assume you want frame 0 in this row:
	GLfloat frameU = (GLfloat)(player->animState.currentFrame * uPerFrame);
	GLfloat frameV = (GLfloat)(player->currDir * vPerRow);

	// draw quad
	glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

	// TL
	glTexCoord2f(frameU, frameV);
	glVertex3f(xPositionLeft, yPositionTop, PLAYER_DRAW_DEPTH);

	// BL
	glTexCoord2f(frameU, frameV + vPerRow);
	glVertex3f(xPositionLeft, yPositionBottom, PLAYER_DRAW_DEPTH);

	// TR
	glTexCoord2f(frameU + uPerFrame, frameV);
	glVertex3f(xPositionRight, yPositionTop, PLAYER_DRAW_DEPTH);

	// BR
	glTexCoord2f(frameU + uPerFrame, frameV + vPerRow);
	glVertex3f(xPositionRight, yPositionBottom, PLAYER_DRAW_DEPTH);

	glEnd();

}


void _playerFixedUpdate(Object* obj, uint32_t milliseconds)
{
	Player* player = (Player*)obj;
#ifdef _DEBUG
	if (inputKeyPressed(VK_SPACE))
	{
		player->currDir = (player->currDir + 1) % DIR_COUNT;
	}
#endif // DEBUG
}

Player* createPlayerWithData(const char* jsonData)
{
	cJSON* root = cJSON_Parse(jsonData);
	//assert(root);
	if (!root) {
		const char* errorPtr = cJSON_GetErrorPtr();
		if (errorPtr != NULL) {
			printf("JSON parse error before: %s\n", errorPtr);
		}
		else {
			printf("Unknown JSON parse error\n");
		}
		return NULL;
	}

	Player* player = (Player*)malloc(sizeof(Player));
	assert(player);
	if (!player) return NULL;
	memset(player, 0, sizeof(Player));

	// Parse stats
	cJSON* stats = cJSON_GetObjectItem(root, "stats");
	player->stats = (PlayerStats*)malloc(sizeof(PlayerStats));
	if (!player->stats) return NULL;

	// Load stats
	player->stats->health = cJSON_GetObjectItem(stats, "health")->valueint;
	player->stats->attack = cJSON_GetObjectItem(stats, "attack")->valueint;
	player->stats->defense = cJSON_GetObjectItem(stats, "defense")->valueint;
	player->stats->speed = cJSON_GetObjectItem(stats, "speed")->valueint;

	// Parse spritesheets
	cJSON* sheets = cJSON_GetObjectItem(root, "spritesheets");
	int sheetCount = cJSON_GetArraySize(sheets);
	player->numSpriteSheets = sheetCount;

	for (int i = 0; i < sheetCount && i < MAX_SPRITESHEETS; i++) {
		cJSON* sheetItem = cJSON_GetArrayItem(sheets, i);

		SpriteSheet* ss = &player->spriteSheets[i];
		cJSON* name = cJSON_GetObjectItem(sheetItem, "name");
		cJSON* spriteSheet = cJSON_GetObjectItem(sheetItem, "spriteSheetPath");
		cJSON* frameWidth = cJSON_GetObjectItem(sheetItem, "frameWidth");
		cJSON* frameHeight = cJSON_GetObjectItem(sheetItem, "frameHeight");
		cJSON* textureWidth = cJSON_GetObjectItem(sheetItem, "textureWidth");
		cJSON* textureHeight = cJSON_GetObjectItem(sheetItem, "textureHeight");
		cJSON* frameDuration = cJSON_GetObjectItem(sheetItem, "frameDuration");

		ss->name = _strdup(name->valuestring);
		ss->spriteSheetPath = _strdup(spriteSheet->valuestring);
		ss->frameWidth = frameWidth->valueint;
		ss->frameHeight = frameHeight->valueint;
		ss->textureWidth = textureWidth->valueint;
		ss->textureHeight = textureHeight->valueint;
		ss->numFramesPerRow = ss->textureWidth / ss->frameWidth;
		player->animState.frameDuration = (float)frameDuration->valuedouble;

		cJSON* dirs = cJSON_GetObjectItem(sheetItem, "directions");
		int dirCount = cJSON_GetArraySize(dirs);
		ss->numDirections = dirCount;

		for (int j = 0; j < dirCount && j < MAX_DIRECTIONS; j++) {
			cJSON* dirItem = cJSON_GetArrayItem(dirs, j);

			cJSON* dirName = cJSON_GetObjectItem(dirItem, "name");
			cJSON* y = cJSON_GetObjectItem(dirItem, "yOffset");

			ss->directions[j].name = _strdup(dirName->valuestring);
			ss->directions[j].yOffset = y->valueint;
		}
	}

	cJSON_Delete(root);
	return player;
}

bool initPlayerTextures(Player* player)
{
	for (int i = 0; i < player->numSpriteSheets; i++)
	{
		SpriteSheet* sheet = &player->spriteSheets[i];

		if (sheet->textureHandle == 0)
		{
			sheet->textureHandle = SOIL_load_OGL_texture(
				sheet->spriteSheetPath,
				SOIL_LOAD_AUTO,
				SOIL_CREATE_NEW_ID,
				SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

			assert(sheet->textureHandle != 0);

			if (sheet->textureHandle == 0)
			{
				return false;  // Stop if failed
			}
		}
	}

	return true;
}

