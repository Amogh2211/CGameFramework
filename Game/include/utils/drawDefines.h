#pragma once

// Draw depth ranges from -0.99 to 1, 
// 1 being the highest z-height and -0.99 will be the things on the bottom

#define PLAYER_DRAW_DEPTH 0.9f
#define ENEMY_DRAW_DEPTH PLAYER_DRAW_DEPTH
#define UI_DRAW_DEPTH -0.99f