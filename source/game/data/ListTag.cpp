#include "game/data/ListTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/EndTag.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/data/NumericTag.hpp"
#include "game/data/Tag.hpp"

#include <cstring>
#include <cwchar>
#include <ostream>
#include <string>
#include <vector>

template <class T>
void deleteElements(const std::vector<T*, std::allocator<T*> >& elements);

template <class T>
void deleteElements(const std::vector<T*, std::allocator<T*> >& elements)
{
    for (typename std::vector<T*, std::allocator<T*> >::const_iterator it = elements.begin(), end = elements.end();
         it != end; ++it)
    {
        delete *it;
    }
}

ListTag::ListTag(int type) : mList(), mType(type)
{
}

ListTag::~ListTag()
{
    deleteElements(mList);
}

void ListTag::write(DataOutput* output)
{
    if (mList.size() == 0)
    {
        mType = Tag::TAG_END;
    }
    else
    {
        mType = mList[0]->getId();
    }

    output->writeByte(mType);
    output->writeInt(mList.size());

    for (std::vector<Tag*>::iterator it = mList.begin(), end = mList.end(); it != end; ++it)
    {
        (*it)->write(output);
    }
}

void ListTag::load(DataInput* input, int depth)
{
    if (depth > 512)
    {
        return;
    }

    mType = input->readByte();

    int count = input->readInt();

    deleteElements(mList);
    mList.clear();

    for (int i = 0; i < count; ++i)
    {
        Tag* tag = Tag::newTag(mType);
        tag->load(input, depth);
        mList.push_back(tag);
    }
}

unsigned char ListTag::getId()
{
    return 9;
}

std::wstring ListTag::toString()
{
    static wchar_t buffer[64];

    swprintf(buffer, 64, L"%zd entries of type %ls", mList.size(), Tag::getTagTypeName(mType));

    return buffer;
}

void ListTag::print(const std::wstring& name, char* indent, std::wostream& out)
{
    Tag::print(name, indent, out);

    out << indent << L"{" << std::endl;

    char* childIndent = new char[strlen(indent) + 4];
    strcpy(childIndent, indent);
    strcat(childIndent, "   ");

    for (std::vector<Tag*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
        (*it)->print(name, childIndent, out);
    }

    delete[] childIndent;

    out << indent << "}" << std::endl;
}

void ListTag::add(Tag* tag)
{
    if (tag->getId() == Tag::TAG_END)
    {
        return;
    }

    if (mType == Tag::TAG_END)
    {
        mType = tag->getId();
    }
    else if (mType != tag->getId())
    {
        if (mList.size() != 0)
        {
            return;
        }

        mType = tag->getId();
    }

    mList.push_back(tag);
}

void ListTag::set(int index, Tag* tag)
{
    if (tag->getId() == Tag::TAG_END)
    {
        return;
    }

    if (index < 0 || index >= mList.size())
    {
        return;
    }

    if (mType == Tag::TAG_END)
    {
        mType = tag->getId();
    }
    else if (mType != tag->getId())
    {
        if (mList.size() != 0 || index != 0)
        {
            return;
        }

        mType = tag->getId();
    }

    if (mList[index] != tag)
    {
        delete mList[index];
        mList[index] = tag;
    }
}

Tag* ListTag::get(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        return mList.at(index);
    }

    return new EndTag();
}

Tag* ListTag::remove(int index)
{
    Tag* tag = get(index);

    mList.erase(mList.begin() + index);

    return tag;
}

void ListTag::stripEmptyChildren()
{
    std::vector<Tag*>::iterator it = mList.begin();

    while (it != mList.end())
    {
        if ((*it)->isEmpty())
        {
            delete *it;
            it = mList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool ListTag::isEmpty() const
{
    return mList.size() == 0;
}

CompoundTag* ListTag::getCompound(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == Tag::TAG_COMPOUND)
        {
            return static_cast<CompoundTag*>(tag);
        }
    }

    return new CompoundTag();
}

int ListTag::getInt(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == Tag::TAG_INT)
        {
            return static_cast<NumericTag*>(tag)->getAsInt();
        }
    }

    return 0;
}

arrayWithLength<int> ListTag::getIntArray(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == Tag::TAG_INT_ARRAY)
        {
            return static_cast<IntArrayTag*>(tag)->getAsIntArray();
        }
    }

    return arrayWithLength<int>();
}

double ListTag::getDouble(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == Tag::TAG_DOUBLE)
        {
            return static_cast<NumericTag*>(tag)->getAsDouble();
        }
    }

    return 0.0;
}

float ListTag::getFloat(int index) const
{
    if (index >= 0 && index < mList.size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == Tag::TAG_FLOAT)
        {
            return static_cast<NumericTag*>(tag)->getAsFloat();
        }
    }

    return 0.0f;
}

std::wstring ListTag::getString(int index) const
{
    if (index < 0 || index >= mList.size())
    {
        return L"";
    }

    Tag* tag = mList.at(index);

    if (tag->getId() == Tag::TAG_STRING)
    {
        return tag->getAsString();
    }

    return tag->toString();
}

ListTag* ListTag::copy()
{
    ListTag* list = new ListTag();

    list->mType = mType;

    for (std::vector<Tag*>::iterator it = mList.begin(), end = mList.end(); it != end; ++it)
    {
        Tag* tag = (*it)->copy();
        list->mList.push_back(tag);
    }

    return list;
}

bool ListTag::equals(Tag* other)
{
    if (Tag::equals(other))
    {
        ListTag* list = (ListTag*)other;

        if (mType == list->mType)
        {
            bool equal = false;

            if (mList.size() == list->mList.size())
            {
                equal = true;

                for (std::vector<Tag*>::iterator it = mList.begin(), end = mList.end(); it != end; ++it)
                {
                    bool found = false;

                    for (std::vector<Tag*>::iterator otherIt = list->mList.begin(); otherIt != list->mList.end();
                         ++otherIt)
                    {
                        if ((*it)->equals(*otherIt))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        equal = false;
                        break;
                    }
                }
            }

            return equal;
        }
    }

    return false;
}

unsigned char ListTag::getType()
{
    return mType;
}

int ListTag::size() const
{
    return mList.size();
}
