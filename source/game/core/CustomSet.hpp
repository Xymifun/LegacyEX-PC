#ifndef GAME_CORE_CUSTOMSET_HPP
#define GAME_CORE_CUSTOMSET_HPP

#include "game/world/chunk/ChunkPos.hpp"

class CustomSet
{
public:
    CustomSet();
    ~CustomSet();

    void clear();
    int end();
    ChunkPos get(int index);
    void insert(const ChunkPos& pos);

private:
    struct Node
    {
        unsigned int mHash;
        ChunkPos mPos;
        Node* mNext;
    };

    Node* find(const ChunkPos& pos);
    void resize();

    Node** mEntries;
    int mCapacity;
    int mCount;
    int mBucketCount;
    Node** mBuckets;
};

#endif
