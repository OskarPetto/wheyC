#ifndef ARRAY_H
#define ARRAY_H

#include "Types.h"

struct Gc;
struct Object;

struct Array
{
    struct Object **objects;
    Integer objectCount;
    Integer slotCount;
};

struct Object *arrayNew(struct Gc *gc, Integer initialObjectCount);
Integer arraySize(struct Array *array);
struct Object *arrayGet(struct Array *array, Integer index);
struct Object *arraySet(struct Array *array, Integer index, struct Object *setObject);
void arrayInsert(struct Array *array, Integer index, struct Object *insertObject);
void arrayInsertAll(struct Array *array, Integer index, struct Array *insertArray);
struct Object *arrayRemove(struct Array *array, Integer index);
void arrayAppendInteger(struct Gc *gc, struct Array *array, Integer integer);

struct Object *arrayCopy(struct Gc *gc, struct Array *array);
Integer arrayEquals(struct Array *array1, struct Array *array2);
Integer arrayHash(struct Array *array);
struct Object *arrayToString(struct Gc *gc, struct Array *array);

void arrayMark(struct Array *array);
void arrayFree(struct Array *array);

#endif