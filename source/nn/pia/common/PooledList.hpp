#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_POOLED_LIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_POOLED_LIST_HPP

#include <cstddef>
#include <new>

#include "nn/pia/ListBase.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class OffsetList : public ListBase
{
public:
    OffsetList() : m_offset(-1) {}

    void SetOffset(int offset) { m_offset = offset; }

    ListNode* ToListNode(void* object) const
    {
        return reinterpret_cast<ListNode*>(static_cast<char*>(object) + m_offset);
    }

    void* FromListNode(ListNode* node) const
    {
        if (node == 0)
            return 0;
        return reinterpret_cast<char*>(node) - m_offset;
    }

    void* ObjectOf(ListNode* node) const
    {
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(node) + -m_offset);
    }

    void* Begin() { return ObjectOf(Sentinel()->next); }
    void* End() { return ObjectOf(Sentinel()); }
    void* Next(void* object) const { return ObjectOf(ToListNode(object)->next); }

    class iterator
    {
    public:
        iterator(OffsetList* list, ListNode* node) : m_list(list), m_object(list->ObjectOf(node)) {}
        void* operator*() const { return m_object; }
        iterator& operator++()
        {
            m_object = m_list->ObjectOf(m_list->ToListNode(m_object)->next);
            return *this;
        }
        bool operator==(const iterator& other) const { return m_object == other.m_object; }
        bool operator!=(const iterator& other) const { return m_object != other.m_object; }

    private:
        OffsetList* m_list;
        void* m_object;
    };

    iterator begin() { return iterator(this, Sentinel()->next); }
    iterator end() { return iterator(this, Sentinel()); }

private:
    int m_offset;
};

template <typename T>
class PooledList : public ListBase
{
public:
    struct Node : public ListNode
    {
        T value;
    };

    PooledList() : m_nodes(0), m_nodeNum(0) { m_freeList.SetOffset(0); }

    bool IsInitialized() const { return m_nodes != 0; }
    unsigned int GetCapacity() const { return m_nodeNum; }
    unsigned int GetFreeNum() const { return m_freeList.GetNodeNum(); }

    void Initialize(void* buffer, unsigned int nodeNum)
    {
        if (nodeNum == 0 || buffer == 0)
            return;
        m_nodes = static_cast<Node*>(buffer);
        m_nodeNum = nodeNum;
        for (unsigned int i = 0; i < nodeNum; ++i)
        {
            new (static_cast<ListNode*>(&m_nodes[i])) ListNode;
            m_freeList.InsertBeforeNode(m_freeList.Sentinel(), m_freeList.ToListNode(&m_nodes[i]));
        }
    }

    void Clear()
    {
        FreeAllNodes();
        Init();
    }

    T* Front()
    {
        if (GetNodeNum() == 0)
            return 0;
        ListNode* node = Sentinel()->next;
        if (node == 0)
            return 0;
        return ValueOf(node);
    }

    T* Back()
    {
        if (GetNodeNum() == 0)
            return 0;
        ListNode* node = Sentinel()->prev;
        if (node == 0)
            return 0;
        return ValueOf(node);
    }

    T* Next(T* value)
    {
        ListNode* next = NodeOf(value)->next;
        if (next == Sentinel())
            return 0;
        return ValueOf(next);
    }

    T* Alloc()
    {
        if (m_freeList.GetNodeNum() == 0)
            return 0;
        Node* node = static_cast<Node*>(m_freeList.FromListNode(m_freeList.PopBackNode()));
        new (&node->value) T();
        InsertBeforeNode(Sentinel(), node);
        return ValueOf(node);
    }

    T* AllocFront()
    {
        if (m_freeList.GetNodeNum() == 0)
            return 0;
        Node* node = static_cast<Node*>(m_freeList.FromListNode(m_freeList.PopBackNode()));
        new (&node->value) T();
        InsertAfterNode(Sentinel(), node);
        return ValueOf(node);
    }

    void Free(Node* node)
    {
        EraseNode(node);
        m_freeList.InsertAfterNode(m_freeList.Sentinel(), m_freeList.ToListNode(node));
    }

    void Destroy(T* value)
    {
        Node* node = NodeOf(value);
        value->~T();
        Free(node);
    }

    static Node* NodeOf(T* value)
    {
        return reinterpret_cast<Node*>(reinterpret_cast<char*>(value) - offsetof(Node, value));
    }
    static T* ValueOf(ListNode* node) { return &static_cast<Node*>(node)->value; }
    static const T* ValueOf(const ListNode* node) { return &static_cast<const Node*>(node)->value; }

private:
    void FreeAllNodes()
    {
        ListNode* node = Sentinel()->next;
        ListNode* end = Sentinel();
        while (node != end)
        {
            ListNode* current = node;
            node = node->next;
            current->Reset();
            m_freeList.InsertAfterNode(m_freeList.Sentinel(), m_freeList.ToListNode(current));
        }
    }

    OffsetList m_freeList;
    Node* m_nodes;
    unsigned int m_nodeNum;
};

template <typename T, unsigned int N>
class FixedPooledList : public PooledList<T>
{
public:
    FixedPooledList()
    {
        void* buffer = m_nodeBuffer;
        this->Initialize(buffer, N);
    }

private:
    unsigned char m_nodeBuffer[sizeof(typename PooledList<T>::Node) * N];
};

}
}
}

#endif
