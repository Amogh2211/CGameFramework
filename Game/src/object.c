#include "baseTypes.h"
#include "object.h"

static ObjRegistrationFunc _registerFunc = NULL;
static ObjRegistrationFunc _deregisterFunc = NULL;

/// @brief Enable callback to a registrar on ObjInit/Deinit
/// @param registerFunc 
/// @param deregisterFunc 
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc)
{
    _registerFunc = registerFunc;
    _deregisterFunc = deregisterFunc;
}

/// @brief Disable registration during ObjInit/Deinit
void objDisableRegistration()
{
    _registerFunc = _deregisterFunc = NULL;
}

/// @brief Initialize an object. Intended to be called from subclass constructors
/// @param obj 
/// @param vtable 
/// @param pos 
/// @param vel 
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, Coord2D vel)
{
    obj->vtable = vtable;
    obj->position = pos;
    obj->velocity = vel;
    obj->nextUpdate = (uint32_t)FRAME_TIME_MS;
    if (_registerFunc != NULL)
    {
        _registerFunc(obj);
    }
}

/// @brief Deinitialize an object
/// @param obj 
void objDeinit(Object* obj)
{
    if (_deregisterFunc != NULL)
    {
        _deregisterFunc(obj);
    }
}

/// @brief Draw this object, using it's vtable
/// @param obj 
void objDraw(Object* obj)
{
    if (obj->vtable != NULL && obj->vtable->draw != NULL) 
    {
        obj->vtable->draw(obj);
    }
}

/// @brief Update this object, using it's vtable
/// @param obj 
/// @param milliseconds 
void objUpdate(Object* obj, uint32_t milliseconds)
{
    if (obj->vtable != NULL && obj->vtable->update != NULL) 
    {
        obj->vtable->update(obj, milliseconds);
        return;
    }

    objDefaultUpdate(obj, milliseconds);
}

void objFixedUpdate(Object* obj, uint32_t milliseconds)
{

    //// Check if the object has to be updated
    //if (obj->nextUpdate > milliseconds)
    //{
    //    obj->nextUpdate -= milliseconds;
    //    return;
    //}

    // if object has to be updated, call that function
    if (obj->vtable != NULL && obj->vtable->fixedUpdate != NULL)
    {
        obj->vtable->fixedUpdate(obj, milliseconds);
    }
}

void objDefaultUpdate(Object* obj, uint32_t milliseconds)
{
    obj->position.x += obj->velocity.x;
    obj->position.y += obj->velocity.y;
}