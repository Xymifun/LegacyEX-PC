#include "game/data/CompoundTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/data/ByteTag.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/LongTag.hpp"
#include "game/data/NumericTag.hpp"
#include "game/data/ShortTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/data/Tag.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/StringFormat.hpp"

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

template <class K, class T>
void deleteElements(const boost::unordered_map<K, T*>& elements);

template <class K, class T>
void deleteElements(const boost::unordered_map<K, T*>& elements)
{
    for (typename boost::unordered_map<K, T*>::const_iterator it = elements.begin(); it != elements.end(); ++it)
    {
        if (it->first != L"Passengers")
        {
            delete it->second;
        }
    }
}

typedef boost::unordered_map<std::wstring, Tag*> TagMap;

static CompoundTag sEmptyCompoundTag;
static ListTag sEmptyListTag;

CompoundTag::CompoundTag() : mTags()
{
}

CompoundTag::~CompoundTag()
{
    deleteElements(mTags);
}

void CompoundTag::write(DataOutput* output)
{
    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        writeNamedTag(it->first, it->second, output);
    }

    output->writeByte(0);
}

void CompoundTag::load(DataInput* input, int depth)
{
    if (depth > 512)
    {
        return;
    }

    deleteElements(mTags);
    mTags.clear();

    for (unsigned char type = readNamedTagType(input); type != 0; type = readNamedTagType(input))
    {
        std::wstring name = readNamedTagName(input);
        Tag* tag = readNamedTagData(type, name, input, depth + 1);
        mTags[name] = tag;
    }
}

std::wstring CompoundTag::toString()
{
    return formatwstr(L"%zd entries", mTags.size());
}

unsigned char CompoundTag::getId()
{
    return 10;
}

void CompoundTag::print(const std::wstring& name, char* indent, std::wostream& out)
{
}

bool CompoundTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    CompoundTag* otherCompound = (CompoundTag*)other;

    if (mTags.size() != otherCompound->mTags.size())
    {
        return false;
    }

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        TagMap::const_iterator found = otherCompound->mTags.find(it->first);

        if (found == otherCompound->mTags.end())
        {
            return false;
        }

        if (!it->second->equals(found->second))
        {
            return false;
        }
    }

    return true;
}

CompoundTag* CompoundTag::copy()
{
    CompoundTag* result = new CompoundTag();

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        result->put(it->first.c_str(), it->second->copy());
    }

    return result;
}

void CompoundTag::stripEmptyChildren()
{
    TagMap::iterator it = mTags.begin();

    while (it != mTags.end())
    {
        Tag* tag = it->second;
        tag->stripEmptyChildren();

        if (tag->isEmpty())
        {
            delete it->second;
            it = mTags.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool CompoundTag::isEmpty() const
{
    return mTags.empty();
}

void CompoundTag::writeNamedTag(const std::wstring& name, Tag* tag, DataOutput* output)
{
    output->writeByte(tag->getId());

    if (tag->getId() != Tag::TAG_END)
    {
        output->writeUTF(name);
        tag->write(output);
    }
}

unsigned char CompoundTag::readNamedTagType(DataInput* input)
{
    return input->readByte();
}

std::wstring CompoundTag::readNamedTagName(DataInput* input)
{
    return input->readUTF();
}

Tag* CompoundTag::readNamedTagData(unsigned char type, const std::wstring& name, DataInput* input, int depth)
{
    Tag* tag = Tag::newTag(type);
    tag->load(input, depth);
    return tag;
}

unsigned char CompoundTag::getTagType(const std::wstring& name) const
{
    TagMap::const_iterator it = mTags.find(name);

    if (it == mTags.end())
    {
        return 0;
    }

    return it->second->getId();
}

bool CompoundTag::contains(const std::wstring& name, int type)
{
    unsigned char tagType = getTagType(name);

    if (tagType == type)
    {
        return true;
    }

    if (type != Tag::TAG_ANY_NUMERIC)
    {
        return false;
    }

    return tagType >= Tag::TAG_BYTE && tagType <= Tag::TAG_DOUBLE;
}

bool CompoundTag::contains(const std::wstring& name)
{
    return mTags.find(name) != mTags.end();
}

Tag* CompoundTag::get(const std::wstring& name)
{
    TagMap::iterator it = mTags.find(name);

    if (it == mTags.end())
    {
        return NULL;
    }

    return it->second;
}

void CompoundTag::put(const std::wstring& name, Tag* tag)
{
    Tag* oldTag = get(name);

    if (oldTag != tag)
    {
        delete oldTag;
    }

    tag->getId();
    mTags[name] = tag;
}

void CompoundTag::putByte(const std::wstring& name, unsigned char value)
{
    delete get(name);
    mTags[name] = new ByteTag(value);
}

void CompoundTag::putShort(const std::wstring& name, short value)
{
    delete get(name);
    mTags[name] = new ShortTag(value);
}

void CompoundTag::putInt(const std::wstring& name, int value)
{
    delete get(name);
    mTags[name] = new IntTag(value);
}

void CompoundTag::putLong(const std::wstring& name, long long value)
{
    delete get(name);
    mTags[name] = new LongTag(value);
}

void CompoundTag::putFloat(const std::wstring& name, float value)
{
    delete get(name);
    mTags[name] = new FloatTag(value);
}

void CompoundTag::putDouble(const std::wstring& name, double value)
{
    delete get(name);
    mTags[name] = new DoubleTag(value);
}

void CompoundTag::putString(const std::wstring& name, const std::wstring& value)
{
    delete get(name);
    mTags[name] = new StringTag(value);
}

void CompoundTag::putByteArray(const std::wstring& name, arrayWithLength<unsigned char> value)
{
    delete get(name);
    mTags[name] = new ByteArrayTag(value, true);
}

void CompoundTag::putIntArray(const std::wstring& name, arrayWithLength<int> value)
{
    delete get(name);
    mTags[name] = new IntArrayTag(value);
}

void CompoundTag::putBoolean(const std::wstring& name, bool value)
{
    putByte(name, value);
}

unsigned char CompoundTag::getByte(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0;
    }

    return ((NumericTag*)mTags[name])->getAsByte();
}

short CompoundTag::getShort(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0;
    }

    return ((NumericTag*)mTags[name])->getAsShort();
}

int CompoundTag::getInt(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0;
    }

    return ((NumericTag*)mTags[name])->getAsInt();
}

