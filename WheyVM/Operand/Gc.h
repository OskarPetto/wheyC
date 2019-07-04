#ifndef GC_H
#define GC_H

#include "Types.h"

struct Object;

struct GcObject;

struct GcObject
{
    struct Object *object;
    struct GcObject *next;
};

struct Gc
{
    struct GcObject *head;
    uint64_t newCount;
    uint64_t freeCount;
    uint32_t size;
    uint32_t maxSize;
    double loadFactor;
    uint8_t outOfMemory;
    uint64_t sizeBeforeLastMarkAndSweep;
    uint64_t allocatedSize;
    uint64_t claimedSize;
};

struct Gc *gcNew(uint32_t maxSize, double loadFactor);
void gcRegisterObject(struct Gc *gc, struct Object *object);
void gcRequestMemory(struct Gc *gc, uint32_t size);
void gcReleaseMemory(struct Gc *gc, uint32_t size);
uint8_t gcShouldMarkAndSweep(struct Gc *gc);
void gcSweep(struct Gc *gc);
void gcFree(struct Gc *gc);

#endif