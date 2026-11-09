#include "game/core/Node.hpp"

#include <cstdlib>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPathTypes.hpp"

unsigned int Node::poolSize = 0;
bool Node::usePool = true;
Node Node::pool[1024];
std::vector<Node*> Node::allocated;

void Node::ResetPool()
{
    poolSize = 0;
}

unsigned int Node::createHash(int x, int y, int z)
{
    return (y & 0xff) | ((x & 0x7fff) << 8) | ((z & 0x7fff) << 24) | (x < 0 ? 0x80000000 : 0) | (z < 0 ? 0x8000 : 0);
}

void Node::_init()
{
    heapIdx = -1;
    closed = false;
    cameFrom = 0;
    walkedDistance = 0.0f;
    cost = 0.0f;
    costMalus = 0.0f;
    type = BlockPathTypes::BLOCKED;
    hasAcceptedNode = false;
    acceptedNode = 0;
}

Node::Node(int xValue, int yValue, int zValue)
    : x(xValue),
      y(yValue),
      z(zValue),
      hash(createHash(xValue, yValue, zValue))
{
    _init();
}

Node::Node() : x(0), y(0), z(0), hash(0)
{
    _init();
}

Node::Node(const Node& node)
    : x(node.x),
      y(node.y),
      z(node.z),
      hash(node.hash),
      heapIdx(node.heapIdx),
      g(node.g),
      h(node.h),
      f(node.f),
      cameFrom(node.cameFrom),
      closed(node.closed),
      walkedDistance(node.walkedDistance),
      cost(node.cost),
      costMalus(node.costMalus),
      type(node.type),
      hasAcceptedNode(node.hasAcceptedNode),
      acceptedNode(node.acceptedNode)
{
}


Node* Node::GetNodeFromPool(int xValue, int yValue, int zValue)
{
    Node* node = NULL;
    if (usePool)
    {
        if (poolSize < 1024)
        {
            node = &pool[poolSize];
            node->x = xValue;
            node->y = yValue;
            node->z = zValue;
            node->hash = createHash(xValue, yValue, zValue);
            node->_init();
            ++poolSize;
        }
        else if (poolSize - 1024 < allocated.size())
        {
            node = allocated[poolSize - 1024];
            node->x = xValue;
            node->y = yValue;
            node->z = zValue;
            node->hash = createHash(xValue, yValue, zValue);
            node->_init();
            ++poolSize;
        }
        else
        {
            node = new Node(xValue, yValue, zValue);
            allocated.push_back(node);
            node->x = xValue;
            node->y = yValue;
            node->z = zValue;
            node->hash = createHash(xValue, yValue, zValue);
            node->_init();
            ++poolSize;
        }
    }
    else
    {
        node = new Node(xValue, yValue, zValue);
    }
    return node;
}

Node* Node::cloneMove(int xValue, int yValue, int zValue)
{
    Node* node = GetNodeFromPool(xValue, yValue, zValue);
    node->heapIdx = heapIdx;
    node->g = g;
    node->h = h;
    node->f = f;
    node->cameFrom = cameFrom;
    node->closed = closed;
    node->walkedDistance = walkedDistance;
    node->cost = cost;
    node->costMalus = costMalus;
    node->type = type;
    return node;
}

float Node::distanceTo(Node* node)
{
    float dx = static_cast<float>(node->x - x);
    float dy = static_cast<float>(node->y - y);
    float dz = static_cast<float>(node->z - z);
    return Mth::sqrt(dx * dx + dy * dy + dz * dz);
}

float Node::distanceManhattan(Node* node)
{
    float dx = static_cast<float>(std::abs(node->x - x));
    float dy = static_cast<float>(std::abs(node->y - y));
    float dz = static_cast<float>(std::abs(node->z - z));
    return dx + dy + dz;
}

bool Node::equals(Node* node)
{
    return hash == node->hash && x == node->x && y == node->y && z == node->z;
}

bool Node::inOpenSet()
{
    return heapIdx >= 0;
}

void Node::writeToStream(DataOutputStream* stream)
{
    stream->writeInt(x);
    stream->writeInt(y);
    stream->writeInt(z);
    stream->writeFloat(walkedDistance);
    stream->writeFloat(cost);
    stream->writeFloat(costMalus);
    stream->writeBoolean(closed);
    stream->writeInt(type->ordinal());
    stream->writeFloat(f);
}

Node* Node::createFromStream(DataInputStream* stream)
{
    int xValue = stream->readInt();
    int yValue = stream->readInt();
    int zValue = stream->readInt();
    Node* node = GetNodeFromPool(xValue, yValue, zValue);
    node->walkedDistance = stream->readFloat();
    node->cost = stream->readFloat();
    node->costMalus = stream->readFloat();
    node->closed = stream->readBoolean();
    node->type = BlockPathTypes::VALUES[stream->readInt()];
    node->f = stream->readFloat();
    return node;
}
