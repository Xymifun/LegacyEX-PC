#ifndef GAME_CORE_PATH_HPP
#define GAME_CORE_PATH_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Node.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class Vec3;

class Path
{
public:
    arrayWithLength<Node*> nodes;
    arrayWithLength<Node*> openSet;
    arrayWithLength<Node*> closedSet;
    Node* target;
    int index;
    int size;

    Path(arrayWithLength<Node*> nodes);
    Path(const Path& other);
    ~Path();

    static Path* createFromStream(DataInputStream* stream);

    Path* clone();

    bool sameAs(Path* other);

    int getSize();
    void setSize(int size);

    int getIndex();
    void setIndex(int index);

    void next();
    bool isDone();
    bool IsWithinStepsToTheEnd(int steps);

    Node* get(int index);
    void set(int index, Node* node);

    Node* last();
    Node* getTarget();

    Vec3* getPos(boost::shared_ptr<Entity> entity, int index);
    Vec3* currentPos(boost::shared_ptr<Entity> entity);
    Vec3* currentPos();

    bool endsIn(Vec3* pos);
    bool endsInXZ(Vec3* pos);

    void setDebug(arrayWithLength<Node*> openSet, arrayWithLength<Node*> closedSet, Node* target);
    arrayWithLength<Node*> getOpenSet();
    arrayWithLength<Node*> getClosedSet();

    void writeToStream(DataOutputStream* stream);
};

#endif
