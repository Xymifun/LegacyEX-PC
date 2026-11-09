#include "game/core/BinaryHeap.hpp"

#include "game/core/Node.hpp"
#include "game/core/System.hpp"

void BinaryHeap::_init()
{
    mHeap = arrayWithLength<Node*>(256, true);
    mSize = 0;
}

BinaryHeap::BinaryHeap()
{
    _init();
}

BinaryHeap::~BinaryHeap()
{
    delete[] mHeap.data;
}

void BinaryHeap::upHeap(int index)
{
    Node** heap = mHeap.data;
    Node* node = heap[index];
    float cost = node->f;

    while (index > 0)
    {
        int parent = (index - 1) >> 1;
        Node* parentNode = heap[parent];

        if (cost >= parentNode->f)
        {
            break;
        }

        parentNode->heapIdx = index;
        heap[index] = parentNode;
        index = parent;
    }

    heap[index] = node;
    node->heapIdx = index;
}

Node* BinaryHeap::insert(Node* node)
{
    if (mSize == mHeap.length)
    {
        arrayWithLength<Node*> grown(mSize * 2, true);

        System::arraycopy(mHeap, 0, &grown, 0, mSize);

        delete[] mHeap.data;

        mHeap = grown;
    }

    node->heapIdx = mSize;
    mHeap.data[mSize] = node;

    upHeap(mSize++);

    return node;
}

void BinaryHeap::clear()
{
    mSize = 0;
}

void BinaryHeap::downHeap(int index)
{
    Node* node = mHeap.data[index];
    Node** heap = mHeap.data;
    float cost = node->f;
    int left;
    int right;
    int count = mSize;

    while (true)
    {
        left = index * 2 + 1;
        right = index * 2 + 2;

        if (left >= count)
        {
            break;
        }

        Node* leftNode = heap[left];
        float leftCost = leftNode->f;

        Node* rightNode;
        float rightCost;

        if (right >= count)
        {
            rightNode = NULL;
            rightCost = 2139095040.0f;
        }
        else
        {
            rightNode = heap[right];
            rightCost = rightNode->f;
        }

        if (leftCost < rightCost)
        {
            if (leftCost >= cost)
            {
                break;
            }

            leftNode->heapIdx = index;
            heap[index] = leftNode;
            index = left;
        }
        else
        {
            if (rightCost >= cost)
            {
                break;
            }

            rightNode->heapIdx = index;
            heap[index] = rightNode;
            index = right;
        }
    }

    heap[index] = node;
    node->heapIdx = index;
}

Node* BinaryHeap::pop()
{
    Node** heap = mHeap.data;
    Node* node = heap[0];

    heap[0] = heap[--mSize];
    heap[mSize] = NULL;

    if (mSize > 0)
    {
        downHeap(0);
    }

    node->heapIdx = -1;

    return node;
}

void BinaryHeap::changeCost(Node* node, float cost)
{
    float oldCost = node->f;

    node->f = cost;

    if (cost < oldCost)
    {
        upHeap(node->heapIdx);
    }
    else
    {
        downHeap(node->heapIdx);
    }
}

bool BinaryHeap::isEmpty()
{
    return mSize == 0;
}
