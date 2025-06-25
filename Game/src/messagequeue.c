#include <stdlib.h>
#include "stdbool.h"
#include <stdio.h>
#include <assert.h>
#include "string.h"
#include "baseTypes.h"
#include <gl/GLU.h>
#include "SOIL.h"

#include "messagequeue.h"
#include "input.h"


// vtable
void _battleMessageQueueDraw(Object* queue);
void _battleMessageQueueUpdate(Object* queue, uint32_t milliseconds);
void _battleMessageQueueFixedUpdate(Object* obj, uint32_t milliseconds);
static ObjVtable _battleMessageQueueVtable = {
	_battleMessageQueueDraw,
	_battleMessageQueueUpdate,
	_battleMessageQueueFixedUpdate
};

// Draw functions
static void drawBattleMessageUIBox();
// Think about making a full font png with all the letters
static void drawUIText(const char* statement);

void initBattleMessage(Object* obj)
{
	if (!obj)
		return;

	BattleMessageQueue* battleMessageQueue = (BattleMessageQueue*)obj;
	battleMessageQueue->head = 0;
	battleMessageQueue->tail = 0;
	battleMessageQueue->currentTimer = 0.0f;
	battleMessageQueue->isActive = false;

	Coord2D coord = { 0,0 };
	// object params
	obj->position = coord;
	obj->velocity = coord;
	obj->vtable = &_battleMessageQueueVtable;
}
/// @brief 
/// @param queue 
/// @param text 
/// @param displayTime 
/// @param  
void enqueueBattleMessage(BattleMessageQueue* queue, const char* text, float displayTime, bool waitForInput)
{
	if (((queue->tail + 1) % MAX_BATTLE_MESSAGES) == queue->head)
	{
		// queue full
		return;
	}

	BattleMessage* msg = &queue->messages[queue->tail];
	msg->text = _strdup(text); // cross-platform → use strdup or write a wrapper
	msg->displayTime = displayTime;
	msg->waitForInput = waitForInput;
	msg->onFinish = NULL;
	msg->userData = NULL;

	queue->tail = (queue->tail + 1) % MAX_BATTLE_MESSAGES;
	queue->isActive = true;
}
/// @brief 
/// @param queue 
void _battleMessageQueueDraw(Object* obj)
{
    BattleMessageQueue* queue = (BattleMessageQueue*)obj;

    if (!queue->isActive || queue->head == queue->tail)
    {
        return;
    }

    BattleMessage* msg = &queue->messages[queue->head];
    // Draw the text box + message here
    // Example → center of screen
    //drawUITextBox(msg->text, 100, 400); // your own UI draw function
    drawBattleMessageUIBox();
    // Draw text on top of UI Box, can be set with depth as well
    drawUIText(msg->text);
}
/// @brief Outer box for message UI
void drawBattleMessageUIBox()
{
}
/// @brief Actual text for the message UI
/// @param statement 
void drawUIText(const char* statement)
{
    
}
/// @brief 
/// @param obj 
/// @param milliseconds 
void _battleMessageQueueUpdate(Object* obj, uint32_t milliseconds)
{
    if (!obj)
        return;

    BattleMessageQueue* queue = (BattleMessageQueue*)obj;

    if (!queue->isActive || queue->head == queue->tail)
    {
        queue->isActive = false;
        return;
    }

    BattleMessage* msg = &queue->messages[queue->head];

    if (msg->waitForInput)
    {
        // Wait for player keypress (ex: Z or Space)
        if (inputKeyPressed(VK_SPACE))
        {
            // Advance to next message
            free(msg->text);
            queue->head = (queue->head + 1) % MAX_BATTLE_MESSAGES;
        }
    }
    else
    {
        // Timer-based message - easier to work with seconds
        queue->currentTimer += milliseconds / 1000;

        if (queue->currentTimer >= msg->displayTime)
        {
            // Advance
            free(msg->text);
            queue->head = (queue->head + 1) % MAX_BATTLE_MESSAGES;
            queue->currentTimer = 0.0f;
        }
    }

    // If queue is now empty
    if (queue->head == queue->tail)
    {
        queue->isActive = false;
    }
}
/// @brief 
/// @param obj 
/// @param milliseconds 
void _battleMessageQueueFixedUpdate(Object* obj, uint32_t milliseconds)
{
	// Should ideally do nothing, unless I need to move the update into this
}


