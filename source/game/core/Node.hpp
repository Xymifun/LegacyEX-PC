#ifndef GAME_CORE_NODE_HPP
#define GAME_CORE_NODE_HPP

#include <vector>
#include <string>

class BlockPathTypes;
class BlockPos;
class DataInputStream;
class DataOutputStream;

struct Node
{
public:
    int x;
    int y;
    int z;

    Node();
    Node(int x, int y, int z);
    Node(const Node& node);

    static void ResetPool();
    static unsigned int createHash(int x, int y, int z);
    void _init();
    static Node* GetNodeFromPool(int x, int y, int z);
    Node* cloneMove(int x, int y, int z);
    float distanceTo(Node* node);
    double distanceToSqr(Node* node);
    float distanceManhattan(Node* node);
    double distanceManhattan(const BlockPos& pos);
    bool equals(Node* node);
    bool inOpenSet();
    void writeToStream(DataOutputStream* stream);
    static Node* createFromStream(DataInputStream* stream);
    const BlockPathTypes* GetType() { return type; }
    int hashCode();
    std::wstring toString();

private:
    unsigned int hash;

public:
    int heapIdx;
    float g;
    float h;
    float f;
    Node* cameFrom;
    bool closed;
    float walkedDistance;
    float cost;
    float costMalus;
    const BlockPathTypes* type;
    bool hasAcceptedNode;
    Node* acceptedNode;

private:
    static unsigned int poolSize;
    static bool usePool;
    static Node pool[1024];
    static std::vector<Node*> allocated;
};

#endif
