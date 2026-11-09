#ifndef PLATFORM_WIIU_PEAD_HPP
#define PLATFORM_WIIU_PEAD_HPP

#include <climits>
#include <limits>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <coreinit/core.h>
#include <coreinit/fastmutex.h>
#include <coreinit/memory.h>
#include <coreinit/messagequeue.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#else
#include <coreinit/atomic.h>
#endif

struct OSThread;

namespace nn
{
namespace pia
{
namespace common
{
class BackgroundScheduler;
}
namespace transport
{
class PacketHandler;
class StationPacketHandler;
class TransportThreadStream;
}
}
}

namespace pead
{
class Arena;
class Heap;
class StackSymbolResolver;
class StreamFormat;
class WriteStream;

class MemUtil
{
public:
    static void* copy(void* destination, const void* source, unsigned int size)
    {
        return OSBlockMove(destination, source, size, FALSE);
    }
    static void* fill(void* destination, unsigned char value, unsigned int size)
    {
        return OSBlockSet(destination, value, size);
    }
};

template <typename T> class MathCalcCommon
{
public:
    static T maxNumber() { return std::numeric_limits<T>::max(); }
    static T minNumber()
    {
        return std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::min() : -std::numeric_limits<T>::max();
    }
};

struct Endian
{
    enum Types
    {
        cBig = 0,
        cLittle = 1,
        cHost = cBig
    };
};

template <typename T> class SafeStringBase
{
public:
    class iterator
    {
    public:
        explicit iterator(const SafeStringBase* string) : mString(string), mIndex(0) {}
        iterator(const SafeStringBase* string, int index) : mString(string), mIndex(index) {}
        const T& operator*() const { return mString->at(mIndex); }
        iterator& operator++()
        {
            ++mIndex;
            return *this;
        }
        bool operator==(const iterator& other) const { return mString == other.mString && mIndex == other.mIndex; }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        int getIndex() const { return mIndex; }

    private:
        const SafeStringBase* mString;
        int mIndex;
    };

    SafeStringBase() : mStringTop(&cNullChar) {}
    SafeStringBase(const T* text) : mStringTop(text) {}
    virtual ~SafeStringBase() {}
    virtual SafeStringBase& operator=(const SafeStringBase& other);
    virtual void assureTerminationImpl_() const {}
    const T* cstr() const
    {
        assureTerminationImpl_();
        return mStringTop;
    }
    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, calcLength()); }
    int calcLength() const
    {
        assureTerminationImpl_();
        int length = 0;
        for (;;)
        {
            if (length > cMaximumLength || mStringTop[length] == cNullChar)
                break;
            ++length;
        }
        if (length > cMaximumLength)
            return 0;
        return length;
    }
    bool isEmpty() const { return mStringTop[0] == cNullChar; }
    bool isEqual(const SafeStringBase& other) const
    {
        assureTerminationImpl_();
        if (cstr() == other.cstr())
            return true;
        for (int index = 0; index <= cMaximumLength; ++index)
        {
            if (mStringTop[index] != other.mStringTop[index])
                return false;
            if (mStringTop[index] == cNullChar)
                return true;
        }
        return false;
    }
    bool operator==(const SafeStringBase& other) const { return isEqual(other); }
    const T& at(int index) const
    {
        int length = calcLength();
        if (index < 0 || index > length)
            return cNullChar;
        return mStringTop[index];
    }
    bool include(const T& character) const
    {
        const T* text = cstr();
        for (int index = 0; index <= cMaximumLength; ++index)
        {
            if (text[index] == cNullChar)
                break;
            if (text[index] == character)
                return true;
        }
        return character == 0;
    }

    static const T cNullChar;
    static const int cMaximumLength = 0x80000;

protected:
    const T* mStringTop;
};

template <> const char SafeStringBase<char>::cNullChar;
template <> const wchar_t SafeStringBase<wchar_t>::cNullChar;
template <> SafeStringBase<char>& SafeStringBase<char>::operator=(const SafeStringBase<char>& other);
template <> SafeStringBase<wchar_t>& SafeStringBase<wchar_t>::operator=(const SafeStringBase<wchar_t>& other);

template <typename T> class Buffer
{
public:
    Buffer() : mSize(0), mBuffer(0) {}
    Buffer(int size, T* buffer) : mSize(size), mBuffer(buffer) {}
    T* begin() const { return mBuffer; }
    T* end() const { return mBuffer + mSize; }
    int size() const { return mSize; }
    bool isBufferReady() const { return mBuffer != 0; }
    bool isIndexValid(int index) const { return static_cast<unsigned int>(index) < static_cast<unsigned int>(mSize); }
    T& operator[](int index)
    {
        if (isIndexValid(index))
            return mBuffer[index];
        return mBuffer[0];
    }
    const T& operator[](int index) const
    {
        if (isIndexValid(index))
            return mBuffer[index];
        return mBuffer[0];
    }
    T* get(int index) const { return isIndexValid(index) ? mBuffer + index : 0; }
    T* getBufferPtr() const { return mBuffer; }
    void setBuffer(int size, T* buffer)
    {
        if (size > 0 && buffer)
        {
            mSize = size;
            mBuffer = buffer;
        }
    }
    bool tryAllocBuffer(int size, Heap* heap, int alignment = sizeof(void*));
    void freeBuffer();

private:
    int mSize;
    T* mBuffer;
};

template <typename T> class RingBuffer
{
public:
    RingBuffer() : mBuffer(NULL), mCapacity(0), mHead(0), mSize(0) {}

    void setBuffer(int capacity, T* buffer)
    {
        if (capacity > 0)
        {
            mBuffer = buffer;
            mSize = 0;
            mHead = 0;
            mCapacity = capacity;
        }
    }

    T& operator()(int index)
    {
        if (index < mSize)
            return mBuffer[calcRealIdx(index)];
        return mBuffer[0];
    }

    T& front() { return operator()(0); }

    bool pushBack(const T& item)
    {
        int size = mSize;
        if (size < mCapacity)
        {
            mSize = size + 1;
            mBuffer[calcRealIdx(size)] = item;
            return true;
        }
        return false;
    }

    void popFront()
    {
        if (mSize > 0)
        {
            ++mHead;
            if (mHead >= mCapacity)
                mHead = 0;
            --mSize;
        }
    }

    void clear()
    {
        mSize = 0;
        mHead = 0;
    }

    int size() const { return mSize; }
    int capacity() const { return mCapacity; }
    bool isEmpty() const { return mSize == 0; }
    bool isFull() const { return mSize >= mCapacity; }

    int calcRealIdx(int index) const
    {
        int realIndex = mHead + index;
        if (realIndex >= mCapacity)
            realIndex -= mCapacity;
        return realIndex;
    }

    T* mBuffer;
    int mCapacity;
    int mHead;
    int mSize;
};

template <typename T> class BitFlag
{
public:
    BitFlag(T value = 0) : mValue(value) {}
    T getDirect() const { return mValue; }
    void set(T mask) { mValue |= mask; }
    void reset(T mask) { mValue &= ~mask; }
    bool isOn(T mask) const { return mValue & mask; }

private:
    T mValue;
};

template <typename T> class BufferedSafeStringBase : public SafeStringBase<T>
{
public:
    BufferedSafeStringBase(BufferedSafeStringBase<T>* original, int position)
        : SafeStringBase<T>(original->getBuffer() + position),
          mBufferSize(0)
    {
        if (position < original->mBufferSize && position >= 0)
        {
            mBufferSize = original->mBufferSize - position;
            assureTerminationImpl_();
        }
        else
        {
            this->mStringTop = 0;
        }
    }
    BufferedSafeStringBase(T* buffer, int size) : SafeStringBase<T>(buffer), mBufferSize(size)
    {
        if (size <= 0)
        {
            this->mStringTop = 0;
            mBufferSize = 0;
        }
        else
        {
            assureTerminationImpl_();
        }
    }
    virtual ~BufferedSafeStringBase() {}
    virtual BufferedSafeStringBase& operator=(const SafeStringBase<T>& other);
    virtual void assureTerminationImpl_() const;
    static int formatImpl_(T* buffer, int size, const T* format, va_list args);
    int formatV(const T* format, va_list args);
    int format(const T* format, ...);
    int appendWithFormatV(const T* format, va_list args);
    int appendWithFormat(const T* format, ...);
    int getBufferSize() const { return mBufferSize; }
    T* getBuffer()
    {
        assureTerminationImpl_();
        return getMutableStringTop_();
    }
    const T* getBuffer() const
    {
        assureTerminationImpl_();
        return this->mStringTop;
    }
    void clear() { getMutableStringTop_()[0] = SafeStringBase<T>::cNullChar; }
    int copy(const SafeStringBase<T>& source, int length = -1);
    int copyAt(int index, const SafeStringBase<T>& source, int length)
    {
        int currentLength = this->calcLength();
        if (index < 0)
        {
            index = currentLength + index + 1;
            if (index < 0)
                index = 0;
        }
        if (length >= mBufferSize - index)
            length = mBufferSize - index - 1;
        if (length <= 0)
            return 0;
        MemUtil::copy(getMutableStringTop_() + index, source.cstr(), length * sizeof(T));
        if (index + length > currentLength)
            getMutableStringTop_()[index + length] = SafeStringBase<T>::cNullChar;
        return length;
    }
    int append(T character)
    {
        int length = this->calcLength();
        if (length >= mBufferSize - 1)
            return 0;
        getMutableStringTop_()[length] = character;
        getMutableStringTop_()[length + 1] = SafeStringBase<T>::cNullChar;
        return 1;
    }
    int append(T character, int count)
    {
        if (count <= 0)
            return 0;
        int length = this->calcLength();
        if (count >= mBufferSize - length)
            count = mBufferSize - length - 1;
        T* buffer = getMutableStringTop_();
        for (int index = 0; index < count; ++index)
            buffer[length + index] = character;
        buffer[length + count] = SafeStringBase<T>::cNullChar;
        return count;
    }
    int trim(int length)
    {
        if (length >= mBufferSize)
            return this->calcLength();
        if (length < 0)
            length = 0;
        getMutableStringTop_()[length] = SafeStringBase<T>::cNullChar;
        return length;
    }
    const T& operator[](int index) const
    {
        if (index < 0 || index > mBufferSize - 1)
            return SafeStringBase<T>::cNullChar;
        return this->mStringTop[index];
    }

protected:
    T* getMutableStringTop_() { return const_cast<T*>(this->mStringTop); }

    int mBufferSize;
};

