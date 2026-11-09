#include "game/core/CustomSet.hpp"

#include <stdlib.h>
#include <string.h>

static const int HASH_SIZE = 1024;
static const int GROW_SIZE = 512;
static const int COORD_MASK = 31;

CustomSet::CustomSet()
{
    mEntries = NULL;
    mCapacity = 0;
    mCount = 0;
    mBucketCount = HASH_SIZE;

    mBuckets = (Node**)malloc(HASH_SIZE * sizeof(Node*));
    memset(mBuckets, 0, mBucketCount * sizeof(Node*));

    clear();
}

CustomSet::~CustomSet()
{
    for (int i = 0; i < mCapacity; i++)
    {
        free(mEntries[i]);
    }

    free(mEntries);
    free(mBuckets);
}

void CustomSet::clear()
{
    mCount = 0;

    memset(mBuckets, 0, mBucketCount * sizeof(Node*));
}

int CustomSet::end()
{
    return mCount;
}

ChunkPos CustomSet::get(int index)
{
    return mEntries[index]->mPos;
}

CustomSet::Node* CustomSet::find(const ChunkPos& pos)
{
    unsigned int hash = (pos.x & COORD_MASK) | ((pos.z & COORD_MASK) << 5);

    Node* node = mBuckets[hash & (mBucketCount - 1)];

    while (node != NULL && node->mHash != hash)
    {
        node = node->mNext;
    }

    return node;
}

void CustomSet::resize()
{
    int oldCapacity = mCapacity;

    mCapacity = oldCapacity + GROW_SIZE;

    Node** entries;

    if (mEntries != NULL)
    {
        entries = (Node**)realloc(mEntries, mCapacity * sizeof(Node*));
    }
    else
    {
        entries = (Node**)malloc(mCapacity * sizeof(Node*));
    }

    for (int i = 0; i < mCapacity - oldCapacity; i++)
    {
        entries[oldCapacity + i] = (Node*)malloc(sizeof(Node));
    }

    mEntries = entries;
}

void CustomSet::insert(const ChunkPos& pos)
{
    if (find(pos) != NULL)
    {
        return;
    }

    if (mCount >= mCapacity)
    {
        resize();
    }

    Node* node = mEntries[mCount++];

    unsigned int hash = (pos.x & COORD_MASK) | ((pos.z & COORD_MASK) << 5);
    unsigned int index = hash & (mBucketCount - 1);

    node->mHash = hash;
    node->mNext = NULL;
    node->mPos = pos;

    Node* bucket = mBuckets[index];

    if (bucket == NULL)
    {
        mBuckets[index] = node;
        return;
    }

    while (bucket->mNext != NULL)
    {
        bucket = bucket->mNext;
    }

    bucket->mNext = node;
}
