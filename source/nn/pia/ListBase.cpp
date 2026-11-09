#include "nn/pia/ListBase.hpp"

namespace nn
{
namespace pia
{
namespace common
{


void ListBase::Init()
{
    mSentinel.prev = &mSentinel;
    mSentinel.next = &mSentinel;
    mSize = 0;
}

void ListBase::InsertBeforeNode(ListNode* node, ListNode* newNode)
{
    ListNode* prev = node->prev;
    newNode->next = node;
    newNode->prev = prev;
    node->prev->next = newNode;
    node->prev = newNode;
    ++mSize;
}

void ListBase::InsertAfterNode(ListNode* node, ListNode* newNode)
{
    newNode->prev = node;
    newNode->next = node->next;
    node->next->prev = newNode;
    node->next = newNode;
    ++mSize;
}

void ListBase::EraseNode(ListNode* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = 0;
    node->next = 0;
    --mSize;
}

bool ListBase::IsIncludeNode(const ListNode* node) const
{
    if (node->prev == 0 || node->next == 0)
        return false;
    for (const ListNode* current = mSentinel.next; current != &mSentinel; current = current->next)
    {
        if (current == node)
            return true;
    }
    return false;
}

ListNode* ListBase::PopFrontNode()
{
    if (mSize == 0)
        return 0;
    ListNode* node = mSentinel.next;
    EraseNode(node);
    return node;
}

ListNode* ListBase::PopBackNode()
{
    if (mSize == 0)
        return 0;
    ListNode* node = mSentinel.prev;
    EraseNode(node);
    return node;
}

}
}
}