template <typename T> inline int BufferedSafeStringBase<T>::copy(const SafeStringBase<T>& source, int length)
{
    T* buffer = getMutableStringTop_();
    const T* text = source.cstr();
    if (buffer == text)
        return 0;
    if (length < 0)
        length = source.calcLength();
    if (length >= mBufferSize)
        length = mBufferSize - 1;
    MemUtil::copy(buffer, text, length * sizeof(T));
    buffer[length] = SafeStringBase<T>::cNullChar;
    return length;
}

template <> BufferedSafeStringBase<char>& BufferedSafeStringBase<char>::operator=(const SafeStringBase<char>& other);
template <> BufferedSafeStringBase<wchar_t>& BufferedSafeStringBase<wchar_t>::operator=(
    const SafeStringBase<wchar_t>& other);
template <> void BufferedSafeStringBase<char>::assureTerminationImpl_() const;
template <> void BufferedSafeStringBase<wchar_t>::assureTerminationImpl_() const;
template <> int BufferedSafeStringBase<char>::formatImpl_(char* buffer, int size, const char* format, va_list args);
template <> int BufferedSafeStringBase<wchar_t>::formatImpl_(wchar_t* buffer, int size, const wchar_t* format,
                                                             va_list args);
template <> int BufferedSafeStringBase<char>::formatV(const char* format, va_list args);
template <> int BufferedSafeStringBase<wchar_t>::formatV(const wchar_t* format, va_list args);
template <> int BufferedSafeStringBase<char>::format(const char* format, ...);
template <> int BufferedSafeStringBase<wchar_t>::format(const wchar_t* format, ...);
template <> int BufferedSafeStringBase<char>::appendWithFormatV(const char* format, va_list args);
template <> int BufferedSafeStringBase<wchar_t>::appendWithFormatV(const wchar_t* format, va_list args);
template <> int BufferedSafeStringBase<char>::appendWithFormat(const char* format, ...);
template <> int BufferedSafeStringBase<wchar_t>::appendWithFormat(const wchar_t* format, ...);

template <typename T> class StringBuilderBase : public BufferedSafeStringBase<T>
{
public:
    StringBuilderBase(T* buffer, int size) : BufferedSafeStringBase<T>(buffer, size) {}
};

template <typename T> class HeapSafeStringBase : public BufferedSafeStringBase<T>
{
public:
    HeapSafeStringBase(T* buffer, int size) : BufferedSafeStringBase<T>(buffer, size) {}
    virtual ~HeapSafeStringBase();
    virtual HeapSafeStringBase& operator=(const SafeStringBase<T>& other);
};

template <> HeapSafeStringBase<char>& HeapSafeStringBase<char>::operator=(const SafeStringBase<char>& other);
template <> HeapSafeStringBase<wchar_t>& HeapSafeStringBase<wchar_t>::operator=(const SafeStringBase<wchar_t>& other);

template <typename T, int N> class FixedSafeStringBase : public BufferedSafeStringBase<T>
{
public:
    FixedSafeStringBase() : BufferedSafeStringBase<T>(mStorage, N) { this->clear(); }
    FixedSafeStringBase(const SafeStringBase<T>& text) : BufferedSafeStringBase<T>(mStorage, N) { this->copy(text); }
    virtual ~FixedSafeStringBase() {}
    virtual FixedSafeStringBase& operator=(const SafeStringBase<T>& text)
    {
        this->copy(text);
        return *this;
    }

private:
    T mStorage[N];
};

template <int N> class FixedSafeString : public FixedSafeStringBase<char, N>
{
public:
    FixedSafeString() {}
    FixedSafeString(const SafeStringBase<char>& text) : FixedSafeStringBase<char, N>(text) {}
    virtual ~FixedSafeString() {}
    virtual FixedSafeString& operator=(const SafeStringBase<char>& text)
    {
        FixedSafeStringBase<char, N>::operator=(text);
        return *this;
    }
};

typedef FixedSafeString<32> FixedSafeString32;
typedef FixedSafeString<128> FixedSafeString128;
typedef FixedSafeString<256> FixedSafeString256;
typedef FixedSafeString<1024> FixedSafeString1024;

namespace RuntimeTypeInfo
{
class Interface
{
public:
    virtual bool isDerived(const Interface*) const = 0;
};

class Root : public Interface
{
public:
    virtual bool isDerived(const Interface* type) const { return type == this; }
};

template <typename BaseType> class Derive : public Interface
{
public:
    virtual bool isDerived(const Interface* type) const
    {
        if (this == type)
            return true;
        return BaseType::getRuntimeTypeInfoStatic()->isDerived(type);
    }
};
}

class ListNode
{
    friend class ListImpl;

public:
    ListNode() : mPrev(0), mNext(0) {}
    void insertBack_(ListNode* node);
    void insertFront_(ListNode* node);
    void erase_();
    ListNode* prev() const { return mPrev; }
    ListNode* next() const { return mNext; }

private:
    ListNode* mPrev;
    ListNode* mNext;
};

class Random;

class ListImpl
{
public:
    ListImpl() : mStartEnd(), mCount(0)
    {
        mStartEnd.mNext = &mStartEnd;
        mStartEnd.mPrev = &mStartEnd;
    }
    bool isEmpty() const { return mCount == 0; }
    int size() const { return mCount; }
    void pushBack(ListNode* item)
    {
        mStartEnd.insertFront_(item);
        ++mCount;
    }
    void pushFront(ListNode* item)
    {
        mStartEnd.insertBack_(item);
        ++mCount;
    }
    void insertBefore(ListNode* basis, ListNode* item)
    {
        basis->insertFront_(item);
        ++mCount;
    }
    void erase(ListNode* item)
    {
        item->erase_();
        --mCount;
    }
    ListNode* front() const { return mCount > 0 ? mStartEnd.mNext : 0; }
    ListNode* back() const { return mCount > 0 ? mStartEnd.mPrev : 0; }
    ListNode* nth(int index) const;
    void sort(int offset, int (*compare)(const void*, const void*));
    void clear();
    void reverse();
    void swap(ListNode*, ListNode*);
    void shuffle(Random*);
    static ListNode* mergeSortImpl(ListNode*, ListNode*, int, int, int (*)(const void*, const void*));
    void mergeSort(int, int (*)(const void*, const void*));
    void popBack();
    void popFront();
    int indexOf(const ListNode*) const;
    void moveAfter(ListNode*, ListNode*);
    void moveBefore(ListNode*, ListNode*);
    ListNode* find(const void*, int, int (*)(const void*, const void*)) const;
    void uniq(int, int (*)(const void*, const void*));
    bool checkLinks() const;

protected:
    ListNode mStartEnd;
    int mCount;
};

template <typename T> class OffsetList : public ListImpl
{
public:
    class iterator
    {
    public:
        iterator(T* object, int offset) : mObject(object), mOffset(offset) {}
        T& operator*() const { return *mObject; }
        T* operator->() const { return mObject; }
        iterator& operator++()
        {
            ListNode* node = reinterpret_cast<ListNode*>(reinterpret_cast<char*>(mObject) + mOffset)->next();
            mObject = reinterpret_cast<T*>(reinterpret_cast<char*>(node) + -mOffset);
            return *this;
        }
        bool operator==(const iterator& other) const { return mObject == other.mObject; }
        bool operator!=(const iterator& other) const { return mObject != other.mObject; }

    private:
        T* mObject;
        int mOffset;
    };

    OffsetList() : mOffset(-1) {}
    void initOffset(int offset) { mOffset = offset; }
    void pushBack(T* object) { ListImpl::pushBack(objToListNode(object)); }
    void pushFront(T* object) { ListImpl::pushFront(objToListNode(object)); }
    void insertBefore(T* basis, T* object) { ListImpl::insertBefore(objToListNode(basis), objToListNode(object)); }
    void erase(T* object) { ListImpl::erase(objToListNode(object)); }
    T* front() const { return listNodeToObjWithNullCheck(ListImpl::front()); }
    T* back() const { return listNodeToObjWithNullCheck(ListImpl::back()); }
    T* prev(const T* object) const
    {
        ListNode* node = objToListNode(object)->prev();
        if (node == &mStartEnd)
            return 0;
        return listNodeToObj(node);
    }
    void clear() { ListImpl::clear(); }
    void sort(int (*compare)(const T*, const T*))
    {
        ListImpl::sort(mOffset, reinterpret_cast<int (*)(const void*, const void*)>(compare));
    }
    iterator begin() const { return iterator(listNodeToObj(mStartEnd.next()), mOffset); }
    iterator end() const { return iterator(listNodeToObj(&mStartEnd), mOffset); }

protected:
    ListNode* objToListNode(const T* object) const
    {
        return reinterpret_cast<ListNode*>(const_cast<char*>(reinterpret_cast<const char*>(object)) + mOffset);
    }
    T* listNodeToObj(const ListNode* node) const
    {
        return reinterpret_cast<T*>(const_cast<char*>(reinterpret_cast<const char*>(node)) + -mOffset);
    }
    T* listNodeToObjWithNullCheck(ListNode* node) const { return node ? listNodeToObj(node) : 0; }

    int mOffset;
};

