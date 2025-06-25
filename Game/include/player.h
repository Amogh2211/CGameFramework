#pragma once
#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct player_t Player;

	typedef struct playerStats_t PlayerStats;

	typedef void (*PlayerCollideCB)(Player*);
	void playerSetCollideCB(PlayerCollideCB cb);
	void playerClearCollideCB();

	Player* playerNew(Bounds2D bounds, const char* jsonPath);
	void playerDelete(Player* player);

#ifdef __cplusplus
}
#endif