#ifndef GC_H
#define GC_H

#include "Object.h"
#include "Array.h"

#define GC_INITIAL_SIZE   20

struct Gc
{
    struct Array *objects;
    Integer newCount;
    Integer freeCount;
};

struct Gc *gcNew();
void gcRegisterObject(struct Gc *gc, struct Object *object);
void gcSweep(struct Gc *gc);
void gcFree(struct Gc *gc);

#endif