template <typename T> class TList;

template <typename T> class TListNode : public ListNode
{
public:
    TListNode() : ListNode(), mData(), mList(0) {}
    void erase()
    {
        TList<T>* list = mList;
        if (list)
            list->erase(this);
    }

    T mData;
    TList<T>* mList;
};

template <typename T> class TList : public ListImpl
{
public:
    TList() : ListImpl() {}
    void pushBack(TListNode<T>* item)
    {
        item->erase();
        item->mList = this;
        ListImpl::pushBack(item);
    }
    void erase(TListNode<T>* item)
    {
        item->mList = 0;
        ListImpl::erase(item);
    }
    TListNode<T>* begin() const { return static_cast<TListNode<T>*>(mStartEnd.next()); }
    const ListNode* end() const { return &mStartEnd; }
    static TListNode<T>* next(const TListNode<T>* item) { return static_cast<TListNode<T>*>(item->next()); }
};

class PtrArrayImpl
{
public:
    PtrArrayImpl() : mPtrNum(0), mPtrNumMax(0), mPtrs(0) {}
    void setBuffer(int, void*);
    bool tryAllocBuffer(int, Heap*, int);
    bool allocBuffer(int, Heap*, int);
    void freeBuffer();
    void erase(int, int);
    bool checkInsert(int, int);
    void insert(int, void*);
    int binarySearch(const void*, int (*)(const void*, const void*)) const;
    void reverse();
    void shuffle(Random*);
    void insertArray(int, void*, int, int);
    void sort(int (*)(const void*, const void*));
    void heapSort(int (*)(const void*, const void*));
    int compare(const PtrArrayImpl&, int (*)(const void*, const void*)) const;
    void uniq(int (*)(const void*, const void*));
    int size() const { return mPtrNum; }
    int maxSize() const { return mPtrNumMax; }
    bool isEmpty() const { return mPtrNum == 0; }
    bool isBufferReady() const { return mPtrs != 0; }

protected:
    int mPtrNum;
    int mPtrNumMax;
    void** mPtrs;
};

template <typename T> class PtrArray : public PtrArrayImpl
{
public:
    T* at(int index) const
    {
        if (static_cast<unsigned int>(index) < static_cast<unsigned int>(mPtrNum))
            return static_cast<T*>(mPtrs[index]);
        return 0;
    }
    T* unsafeAt(int index) const { return static_cast<T*>(mPtrs[index]); }
    T* back() const { return at(mPtrNum - 1); }
    void pushBack(T* value)
    {
        if (mPtrNum < mPtrNumMax)
        {
            mPtrs[mPtrNum] = value;
            ++mPtrNum;
        }
    }
    void popBack()
    {
        if (mPtrNum > 0)
            --mPtrNum;
    }
    template <typename Key> int binarySearch(const Key* key, int (*compare)(const T*, const Key*)) const
    {
        return PtrArrayImpl::binarySearch(key, reinterpret_cast<int (*)(const void*, const void*)>(compare));
    }
    T** begin() const { return reinterpret_cast<T**>(mPtrs); }
    T** end() const { return reinterpret_cast<T**>(mPtrs) + mPtrNum; }
};

template <typename T, int N> class FixedPtrArray : public PtrArray<T>
{
public:
    FixedPtrArray() { this->setBuffer(N, mWork); }

private:
    void* mWork[N];
};

class Random
{
public:
    Random() { init(); }
    void init(unsigned int);
    void init();
    void init(unsigned int, unsigned int, unsigned int, unsigned int);
    unsigned int getU32();
    unsigned int getU32(unsigned int max)
    {
        return static_cast<unsigned int>((static_cast<unsigned long long>(getU32()) * max) >> 32);
    }
    int getS32Range(int a, int b) { return getU32(b - a) + a; }
    unsigned long long getU64();
    void getContext(unsigned int*, unsigned int*, unsigned int*, unsigned int*) const;

private:
    unsigned int mContext[4];
};

class StreamSrc
{
public:
    virtual unsigned int read(void*, unsigned int) = 0;
    virtual unsigned int write(const void*, unsigned int) = 0;
    virtual int skip(int) = 0;
    virtual void rewind() = 0;
    virtual bool isEOF() = 0;
    virtual bool flush() { return true; }
};

class BufferReadStreamSrc : public StreamSrc
{
public:
    BufferReadStreamSrc(StreamSrc*, void*, unsigned int);
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual int skip(int);
    virtual void rewind();
    virtual bool isEOF();
    virtual ~BufferReadStreamSrc();

private:
    StreamSrc* mSrc;
    unsigned char* mBuffer;
    unsigned int mBufferSize;
    unsigned int mBufferEndPos;
    unsigned int mBufferPos;
};

class BufferWriteStreamSrc : public StreamSrc
{
public:
    BufferWriteStreamSrc(StreamSrc*, void*, unsigned int);
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual int skip(int);
    virtual void rewind();
    virtual bool isEOF();
    virtual bool flush();
    virtual ~BufferWriteStreamSrc();

protected:
    StreamSrc* mSrc;
    unsigned char* mBuffer;
    unsigned int mBufferSize;
    unsigned int mBufferPos;
};

class BufferMultiByteTextWriteStreamSrc : public BufferWriteStreamSrc
{
public:
    BufferMultiByteTextWriteStreamSrc(StreamSrc*, void*, unsigned int);
    virtual ~BufferMultiByteTextWriteStreamSrc() {}
    virtual unsigned int write(const void*, unsigned int);

private:
    enum
    {
        cUtf8MultiByteBit = 0x80,
        cUtf8TrailMask = 0xc0,
        cUtf8TrailBits = 0x80,
        cUtf8Lead2Mask = 0xe0,
        cUtf8Lead2Bits = 0xc0,
        cUtf8Lead3Mask = 0xf0,
        cUtf8Lead3Bits = 0xe0,
        cUtf8Lead4Mask = 0xf8,
        cUtf8Lead4Bits = 0xf0,
        cUtf8MaxSequenceLength = 4
    };
};

class BufferMultiByteNullTerminatedTextWriteStreamSrc : public BufferMultiByteTextWriteStreamSrc
{
public:
    BufferMultiByteNullTerminatedTextWriteStreamSrc(StreamSrc* src, void* buffer, unsigned int size)
        : BufferMultiByteTextWriteStreamSrc(src, buffer, size)
    {
    }
    virtual ~BufferMultiByteNullTerminatedTextWriteStreamSrc() {}
    virtual bool flush();
};

class StreamFormat
{
public:
    ~StreamFormat() {}
    virtual unsigned char readU8(StreamSrc*, Endian::Types) = 0;
    virtual unsigned short readU16(StreamSrc*, Endian::Types) = 0;
    virtual unsigned int readU32(StreamSrc*, Endian::Types) = 0;
    virtual unsigned long long readU64(StreamSrc*, Endian::Types) = 0;
    virtual signed char readS8(StreamSrc*, Endian::Types) = 0;
    virtual short readS16(StreamSrc*, Endian::Types) = 0;
    virtual int readS32(StreamSrc*, Endian::Types) = 0;
    virtual long long readS64(StreamSrc*, Endian::Types) = 0;
    virtual float readF32(StreamSrc*, Endian::Types) = 0;
    virtual void readBit(StreamSrc*, void*, unsigned int) = 0;
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int) = 0;
    virtual void readMemBlock(StreamSrc*, void*, unsigned int) = 0;
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char) = 0;
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short) = 0;
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int) = 0;
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long) = 0;
    virtual void writeS8(StreamSrc*, Endian::Types, signed char) = 0;
    virtual void writeS16(StreamSrc*, Endian::Types, short) = 0;
    virtual void writeS32(StreamSrc*, Endian::Types, int) = 0;
    virtual void writeS64(StreamSrc*, Endian::Types, long long) = 0;
    virtual void writeF32(StreamSrc*, Endian::Types, float) = 0;
    virtual void writeBit(StreamSrc*, const void*, unsigned int) = 0;
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int) = 0;
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int) = 0;
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&) = 0;
    virtual void writeNullChar(StreamSrc*) = 0;
    virtual void skip(StreamSrc*, unsigned int) = 0;
    virtual void flush(StreamSrc*) = 0;
    virtual void rewind(StreamSrc*) = 0;
};

