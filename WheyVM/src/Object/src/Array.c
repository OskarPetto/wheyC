#include "../Array.h"
#include "../Object.h"
#include "../String.h"
#include "../Integer.h"

#include <assert.h>
#include <stdlib.h>

static Integer getNextLargerSlotCount(Integer slotCount)
{
    return slotCount + (slotCount >> 1) + 1;
}

struct Object *arrayNew(struct Gc *gc, Integer initialObjectCount)
{
    Integer slotCount = getNextLargerSlotCount(initialObjectCount);
    struct Object *object = objectNew(gc, OBJECT_TYPE_ARRAY);
    object->value.array->objects = (struct Object **)malloc(slotCount * sizeof(struct Object *));
    assert(object->value.array->objects != NULL);
    object->value.array->slotCount = slotCount;
    object->value.array->objectCount = initialObjectCount;
    return object;
}

Integer arraySize(struct Array *array)
{
    return array->objectCount;
}

struct Object *arrayGet(struct Array *array, Integer index)
{
    return array->objects[index];
}

struct Object *arraySet(struct Array *array, Integer index, struct Object *setObject)
{
    struct Object *currentObject = array->objects[index];
    array->objects[index] = setObject;
    return currentObject;
}

void arrayInsert(struct Array *array, Integer index, struct Object *insertObject)
{
    if (array->objectCount >= array->slotCount)
    {
        array->slotCount = getNextLargerSlotCount(array->objectCount);

        array->objects = (struct Object **)realloc(array->objects,
                                                   array->slotCount * sizeof(struct Object *));

        assert(array->objects != NULL);
    }

    for (Integer i = array->objectCount; i > index; i--)
    {
        array->objects[i] = array->objects[i - 1];
    }

    array->objects[index] = insertObject;

    array->objectCount++;
}

void arrayInsertAll(struct Array *array, Integer index, struct Array *insertArray)
{
    if (insertArray->objectCount == 0)
    {
        return;
    }

    Integer insertObjectCount = insertArray->objectCount;
    struct Object **insertObjects = (struct Object **) malloc(insertObjectCount * sizeof(struct Object *));

    for (Integer i = 0; i < insertObjectCount; i++)
    {
        insertObjects[i] = insertArray->objects[i];
    }

    Integer newObjectCount = array->objectCount + insertObjectCount;

    if (newObjectCount > array->slotCount)
    {
        array->slotCount = getNextLargerSlotCount(newObjectCount);

        array->objects = (struct Object **)realloc(array->objects,
                                                   array->slotCount * sizeof(struct Object *));

        assert(array->objects != NULL);
    }

    for (Integer i = array->objectCount - 1; i >= index; i--)
    {
        array->objects[insertObjectCount + i] = array->objects[i];
    }

    for (Integer i = 0; i < insertObjectCount; i++)
    {
        array->objects[index + i] = insertObjects[i];
    }

    free(insertObjects);

    array->objectCount = newObjectCount;
}

struct Object *arrayRemove(struct Array *array, Integer index)
{
    if (array->objectCount < array->slotCount / 2)
    {
        array->slotCount = getNextLargerSlotCount(array->objectCount);

        array->objects = (struct Object **)realloc(array->objects,
                                                   array->slotCount * sizeof(struct Object *));

        assert(array->objects != NULL);
    }

    struct Object *removeObject = array->objects[index];

    for (Integer i = index; i < array->objectCount - 1; i++)
    {
        array->objects[i] = array->objects[i + 1];
    }

    array->objectCount--;

    return removeObject;
}

void arrayAppendInteger(struct Gc *gc, struct Array *array, Integer character)
{
    struct Object *characterObject = integerNew(gc, (Integer)character);
    arrayInsert(array, array->objectCount, characterObject);
}

struct Object *arrayCopy(struct Gc *gc, struct Array *array)
{
    struct Object *copy = arrayNew(gc, array->objectCount);

    for (Integer i = 0; i < array->objectCount; i++)
    {
        copy->value.array->objects[i] = objectCopy(gc, array->objects[i]);
    }

    return copy;
}

Integer arrayEquals(struct Array *array1, struct Array *array2)
{
    if (array1 == array2)
    {
        return BOOLEAN_TRUE;
    }

    if (array1->objectCount != array2->objectCount)
    {
        return BOOLEAN_FALSE;
    }

    for (Integer i = 0; i < array1->objectCount; i++)
    {
        if (!objectEquals(array1->objects[i], array2->objects[i]))
        {
            return BOOLEAN_FALSE;
        }
    }

    return BOOLEAN_TRUE;
}

Integer arrayHash(struct Array *array)
{
    Integer hash = 1;

    for (Integer i = 0; i < array->objectCount; i++)
    {
        hash = 31 * hash + objectHash(array->objects[i]);
    }

    return hash;
}

struct Object *arrayToString(struct Gc *gc, struct Array *array)
{
    struct Object *arrayObject = arrayNew(gc, 0);
    struct Array *charArray = arrayObject->value.array;

    arrayAppendInteger(gc, charArray, '[');

    for (Integer i = 0; i < array->objectCount; i++)
    {
        struct Object *subString = objectToString(gc, array->objects[i]);
        struct Array *subCharArray = stringToCharArray(gc, subString->value.string)->value.array;
        arrayInsertAll(charArray, charArray->objectCount, subCharArray);
    }

    if (array->objectCount > 0)
    {
        struct Object *lastInteger = integerNew(gc, ']');
        charArray->objects[array->objectCount - 1] = lastInteger;
    }
    else
    {
        arrayAppendInteger(gc, charArray, ']');
    }

    return stringFromCharArray(gc, charArray);
}

void arrayMark(struct Array *array)
{
    for (Integer i = 0; i < array->objectCount; i++)
    {
        objectMark(array->objects[i]);
    }
}

void arrayFree(struct Array *array)
{
    free(array->objects);
}
