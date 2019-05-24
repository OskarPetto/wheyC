#include "../Map.h"
#include "../Array.h"
#include "../String.h"

#include <stdlib.h>
#include <assert.h>
#include <math.h>

static Integer indexFor(Integer hash, Integer length)
{
    return hash & (length - 1);
}

static Integer improveHash(Integer hash)
{
    return hash ^ (hash >> 16);
}

static Integer nextLargerBucketCount(Integer entryCount)
{
    return (Integer) pow(2, ceil(log((double) entryCount) / log (2.)));
}

static struct Object *mapPutWithHash(struct Map *map, struct Object *key, struct Object *value, Integer hash)
{
    Integer index = indexFor(hash, map->bucketCount);
    struct ListEntry *currEntry = map->buckets[index];
    struct ListEntry *prevEntry = currEntry;

    while (currEntry != NULL && !objectEquals(currEntry->key, key))
    {
        prevEntry = currEntry;
        currEntry = currEntry->next;
    }

    if (currEntry != NULL) // key found
    {
        struct Object *previousValue = currEntry->value;
        currEntry->value = value;
        return previousValue;
    }

    struct ListEntry *newEntry = (struct ListEntry *) malloc(sizeof (struct ListEntry));
    assert(newEntry != NULL);
    newEntry->key = key;
    newEntry->value = value;
    newEntry->hash = hash;
    newEntry->next = NULL;

    if (prevEntry == NULL)
    {
        map->buckets[index] = newEntry;
    }
    else
    {
        prevEntry->next = newEntry;
    }

    map->entryCount++;

    return NULL;
}

static struct Map *mapMaybeResize(struct Map *map, Integer newEntryCount)
{
    if (newEntryCount >= map->bucketCount * 3 / 4)
    { 
        struct Map *newMap = (struct Map *)malloc(sizeof(struct Map));
        assert(newMap != NULL);
        newMap->buckets = (struct ListEntry **) malloc(getNextLargerBucketCount(newEntryCount) * sizeof(struct ListEntry *));
        assert(newMap->buckets != NULL);

        mapPutAll(newMap, map);

        freeMap(map);
    }
}


struct Object *mapNew(struct Gc *gc, Integer initialEntryCount)
{
    Integer initialBucketCount = getNextLargerBucketCount(initialEntryCount);
    struct Object *map = objectNew(gc, OBJECT_TYPE_MAP);
    map->value.map->buckets = (struct ListEntry **) malloc(initialBucketCount * sizeof(struct ListEntry *));
    assert(map->value.map->buckets != NULL);
    map->value.map->bucketCount = initialBucketCount;
    map->value.map->entryCount = 0;
    return map;
}

Integer mapSize(struct Map *map)
{
    return map->entryCount;
}

struct Object *mapGet(struct Map *map, struct Object *key)
{
    Integer improvedHash = improveHash(objectHash(key));

    Integer index = indexFor(improvedHash, map->bucketCount);

    struct ListEntry *currEntry = map->buckets[index];

    while (currEntry != NULL && !objectEquals(currEntry->key, key))
    {
        currEntry = currEntry->next;
    }

    if (currEntry == NULL)
        return NULL;

    return currEntry->value;
}

struct Object *mapPut(struct Map *map, struct Object *key, struct Object *value)
{
    map = mapMaybeResize(map, map->entryCount + 1);

    Integer improvedHash = improveHash(objectHash(key));

    struct Object *prevObject = mapPutWithHash(map, key, value, improvedHash);

    return prevObject;
}

void mapPutAll(struct Map *map, struct Map *mapToPut)
{
    map = mapMaybeResize(map, map->entryCount + mapToPut->entryCount);

    for (Integer i = 0; i < mapToPut->bucketCount; i++)
    {
        struct ListEntry *currEntry = mapToPut->buckets[i];

        while (currEntry != NULL)
        {
            mapPutWithHash(map, currEntry->key, currEntry->value, currEntry->hash);
            currEntry = currEntry->next;
        }

    }
}

Integer mapHasKey(struct Map *map, struct Object *key)
{
    return mapGet(map, key) != NULL;
}

struct Object *mapEntries(struct Gc *gc, struct Map *map)
{
    struct Object *entries = arrayNew(gc, map->entryCount);
    struct Array *entriesArray = entries->value.array;
    Integer entriesAdded = 0;

    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            struct Object *newPair = pairNew(gc, currEntry->key, currEntry->value);

            entriesArray->objects[entriesAdded++] = newPair;
            currEntry = currEntry->next;
        }
    }

    return entries;
}

struct Object *mapCopy(struct Gc *gc, struct Map *map)
{
    struct Map *copy = mapNew(gc, map->entryCount);

    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            mapPutWithHash(map, objectCopy(gc, currEntry->key), objectCopy(gc, currEntry->value), currEntry->hash);
            currEntry = currEntry->next;
        }

    }   

    return copy;
}

Integer mapEquals(struct Map *map1, struct Map *map2)
{
    if (map1->entryCount != map2->entryCount)
    {
        return BOOLEAN_FALSE;
    }

    for (Integer i = 0; i < map1->bucketCount; i++)
    {
        struct ListEntry *currEntry = map1->buckets[i];

        while (currEntry != NULL)
        {
            struct Object *otherValue = mapGet(map2, currEntry->key);

            if (!objectEquals(currEntry->value, otherValue))
            {
                return BOOLEAN_FALSE;
            }

            currEntry = currEntry->next;
        }

    }

    return BOOLEAN_TRUE;   
}

Integer mapHash(struct Map *map)
{
    Integer hash = 1;

    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            hash = 31 * hash + currEntry->hash + objectHash(currEntry->value);
            currEntry = currEntry->next;
        }

    }

    return hash;

}

struct Object *mapToString(struct Gc *gc, struct Map *map)
{
    struct Object *arrayObject = arrayNew(gc, 0);
    struct Array *charArray = arrayObject->value.array;

    arrayAppendInteger(gc, charArray, '{');

    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            struct Object *subStringObject1 = objectString(gc, currEntry->key);
            struct Array *subCharArray1 = stringToCharArray(gc, subStringObject1->value.string)->value.array;
            arrayInsertAll(charArray, charArray->objectCount, subCharArray1);

            arrayAppendInteger(gc, charArray, ':');

            struct Object *subStringObject2 = objectString(gc, currEntry->value);
            struct Array *subCharArray2 = stringToCharArray(gc, subStringObject2->value.string)->value.array;
            arrayInsertAll(charArray, charArray->objectCount, subCharArray1);

            arrayAppendInteger(gc, charArray, ',');

            currEntry = currEntry->next;
        }

    }

    if (map->entryCount > 0)
    {
        struct Object *lastChar = integerNew(gc, '}');
        charArray->objects[charArray->objectCount - 1] = lastChar;
    } 
    else 
    {
        arrayAppendInteger(gc, charArray, '}');
    }

    return stringFromCharArray(gc, charArray);
}

void mapMark(struct Map *map)
{
    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            objectMark(currEntry->key);
            objectMark(currEntry->value);

            currEntry = currEntry->next;
        }

    }
}

void mapFree(struct Map *map)
{
    for (Integer i = 0; i < map->bucketCount; i++)
    {
        struct ListEntry *currEntry = map->buckets[i];

        while (currEntry != NULL)
        {
            struct ListEntry *nextEntry = currEntry->next;

            free(currEntry);

            currEntry = nextEntry;

        }
    }

    free(map->buckets);
}