class BinaryStreamFormat : public StreamFormat
{
public:
    virtual unsigned char readU8(StreamSrc*, Endian::Types);
    virtual unsigned short readU16(StreamSrc*, Endian::Types);
    virtual unsigned int readU32(StreamSrc*, Endian::Types);
    virtual unsigned long long readU64(StreamSrc*, Endian::Types);
    virtual signed char readS8(StreamSrc*, Endian::Types);
    virtual short readS16(StreamSrc*, Endian::Types);
    virtual int readS32(StreamSrc*, Endian::Types);
    virtual long long readS64(StreamSrc*, Endian::Types);
    virtual float readF32(StreamSrc*, Endian::Types);
    virtual void readBit(StreamSrc*, void*, unsigned int);
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int);
    virtual void readMemBlock(StreamSrc*, void*, unsigned int);
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char);
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short);
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int);
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long);
    virtual void writeS8(StreamSrc*, Endian::Types, signed char);
    virtual void writeS16(StreamSrc*, Endian::Types, short);
    virtual void writeS32(StreamSrc*, Endian::Types, int);
    virtual void writeS64(StreamSrc*, Endian::Types, long long);
    virtual void writeF32(StreamSrc*, Endian::Types, float);
    virtual void writeBit(StreamSrc*, const void*, unsigned int);
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int);
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int);
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&);
    virtual void writeNullChar(StreamSrc*);
    virtual void skip(StreamSrc*, unsigned int);
    virtual void flush(StreamSrc*);
    virtual void rewind(StreamSrc*);
};

class TextStreamFormat : public BinaryStreamFormat
{
public:
    TextStreamFormat();
    ~TextStreamFormat();
    virtual unsigned char readU8(StreamSrc*, Endian::Types);
    virtual unsigned short readU16(StreamSrc*, Endian::Types);
    virtual unsigned int readU32(StreamSrc*, Endian::Types);
    virtual unsigned long long readU64(StreamSrc*, Endian::Types);
    virtual signed char readS8(StreamSrc*, Endian::Types);
    virtual short readS16(StreamSrc*, Endian::Types);
    virtual int readS32(StreamSrc*, Endian::Types);
    virtual long long readS64(StreamSrc*, Endian::Types);
    virtual float readF32(StreamSrc*, Endian::Types);
    virtual void readBit(StreamSrc*, void*, unsigned int);
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int);
    virtual void readMemBlock(StreamSrc*, void*, unsigned int);
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char);
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short);
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int);
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long);
    virtual void writeS8(StreamSrc*, Endian::Types, signed char);
    virtual void writeS16(StreamSrc*, Endian::Types, short);
    virtual void writeS32(StreamSrc*, Endian::Types, int);
    virtual void writeS64(StreamSrc*, Endian::Types, long long);
    virtual void writeF32(StreamSrc*, Endian::Types, float);
    virtual void writeBit(StreamSrc*, const void*, unsigned int);
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int);
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int);
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&);
    virtual void writeNullChar(StreamSrc*);
    virtual void skip(StreamSrc*, unsigned int);
    virtual void rewind(StreamSrc*);
    virtual void flush(StreamSrc*);

private:
    enum
    {
        cValueTextLength = 32
    };

    void getNextData_(StreamSrc*);
    FixedSafeString128 mSeparator;
};

class Stream
{
public:
    enum Modes
    {
        cBinary = 0,
        cText = 1
    };
    Stream();
    Stream(StreamSrc*, Modes);
    Stream(StreamSrc*, StreamFormat*);
    virtual ~Stream();
    void skip(unsigned int);
    void skip(unsigned int, unsigned int);
    void rewind();
    bool isEOF();
    void setUserFormat(StreamFormat*);
    void setMode(Modes);
    void setBinaryEndian(Endian::Types);
    StreamSrc* getStreamSrc() const { return mSrc; }
    StreamFormat* getStreamFormat() const { return mFormat; }

protected:
    StreamFormat* mFormat;
    StreamSrc* mSrc;
    Endian::Types mEndian;
};

class ReadStream : public Stream
{
public:
    ReadStream();
    ReadStream(StreamSrc*, Modes);
    ReadStream(StreamSrc*, StreamFormat*);
    virtual ~ReadStream();
    unsigned char readU8();
    unsigned short readU16();
    unsigned int readU32();
    unsigned long long readU64();
    signed char readS8();
    short readS16();
    int readS32();
    long long readS64();
    float readF32();
    void readBit(void*, unsigned int);
    void readString(BufferedSafeStringBase<char>*, unsigned int);
    void readMemBlock(void*, unsigned int);
    void readU8(unsigned char&);
    void readU16(unsigned short&);
    void readU32(unsigned int&);
    void readU64(unsigned long long&);
    void readS8(signed char&);
    void readS16(short&);
    void readS32(int&);
    void readS64(long long&);
    void readF32(float&);
    float readF32BitImpl_(unsigned int, unsigned int);
    double readF64BitImpl_(unsigned int, unsigned int);
};

class WriteStream : public Stream
{
public:
    WriteStream();
    WriteStream(StreamSrc*, Modes);
    WriteStream(StreamSrc*, StreamFormat*);
    virtual ~WriteStream();
    void flush();
    void writeU8(unsigned char);
    void writeU16(unsigned short);
    void writeU32(unsigned int);
    void writeU64(unsigned long long);
    void writeS8(signed char);
    void writeS16(short);
    void writeS32(int);
    void writeS64(long long);
    void writeF32(float);
    void writeBit(const void*, unsigned int);
    void writeString(const SafeStringBase<char>&, unsigned int);
    void writeMemBlock(const void*, unsigned int);
    void writeComment(const SafeStringBase<char>&);
    void writeLineComment(const SafeStringBase<char>&);
    void writeDecorationText(const SafeStringBase<char>&);
    void writeNullChar();
    void writeF32BitImpl_(float, unsigned int, unsigned int);
    void writeF64BitImpl_(double, unsigned int, unsigned int);
};

class PrintStreamSrc : public StreamSrc
{
public:
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual int skip(int);
    virtual void rewind();
    virtual bool isEOF();
};

class PrintWriteStream : public WriteStream
{
public:
    PrintWriteStream(Modes);
    PrintWriteStream(StreamFormat*);
    virtual ~PrintWriteStream();

private:
    PrintStreamSrc mPrintSrc;
};

class BufferReadStream : public ReadStream
{
public:
    BufferReadStream(ReadStream*, const void*, unsigned int);
    virtual ~BufferReadStream();

private:
    BufferReadStreamSrc mBufferSrc;
};

class BufferWriteStream : public WriteStream
{
public:
    BufferWriteStream(WriteStream*, void*, unsigned int);
    virtual ~BufferWriteStream();

private:
    BufferWriteStreamSrc mBufferSrc;
};

class BufferMultiByteTextWriteStream : public WriteStream
{
public:
    BufferMultiByteTextWriteStream(WriteStream*, void*, unsigned int);
    virtual ~BufferMultiByteTextWriteStream();

private:
    BufferMultiByteTextWriteStreamSrc mBufferSrc;
};

class Base64
{
public:
    static void encode(char*, const void*, unsigned int, bool);
    static bool decode(void*, unsigned int, const char*, unsigned int, unsigned int*);

private:
    enum
    {
        cEncodedBlockSize = 4,
        cPadding = 64,
        cValueMask = cPadding - 1
    };
};

class BitFlagUtil
{
public:
    static unsigned int countOnBit(unsigned int);
    static int findOnBitFromRight(unsigned int, int);
    static unsigned int countRightOnBit(unsigned int, int);
    static unsigned int countRightOnBit64(unsigned long long, int);
    static int findOnBitFromRight64(unsigned long long, int);
};

class CoreId
{
public:
    CoreId();
    explicit CoreId(int id) : mId(id) {}
    int value() const { return mId; }

private:
    int mId;
};

class CoreIdMask
{
public:
    CoreIdMask(unsigned int mask = 0) : mMask(mask) {}
    int countOnBits() const;
    bool isOn(CoreId id) const { return (mMask & (1u << id.value())) != 0; }
    unsigned int value() const { return mMask; }

private:
    unsigned int mMask;
};

class CoreInfo
{
public:
    static void configure();
    static int getPlatformMask(CoreId);
    static CoreId getCurrentCoreId() { return CoreId(sCoreIndexTable[OSGetCoreId()]); }
    static void dump();

    static unsigned int sCoreCount;
    static unsigned int sCoreIdMap[3];
    static unsigned int sCoreIndexTable[3];
};

class IDisposer
{
    friend class Heap;

public:
    enum HeapNullOption
    {
        cHeapNullOption_AlwaysUseSpecifiedHeap = 0,
        cHeapNullOption_UseSpecifiedOrContainHeap = 1,
        cHeapNullOption_DoNotAppendDisposerIfNoHeapSpecified = 2,
        cHeapNullOption_UseSpecifiedOrCurrentHeap = 3
    };
    IDisposer();
    IDisposer(Heap*, HeapNullOption);
    virtual ~IDisposer();

    static Heap* cDestructedFlag;

private:
    Heap* mDisposerHeap;
    ListNode mListNode;
};

