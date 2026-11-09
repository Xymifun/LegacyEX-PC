#include <string.h>

#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/Entity.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/Vec3.hpp"

Path::Path(arrayWithLength<Node*> nodes)
{
    target = NULL;
    index = 0;
    size = nodes.length;

    this->nodes = arrayWithLength<Node*>(nodes.length, true);

    for (int i = 0; i < size; i++)
    {
        this->nodes[i] = new Node();
        memcpy(this->nodes[i], nodes[i], sizeof(Node));
    }
}

Path::Path(const Path& other)
    : nodes(other.nodes),
      openSet(other.openSet),
      closedSet(other.closedSet),
      target(other.target),
      index(other.index),
      size(other.size)
{
}

Path::~Path()
{
    for (unsigned int i = 0; i < nodes.length; i++)
    {
        delete nodes.data[i];
    }

    delete[] nodes.data;

    for (unsigned int i = 0; i < openSet.length; i++)
    {
        delete openSet.data[i];
    }

    delete[] openSet.data;

    for (unsigned int i = 0; i < closedSet.length; i++)
    {
        delete closedSet.data[i];
    }

    delete[] closedSet.data;
}

Path* Path::createFromStream(DataInputStream* stream)
{
    int index = stream->readInt();

    Node* target = NULL;

    if (stream->readBoolean())
    {
        target = Node::createFromStream(stream);
    }

    int nodeCount = stream->readInt();
    arrayWithLength<Node*> nodes;

    if (nodeCount > 0)
    {
        nodes = arrayWithLength<Node*>(nodeCount, true);

        for (unsigned int i = 0; i < nodes.length; i++)
        {
            nodes.data[i] = Node::createFromStream(stream);
        }
    }

    int openSetCount = stream->readInt();
    arrayWithLength<Node*> openSet;

    if (openSetCount > 0)
    {
        openSet = arrayWithLength<Node*>(openSetCount, true);

        for (unsigned int i = 0; i < openSet.length; i++)
        {
            openSet.data[i] = Node::createFromStream(stream);
        }
    }

    int closedSetCount = stream->readInt();
    arrayWithLength<Node*> closedSet;

    if (closedSetCount > 0)
    {
        closedSet = arrayWithLength<Node*>(closedSetCount, true);

        for (unsigned int i = 0; i < closedSet.length; i++)
        {
            closedSet.data[i] = Node::createFromStream(stream);
        }
    }

    Path* path = new Path(nodes);

    path->openSet = openSet;
    path->closedSet = closedSet;
    path->index = index;
    path->target = target;

    delete[] nodes.data;

    return path;
}

Path* Path::clone()
{
    Path* path = new Path(*this);

    arrayWithLength<Node*> copy(nodes.length, true);

    for (unsigned int i = 0; i < nodes.length; i++)
    {
        copy.data[i] = new Node(*nodes.data[i]);
    }

    path->nodes = copy;

    return path;
}

bool Path::sameAs(Path* other)
{
    if (this == other)
    {
        return true;
    }

    if (other == NULL || other->nodes.length != nodes.length)
    {
        return false;
    }

    for (unsigned int i = 0; i < nodes.length; i++)
    {
        if (nodes[i]->x != other->nodes[i]->x || nodes[i]->y != other->nodes[i]->y || nodes[i]->z != other->nodes[i]->z)
        {
            return false;
        }
    }

    return true;
}

int Path::getSize()
{
    return size;
}

void Path::setSize(int size)
{
    this->size = size;
}

int Path::getIndex()
{
    return index;
}

void Path::setIndex(int index)
{
    this->index = index;
}

void Path::next()
{
    index++;
}

bool Path::isDone()
{
    return index >= size;
}

bool Path::IsWithinStepsToTheEnd(int steps)
{
    return index + steps >= size;
}

Node* Path::get(int index)
{
    return nodes.data[index];
}

void Path::set(int index, Node* node)
{
    memcpy(nodes.data[index], node, sizeof(Node));
}

Node* Path::last()
{
    if (size > 0)
    {
        return nodes[size - 1];
    }

    return NULL;
}

Node* Path::getTarget()
{
    return target;
}

Vec3* Path::getPos(boost::shared_ptr<Entity> entity, int index)
{
    double x = nodes.data[index]->x + (int)(entity->mBbWidth + 1.0f) * 0.5;
    double y = nodes.data[index]->y;
    double z = nodes.data[index]->z + (int)(entity->mBbWidth + 1.0f) * 0.5;

    return Vec3::newTemp(x, y, z);
}

Vec3* Path::currentPos(boost::shared_ptr<Entity> entity)
{
    return getPos(entity, index);
}

Vec3* Path::currentPos()
{
    Node* node = nodes.data[index];

    return Vec3::newTemp(node->x, node->y, node->z);
}

bool Path::endsIn(Vec3* pos)
{
    Node* node = last();

    if (node == NULL)
    {
        return false;
    }

    return node->x == (int)pos->x && node->y == (int)pos->y && node->z == (int)pos->z;
}

bool Path::endsInXZ(Vec3* pos)
{
    Node* node = last();

    if (node == NULL)
    {
        return false;
    }

    return node->x == (int)pos->x && node->z == (int)pos->z;
}

void Path::setDebug(arrayWithLength<Node*> openSetValue, arrayWithLength<Node*> closedSetValue, Node* targetValue)
{
    openSet = openSetValue;
    closedSet = closedSetValue;
    target = targetValue;
}

arrayWithLength<Node*> Path::getOpenSet()
{
    return openSet;
}

arrayWithLength<Node*> Path::getClosedSet()
{
    return closedSet;
}

void Path::writeToStream(DataOutputStream* stream)
{
    stream->writeInt(index);
    stream->writeBoolean(target != NULL);

    if (target != NULL)
    {
        target->writeToStream(stream);
    }

    stream->writeInt(nodes.length);

    for (unsigned int i = 0; i < nodes.length; i++)
    {
        nodes.data[i]->writeToStream(stream);
    }

    stream->writeInt(openSet.length);

    for (unsigned int i = 0; i < openSet.length; i++)
    {
        openSet.data[i]->writeToStream(stream);
    }

    stream->writeInt(closedSet.length);

    for (unsigned int i = 0; i < closedSet.length; i++)
    {
        closedSet.data[i]->writeToStream(stream);
    }
}
