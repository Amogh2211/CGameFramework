#pragma once
#include <stdbool.h>
#include "baseTypes.h"
#include "Object.h"

#define MAX_BATTLE_MESSAGES 32

typedef struct BattleMessage
{
	char* text;
	float displayTime; // seconds (0 = wait for input)
	bool waitForInput;
	void (*onFinish)(void* userData);
	void* userData;
} BattleMessage;

typedef struct battleMessageQueue_t
{
	BattleMessage messages[MAX_BATTLE_MESSAGES];
	int head;  // points to first valid message
	int tail;  // points to next slot to insert
	float currentTimer;
	bool isActive;
} BattleMessageQueue;


void initBattleMessage(Object* battleMessageQueue);

void enqueueBattleMessage(
	BattleMessageQueue* queue,
	const char* text,
	float displayTime,
	bool waitForInput
);