class AtomicU32
{
public:
    AtomicU32(unsigned int value = 0) : mValue(value) {}
    unsigned int load() const { return mValue; }
    void store(unsigned int value) { mValue = value; }
    unsigned int increment()
    {
#ifdef __ghs__
        unsigned int previous;
        do
        {
            previous = __LWARX(&mValue, 0);
            __DCBST(&mValue, 0);
        } while (!__STWCX(&mValue, 0, previous + 1));
        return previous;
#elif defined(__powerpc__) || defined(__PPC__)
        unsigned int previous;
        unsigned int next;
        __asm__ __volatile__("1:\n\tlwarx %0, 0, %2\n\tdcbst 0, %2\n\taddi %1, %0, 1\n\tstwcx. %1, 0, %2\n\tbne- 1b"
                             : "=&b"(previous), "=&r"(next)
                             : "r"(&mValue)
                             : "cr0", "memory");
        return previous;
#else
        return static_cast<unsigned int>(OSAddAtomic(reinterpret_cast<volatile int32_t*>(&mValue), 1));
#endif
    }
    unsigned int fetchOr(unsigned int mask)
    {
#ifdef __ghs__
        unsigned int previous;
        do
        {
            previous = __LWARX(&mValue, 0);
            __DCBST(&mValue, 0);
        } while (!__STWCX(&mValue, 0, previous | mask));
        return previous;
#elif defined(__powerpc__) || defined(__PPC__)
        unsigned int previous;
        unsigned int next;
        __asm__ __volatile__("1:\n\tlwarx %0, 0, %2\n\tdcbst 0, %2\n\tor %1, %0, %3\n\tstwcx. %1, 0, %2\n\tbne- 1b"
                             : "=&r"(previous), "=&r"(next)
                             : "r"(&mValue), "r"(mask)
                             : "cr0", "memory");
        return previous;
#else
        uint32_t previous;
        do
            previous = mValue;
        while (!OSCompareAndSwapAtomicEx(&mValue, previous, previous | mask, &previous));
        return previous;
#endif
    }
    unsigned int fetchAnd(unsigned int mask)
    {
#ifdef __ghs__
        unsigned int previous;
        do
        {
            previous = __LWARX(&mValue, 0);
            __DCBST(&mValue, 0);
        } while (!__STWCX(&mValue, 0, previous & mask));
        return previous;
#elif defined(__powerpc__) || defined(__PPC__)
        unsigned int previous;
        unsigned int next;
        __asm__ __volatile__("1:\n\tlwarx %0, 0, %2\n\tdcbst 0, %2\n\tand %1, %0, %3\n\tstwcx. %1, 0, %2\n\tbne- 1b"
                             : "=&r"(previous), "=&r"(next)
                             : "r"(&mValue), "r"(mask)
                             : "cr0", "memory");
        return previous;
#else
        uint32_t previous;
        do
            previous = mValue;
        while (!OSCompareAndSwapAtomicEx(&mValue, previous, previous & mask, &previous));
        return previous;
#endif
    }
    bool compareExchange(unsigned int expected, unsigned int desired, unsigned int* original)
    {
#ifdef __ghs__
        unsigned int previous;
        do
        {
            previous = __LWARX(&mValue, 0);
            if (previous != expected)
            {
                *original = previous;
                return false;
            }
            __DCBST(&mValue, 0);
        } while (!__STWCX(&mValue, 0, desired));
        *original = previous;
        return true;
#elif defined(__powerpc__) || defined(__PPC__)
        unsigned int previous;
        __asm__ __volatile__(
            "1:\n\tlwarx %0, 0, %1\n\tcmplw %0, %2\n\tbne- 2f\n\tdcbst 0, %1\n\tstwcx. %3, 0, %1\n\tbne- 1b\n2:"
            : "=&r"(previous)
            : "r"(&mValue), "r"(expected), "r"(desired)
            : "cr0", "memory");
        *original = previous;
        return previous == expected;
#else
        uint32_t previous;
        BOOL exchanged = OSCompareAndSwapAtomicEx(&mValue, expected, desired, &previous);
        *original = previous;
        return exchanged != FALSE;
#endif
    }

private:
#ifdef __ghs__
    unsigned int mValue;
#else
    volatile uint32_t mValue;
#endif
};

template <typename T> class IDelegate1
{
public:
    virtual void invoke(T) = 0;
    virtual IDelegate1* clone(Heap*) const { return 0; }
};

template <typename T1, typename T2> class IDelegate2
{
public:
    virtual void invoke(T1, T2) = 0;
    virtual IDelegate2* clone(Heap*) const { return 0; }
};

template <typename Owner, typename T1, typename T2> class Delegate2 : public IDelegate2<T1, T2>
{
public:
    typedef void (Owner::*Method)(T1, T2);
    Delegate2(Owner* owner, Method method) : mOwner(owner), mMethod(method) {}
    virtual void invoke(T1 first, T2 second)
    {
        if (mOwner != 0 && mMethod != 0)
            (mOwner->*mMethod)(first, second);
    }
    virtual IDelegate2<T1, T2>* clone(Heap* heap) const { return new (heap) Delegate2(mOwner, mMethod); }

private:
    Owner* mOwner;
    Method mMethod;
};

template <typename T> class DelegateEvent
{
public:
    class Slot : public ListNode
    {
    public:
        Slot() : mDelegate(0) {}
        IDelegate1<T>* mDelegate;
    };
    DelegateEvent() : mEnabled(true) {}
    ~DelegateEvent() {}
    void invoke(T argument)
    {
        for (ListNode* node = mSlots.next(); node != &mSlots; node = node->next())
            static_cast<Slot*>(node)->mDelegate->invoke(argument);
    }

private:
    ListNode mSlots;
    bool mEnabled;
};

class CriticalSection : public IDisposer
{
public:
    CriticalSection();
    CriticalSection(Heap*);
    virtual ~CriticalSection();
    void lock();
    void unlock();
    bool tryLock();
    void dump(StackSymbolResolver*);

private:
    OSFastMutex mCriticalSectionInner;
};

class Heap : public IDisposer
{
    friend class ExpHeap;
    friend class HeapMgr;
    friend class PrintFormatter;

public:
    enum HeapDirection
    {
        cHeapDirection_Forward = 1,
        cHeapDirection_Reverse = -1
    };
    enum Flag
    {
        cFlag_EnableLock = 1,
        cFlag_Disposing = 2,
        cFlag_EnableWarning = 4
    };
    Heap(const SafeStringBase<char>&, Heap*, void*, unsigned int, HeapDirection, bool);
    virtual ~Heap();
    static const RuntimeTypeInfo::Interface* getRuntimeTypeInfoStatic()
    {
        static const RuntimeTypeInfo::Root typeInfo;
        return &typeInfo;
    }
    static bool checkDerivedRuntimeTypeInfoStatic(const RuntimeTypeInfo::Interface* type)
    {
        return type == getRuntimeTypeInfoStatic();
    }
    virtual bool checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface*) const;
    virtual const RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    virtual void destroy() = 0;
    virtual unsigned int adjust() = 0;
    virtual void* tryAlloc(unsigned int, int) = 0;
    virtual void free(void*) = 0;
    virtual void* resizeFront(void*, unsigned int) = 0;
    virtual void* resizeBack(void*, unsigned int) = 0;
    virtual void* tryRealloc(void*, unsigned int, int);
    virtual void freeAll() = 0;
    virtual void* getStartAddress() const = 0;
    virtual void* getEndAddress() const = 0;
    virtual unsigned int getSize() const = 0;
    virtual unsigned int getFreeSize() const = 0;
    virtual unsigned int getMaxAllocatableSize(int) const = 0;
    virtual bool isInclude(const void*) const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool isFreeable() const = 0;
    virtual bool isResizable() const = 0;
    virtual bool isAdjustable() const = 0;
    virtual void dump() const;
    virtual void dumpYAML(WriteStream&, int) const;
    virtual void pushBackChild_(Heap*);
    void dumpTreeYAML(WriteStream&, int) const;
    const SafeStringBase<char>& getName() const { return mName; }
    Heap* getParent() const { return mParent; }
    HeapDirection getDirection() const { return mDirection; }
    bool isEnableLock() const { return mFlag.isOn(cFlag_EnableLock); }
    Heap* findContainHeap_(const void*);
    void dispose_(const void*, const void*);
    void eraseChild_(Heap*);
    void destruct_();
    void appendDisposer_(IDisposer*);
    void removeDisposer_(IDisposer*);
    void checkAccessThread_() const;

protected:
    SafeStringBase<char> mName;
    void* mStart;
    unsigned int mSize;
    Heap* mParent;
    OffsetList<Heap> mChildren;
    ListNode mListNode;
    OffsetList<IDisposer> mDisposerList;
    HeapDirection mDirection;
    CriticalSection mCS;
    BitFlag<unsigned short> mFlag;
    unsigned short mHeapCheckTag;
};

class MemBlock
{
public:
    MemBlock() : mListNode(), mMagic(0), mOffset(0), mSize(0) {}
    explicit MemBlock(unsigned int size) : mListNode(), mMagic(0), mOffset(0), mSize(size) {}
    unsigned char* memory() const
    {
        return const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(this)) + sizeof(MemBlock) + mOffset;
    }
    void setOffset(unsigned short offset)
    {
        mOffset = offset;
        if (offset)
            reinterpret_cast<uintptr_t*>(memory())[-1] = reinterpret_cast<uintptr_t>(this) + 1;
    }
    static MemBlock* findManageArea(void* memory)
    {
        uintptr_t tag = static_cast<uintptr_t*>(memory)[-1];
        if (tag & 1)
            return reinterpret_cast<MemBlock*>(tag - 1);
        return reinterpret_cast<MemBlock*>(static_cast<unsigned char*>(memory) - sizeof(MemBlock));
    }

    ListNode mListNode;
    unsigned short mMagic;
    unsigned short mOffset;
    unsigned int mSize;
};