long long CompoundTag::getLong(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0;
    }

    return ((NumericTag*)mTags[name])->getAsLong();
}

float CompoundTag::getFloat(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0.0F;
    }

    return ((NumericTag*)mTags[name])->getAsFloat();
}

double CompoundTag::getDouble(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_ANY_NUMERIC))
    {
        return 0.0;
    }

    return ((NumericTag*)mTags[name])->getAsDouble();
}

std::wstring CompoundTag::getString(const std::wstring& name)
{
    if (contains(name, Tag::TAG_STRING))
    {
        return mTags[name]->getAsString();
    }

    return L"";
}

arrayWithLength<unsigned char> CompoundTag::getByteArray(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_BYTE_ARRAY))
    {
        return arrayWithLength<unsigned char>();
    }

    return ((ByteArrayTag*)mTags[name])->getAsByteArray();
}

arrayWithLength<int> CompoundTag::getIntArray(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_INT_ARRAY))
    {
        return arrayWithLength<int>();
    }

    return ((IntArrayTag*)mTags[name])->getAsIntArray();
}

CompoundTag* CompoundTag::getCompound(const std::wstring& name)
{
    if (contains(name, Tag::TAG_COMPOUND))
    {
        return (CompoundTag*)mTags[name];
    }

    return new CompoundTag();
}

ListTag* CompoundTag::getList(const std::wstring& name, int type)
{
    if (getTagType(name) != Tag::TAG_LIST)
    {
        return new ListTag();
    }

    ListTag* list = (ListTag*)mTags[name];

    if (list->isEmpty() || list->getType() == type)
    {
        return list;
    }

    return new ListTag();
}

bool CompoundTag::getBoolean(const std::wstring& name)
{
    return getByte(name) != 0;
}

CompoundTag* CompoundTag::fjSafeGetCompoundTag(const std::wstring& name)
{
    if (!contains(name, Tag::TAG_COMPOUND))
    {
        return &sEmptyCompoundTag;
    }

    return (CompoundTag*)mTags.at(name);
}

ListTag* CompoundTag::fjSafeGetListTag(const std::wstring& name, int type) const
{
    if (getTagType(name) != Tag::TAG_LIST)
    {
        return &sEmptyListTag;
    }

    ListTag* list = (ListTag*)mTags.at(name);

    if (list->isEmpty() || list->getType() == type)
    {
        return list;
    }

    return &sEmptyListTag;
}

void CompoundTag::remove(const std::wstring& name)
{
    TagMap::iterator it = mTags.find(name);

    if (it != mTags.end())
    {
        mTags.erase(it);
    }
}

int CompoundTag::size()
{
    return mTags.size();
}

std::vector<std::wstring>* CompoundTag::getAllKeys()
{
    std::vector<std::wstring>* keys = new std::vector<std::wstring>();

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        keys->push_back(it->first);
    }

    return keys;
}

std::vector<Tag*>* CompoundTag::getAllTags()
{
    std::vector<Tag*>* tags = new std::vector<Tag*>();

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        tags->push_back(it->second);
    }

    return tags;
}

boost::unordered_map<std::wstring, Tag*>* CompoundTag::getAllTagsMap()
{
    return &mTags;
}

bool CompoundTag::equalsWithException(Tag* other, std::vector<std::wstring>* exceptions)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    CompoundTag* otherCompound = (CompoundTag*)other;

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        bool excepted = false;

        if (exceptions != NULL)
        {
            std::wstring key = it->first;

            for (unsigned int i = 0; i < exceptions->size(); i++)
            {
                if (key == exceptions->at(i))
                {
                    excepted = true;
                    break;
                }
            }
        }

        if (!excepted)
        {
            TagMap::const_iterator found = otherCompound->mTags.find(it->first);

            if (found == otherCompound->mTags.end())
            {
                return false;
            }

            if (!it->second->equals(found->second))
            {
                return false;
            }
        }
    }

    return true;
}

int CompoundTag::GetNumValidTagsAfterException(std::vector<std::wstring>* exceptions)
{
    int count = 0;

    for (TagMap::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
    {
        bool excepted = false;

        if (exceptions != NULL)
        {
            std::wstring key = it->first;

            for (unsigned int i = 0; i < exceptions->size(); i++)
            {
                if (key == exceptions->at(i))
                {
                    excepted = true;
                    break;
                }
            }
        }

        if (!excepted)
        {
            count++;
        }
    }

    return count;
}

void CompoundTag::merge(CompoundTag* other)
{
    for (TagMap::const_iterator it = other->mTags.begin(); it != other->mTags.end(); ++it)
    {
        std::wstring key = it->first;
        Tag* tag = it->second;

        if (tag->getId() == Tag::TAG_COMPOUND)
        {
            if (contains(key, Tag::TAG_COMPOUND))
            {
                getCompound(key)->merge((CompoundTag*)tag);
            }
            else
            {
                put(key, tag->copy());
            }
        }
        else
        {
            put(key, tag->copy());
        }
    }
}
