#ifndef GAME_DATA_LISTTAG_HPP
#define GAME_DATA_LISTTAG_HPP

#include <iosfwd>
#include <string>
#include <vector>

#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CompoundTag;
class DataInput;
class DataOutput;

class ListTag : public Tag
{
public:
    ListTag(int type = 0);

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual void print(const std::wstring& name, char* indent, std::wostream& out);
    virtual ~ListTag();
    virtual bool equals(Tag* other);
    virtual ListTag* copy();
    virtual void stripEmptyChildren();
    virtual bool isEmpty() const;

    void add(Tag* tag);
    void set(int index, Tag* tag);
    Tag* get(int index) const;
    Tag* remove(int index);
    CompoundTag* getCompound(int index) const;
    int getInt(int index) const;
    arrayWithLength<int> getIntArray(int index) const;
    double getDouble(int index) const;
    float getFloat(int index) const;
    std::wstring getString(int index) const;
    unsigned char getType();

    int size() const;

private:
    std::vector<Tag*> mList;
    unsigned char mType;
};

#endif