class ExpHeap : public Heap
{
    friend class PrintFormatter;

public:
    enum FindMode
    {
        cFindMode_FirstFit = 0,
        cFindMode_BestFit = 1,
        cFindMode_MaxSize = 2
    };
    enum AllocMode
    {
        cAllocMode_FirstFit = 0,
        cAllocMode_BestFit = 1
    };
    ExpHeap(const SafeStringBase<char>&, Heap*, void*, unsigned int, HeapDirection, bool);
    virtual ~ExpHeap();
    static const RuntimeTypeInfo::Interface* getRuntimeTypeInfoStatic()
    {
        static const RuntimeTypeInfo::Derive<Heap> typeInfo;
        return &typeInfo;
    }
    static bool checkDerivedRuntimeTypeInfoStatic(const RuntimeTypeInfo::Interface* type)
    {
        if (type == getRuntimeTypeInfoStatic())
            return true;
        return Heap::checkDerivedRuntimeTypeInfoStatic(type);
    }
    virtual bool checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface*) const;
    virtual const RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    static ExpHeap* tryCreate(unsigned int, const SafeStringBase<char>&, Heap*, HeapDirection, bool);
    static ExpHeap* create(unsigned int, const SafeStringBase<char>&, Heap*, HeapDirection, bool);
    static ExpHeap* tryCreate(void*, unsigned int, const SafeStringBase<char>&, bool);
    static int getManagementAreaSize(int);
    virtual void destroy();
    virtual unsigned int adjust();
    virtual void* tryAlloc(unsigned int, int);
    virtual void free(void*);
    virtual void* resizeFront(void*, unsigned int);
    virtual void* resizeBack(void*, unsigned int);
    virtual void* tryRealloc(void*, unsigned int, int);
    virtual void freeAll();
    virtual void* getStartAddress() const;
    virtual void* getEndAddress() const;
    virtual unsigned int getSize() const;
    virtual unsigned int getFreeSize() const;
    virtual unsigned int getMaxAllocatableSize(int) const;
    virtual bool isInclude(const void*) const;
    virtual bool isEmpty() const;
    virtual bool isFreeable() const;
    virtual bool isResizable() const;
    virtual bool isAdjustable() const;
    virtual void dump() const;
    virtual void dumpYAML(WriteStream&, int) const;
    virtual void setAllocMode(AllocMode);
    virtual AllocMode getAllocMode() const;
    unsigned int getAllocatedSize(void*);
    void dumpUseList() const;
    void dumpFreeList() const;
    bool tryCheckFreeList() const;
    bool tryCheckUseList() const;

private:
    void pushToFreeList_(MemBlock*);
    static void createMaxSizeFreeMemBlock_(ExpHeap*);
    static void doCreate(ExpHeap*, Heap*);
    static int compareMemBlockAddr_(const MemBlock*, const MemBlock*);
    MemBlock* findLastMemBlockIfFree_();
    unsigned int adjustBack_();
    MemBlock* findFirstMemBlockIfFree_();
    unsigned int adjustFront_();
    MemBlock* findFreeMemBlockFromHead_(unsigned int, FindMode) const;
    MemBlock* findFreeMemBlockFromHead_(unsigned int, int, FindMode) const;
    void pushToUseList_(MemBlock*);
    MemBlock* allocFromHead_(unsigned int);
    MemBlock* allocFromHead_(unsigned int, int);
    MemBlock* findFreeMemBlockFromTail_(unsigned int, FindMode) const;
    MemBlock* findFreeMemBlockFromTail_(unsigned int, int, FindMode) const;
    MemBlock* allocFromTail_(unsigned int);
    MemBlock* allocFromTail_(unsigned int, int);
    void* realloc_(void*, unsigned char*, unsigned int, unsigned int, int);

    AllocMode mAllocMode;
    OffsetList<MemBlock> mFreeList;
    OffsetList<MemBlock> mUseList;
};

class FindContainHeapCache
{
public:
    FindContainHeapCache();
    bool tryRemoveHeap(Heap*);

    AtomicU32 mHeap;
};

class Arena
{
public:
    unsigned char* mStart;
    unsigned int mSize;
};

class HeapMgr
{
    friend class Heap;

public:
    struct AllocCallbackArg
    {
        Heap* heap;
        void* address;
        unsigned int size;
    };
    struct FreeCallbackArg
    {
        Heap* heap;
        void* address;
    };
    struct AllocFailedCallbackArg
    {
        Heap* heap;
        unsigned int requestSize;
        int requestAlignment;
        unsigned int allocSize;
        int allocAlignment;
    };
    typedef IDelegate1<const AllocFailedCallbackArg*> IAllocFailedCallback;
    typedef FixedPtrArray<Heap, 4> RootHeaps;
    typedef FixedPtrArray<Heap, 4> IndependentHeaps;

    HeapMgr();
    virtual ~HeapMgr();
    static void initialize(Arena*);
    static void destroy();
    Heap* findContainHeap(const void*) const;
    Heap* findHeapByName(const SafeStringBase<char>&, int) const;
    Heap* getCurrentHeap() const;
    static bool isContainedInAnyHeap(const void*);
    static void dumpTreeYAML(WriteStream&);
    void setAllocFailedCallback(IAllocFailedCallback*);
    void setCurrentHeap_(Heap*);
    IAllocFailedCallback* getAllocFailedCallback() const { return mAllocFailedCallback; }
    static HeapMgr* instance() { return sInstancePtr; }
    static Heap* getRootHeap(int index) { return sRootHeaps.at(index); }
    static CriticalSection* getHeapTreeLockCS() { return &sHeapTreeLockCS; }
    static unsigned short issueHeapCheckTag() { return static_cast<unsigned short>(sHeapCheckTag.increment()); }

private:
    static void createRootHeap_();
    static void initializeImpl_();
    static Heap* findHeapByName_(Heap*, const SafeStringBase<char>&, int*);
    static void removeFromFindContainHeapCache_(Heap*);

    IAllocFailedCallback* mAllocFailedCallback;

    static Arena* sArena;
    static HeapMgr sInstance;
    static HeapMgr* sInstancePtr;
    static AtomicU32 sHeapCheckTag;
    static RootHeaps sRootHeaps;
    static CriticalSection sHeapTreeLockCS;
    static IndependentHeaps sIndependentHeaps;
};

void* allocateFromSeadHeap(unsigned int, Heap*);
void* allocateFromSeadHeap(unsigned int);
void* allocateFromSeadHeap(unsigned int, Heap*, int);
void freeToSeadHeap(void*);
unsigned int getArraySize(void*);

template <typename T> T* NewArray(unsigned int count, Heap* heap)
{
    void* memory = allocateFromSeadHeap(sizeof(T) * count, heap);
    if (!memory)
        return 0;
    T* array = static_cast<T*>(memory);
    for (unsigned int index = 0; index < count; ++index)
        new (array + index) T();
    return array;
}

template <typename T> T* NewArray(unsigned int count, Heap* heap, int alignment)
{
    void* memory = allocateFromSeadHeap(sizeof(T) * count, heap, alignment);
    if (!memory)
        return 0;
    T* array = static_cast<T*>(memory);
    for (unsigned int index = 0; index < count; ++index)
        new (array + index) T();
    return array;
}

template <typename T> void DeleteArray(T* array)
{
    if (!array)
        return;
    unsigned int count = getArraySize(array) / sizeof(T);
    while (count)
        array[--count].~T();
    freeToSeadHeap(array);
}

template <typename T> inline bool Buffer<T>::tryAllocBuffer(int size, Heap* heap, int alignment)
{
    if (size > 0)
    {
        if (!heap)
            heap = HeapMgr::instance()->getCurrentHeap();
        T* buffer = static_cast<T*>(heap->tryAlloc(sizeof(T) * size, alignment));
        if (buffer)
        {
            for (int index = 0; index < size; ++index)
                new (buffer + index) T();
        }
        if (buffer)
        {
            mSize = size;
            mBuffer = buffer;
            return true;
        }
    }
    return false;
}

template <typename T> inline void Buffer<T>::freeBuffer()
{
    if (mBuffer)
    {
        for (int index = 0; index < mSize; ++index)
            mBuffer[index].~T();
        HeapMgr::instance()->findContainHeap(mBuffer)->free(mBuffer);
        mBuffer = 0;
        mSize = 0;
    }
}

class Mutex : public IDisposer
{
public:
    Mutex();
    Mutex(Heap*);
    virtual ~Mutex();
    void lock();
    void unlock();
    bool tryLock();

private:
    OSFastMutex mCriticalSectionInner;
};

template <typename Lock> class ScopedLock
{
public:
    ScopedLock(Lock* lock) : mLock(lock)
    {
        if (mLock)
            mLock->lock();
    }
    ~ScopedLock()
    {
        if (mLock)
            mLock->unlock();
    }

private:
    Lock* mLock;
};

