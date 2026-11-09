#ifndef GAME_CORE_BINARYHEAP_HPP
#define GAME_CORE_BINARYHEAP_HPP

#include "game/core/Node.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BinaryHeap
{
public:
    BinaryHeap();
    ~BinaryHeap();

    void _init();

    Node* insert(Node* node);
    void clear();
    Node* peek();
    Node* pop();
    void remove(Node* node);
    void changeCost(Node* node, float cost);
    int size();
    bool isEmpty();
    arrayWithLength<Node*> getHeap();

private:
    void upHeap(int index);
    void downHeap(int index);

    arrayWithLength<Node*> mHeap;
    int mSize;
};

#endif
