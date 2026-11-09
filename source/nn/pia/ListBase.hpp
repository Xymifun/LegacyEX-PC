#ifndef NN_PIA_COMMON_LISTBASE_HPP
#define NN_PIA_COMMON_LISTBASE_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace common
{


struct ListNode
{
    ListNode() { Reset(); }

    void Reset()
    {
        prev = 0;
        next = 0;
    }

    ListNode* prev;
    ListNode* next;
};

class ListBase
{
public:
    ListBase() { Init(); }
    void Init();
    void InsertBeforeNode(ListNode* node, ListNode* newNode);
    void InsertAfterNode(ListNode* node, ListNode* newNode);
    void EraseNode(ListNode* node);
    bool IsIncludeNode(const ListNode* node) const;
    ListNode* PopFrontNode();
    ListNode* PopBackNode();
    unsigned int GetNodeNum() const { return mSize; }
    ListNode* Sentinel() { return &mSentinel; }
    const ListNode* Sentinel() const { return &mSentinel; }
    void ClearNode();
    void RotateNode(ListNode* node);

private:
    ListNode mSentinel;
    uint32_t mSize;
};

}
}
}

#endif