template <typename Lock> class ConditionalScopedLock
{
public:
    ConditionalScopedLock(Lock* lock, bool condition) : mLock(0)
    {
        if (condition)
        {
            mLock = lock;
            mLock->lock();
        }
    }
    ~ConditionalScopedLock()
    {
        if (mLock)
            mLock->unlock();
    }

private:
    Lock* mLock;
};

class TickSpan
{
public:
    TickSpan(long long ticks = 0) : mTicks(ticks) {}
    long long toNanoSeconds() const;
    long long toMicroSeconds() const
    {
        const long long cMicroSecondsLimit = LLONG_MAX / 1000000;
        const long long cMilliSecondsLimit = LLONG_MAX / 1000;
        if (-cMicroSecondsLimit <= mTicks && mTicks < cMicroSecondsLimit)
            return mTicks * 1000000 / cFrequency;
        if (-cMilliSecondsLimit <= mTicks && mTicks < cMilliSecondsLimit)
            return mTicks * 1000 / cFrequency * 1000;
        return mTicks / cFrequency * 1000000;
    }
    void setNanoSeconds(long long);
    static TickSpan makeFromMicroSeconds(long long microSeconds)
    {
        return TickSpan(microSeconds * cFrequency / 1000 / 1000);
    }
    static TickSpan makeFromMilliSeconds(long long milliSeconds) { return TickSpan(milliSeconds * cFrequency / 1000); }
    long long getTicks() const { return mTicks; }

    static const long long cFrequency;

private:
    long long mTicks;
};

class Event : public IDisposer
{
public:
    Event();
    Event(Heap*);
    Event(bool);
    Event(Heap*, bool);
    virtual ~Event();
    void initialize(bool);
    void wait();
    bool wait(TickSpan);
    void setSignal();
    void resetSignal();

private:
    unsigned int mStorage[9];
};

class MessageQueue
{
public:
    enum BlockType
    {
        cBlock = 0,
        cNoBlock = 1
    };
    MessageQueue();
    ~MessageQueue();
    void allocate(int, Heap*);
    void free();
    bool push(int, BlockType);
    int pop(BlockType);
    bool jam(int, BlockType);
    int peek(BlockType) const;

private:
    OSMessage* mBuffer;
    OSMessageQueue mQueue;
};

class Thread : public IDisposer
{
    friend class ThreadMgr;

public:
    Thread(Heap*, OSThread*);
    Thread(const SafeStringBase<char>&, Heap*, int, MessageQueue::BlockType, int, int, int);
    virtual ~Thread();
    virtual void destroy();
    virtual bool sendMessage(int, MessageQueue::BlockType);
    virtual int recvMessage(MessageQueue::BlockType);
    virtual MessageQueue* getMessageQueue() const;
    virtual unsigned int start();
    virtual void quit(bool);
    virtual void waitDone();
    virtual void quitAndDestroySingleThread(bool);
    virtual void quitAndWaitDoneSingleThread(bool);
    virtual void setPriority(int);
    virtual int getPriority() const;
    virtual MessageQueue::BlockType getBlockType() const;
    virtual int getStackSize() const;
    virtual int calcStackUsedSizePeak() const;
    virtual void calc_(int) = 0;
    virtual void* getStackCheckStartAddress_() const;
    void setAffinity(const CoreIdMask&);
    static int cafeThreadFunc_(int, void*);
    static void sleep(TickSpan);
    static void yield();
    void run_();
    void checkStackOverFlow(const char*, int) const;
    void checkStackEndCorruption(const char*, int) const;
    void checkStackPointerOverFlow(const char*, int) const;
    void setStackOverflowExceptionEnable(bool);
    void initStackCheck_();
    void initStackCheckWithCurrentStackPointer_();
    Heap* getCurrentHeap() const { return mCurrentHeap; }
    void setCurrentHeap_(Heap* heap) { mCurrentHeap = heap; }
    FindContainHeapCache* getFindContainHeapCache() { return &mFindContainHeapCache; }
    bool isDone() const { return mState == cState_Terminated || mState == cState_Released; }

    enum
    {
        cDefaultQuitMsg = INT_MAX,
        cDefaultMessageQueueSize = 32,
        cThreadAlignment = 8
    };
    static const uintptr_t cUncreatedStackEnd = static_cast<uintptr_t>(-1);

protected:
    enum State
    {
        cState_Initialized = 0,
        cState_Running = 1,
        cState_Quitting = 2,
        cState_Terminated = 3,
        cState_Released = 4
    };

    SafeStringBase<char> mName;
    MessageQueue mMessageQueue;
    int mStackSize;
    TListNode<Thread*> mListNode;
    Heap* mCurrentHeap;
    FindContainHeapCache mFindContainHeapCache;
    MessageQueue::BlockType mBlockType;
    int mQuitMsg;
    unsigned int mId;
    State mState;
    CoreIdMask mAffinity;
    OSThread* mThread;
    unsigned char* mStack;
    int mPriority;
};

class MainThread : public Thread
{
public:
    MainThread(Heap* heap, OSThread* thread) : Thread(heap, thread) {}
    virtual ~MainThread();
    virtual void destroy();
    virtual void quit(bool);
    virtual void waitDone();
    virtual void quitAndDestroySingleThread(bool);
    virtual void setPriority(int);
    virtual void calc_(int);
};

class DelegateThread : public Thread
{
public:
    DelegateThread(const SafeStringBase<char>&, IDelegate2<Thread*, int>*, Heap*, int, MessageQueue::BlockType, int,
                   int, int);
    virtual ~DelegateThread();
    virtual void calc_(int);

private:
    IDelegate2<Thread*, int>* mDelegate;
};

class ThreadMgr
{
public:
    class SingletonDisposer_ : public IDisposer
    {
    public:
        explicit SingletonDisposer_(Heap* heap) : IDisposer(heap, cHeapNullOption_UseSpecifiedOrCurrentHeap) {}
        virtual ~SingletonDisposer_();

        static SingletonDisposer_* sStaticDisposer;
    };
    typedef TList<Thread*> ThreadList;

    ThreadMgr();
    virtual ~ThreadMgr();
    static ThreadMgr* instance() { return sInstance; }
    static ThreadMgr* createInstance(Heap*);
    static void deleteInstance();
    void initialize(Heap*);
    void destroy();
    Thread* getCurrentThread() const;
    MainThread* getMainThread() const { return mMainThread; }
    bool isMainThread() const;
    static void waitDoneMultipleThread(Thread* const*, int);
    static void quitAndWaitDoneMultipleThread(Thread**, int, bool);
    static void checkCurrentThreadStackOverFlow(const char*, int);
    static void checkCurrentThreadStackEndCorruption(const char*, int);
    static void checkCurrentThreadStackPointerOverFlow(const char*, int);

private:
    friend class Thread;
    friend class HeapMgr;

    void initMainThread_(Heap*);
    void destroyMainThread_();
    static unsigned int getCurrentThreadID_();

    unsigned int mSingletonDisposerBuf_[sizeof(SingletonDisposer_) / sizeof(unsigned int)];
    ThreadList mList;
    mutable CriticalSection mListCS;
    MainThread* mMainThread;

    static ThreadMgr* sInstance;
};

class PrintFormatter;

class PrintOutput
{
public:
    virtual ~PrintOutput();
    virtual void write(const char*, int) = 0;
    void writeLineBreak();
    void operator<<(PrintFormatter&);
    void __ls(PrintFormatter&);
};

class BufferingPrintOutput : public PrintOutput
{
public:
    BufferingPrintOutput(StreamSrc* src, void* buffer, unsigned int size) : mStreamSrc(src, buffer, size) {}
    virtual ~BufferingPrintOutput();
    virtual void write(const char*, int);

private:
    BufferMultiByteNullTerminatedTextWriteStreamSrc mStreamSrc;
};

class StringPrintOutput : public PrintOutput
{
public:
    StringPrintOutput(BufferedSafeStringBase<char>* string) : mString(string), mPosition(0) {}
    virtual ~StringPrintOutput();
    virtual void write(const char*, int);

protected:
    BufferedSafeStringBase<char>* mString;
    int mPosition;
};

class StringCutOffPrintOutput : public StringPrintOutput
{
public:
    StringCutOffPrintOutput(BufferedSafeStringBase<char>* string) : StringPrintOutput(string) {}
    virtual ~StringCutOffPrintOutput();
    virtual void write(const char*, int);
};

class StreamPrintOutput : public PrintOutput
{
public:
    StreamPrintOutput(StreamSrc* src) : mSrc(src) {}
    virtual ~StreamPrintOutput();
    virtual void write(const char*, int);

private:
    StreamSrc* mSrc;
};

class PrintFormatter
{
public:
    template <typename T, template <typename> class Wrapper> class OutImpl
    {
    public:
        static void out(const Wrapper<T>&, const char*, PrintOutput*);
    };

    PrintFormatter(const char*, PrintOutput*);
    void flush();
    void flushWithLineBreak();
    PrintFormatter& operator<<(const char*);
    PrintFormatter& __ls(const char*);
    static void outputString_(const char*, PrintOutput*, const char*, int);
    static void outputPtr_(const char*, PrintOutput*, unsigned int);
    template <typename T> static void out(const T&, const char*, PrintOutput*);
    template <typename T> static void out(const T*, const char*, PrintOutput*);

private:
    enum
    {
        cMaxSpecifierLength = 30
    };

