#ifndef GAME_DATA_COMPOUNDTAG_HPP
#define GAME_DATA_COMPOUNDTAG_HPP

#include <iosfwd>
#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInput;
class DataOutput;
class ListTag;

class CompoundTag : public Tag
{
public:
    CompoundTag();

    virtual void write(DataOutput* output);
    virtual void load(DataInput* input, int depth);
    virtual std::wstring toString();
    virtual unsigned char getId();
    virtual void print(const std::wstring& name, char* indent, std::wostream& out);
    virtual ~CompoundTag();
    virtual bool equals(Tag* other);
    virtual CompoundTag* copy();
    virtual void stripEmptyChildren();
    virtual bool isEmpty() const;

    static void writeNamedTag(const std::wstring& name, Tag* tag, DataOutput* output);
    static unsigned char readNamedTagType(DataInput* input);
    static std::wstring readNamedTagName(DataInput* input);
    static Tag* readNamedTagData(unsigned char type, const std::wstring& name, DataInput* input, int depth);

    unsigned char getTagType(const std::wstring& name) const;
    bool contains(const std::wstring& name, int type);
    bool contains(const std::wstring& name);

    Tag* get(const std::wstring& name);
    void put(const std::wstring& name, Tag* tag);
    void putByte(const std::wstring& name, unsigned char value);
    void putShort(const std::wstring& name, short value);
    void putInt(const std::wstring& name, int value);
    void putLong(const std::wstring& name, long long value);
    void putFloat(const std::wstring& name, float value);
    void putDouble(const std::wstring& name, double value);
    void putString(const std::wstring& name, const std::wstring& value);
    void putByteArray(const std::wstring& name, arrayWithLength<unsigned char> value);
    void putIntArray(const std::wstring& name, arrayWithLength<int> value);
    void putBoolean(const std::wstring& name, bool value);

    unsigned char getByte(const std::wstring& name);
    short getShort(const std::wstring& name);
    int getInt(const std::wstring& name);
    long long getLong(const std::wstring& name);
    float getFloat(const std::wstring& name);
    double getDouble(const std::wstring& name);
    std::wstring getString(const std::wstring& name);
    arrayWithLength<unsigned char> getByteArray(const std::wstring& name);
    arrayWithLength<int> getIntArray(const std::wstring& name);
    CompoundTag* getCompound(const std::wstring& name);
    ListTag* getList(const std::wstring& name, int type);
    bool getBoolean(const std::wstring& name);

    CompoundTag* fjSafeGetCompoundTag(const std::wstring& name);
    ListTag* fjSafeGetListTag(const std::wstring& name, int type) const;

    void remove(const std::wstring& name);
    int size();
    std::vector<std::wstring>* getAllKeys();
    std::vector<Tag*>* getAllTags();
    boost::unordered_map<std::wstring, Tag*>* getAllTagsMap();

    bool equalsWithException(Tag* other, std::vector<std::wstring>* exceptions);
    int GetNumValidTagsAfterException(std::vector<std::wstring>* exceptions);
    void merge(CompoundTag* other);

private:
    boost::unordered_map<std::wstring, Tag*> mTags;
};

#endif