    bool proceedToFormatMark_(char*);
    const char* mFormat;
    PrintOutput* mOutput;
    int mPosition;
    int mLength;
    bool mPassThrough;
};

class BufferingPrintFormatter : public PrintFormatter
{
public:
    BufferingPrintFormatter();
    BufferingPrintFormatter(const char*);
};

class StringPrintFormatter : public PrintFormatter
{
public:
    StringPrintFormatter(BufferedSafeStringBase<char>*);
    StringPrintFormatter(BufferedSafeStringBase<char>*, const char*);

private:
    StringPrintOutput mStringOutput;
};

class StringCutOffPrintFormatter : public PrintFormatter
{
public:
    StringCutOffPrintFormatter(BufferedSafeStringBase<char>*);
    StringCutOffPrintFormatter(BufferedSafeStringBase<char>*, const char*);

private:
    StringCutOffPrintOutput mStringOutput;
};

class StreamPrintFormatter : public PrintFormatter
{
public:
    StreamPrintFormatter(StreamSrc*);
    StreamPrintFormatter(StreamSrc*, const char*);
    void flushAndWriteNullChar();

private:
    StreamPrintOutput mStreamOutput;
};

class PrintConfig
{
public:
    struct PrintEventArg
    {
        PrintEventArg(const char* value = 0, int printLevel = 0) : text(value), level(printLevel) {}
        const char* text;
        int level;
    };
    static void registerFinalCallback(IDelegate1<const PrintEventArg&>*);
    static void execCallbacks(const PrintEventArg&);
    static void registerCallback(DelegateEvent<const PrintEventArg&>::Slot&);
    static void unregisterCallback(DelegateEvent<const PrintEventArg&>::Slot&);
};

namespace StringUtil
{
enum CardinalNumber
{
    cAuto = -1,
    cDecimal = 10,
    cHexadecimal = 16
};
struct Char16Pair
{
    wchar_t from;
    wchar_t to;
};
bool tryParseU64(unsigned long long*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseS64(long long*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseU32(unsigned int*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseS32(int*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseU16(unsigned short*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseS16(short*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseU8(unsigned char*, const SafeStringBase<char>&, CardinalNumber);
bool tryParseS8(signed char*, const SafeStringBase<char>&, CardinalNumber);
int parseS32(const SafeStringBase<char>&, CardinalNumber);
int vsw16printf(wchar_t*, unsigned int, const wchar_t*, va_list);
int vsnprintf(char*, unsigned int, const char*, va_list);
int convertUtf16ToSjis(char*, unsigned int, const wchar_t*, int);
int convertUtf16ToUtf8(char*, unsigned int, const wchar_t*, int);
int convertUtf8ToSjis(char*, unsigned int, const char*, int);
int compareChar16Pair(const Char16Pair&, const wchar_t&);
wchar_t replace(wchar_t, const Buffer<const Char16Pair>&);
wchar_t toUpperCapital(wchar_t);
void toUpperCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>*);
wchar_t toLowerCapital(wchar_t);
void toLowerCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>*);
bool tryParseF32(float*, const SafeStringBase<char>&);
bool tryParseF64(double*, const SafeStringBase<char>&);
unsigned long long parseU64(const SafeStringBase<char>&, CardinalNumber);
long long parseS64(const SafeStringBase<char>&, CardinalNumber);
unsigned int parseU32(const SafeStringBase<char>&, CardinalNumber);
unsigned short parseU16(const SafeStringBase<char>&, CardinalNumber);
short parseS16(const SafeStringBase<char>&, CardinalNumber);
unsigned char parseU8(const SafeStringBase<char>&, CardinalNumber);
signed char parseS8(const SafeStringBase<char>&, CardinalNumber);
float parseF32(const SafeStringBase<char>&);
double parseF64(const SafeStringBase<char>&);
void wcs16cpy(wchar_t*, unsigned int, const wchar_t*);
int sw16printf(wchar_t*, unsigned int, const wchar_t*, ...);
int snprintf(char*, unsigned int, const char*, ...);
int convertSjisToUtf16(wchar_t*, unsigned int, const char*, int);
int convertUtf8ToUtf16(wchar_t*, unsigned int, const char*, int);
int convertSjisToUtf8(char*, unsigned int, const char*, int);

namespace __N_18_peadStringUtil_cpp_6714fef4
{
struct OutBuffer16
{
    OutBuffer16(wchar_t* text, int size) : buffer(text), capacity(size), length(0), overflow(false) {}
    void put(wchar_t character)
    {
        if (length < capacity)
            buffer[length++] = character;
        else
            overflow = true;
    }
    void put(wchar_t character, unsigned int count)
    {
        for (; count != 0; --count)
        {
            if (length >= capacity)
            {
                overflow = true;
                break;
            }
            buffer[length++] = character;
        }
    }

    wchar_t* buffer;
    int capacity;
    int length;
    bool overflow;
};

class FormatParser16
{
public:
    enum Flag
    {
        cFlagLeftJustify = 1,
        cFlagForceSign = 2,
        cFlagZeroPad = 4,
        cFlagBlankSign = 8,
        cFlagAlternate = 16
    };
    enum Size
    {
        cSizeChar = 0,
        cSizeShort = 1,
        cSizeInt = 2,
        cSizeLong = 3,
        cSizeLongLong = 4
    };
    enum
    {
        cBaseOctal = 8,
        cBaseDecimal = 10,
        cBaseHexadecimal = 16,
        cOctalDigitBits = 3,
        cOctalDigitMask = 7,
        cHexDigitBits = 4,
        cHexDigitMask = 15,
        cIntegerDigitBufferLength = 32,
        cIntegerDigitCapacity = cIntegerDigitBufferLength - 1,
        cPointerPrecision = 8,
        cDefaultFloatPrecision = 6
    };

    FormatParser16(va_list);
    void clear_();
    const wchar_t* parseFlag_(const wchar_t*);
    const wchar_t* parseWidth_(const wchar_t*);
    const wchar_t* parsePrecision_(const wchar_t*);
    const wchar_t* parseSize_(const wchar_t*);
    void printInteger_(OutBuffer16*, unsigned int, bool, bool);
    void printCharacter_(OutBuffer16*);
    void printString_(OutBuffer16*);
    void printFloat_(OutBuffer16*, wchar_t, bool);
    int execute(OutBuffer16*, const wchar_t*);
    template <typename T> static void printIntegerInverseString_(OutBuffer16*, unsigned int, bool, T);
    template <typename U, typename S, typename A>
    void printIntegerInverseStringFromArg_(OutBuffer16*, bool*, bool, unsigned int, bool);
    template <typename T> void printStringCore_(OutBuffer16*, const T*);

private:
    va_list mArgs;
    unsigned int mFlags;
    int mWidth;
    int mPrecision;
    bool mHasPrecision;
    Size mSize;
};

template <typename T> struct UnsignedType
{
    typedef T Type;
};
template <> struct UnsignedType<long long>
{
    typedef unsigned long long Type;
};
template <> struct UnsignedType<int>
{
    typedef unsigned int Type;
};
template <> struct UnsignedType<short>
{
    typedef unsigned short Type;
};
template <> struct UnsignedType<signed char>
{
    typedef unsigned char Type;
};

int checkAndConvertDecChar_(char);
int checkAndConvertHexChar_(char);
void tryParseSignImpl_(int*, SafeStringBase<char>::iterator&);
bool tryParsePrefixImpl_(int*, SafeStringBase<char>::iterator&, const SafeStringBase<char>&);
template <typename T> bool tryParseNumberImpl_(T*, SafeStringBase<char>::iterator&, int);
template <typename T> bool tryParseDecImpl_(T*, SafeStringBase<char>::iterator&, int);
template <typename T> bool tryParseImpl_(T*, const SafeStringBase<char>&, int);
template <typename T>
void tryParseUnderDecimalPointImpl_(T*, SafeStringBase<char>::iterator&, T, T);
template <typename T> bool tryParseDecimalFractionImpl_(T*, const SafeStringBase<char>&);
template <typename T> T parseImpl_(const SafeStringBase<char>&, CardinalNumber);
template <typename T> T parseDecimalFractionImpl_(const SafeStringBase<char>&);
}
}

namespace __N_16_peadCoreInfo_cpp_b178e7d4
{
int countNumBits(unsigned int);
}

namespace __N_24_peadTextStreamFormat_cpp_8d174ff4
{
bool isSJISCode_(unsigned char);
}

namespace system
{
void PrintStringImpl(const char*, int);
void PrintString(const char*, int);
void PrintV(const char*, va_list);
void Print(const char*, ...);
}

template <typename T> inline bool isMatchString_(const T* text, const T* pattern, int length)
{
    for (int index = 0; index < length; ++index)
    {
        if (text[index] != pattern[index])
            return false;
    }
    return true;
}
template <typename T>
int replaceStringImpl_(T*, int*, int, const T*, int, const SafeStringBase<T>&, const SafeStringBase<T>&, bool*);
}

inline void* operator new(size_t size, pead::Heap* heap)
{
    return pead::allocateFromSeadHeap(size, heap);
}

inline void operator delete(void* memory, pead::Heap*)
{
    pead::freeToSeadHeap(memory);
}

#endif
