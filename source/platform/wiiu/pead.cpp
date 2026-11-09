#include "platform/wiiu/pead.hpp"

#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <new>

#include <coreinit/cache.h>
#include <coreinit/time.h>
#include <coreinit/core.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <coreinit/event.h>

namespace pead
{
namespace
{
BinaryStreamFormat sBinaryFormat;
TextStreamFormat sTextFormat;
FixedSafeString1024 sTextBuffer;
Mutex sMutex;
IDelegate1<const PrintConfig::PrintEventArg&>* sFinalPrintCallback;
bool sPrintEventEnabled = false;
DelegateEvent<const PrintConfig::PrintEventArg&> sPrintEvent;

template <typename T>
T byteSwap(T value)
{
    union Bytes
    {
        T value;
        unsigned char data[sizeof(T)];
    } source, destination;
    source.value = value;
    for (unsigned int index = 0; index < sizeof(T); ++index)
        destination.data[index] = source.data[sizeof(T) - index - 1];
    return destination.value;
}

template <typename T>
T convertEndian(T value, Endian::Types endian)
{
    return endian == Endian::cLittle ? byteSwap(value) : value;
}

int appendText(char* destination, unsigned int capacity, const char* source, int length)
{
    if (!destination || capacity == 0)
        return 0;
    unsigned int current = static_cast<unsigned int>(strlen(destination));
    unsigned int requested = length < 0 ? static_cast<unsigned int>(strlen(source)) : static_cast<unsigned int>(length);
    unsigned int available = current < capacity ? capacity - current - 1 : 0;
    unsigned int copied = requested < available ? requested : available;
    if (copied)
        memcpy(destination + current, source, copied);
    destination[current + copied] = 0;
    return static_cast<int>(copied);
}

}

void ListNode::insertBack_(ListNode* node)
{
    ListNode* next = mNext;
    mNext = node;
    node->mPrev = this;
    node->mNext = next;
    if (next)
        next->mPrev = node;
}

void ListNode::insertFront_(ListNode* node)
{
    ListNode* prev = mPrev;
    mPrev = node;
    node->mPrev = prev;
    node->mNext = this;
    if (prev)
        prev->mNext = node;
}

void ListNode::erase_()
{
    if (mPrev)
        mPrev->mNext = mNext;
    if (mNext)
        mNext->mPrev = mPrev;
    mNext = 0;
    mPrev = 0;
}

ListNode* ListImpl::nth(int index) const
{
    if (index < 0 || index >= mCount)
        return 0;
    ListNode* node = mStartEnd.next();
    while (index--)
        node = node->next();
    return node;
}

void ListImpl::sort(int offset, int (*compare)(const void*, const void*))
{
    if (mCount <= 1)
        return;

    ListNode* node = nth(1);
    while (node != &mStartEnd)
    {
        ListNode* next = node->next();
        if (compare(reinterpret_cast<const char*>(node->prev()) - offset, reinterpret_cast<const char*>(node) - offset)
            > 0)
        {
            ListNode* insertion = node->prev();
            while (insertion != &mStartEnd
                   && compare(reinterpret_cast<const char*>(insertion) - offset,
                              reinterpret_cast<const char*>(node) - offset)
                          > 0)
                insertion = insertion->prev();
            node->erase_();
            insertion->insertBack_(node);
        }
        node = next;
    }
}

void ListImpl::clear()
{
    ListNode* node = mStartEnd.mNext;
    while (node != &mStartEnd)
    {
        ListNode* current = node;
        node = node->mNext;
        current->mNext = 0;
        current->mPrev = 0;
    }
    mStartEnd.mPrev = &mStartEnd;
    mStartEnd.mNext = &mStartEnd;
    mCount = 0;
}

void PtrArrayImpl::setBuffer(int capacity, void* buffer)
{
    if (capacity < 1)
        return;
    if (!buffer)
        return;
    mPtrNumMax = capacity;
    mPtrs = static_cast<void**>(buffer);
    mPtrNum = 0;
}

bool PtrArrayImpl::tryAllocBuffer(int capacity, Heap* heap, int alignment)
{
    if (capacity > 0)
    {
        unsigned int size = sizeof(void*) * capacity;
        unsigned char* buffer = static_cast<unsigned char*>(allocateFromSeadHeap(size, heap, alignment));
        if (!buffer)
            return false;
        for (unsigned char* byte = buffer; size != 0; --size, ++byte)
            new (byte) unsigned char();
        if (buffer)
        {
            setBuffer(capacity, buffer);
            return true;
        }
    }
    return false;
}

bool PtrArrayImpl::allocBuffer(int capacity, Heap* heap, int alignment)
{
    return tryAllocBuffer(capacity, heap, alignment);
}

void PtrArrayImpl::freeBuffer()
{
    if (mPtrs)
    {
        DeleteArray(reinterpret_cast<unsigned char*>(mPtrs));
        mPtrs = 0;
        mPtrNum = 0;
        mPtrNumMax = 0;
    }
}

void PtrArrayImpl::erase(int index, int count)
{
    if (index >= 0 && count >= 0)
    {
        int end = index + count;
        if (end <= mPtrNum)
        {
            if (end < mPtrNum)
                memmove(mPtrs + index, mPtrs + end, sizeof(void*) * (mPtrNum - end));
            mPtrNum -= count;
        }
    }
}

bool PtrArrayImpl::checkInsert(int index, int count)
{
    return index >= 0 && mPtrNum + count <= mPtrNumMax && index <= mPtrNum;
}

void PtrArrayImpl::insert(int index, void* value)
{
    if (checkInsert(index, 1))
    {
        if (index < mPtrNum)
            memmove(mPtrs + index + 1, mPtrs + index, sizeof(void*) * (mPtrNum - index));
        mPtrs[index] = value;
        ++mPtrNum;
    }
}

int PtrArrayImpl::binarySearch(const void* value, int (*compare)(const void*, const void*)) const
{
    if (mPtrNum == 0)
        return -1;
    int low = 0;
    int high = mPtrNum - 1;
    while (low < high)
    {
        int middle = (low + high) / 2;
        int result = compare(mPtrs[middle], value);
        if (result == 0)
            return middle;
        if (result < 0)
            low = middle + 1;
        else
            high = middle;
    }
    if (compare(mPtrs[low], value) == 0)
        return low;
    return -1;
}

void pead::Random::init(unsigned int seed)
{
    mContext[0] = (seed ^ (seed >> 30)) * 0x6c078965u + 1;
    mContext[1] = (mContext[0] ^ (mContext[0] >> 30)) * 0x6c078965u + 2;
    mContext[2] = (mContext[1] ^ (mContext[1] >> 30)) * 0x6c078965u + 3;
    mContext[3] = (mContext[2] ^ (mContext[2] >> 30)) * 0x6c078965u + 4;
}

void pead::Random::init()
{
    init(static_cast<unsigned int>(OSGetSystemTime()));
}

unsigned int Random::getU32()
{
    unsigned int temporary = mContext[0] ^ (mContext[0] << 11);
    mContext[0] = mContext[1];
    mContext[1] = mContext[2];
    mContext[2] = mContext[3];
    mContext[3] = mContext[3] ^ (mContext[3] >> 19) ^ temporary ^ (temporary >> 8);
    return mContext[3];
}

unsigned long long Random::getU64()
{
    return static_cast<unsigned long long>(getU32()) << 32 | getU32();
}

BufferReadStreamSrc::BufferReadStreamSrc(StreamSrc* src, void* buffer, unsigned int size)
    : mSrc(src),
      mBuffer(static_cast<unsigned char*>(buffer)),
      mBufferSize(size),
      mBufferEndPos(0),
      mBufferPos(0)
{
}

BufferReadStreamSrc::~BufferReadStreamSrc()
{
}

int BufferReadStreamSrc::skip(int size)
{
    const int buffered = mBufferEndPos - mBufferPos;
    if (size <= buffered)
    {
        mBufferPos += size;
        return size;
    }
    mBufferPos = 0;
    mBufferEndPos = 0;
    return buffered + mSrc->skip(size - buffered);
}

unsigned int BufferReadStreamSrc::read(void* destination, unsigned int size)
{
    unsigned int readSize = 0;
    while (true)
    {
        if (mBufferPos < mBufferEndPos)
        {
            unsigned int copySize = size - readSize;
            if (mBufferEndPos - mBufferPos < copySize)
                copySize = mBufferEndPos - mBufferPos;
            MemUtil::copy(static_cast<unsigned char*>(destination) + readSize, mBuffer + mBufferPos, copySize);
            readSize += copySize;
            mBufferPos += copySize;
        }
        if (readSize >= size)
            break;
        mBufferEndPos = mSrc->read(mBuffer, mBufferSize);
        mBufferPos = 0;
        if (mBufferEndPos == 0)
            break;
    }
    return readSize;
}

unsigned int BufferReadStreamSrc::write(const void*, unsigned int)
{
    return 0;
}

void BufferReadStreamSrc::rewind()
{
    mSrc->rewind();
    mBufferPos = 0;
    mBufferEndPos = 0;
}

bool BufferReadStreamSrc::isEOF()
{
    return mSrc->isEOF() && mBufferPos >= mBufferEndPos;
}

BufferWriteStreamSrc::BufferWriteStreamSrc(StreamSrc* src, void* buffer, unsigned int size)
    : mSrc(src),
      mBuffer(static_cast<unsigned char*>(buffer)),
      mBufferSize(size),
      mBufferPos(0)
{
}

BufferWriteStreamSrc::~BufferWriteStreamSrc()
{
}
int BufferWriteStreamSrc::skip(int)
{
    return 0;
}
unsigned int BufferWriteStreamSrc::read(void*, unsigned int)
{
    return 0;
}

unsigned int BufferWriteStreamSrc::write(const void* source, unsigned int size)
{
    unsigned int writtenSize = 0;
    do
    {
        if (mBufferPos < mBufferSize)
        {
            unsigned int copySize = size - writtenSize;
            if (mBufferSize - mBufferPos < copySize)
                copySize = mBufferSize - mBufferPos;
            MemUtil::copy(mBuffer + mBufferPos, static_cast<const unsigned char*>(source) + writtenSize, copySize);
            writtenSize += copySize;
            mBufferPos += copySize;
        }
    } while (writtenSize < size && flush());
    return writtenSize;
}

void BufferWriteStreamSrc::rewind()
{
    flush();
    mSrc->rewind();
}

bool BufferWriteStreamSrc::isEOF()
{
    return mSrc->isEOF();
}

bool BufferWriteStreamSrc::flush()
{
    if (mBufferPos == 0)
        return true;
    const unsigned int writtenSize = mSrc->write(mBuffer, mBufferPos);
    const unsigned int bufferedSize = mBufferPos;
    mBufferPos = 0;
    return writtenSize >= bufferedSize;
}

BufferMultiByteTextWriteStreamSrc::BufferMultiByteTextWriteStreamSrc(StreamSrc* src, void* buffer, unsigned int size)
    : BufferWriteStreamSrc(src, buffer, size)
{
}

unsigned int BufferMultiByteTextWriteStreamSrc::write(const void* source, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(source);
    unsigned int writtenSize = 0;
    while (true)
    {
        if (mBufferPos < mBufferSize)
        {
            const unsigned int space = mBufferSize - mBufferPos;
            unsigned int copySize = size - writtenSize;
            if (space < copySize)
            {
                copySize = space;
                const unsigned char last = bytes[writtenSize + space - 1];
                if (last & cUtf8MultiByteBit)
                {
                    if ((last & cUtf8TrailMask) == cUtf8TrailBits)
                    {
                        const int searchLength = static_cast<int>(space) > cUtf8MaxSequenceLength
                                                     ? static_cast<int>(cUtf8MaxSequenceLength)
                                                     : static_cast<int>(space);
                        int tailLength = 2;
                        if (searchLength > 1)
                        {
                            const unsigned char* lead = bytes + writtenSize + space - 2;
                            bool leadFound = true;
                            while ((*lead & cUtf8TrailMask) == cUtf8TrailBits)
                            {
                                ++tailLength;
                                if (tailLength > searchLength)
                                {
                                    leadFound = false;
                                    break;
                                }
                                --lead;
                            }
                            if (leadFound)
                            {
                                int sequenceLength = 0;
                                if ((*lead & cUtf8Lead2Mask) == cUtf8Lead2Bits)
                                    sequenceLength = 2;
                                else if ((*lead & cUtf8Lead3Mask) == cUtf8Lead3Bits)
                                    sequenceLength = 3;
                                else if ((*lead & cUtf8Lead4Mask) == cUtf8Lead4Bits)
                                    sequenceLength = cUtf8MaxSequenceLength;
                                if (tailLength < sequenceLength && tailLength > 0)
                                {
                                    mBuffer[mBufferSize - tailLength] = 0;
                                    copySize = space - tailLength;
                                }
                            }
                        }
                    }
                    else
                    {
                        copySize = space - 1;
                        mBuffer[mBufferSize - 1] = 0;
                    }
                }
            }
            MemUtil::copy(mBuffer + mBufferPos, bytes + writtenSize, copySize);
            writtenSize += copySize;
            mBufferPos += copySize;
        }
        if (writtenSize >= size || !flush())
            return writtenSize;
    }
}

bool BufferMultiByteNullTerminatedTextWriteStreamSrc::flush()
{
    mBuffer[mBufferPos] = 0;
    return BufferWriteStreamSrc::flush();
}

template <typename T>
T readBinary(StreamSrc* source, Endian::Types endian)
{
    T value = 0;
    source->read(&value, sizeof(value));
    return convertEndian(value, endian);
}

template <typename T>
void writeBinary(StreamSrc* source, Endian::Types endian, T value)
{
    value = convertEndian(value, endian);
    source->write(&value, sizeof(value));
}

unsigned char BinaryStreamFormat::readU8(StreamSrc* src, Endian::Types e)
{
    return readBinary<unsigned char>(src, e);
}
unsigned short BinaryStreamFormat::readU16(StreamSrc* src, Endian::Types e)
{
    return readBinary<unsigned short>(src, e);
}
unsigned int BinaryStreamFormat::readU32(StreamSrc* src, Endian::Types e)
{
    return readBinary<unsigned int>(src, e);
}
unsigned long long BinaryStreamFormat::readU64(StreamSrc* src, Endian::Types e)
{
    return readBinary<unsigned long long>(src, e);
}
signed char BinaryStreamFormat::readS8(StreamSrc* src, Endian::Types e)
{
    return readBinary<signed char>(src, e);
}
short BinaryStreamFormat::readS16(StreamSrc* src, Endian::Types e)
{
    return readBinary<short>(src, e);
}
int BinaryStreamFormat::readS32(StreamSrc* src, Endian::Types e)
{
    return readBinary<int>(src, e);
}
long long BinaryStreamFormat::readS64(StreamSrc* src, Endian::Types e)
{
    return readBinary<long long>(src, e);
}
float BinaryStreamFormat::readF32(StreamSrc* src, Endian::Types e)
{
    return readBinary<float>(src, e);
}

void BinaryStreamFormat::readBit(StreamSrc* src, void* data, unsigned int bits)
{
    unsigned int bytes = bits / 8;
    src->read(data, bytes);
    unsigned int remaining = bits & 7;
    if (remaining)
    {
        unsigned char tail = 0;
        src->read(&tail, 1);
        unsigned char mask = static_cast<unsigned char>(0xffu << remaining);
        static_cast<unsigned char*>(data)[bytes] = static_cast<unsigned char>(
            (static_cast<unsigned char*>(data)[bytes] & mask) | (tail & ~mask));
    }
}

void BinaryStreamFormat::readString(StreamSrc* src, BufferedSafeStringBase<char>* string, unsigned int size)
{
    unsigned int restSize = 0;
    if (size > static_cast<unsigned int>(string->getBufferSize()))
    {
        restSize = size - string->getBufferSize();
        size = string->getBufferSize();
    }
    src->read(string->getBuffer(), size);
    if (size + 1 < static_cast<unsigned int>(string->getBufferSize()))
        string->trim(size);
    else
        string->trim(string->getBufferSize() - 1);
    if (restSize != 0)
        src->read(string->getBuffer(), restSize);
}

void BinaryStreamFormat::readMemBlock(StreamSrc* src, void* data, unsigned int size)
{
    src->read(data, size);
}
void BinaryStreamFormat::writeU8(StreamSrc* src, Endian::Types e, unsigned char v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeU16(StreamSrc* src, Endian::Types e, unsigned short v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeU32(StreamSrc* src, Endian::Types e, unsigned int v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeU64(StreamSrc* src, Endian::Types e, unsigned long long v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeS8(StreamSrc* src, Endian::Types e, signed char v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeS16(StreamSrc* src, Endian::Types e, short v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeS32(StreamSrc* src, Endian::Types e, int v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeS64(StreamSrc* src, Endian::Types e, long long v)
{
    writeBinary(src, e, v);
}
void BinaryStreamFormat::writeF32(StreamSrc* src, Endian::Types e, float v)
{
    writeBinary(src, e, v);
}

void BinaryStreamFormat::writeBit(StreamSrc* src, const void* data, unsigned int bitNum)
{
    const unsigned char byteNum = bitNum / 8;
    src->write(data, byteNum);
    if (bitNum != byteNum * 8u)
        src->write(static_cast<const unsigned char*>(data) + byteNum, 1);
}

void BinaryStreamFormat::writeString(StreamSrc* src, const SafeStringBase<char>& string, unsigned int size)
{
    unsigned int length = static_cast<unsigned int>(string.calcLength());
    if (length > size)
        length = size;
    src->write(string.cstr(), length);
    const char padding = 0;
    for (unsigned int index = length; index < size; ++index)
        src->write(&padding, 1);
}

void BinaryStreamFormat::writeMemBlock(StreamSrc* src, const void* data, unsigned int size)
{
    src->write(data, size);
}
void BinaryStreamFormat::writeDecorationText(StreamSrc*, const SafeStringBase<char>&)
{
}
void BinaryStreamFormat::writeNullChar(StreamSrc*)
{
}
void BinaryStreamFormat::skip(StreamSrc* src, unsigned int size)
{
    src->skip(static_cast<int>(size));
}
void BinaryStreamFormat::flush(StreamSrc*)
{
}
void BinaryStreamFormat::rewind(StreamSrc* src)
{
    src->rewind();
}

TextStreamFormat::~TextStreamFormat()
{
}
TextStreamFormat::TextStreamFormat() : mSeparator(" \t\n")
{
}
void TextStreamFormat::getNextData_(StreamSrc* src)
{
    char commentEnd = '\0';
    bool quoted = false;
    bool multiByte = false;
    char character;
    sTextBuffer.clear();
    int length = 0;
    while (src->read(&character, 1) != 0)
    {
        if (commentEnd != '\0')
        {
            if (__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                if (src->read(&character, 1) == 0)
                    return;
                continue;
            }
            if (character == commentEnd)
            {
                if (commentEnd == '*')
                {
                    commentEnd = '/';
                    continue;
                }
                commentEnd = '\0';
                character = mSeparator[0];
            }
            else
            {
                if (commentEnd == '/')
                    commentEnd = '*';
                continue;
            }
        }
        if (quoted)
        {
            if (character == '"')
            {
                if (!multiByte && length > 1 && sTextBuffer[length - 1] == '\\')
                {
                    sTextBuffer.copyAt(length - 1, "\"", 1);
                    continue;
                }
                return;
            }
            sTextBuffer.append(character);
            if (!__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                multiByte = false;
                ++length;
                continue;
            }
        }
        else
        {
            if (length == 0 && character == '"')
            {
                quoted = true;
                continue;
            }
            if (mSeparator.include(character))
            {
                if (!sTextBuffer.isEmpty())
                    return;
                continue;
            }
            sTextBuffer.append(character);
            if (!__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                multiByte = false;
                ++length;
                if (length >= 1 && sTextBuffer[length - 1] == '#')
                {
                    commentEnd = '\n';
                    --length;
                    sTextBuffer.trim(length);
                }
                if (length >= 2 && sTextBuffer[length - 2] == '/' && sTextBuffer[length - 1] == '/')
                {
                    commentEnd = '\n';
                    length -= 2;
                    sTextBuffer.trim(length);
                }
                else if (length >= 2 && sTextBuffer[length - 2] == '/' && sTextBuffer[length - 1] == '*')
                {
                    commentEnd = '*';
                    length -= 2;
                    sTextBuffer.trim(length);
                }
                continue;
            }
        }
        if (src->read(&character, 1) == 0)
            return;
        sTextBuffer.append(character);
        multiByte = true;
        length += 2;
    }
}
unsigned char TextStreamFormat::readU8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned char value = 0;
    getNextData_(src);
    StringUtil::tryParseU8(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned short TextStreamFormat::readU16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned short value = 0;
    getNextData_(src);
    StringUtil::tryParseU16(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned int TextStreamFormat::readU32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned int value = 0;
    getNextData_(src);
    StringUtil::tryParseU32(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned long long TextStreamFormat::readU64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned long long value = 0;
    getNextData_(src);
    StringUtil::tryParseU64(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
signed char TextStreamFormat::readS8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    signed char value = 0;
    getNextData_(src);
    StringUtil::tryParseS8(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
short TextStreamFormat::readS16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    short value = 0;
    getNextData_(src);
    StringUtil::tryParseS16(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
int TextStreamFormat::readS32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    int value = 0;
    getNextData_(src);
    StringUtil::tryParseS32(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
long long TextStreamFormat::readS64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    long long value = 0;
    getNextData_(src);
    StringUtil::tryParseS64(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
float TextStreamFormat::readF32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    float value = 0.0f;
    getNextData_(src);
    if (sTextBuffer.calcLength() != 0)
        ::sscanf(sTextBuffer.cstr(), "%f", &value);
    return value;
}
void TextStreamFormat::readBit(StreamSrc* src, void* data, unsigned int bitNum)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    SafeStringBase<char> text(sTextBuffer.cstr());
    if (text.calcLength() >= 2 && text.at(0) == '0' && text.at(1) == 'b')
        text = SafeStringBase<char>(text.cstr() + 2);
    unsigned char* bytes = static_cast<unsigned char*>(data);
    unsigned char value = 0;
    unsigned int bit = 0;
    int length = text.calcLength();
    for (int index = 0; index != length + 1 && bit < bitNum; ++index)
    {
        value <<= 1;
        if (text.at(index) == '1')
            value |= 1;
        ++bit;
        if ((bit & 7) == 0)
        {
            bytes[(bit >> 3) - 1] = value;
            value = 0;
        }
    }
    if ((bit & 7) != 0)
        bytes[bit >> 3] = static_cast<unsigned char>((bytes[bit >> 3] & (0xff << (bit & 7))) | value);
}
void TextStreamFormat::readString(StreamSrc* src, BufferedSafeStringBase<char>* dst, unsigned int)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    dst->copy(sTextBuffer);
}
void TextStreamFormat::readMemBlock(StreamSrc* src, void* dst, unsigned int size)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    unsigned int decodedSize = 0;
    Base64::decode(dst, size, sTextBuffer.cstr(), sTextBuffer.calcLength(), &decodedSize);
}

void TextStreamFormat::writeU8(StreamSrc* src, Endian::Types, unsigned char value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%u", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeU16(StreamSrc* src, Endian::Types, unsigned short value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%u", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeU32(StreamSrc* src, Endian::Types, unsigned int value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%u", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeU64(StreamSrc* src, Endian::Types, unsigned long long value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%llu", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeS8(StreamSrc* src, Endian::Types, signed char value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%d", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeS16(StreamSrc* src, Endian::Types, short value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%d", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeS32(StreamSrc* src, Endian::Types, int value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%d", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeS64(StreamSrc* src, Endian::Types, long long value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%lld", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeF32(StreamSrc* src, Endian::Types, float value)
{
    FixedSafeString<cValueTextLength> text;
    text.format("%.8f", value);
    src->write(text.cstr(), text.calcLength());
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeBit(StreamSrc* src, const void* data, unsigned int bitNum)
{
    ScopedLock<Mutex> lock(&sMutex);
    sTextBuffer.copy("0b");
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    unsigned int restBitNum = bitNum;
    for (unsigned int byteIndex = 0; byteIndex < (bitNum + 7) / 8; ++byteIndex)
    {
        unsigned int byteBitNum = restBitNum > 8 ? 8 : restBitNum;
        for (int bit = static_cast<int>(byteBitNum) - 1; bit >= 0; --bit)
        {
            if (bytes[byteIndex] & (1 << bit))
                sTextBuffer.append('1');
            else
                sTextBuffer.append('0');
        }
        restBitNum -= 8;
    }
    src->write(sTextBuffer.cstr(), bitNum + 2);
    src->write(mSeparator.cstr(), 1);
}
void TextStreamFormat::writeString(StreamSrc* src, const SafeStringBase<char>& string, unsigned int size)
{
    unsigned int length = string.calcLength();
    if (size > length)
        size = length;
    const char quote = '"';
    const char escape = '\\';
    src->write(&quote, 1);
    for (int index = 0; size != 0; --size, ++index)
    {
        if (string.at(index) == '"'
            || ((index == 0 || !__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(string.at(index - 1)))
                && string.at(index) == '"'))
            src->write(&escape, 1);
        src->write(string.cstr() + index, 1);
    }
    src->write(&quote, 1);
}
void TextStreamFormat::writeMemBlock(StreamSrc* src, const void* data, unsigned int size)
{
    ScopedLock<Mutex> lock(&sMutex);
    sTextBuffer.clear();
    unsigned int encodedSize = (size / 3 + (size % 3 != 0)) * 4;
    if (encodedSize + 1 < sTextBuffer.getBufferSize())
    {
        char* buffer = sTextBuffer.getBuffer();
        buffer[encodedSize] = '\0';
        Base64::encode(buffer, data, size, false);
        src->write("\"", 1);
        src->write(sTextBuffer.cstr(), sTextBuffer.calcLength());
        src->write("\"", 1);
        src->write(mSeparator.cstr(), 1);
    }
}
void TextStreamFormat::writeDecorationText(StreamSrc* s, const SafeStringBase<char>& v)
{
    s->write(v.cstr(), v.calcLength());
}
void TextStreamFormat::writeNullChar(StreamSrc* src)
{
    const char nullChar = 0;
    src->write(&nullChar, 1);
}
void TextStreamFormat::skip(StreamSrc* src, unsigned int)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
}
void TextStreamFormat::rewind(StreamSrc* s)
{
    s->rewind();
}
void TextStreamFormat::flush(StreamSrc*)
{
}

Stream::~Stream()
{
}
void Stream::skip(unsigned int size)
{
    mFormat->skip(mSrc, size);
}
void Stream::skip(unsigned int size, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
        mFormat->skip(mSrc, size);
}
void Stream::rewind()
{
    mFormat->rewind(mSrc);
}
bool Stream::isEOF()
{
    return mSrc->isEOF();
}
ReadStream::~ReadStream()
{
}
unsigned char pead::ReadStream::readU8()
{
    return mFormat->readU8(mSrc, mEndian);
}
unsigned short pead::ReadStream::readU16()
{
    return mFormat->readU16(mSrc, mEndian);
}
unsigned int pead::ReadStream::readU32()
{
    return mFormat->readU32(mSrc, mEndian);
}
unsigned long long pead::ReadStream::readU64()
{
    return mFormat->readU64(mSrc, mEndian);
}
signed char pead::ReadStream::readS8()
{
    return mFormat->readS8(mSrc, mEndian);
}
short pead::ReadStream::readS16()
{
    return mFormat->readS16(mSrc, mEndian);
}
int pead::ReadStream::readS32()
{
    return mFormat->readS32(mSrc, mEndian);
}
long long pead::ReadStream::readS64()
{
    return mFormat->readS64(mSrc, mEndian);
}
float pead::ReadStream::readF32()
{
    return mFormat->readF32(mSrc, mEndian);
}
void ReadStream::readBit(void* d, unsigned int n)
{
    mFormat->readBit(mSrc, d, n);
}
void ReadStream::readString(BufferedSafeStringBase<char>* s, unsigned int n)
{
    mFormat->readString(mSrc, s, n);
}
void ReadStream::readMemBlock(void* d, unsigned int n)
{
    mFormat->readMemBlock(mSrc, d, n);
}

inline WriteStream::~WriteStream()
{
    if (mSrc)
        flush();
}
void WriteStream::flush()
{
    mFormat->flush(mSrc);
    mSrc->flush();
}
void WriteStream::writeU8(unsigned char v)
{
    mFormat->writeU8(mSrc, mEndian, v);
}
void WriteStream::writeU16(unsigned short v)
{
    mFormat->writeU16(mSrc, mEndian, v);
}
void WriteStream::writeU32(unsigned int v)
{
    mFormat->writeU32(mSrc, mEndian, v);
}
void WriteStream::writeU64(unsigned long long v)
{
    mFormat->writeU64(mSrc, mEndian, v);
}
void WriteStream::writeS8(signed char v)
{
    mFormat->writeS8(mSrc, mEndian, v);
}
void WriteStream::writeS16(short v)
{
    mFormat->writeS16(mSrc, mEndian, v);
}
void WriteStream::writeS32(int v)
{
    mFormat->writeS32(mSrc, mEndian, v);
}
void WriteStream::writeS64(long long v)
{
    mFormat->writeS64(mSrc, mEndian, v);
}
void WriteStream::writeF32(float v)
{
    mFormat->writeF32(mSrc, mEndian, v);
}
void WriteStream::writeBit(const void* d, unsigned int n)
{
    mFormat->writeBit(mSrc, d, n);
}
void WriteStream::writeString(const SafeStringBase<char>& s, unsigned int n)
{
    mFormat->writeString(mSrc, s, n);
}
void WriteStream::writeMemBlock(const void* d, unsigned int n)
{
    mFormat->writeMemBlock(mSrc, d, n);
}
void WriteStream::writeComment(const SafeStringBase<char>& s)
{
    mFormat->writeDecorationText(mSrc, SafeStringBase<char>("/* "));
    mFormat->writeDecorationText(mSrc, s);
    mFormat->writeDecorationText(mSrc, SafeStringBase<char>(" */"));
}
void WriteStream::writeLineComment(const SafeStringBase<char>& s)
{
    mFormat->writeDecorationText(mSrc, SafeStringBase<char>("// "));
    mFormat->writeDecorationText(mSrc, s);
    mFormat->writeDecorationText(mSrc, SafeStringBase<char>("\n"));
}
void WriteStream::writeDecorationText(const SafeStringBase<char>& s)
{
    mFormat->writeDecorationText(mSrc, s);
}
void WriteStream::writeNullChar()
{
    mFormat->writeNullChar(mSrc);
}

BufferReadStream::~BufferReadStream()
{
    mSrc = NULL;
}
BufferWriteStream::~BufferWriteStream()
{
    flush();
    mSrc = NULL;
}
BufferMultiByteTextWriteStream::~BufferMultiByteTextWriteStream()
{
    flush();
    mSrc = NULL;
}

unsigned int PrintStreamSrc::read(void*, unsigned int)
{
    return 0;
}
unsigned int PrintStreamSrc::write(const void*, unsigned int size)
{
    return size;
}
int PrintStreamSrc::skip(int)
{
    return 0;
}
void PrintStreamSrc::rewind()
{
}
bool PrintStreamSrc::isEOF()
{
    return false;
}
PrintWriteStream::~PrintWriteStream()
{
    flush();
    mSrc = NULL;
}

void Base64::encode(char* destination, const void* source, unsigned int size, bool urlSafe)
{
    const char* alphabet = urlSafe ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
                                   : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char* input = static_cast<const unsigned char*>(source);
    unsigned int inputIndex = 0;
    unsigned int outputIndex = 0;
    while (inputIndex + 2 < size)
    {
        unsigned int value = input[inputIndex] << 16 | input[inputIndex + 1] << 8 | input[inputIndex + 2];
        destination[outputIndex++] = alphabet[(value >> 18) & 63];
        destination[outputIndex++] = alphabet[(value >> 12) & 63];
        destination[outputIndex++] = alphabet[(value >> 6) & 63];
        destination[outputIndex++] = alphabet[value & 63];
        inputIndex += 3;
    }
    if (inputIndex < size)
    {
        unsigned int value = input[inputIndex] << 16;
        if (inputIndex + 1 < size)
            value |= input[inputIndex + 1] << 8;
        destination[outputIndex++] = alphabet[(value >> 18) & 63];
        destination[outputIndex++] = alphabet[(value >> 12) & 63];
        destination[outputIndex++] = inputIndex + 1 < size ? alphabet[(value >> 6) & 63] : '=';
        destination[outputIndex++] = '=';
    }
}

bool Base64::decode(void* destination, unsigned int capacity, const char* source, unsigned int size,
                    unsigned int* resultSize)
{
    unsigned char* output = static_cast<unsigned char*>(destination);
    const unsigned char* input = reinterpret_cast<const unsigned char*>(source);
    unsigned int written = 0;
    unsigned int index = 0;
    while (index < size)
    {
        if (input[index] < ' ')
        {
            ++index;
            if (index >= size)
                break;
            if (input[index] < ' ')
            {
                ++index;
                if (index >= size)
                    break;
            }
        }
        if (index + cEncodedBlockSize > size)
            return false;
        unsigned int values[cEncodedBlockSize];
        for (int position = 0; position < cEncodedBlockSize; ++position)
        {
            const unsigned char character = input[index + position];
            values[position] = character >= 'A' && character <= 'Z'   ? character - 'A'
                               : character >= 'a' && character <= 'z' ? character - 'a' + 26
                               : character >= '0' && character <= '9' ? character - '0' + 52
                               : character == '+' || character == '-' ? 62
                               : character == '/' || character == '_' ? 63
                                                                      : cPadding;
        }
        const unsigned int first = values[0] & cValueMask;
        const unsigned int second = values[1] & cValueMask;
        const unsigned int third = values[2];
        const unsigned int fourth = values[3];
        index += cEncodedBlockSize;
        if (written >= capacity)
            return false;
        output[written++] = static_cast<unsigned char>(first << 2 | second >> 4);
        if (third < cPadding)
        {
            if (written >= capacity)
                return false;
            output[written++] = static_cast<unsigned char>(second << 4 | third >> 2);
            if (fourth < cPadding)
            {
                if (written >= capacity)
                    return false;
                output[written++] = static_cast<unsigned char>(third << 6 | fourth);
            }
        }
    }
    if (resultSize)
        *resultSize = written;
    return true;
}

unsigned int BitFlagUtil::countOnBit(unsigned int value)
{
    value = value - ((value >> 1) & 0x55555555u);
    value = (value & 0x33333333u) + ((value >> 2) & 0x33333333u);
    return (((value + (value >> 4)) & 0x0f0f0f0fu) * 0x01010101u) >> 24;
}

int BitFlagUtil::findOnBitFromRight(unsigned int value, int ordinal)
{
    while (value != 0)
    {
        ordinal = static_cast<int>(static_cast<unsigned int>(ordinal) - 1u);
        if (ordinal <= 0)
            return countOnBit((value & (0u - value)) - 1u);
        value &= value - 1u;
    }
    return -1;
}

CoreId::CoreId() : mId(0)
{
}
unsigned int CoreInfo::sCoreCount = 1;
unsigned int CoreInfo::sCoreIdMap[3];
unsigned int CoreInfo::sCoreIndexTable[3];

void CoreInfo::configure()
{
    sCoreCount = 3;
    sCoreIdMap[0] = 1;
    sCoreIdMap[1] = 0;
    sCoreIdMap[2] = 2;
    for (unsigned int index = 0; index < sCoreCount; ++index)
        sCoreIndexTable[sCoreIdMap[index]] = index;
}

int CoreInfo::getPlatformMask(CoreId id)
{
    return 1 << (sCoreIdMap[id.value()] & 0x3f);
}

bool __N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(unsigned char)
{
    return false;
}

Heap* IDisposer::cDestructedFlag = reinterpret_cast<Heap*>(1);

IDisposer::IDisposer() : mDisposerHeap(0), mListNode()
{
    if (HeapMgr::instance())
    {
        mDisposerHeap = HeapMgr::instance()->findContainHeap(this);
        if (mDisposerHeap)
            mDisposerHeap->appendDisposer_(this);
    }
}

IDisposer::IDisposer(Heap* heap, HeapNullOption option) : mDisposerHeap(heap), mListNode()
{
    if (!mDisposerHeap)
    {
        switch (option)
        {
        case cHeapNullOption_AlwaysUseSpecifiedHeap:
        case cHeapNullOption_UseSpecifiedOrContainHeap:
            if (!HeapMgr::instance())
                return;
            mDisposerHeap = HeapMgr::instance()->findContainHeap(this);
            break;
        case cHeapNullOption_UseSpecifiedOrCurrentHeap:
            if (!HeapMgr::instance())
                return;
            mDisposerHeap = HeapMgr::instance()->getCurrentHeap();
            break;
        default:
            return;
        }
        if (!mDisposerHeap)
            return;
    }
    mDisposerHeap->appendDisposer_(this);
}

IDisposer::~IDisposer()
{
    if (mDisposerHeap != cDestructedFlag)
    {
        if (mDisposerHeap)
            mDisposerHeap->removeDisposer_(this);
        mDisposerHeap = cDestructedFlag;
    }
}

Heap::Heap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size, HeapDirection direction,
           bool enableLock)
    : IDisposer(parent, cHeapNullOption_UseSpecifiedOrContainHeap),
      mName(name),
      mStart(start),
      mSize(size),
      mParent(parent),
      mChildren(),
      mListNode(),
      mDisposerList(),
      mDirection(direction),
      mCS(parent),
      mFlag(cFlag_EnableWarning),
      mHeapCheckTag(HeapMgr::issueHeapCheckTag())
{
    if (enableLock)
        mFlag.set(cFlag_EnableLock);
    else
        mFlag.reset(cFlag_EnableLock);
    ConditionalScopedLock<CriticalSection> lock(&mCS, enableLock);
    mChildren.initOffset(offsetof(Heap, mListNode));
    mDisposerList.initOffset(offsetof(IDisposer, mListNode));
}

Heap::~Heap()
{
}

bool Heap::checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}
const RuntimeTypeInfo::Interface* Heap::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}
bool ExpHeap::checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}
const RuntimeTypeInfo::Interface* ExpHeap::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}
void* Heap::tryRealloc(void*, unsigned int, int)
{
    return 0;
}
void Heap::dump() const
{
}

void Heap::dumpYAML(WriteStream& stream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("- name: \"%s\"\n", mName.cstr());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  start_address: 0x%08X\n", getStartAddress());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  end_address: 0x%08X\n", getEndAddress());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    if (!mParent)
        line.appendWithFormat("  parent: %s\n", "~");
    else
        line.appendWithFormat("  parent: %s\n", mParent->mName.cstr());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  direction: %s\n", mDirection == cHeapDirection_Forward ? "Forward" : "Reverse");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  size: %u\n", getSize());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  free_size: %u\n", getFreeSize());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  max_allocatable_size: %u\n", getMaxAllocatableSize(4));
    stream.writeDecorationText(line);
}

void Heap::pushBackChild_(Heap* child)
{
    ScopedLock<CriticalSection> treeLock(HeapMgr::getHeapTreeLockCS());
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mChildren.pushBack(child);
}

void Heap::dumpTreeYAML(WriteStream& stream, int indent) const
{
    dumpYAML(stream, indent);
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("  children:\n");
    stream.writeDecorationText(line);
    for (OffsetList<Heap>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
        it->dumpTreeYAML(stream, indent + 4);
}

Heap* Heap::findContainHeap_(const void* address)
{
    if (!isInclude(address))
        return 0;
    for (OffsetList<Heap>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (it->isInclude(address))
            return it->findContainHeap_(address);
    }
    return this;
}

void Heap::dispose_(const void* begin, const void* end)
{
    mFlag.set(cFlag_Disposing);
    OffsetList<IDisposer>::iterator it = mDisposerList.begin();
    while (it != mDisposerList.end())
    {
        IDisposer* disposer = &*it;
        if (disposer->mDisposerHeap && ((!begin && !end) || (begin <= disposer && disposer < end)))
        {
            disposer->~IDisposer();
            it = mDisposerList.begin();
        }
        else
        {
            ++it;
        }
    }
    mFlag.reset(cFlag_Disposing);
}

void Heap::eraseChild_(Heap* child)
{
    ScopedLock<CriticalSection> treeLock(HeapMgr::getHeapTreeLockCS());
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mChildren.erase(child);
}

void Heap::destruct_()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    dispose_(0, 0);
    HeapMgr::removeFromFindContainHeapCache_(this);
    if (mParent)
        mParent->eraseChild_(this);
}

void Heap::appendDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mDisposerList.pushBack(disposer);
}

void Heap::removeDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mDisposerList.erase(disposer);
}

ExpHeap::ExpHeap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size,
                 HeapDirection direction, bool enableLock)
    : Heap(name, parent, start, size, direction, enableLock),
      mAllocMode(cAllocMode_FirstFit),
      mFreeList(),
      mUseList()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mFreeList.initOffset(offsetof(MemBlock, mListNode));
    mUseList.initOffset(offsetof(MemBlock, mListNode));
}

ExpHeap::~ExpHeap()
{
    destruct_();
}

ExpHeap* ExpHeap::tryCreate(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction,
                            bool enableLock)
{
    if (!parent)
    {
        parent = HeapMgr::instance()->getCurrentHeap();
        if (!parent)
            return 0;
    }
    if (size == 0)
        size = parent->getMaxAllocatableSize(4) & ~3u;
    else
        size = (size + 3) & ~3u;
    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
        return 0;
    unsigned char* start = static_cast<unsigned char*>(parent->tryAlloc(size, direction * 4));
    if (!start)
        return 0;
    if (parent->mDirection == cHeapDirection_Reverse)
        direction = static_cast<HeapDirection>(-direction);
    ExpHeap* heap;
    if (direction == cHeapDirection_Forward)
        heap = new (start) ExpHeap(name, parent, start, size, cHeapDirection_Forward, enableLock);
    else
        heap = new (start + size - sizeof(ExpHeap)) ExpHeap(name, parent, start, size, direction, enableLock);
    doCreate(heap, parent);
    return heap;
}

ExpHeap* ExpHeap::create(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction,
                         bool enableLock)
{
    return tryCreate(size, name, parent, direction, enableLock);
}

ExpHeap* ExpHeap::tryCreate(void* address, unsigned int size, const SafeStringBase<char>& name, bool enableLock)
{
    size &= ~3u;
    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
        return 0;
    ExpHeap* heap = new (address) ExpHeap(name, 0, address, size, cHeapDirection_Forward, enableLock);
    doCreate(heap, 0);
    return heap;
}

void ExpHeap::pushToFreeList_(MemBlock* block)
{
    MemBlock* previous = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        MemBlock* next = &*it;
        if (block < next)
        {
            bool merged = false;
            if (previous && previous->memory() + previous->mSize == reinterpret_cast<unsigned char*>(block))
            {
                previous->mSize += block->mOffset + block->mSize + sizeof(MemBlock);
                block = previous;
                merged = true;
            }
            if (block->memory() + block->mSize == reinterpret_cast<unsigned char*>(next))
            {
                if (!merged)
                    mFreeList.insertBefore(next, block);
                block->mSize += next->mOffset + next->mSize + sizeof(MemBlock);
                mFreeList.erase(next);
            }
            else if (!merged)
            {
                mFreeList.insertBefore(next, block);
            }
            return;
        }
        previous = next;
    }
    if (mFreeList.size() != 0 && previous->memory() + previous->mSize == reinterpret_cast<unsigned char*>(block))
    {
        previous->mSize += block->mOffset + block->mSize + sizeof(MemBlock);
        return;
    }
    mFreeList.pushBack(block);
}

void ExpHeap::createMaxSizeFreeMemBlock_(ExpHeap* heap)
{
    ConditionalScopedLock<CriticalSection> lock(&heap->mCS, heap->isEnableLock());
    void* address = heap->mDirection == cHeapDirection_Forward ? static_cast<void*>(heap + 1) : heap->mStart;
    MemBlock* block = new (address) MemBlock();
    block->mSize = heap->mSize - sizeof(ExpHeap) - sizeof(MemBlock);
    heap->pushToFreeList_(block);
}

void ExpHeap::doCreate(ExpHeap* heap, Heap* parent)
{
    createMaxSizeFreeMemBlock_(heap);
    if (parent)
        parent->pushBackChild_(heap);
}

int ExpHeap::compareMemBlockAddr_(const MemBlock* first, const MemBlock* second)
{
    if (reinterpret_cast<const char*>(first) - reinterpret_cast<const char*>(second) < 0)
        return -1;
    return 1;
}

MemBlock* ExpHeap::findLastMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* lastFree = mFreeList.back();
    MemBlock* lastUse = mUseList.back();
    if (!lastUse)
    {
        if (lastFree)
            return lastFree;
    }
    else if (lastUse < lastFree)
    {
        return lastFree;
    }
    return 0;
}

unsigned int ExpHeap::adjustBack_()
{
    MemBlock* block = findLastMemBlockIfFree_();
    if (block)
    {
        unsigned int size = reinterpret_cast<unsigned char*>(block) - static_cast<unsigned char*>(mStart);
        mFreeList.erase(block);
        if (mParent->resizeBack(mStart, size))
        {
            mSize = size;
            return size;
        }
    }
    return mSize;
}

MemBlock* ExpHeap::findFirstMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* firstFree = mFreeList.front();
    MemBlock* firstUse = mUseList.front();
    if (firstUse && firstUse <= firstFree)
        return 0;
    return firstFree;
}

unsigned int ExpHeap::adjustFront_()
{
    MemBlock* block = findFirstMemBlockIfFree_();
    if (block)
    {
        unsigned int size = mSize - (block->mOffset + block->mSize + sizeof(MemBlock));
        mFreeList.erase(block);
        void* start = mParent->resizeFront(mStart, size);
        if (start)
        {
            mSize = size;
            OSMemoryBarrier();
            mStart = start;
            return size;
        }
    }
    return mSize;
}

unsigned int ExpHeap::adjust()
{
    if (!mParent)
        return mSize;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    ConditionalScopedLock<CriticalSection> parentLock(&mParent->mCS, mParent->isEnableLock());
    if (mDirection == cHeapDirection_Forward)
        return adjustBack_();
    return adjustFront_();
}

MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, FindMode mode) const
{
    MemBlock* found = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        unsigned int blockSize = it->mSize;
        if (size <= blockSize)
        {
            if (mode == cFindMode_FirstFit)
                return &*it;
            if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize)
                || (mode == cFindMode_MaxSize && found->mSize < blockSize))
                found = &*it;
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromHead_(unsigned int size)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, static_cast<FindMode>(mAllocMode));
    if (block)
    {
        unsigned short offset = block->mOffset;
        unsigned int blockSize = block->mSize;
        block->mSize = size;
        unsigned int remainSize = blockSize - size;
        mFreeList.erase(block);
        pushToUseList_(block);
        if (remainSize > sizeof(MemBlock))
        {
            MemBlock* freeBlock = new (reinterpret_cast<unsigned char*>(block) + offset + sizeof(MemBlock) + size)
                MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        else if (remainSize != 0)
        {
            block->mSize = size + remainSize;
        }
    }
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        unsigned int blockSize = it->mSize;
        if (size <= blockSize)
        {
            uintptr_t memory = reinterpret_cast<uintptr_t>(it->memory());
            uintptr_t aligned = (memory + alignment - 1) & ~static_cast<uintptr_t>(alignment - 1);
            if (size + (aligned - memory) <= blockSize)
            {
                if (mode == cFindMode_FirstFit)
                    return &*it;
                if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize)
                    || (mode == cFindMode_MaxSize && found->mSize < blockSize))
                    found = &*it;
            }
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromHead_(unsigned int size, int alignment)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, alignment, static_cast<FindMode>(mAllocMode));
    if (block)
    {
        uintptr_t memory = reinterpret_cast<uintptr_t>(block->memory());
        unsigned int padding = ((memory + alignment - 1) & ~static_cast<uintptr_t>(alignment - 1)) - memory;
        unsigned int remainSize = block->mSize - (size + padding);
        if (padding < 0x10000)
        {
            block->setOffset(padding);
            block->mSize = size;
            mFreeList.erase(block);
            pushToUseList_(block);
        }
        else
        {
            block->mOffset = 0;
            MemBlock* useBlock = reinterpret_cast<MemBlock*>(reinterpret_cast<unsigned char*>(block) + padding);
            block->mSize = padding - sizeof(MemBlock);
            useBlock = new (useBlock) MemBlock();
            useBlock->mOffset = 0;
            useBlock->mSize = size;
            pushToUseList_(useBlock);
            block = useBlock;
        }
        if (remainSize > sizeof(MemBlock))
        {
            MemBlock* freeBlock = new (block->memory() + size) MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        else if (remainSize != 0)
        {
            block->mSize = size + remainSize;
        }
    }
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, FindMode mode) const
{
    MemBlock* found = 0;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        unsigned int blockSize = block->mSize;
        if (size <= blockSize)
        {
            if (mode == cFindMode_FirstFit)
                return block;
            if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize)
                || (mode == cFindMode_MaxSize && found->mSize < blockSize))
                found = block;
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromTail_(unsigned int size)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, static_cast<FindMode>(mAllocMode));
    if (!block)
        return 0;
    if (block->mSize - size > sizeof(MemBlock))
    {
        unsigned int freeSize = block->mSize - size - sizeof(MemBlock);
        unsigned char* address = block->memory() + freeSize;
        block->mSize = freeSize;
        MemBlock* useBlock = new (address) MemBlock(size);
        pushToUseList_(useBlock);
        return useBlock;
    }
    mFreeList.erase(block);
    pushToUseList_(block);
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        unsigned int blockSize = block->mSize;
        if (size <= blockSize)
        {
            uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + (blockSize - size);
            if (size + (address - (address & ~static_cast<uintptr_t>(alignment - 1))) <= blockSize)
            {
                if (mode == cFindMode_FirstFit)
                    return block;
                if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize)
                    || (mode == cFindMode_MaxSize && found->mSize < blockSize))
                    found = block;
            }
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromTail_(unsigned int size, int alignment)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, alignment, static_cast<FindMode>(mAllocMode));
    if (!block)
        return 0;
    uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + (block->mSize - size);
    unsigned int useSize = size + (address - (address & ~static_cast<uintptr_t>(alignment - 1)));
    unsigned int remainSize = block->mSize - useSize;
    if (remainSize > sizeof(MemBlock))
    {
        unsigned int freeSize = remainSize - sizeof(MemBlock);
        unsigned char* useAddress = block->memory() + freeSize;
        block->mSize = freeSize;
        MemBlock* useBlock = new (useAddress) MemBlock(useSize);
        pushToUseList_(useBlock);
        return useBlock;
    }
    block->setOffset(remainSize);
    block->mSize = useSize;
    mFreeList.erase(block);
    pushToUseList_(block);
    return block;
}

void ExpHeap::pushToUseList_(MemBlock* block)
{
    if (mDirection == cHeapDirection_Forward)
        mUseList.pushBack(block);
    else
        mUseList.pushFront(block);
}

void* ExpHeap::realloc_(void* address, unsigned char* memory, unsigned int copySize, unsigned int size, int alignment)
{
    void* newAddress = tryAlloc(size, alignment);
    if (newAddress)
    {
        MemUtil::copy(newAddress, memory, copySize);
        free(address);
    }
    return newAddress;
}

void ExpHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;
    this->~ExpHeap();
    if (parent && parent->isFreeable())
        parent->free(start);
}

void* ExpHeap::tryAlloc(unsigned int size, int alignment)
{
    unsigned int allocSize = size < 4 ? 4 : size;
    if (allocSize >= mSize)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = alignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    unsigned int absAlignment = alignment < 0 ? -alignment : alignment;
    if (((absAlignment - 1) & absAlignment) != 0)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = alignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    allocSize = (allocSize + 3) & ~3u;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    int directedAlignment = alignment * mDirection;
    MemBlock* block;
    if (directedAlignment < 0)
    {
        directedAlignment = -directedAlignment;
        if (directedAlignment < 5)
            block = allocFromTail_(allocSize);
        else
            block = allocFromTail_(allocSize, directedAlignment);
    }
    else if (directedAlignment < 5)
    {
        block = allocFromHead_(allocSize);
    }
    else
    {
        block = allocFromHead_(allocSize, directedAlignment);
    }
    if (!block)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = directedAlignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    block->mMagic = mHeapCheckTag;
    return block->memory();
}

void ExpHeap::dumpUseList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
}

void ExpHeap::dumpFreeList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
}

void ExpHeap::free(void* address)
{
    if (!address)
        return;
    if (!isInclude(address))
        return;
    if (mFlag.isOn(cFlag_Disposing))
        return;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    if (!block)
    {
        dumpUseList();
        dumpFreeList();
    }
    else if (block->mMagic == mHeapCheckTag)
    {
        mUseList.erase(block);
        unsigned short offset = block->mOffset;
        block->mOffset = 0;
        block->mSize += offset;
        pushToFreeList_(block);
    }
}

void* ExpHeap::resizeFront(void* address, unsigned int size)
{
    if (!isInclude(address))
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (size < blockSize)
    {
        unsigned int span = block->mOffset + blockSize;
        unsigned int freeSpan = span + sizeof(MemBlock) - size;
        if (freeSpan - sizeof(MemBlock) >= sizeof(MemBlock))
        {
            MemBlock* useBlock = new (reinterpret_cast<unsigned char*>(block) + (freeSpan - sizeof(MemBlock)))
                MemBlock();
            useBlock->mMagic = mHeapCheckTag;
            useBlock->mSize = size;
            pushToUseList_(useBlock);
            mUseList.erase(block);
            block->mOffset = 0;
            block->mSize = freeSpan - 2 * sizeof(MemBlock);
            pushToFreeList_(block);
            return useBlock->memory();
        }
        block->setOffset(span - size);
        return block->memory();
    }
    if (size <= blockSize)
        return block->memory();
    return 0;
}

void* ExpHeap::resizeBack(void* address, unsigned int size)
{
    if (!isInclude(address))
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (size < blockSize)
    {
        unsigned int remainSize = blockSize - size;
        if (remainSize >= sizeof(MemBlock))
        {
            unsigned char* freeAddress = block->memory() + size;
            block->mSize = size;
            MemBlock* freeBlock = new (freeAddress) MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        return block->memory();
    }
    if (size <= blockSize)
        return block->memory();
    return 0;
}

void* ExpHeap::tryRealloc(void* address, unsigned int size, int alignment)
{
    if (!address)
        return tryAlloc(size, alignment);
    if (size == 0)
    {
        free(address);
        return 0;
    }
    if (!isInclude(address) || alignment < 0)
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (blockSize <= size)
    {
        if (blockSize < size)
        {
            if (alignment == 0)
                alignment = 4;
            return realloc_(address, block->memory(), blockSize, size, alignment);
        }
        unsigned char* memory = block->memory();
        if (alignment != 0 && (reinterpret_cast<uintptr_t>(memory) & (alignment - 1)) != 0)
            return realloc_(address, memory, blockSize, size, alignment);
        return memory;
    }
    if (alignment != 0)
    {
        unsigned char* memory = block->memory();
        if ((reinterpret_cast<uintptr_t>(memory) & (alignment - 1)) != 0)
            return realloc_(address, memory, size, size, alignment);
    }
    unsigned int remainSize = blockSize - size;
    if (remainSize >= sizeof(MemBlock))
    {
        unsigned char* freeAddress = block->memory() + size;
        block->mSize = size;
        MemBlock* freeBlock = new (freeAddress) MemBlock(remainSize - sizeof(MemBlock));
        pushToFreeList_(freeBlock);
    }
    return block->memory();
}

void ExpHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    dispose_(0, 0);
    mUseList.clear();
    mFreeList.clear();
    createMaxSizeFreeMemBlock_(this);
}

void* ExpHeap::getStartAddress() const
{
    return mStart;
}
void* ExpHeap::getEndAddress() const
{
    return static_cast<unsigned char*>(mStart) + mSize;
}

unsigned int ExpHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    unsigned int freeSize = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
        freeSize += it->mSize;
    return freeSize;
}

unsigned int ExpHeap::getSize() const
{
    return mSize;
}

unsigned int ExpHeap::getMaxAllocatableSize(int alignment) const
{
    unsigned int absAlignment = alignment < 0 ? -alignment : alignment;
    if (((absAlignment - 1) & absAlignment) != 0)
        return 0;
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    int directedAlignment = alignment * mDirection;
    MemBlock* block;
    if (directedAlignment < 0)
    {
        directedAlignment = -directedAlignment;
        if (directedAlignment > 4)
        {
            block = findFreeMemBlockFromTail_(4, directedAlignment, cFindMode_MaxSize);
            if (!block)
                return 0;
            uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + block->mSize - 4;
            return block->mSize - (address - (address & ~static_cast<uintptr_t>(directedAlignment - 1)));
        }
        block = findFreeMemBlockFromTail_(4, cFindMode_MaxSize);
    }
    else
    {
        if (directedAlignment > 4)
        {
            block = findFreeMemBlockFromHead_(4, directedAlignment, cFindMode_MaxSize);
            if (!block)
                return 0;
            uintptr_t memory = reinterpret_cast<uintptr_t>(block->memory());
            return block->mSize
                   - (((memory + directedAlignment - 1) & ~static_cast<uintptr_t>(directedAlignment - 1)) - memory);
        }
        block = findFreeMemBlockFromHead_(4, cFindMode_MaxSize);
    }
    if (block)
        return block->mSize;
    return 0;
}

bool ExpHeap::isInclude(const void* address) const
{
    const unsigned char* start = static_cast<const unsigned char*>(mStart);
    unsigned int size = mSize;
    const unsigned char* begin;
    if (mDirection == cHeapDirection_Forward)
    {
        begin = start + sizeof(ExpHeap);
    }
    else
    {
        size -= sizeof(ExpHeap);
        begin = start;
    }
    return begin <= address && address < start + size;
}

bool ExpHeap::isEmpty() const
{
    return mUseList.size() == 0;
}
bool ExpHeap::isFreeable() const
{
    return true;
}
bool ExpHeap::isResizable() const
{
    return true;
}
bool ExpHeap::isAdjustable() const
{
    return true;
}

void ExpHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    dumpUseList();
    dumpFreeList();
}

unsigned int ExpHeap::getAllocatedSize(void* address)
{
    if (!isInclude(address))
        return 0;
    return MemBlock::findManageArea(address)->mSize;
}

void ExpHeap::dumpYAML(WriteStream& stream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    Heap::dumpYAML(stream, indent);
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("  heap_type: ExpHeap\n");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  alloc_mode: %s\n", getAllocMode() == cAllocMode_FirstFit ? "First Fit" : "Best Fit");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  use_list_size: %d\n", mUseList.size());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  free_list_size: %d\n", mFreeList.size());
    stream.writeDecorationText(line);
}

void ExpHeap::setAllocMode(AllocMode mode)
{
    mAllocMode = mode;
}
ExpHeap::AllocMode ExpHeap::getAllocMode() const
{
    return mAllocMode;
}

FindContainHeapCache::FindContainHeapCache() : mHeap(0)
{
}

bool FindContainHeapCache::tryRemoveHeap(Heap* heap)
{
    unsigned int original;
    if (mHeap.compareExchange(reinterpret_cast<uintptr_t>(heap), 0, &original))
        return true;
    return (original & ~1u) != reinterpret_cast<uintptr_t>(heap);
}

Arena* HeapMgr::sArena = 0;
HeapMgr HeapMgr::sInstance;
HeapMgr* HeapMgr::sInstancePtr = 0;
AtomicU32 HeapMgr::sHeapCheckTag;
HeapMgr::RootHeaps HeapMgr::sRootHeaps;
CriticalSection HeapMgr::sHeapTreeLockCS;
HeapMgr::IndependentHeaps HeapMgr::sIndependentHeaps;

HeapMgr::HeapMgr() : mAllocFailedCallback(0)
{
}
HeapMgr::~HeapMgr()
{
}

void HeapMgr::createRootHeap_()
{
    sRootHeaps.pushBack(ExpHeap::tryCreate(sArena->mStart, sArena->mSize, SafeStringBase<char>("RootHeap"), false));
}

void HeapMgr::initializeImpl_()
{
    sInstance.mAllocFailedCallback = 0;
    createRootHeap_();
    sInstancePtr = &sInstance;
}

void HeapMgr::initialize(Arena* arena)
{
    sArena = arena;
    initializeImpl_();
}

void HeapMgr::destroy()
{
    sHeapTreeLockCS.lock();
    sInstance.mAllocFailedCallback = 0;
    while (!sIndependentHeaps.isEmpty())
    {
        sIndependentHeaps.back()->destroy();
        sIndependentHeaps.popBack();
    }
    while (!sRootHeaps.isEmpty())
    {
        sRootHeaps.back()->destroy();
        sRootHeaps.popBack();
    }
    sInstancePtr = 0;
    sArena->mSize = 0;
    sArena->mStart = 0;
    sArena = 0;
    sHeapTreeLockCS.unlock();
}

Heap* HeapMgr::findContainHeap(const void* address) const
{
    FindContainHeapCache* cache = 0;
    if (ThreadMgr::instance())
    {
        Thread* thread = ThreadMgr::instance()->getCurrentThread();
        if (thread)
        {
            cache = thread->getFindContainHeapCache();
            if (cache)
            {
                Heap* cached = reinterpret_cast<Heap*>(cache->mHeap.fetchOr(1));
                if (cached && cached->mChildren.size() == 0 && cached->isInclude(address))
                {
                    cache->mHeap.fetchAnd(~1u);
                    return cached;
                }
                cache->mHeap.fetchAnd(~1u);
            }
        }
    }
    ScopedLock<CriticalSection> lock(&sHeapTreeLockCS);
    if (cache)
    {
        Heap* cached = reinterpret_cast<Heap*>(cache->mHeap.load());
        if (cached)
        {
            Heap* heap = cached->findContainHeap_(address);
            if (heap)
            {
                if (heap != cached)
                    cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
                return heap;
            }
        }
    }
    for (Heap** it = sRootHeaps.begin(); it != sRootHeaps.end(); ++it)
    {
        Heap* heap = (*it)->findContainHeap_(address);
        if (heap)
        {
            if (cache)
                cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
            return heap;
        }
    }
    for (Heap** it = sIndependentHeaps.begin(); it != sIndependentHeaps.end(); ++it)
    {
        Heap* heap = (*it)->findContainHeap_(address);
        if (heap)
        {
            if (cache)
                cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
            return heap;
        }
    }
    return 0;
}

Heap* HeapMgr::findHeapByName_(Heap* heap, const SafeStringBase<char>& name, int* index)
{
    if (heap->mName == name)
    {
        if (*index == 0)
            return heap;
        --*index;
    }
    for (OffsetList<Heap>::iterator it = heap->mChildren.begin(); it != heap->mChildren.end(); ++it)
    {
        Heap* found = findHeapByName_(&*it, name, index);
        if (found)
            return found;
    }
    return 0;
}

Heap* HeapMgr::getCurrentHeap() const
{
    return ThreadMgr::instance()->getCurrentThread()->getCurrentHeap();
}

bool HeapMgr::isContainedInAnyHeap(const void* address)
{
    for (Heap** it = sRootHeaps.begin(); it != sRootHeaps.end(); ++it)
    {
        if ((*it)->isInclude(address))
            return true;
    }
    for (Heap** it = sIndependentHeaps.begin(); it != sIndependentHeaps.end(); ++it)
    {
        if ((*it)->isInclude(address))
            return true;
    }
    return false;
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap)
{
    if (!heap)
    {
        heap = HeapMgr::instance()->getCurrentHeap();
        if (!heap)
            return 0;
    }
    void* address = heap->tryAlloc(size, 4);
    if (!address)
        return 0;
    return address;
}

void* allocateFromSeadHeap(unsigned int size)
{
    Heap* heap = HeapMgr::instance()->getCurrentHeap();
    if (!heap)
        return 0;
    void* address = heap->tryAlloc(size, 4);
    if (!address)
        return 0;
    return address;
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap, int alignment)
{
    if (!heap)
    {
        heap = HeapMgr::instance()->getCurrentHeap();
        if (!heap)
            return 0;
    }
    void* address = heap->tryAlloc(size, alignment);
    if (!address)
        return 0;
    return address;
}

void freeToSeadHeap(void* address)
{
    Heap* heap = HeapMgr::instance()->findContainHeap(address);
    if (heap)
        heap->free(address);
}

unsigned int getArraySize(void* address)
{
    return MemBlock::findManageArea(address)->mSize;
}

CriticalSection::CriticalSection() : IDisposer(), mCriticalSectionInner()
{
    OSFastMutex_Init(&mCriticalSectionInner, NULL);
}
CriticalSection::CriticalSection(Heap* heap)
    : IDisposer(heap, cHeapNullOption_UseSpecifiedOrContainHeap),
      mCriticalSectionInner()
{
    OSFastMutex_Init(&mCriticalSectionInner, NULL);
}
CriticalSection::~CriticalSection()
{
}
void CriticalSection::lock()
{
    OSFastMutex_Lock(&mCriticalSectionInner);
}
void CriticalSection::unlock()
{
    OSFastMutex_Unlock(&mCriticalSectionInner);
}

pead::Mutex::Mutex() : IDisposer(), mCriticalSectionInner()
{
    OSFastMutex_Init(&mCriticalSectionInner, NULL);
}
Mutex::~Mutex()
{
}
void Mutex::lock()
{
    OSFastMutex_Lock(&mCriticalSectionInner);
}
void Mutex::unlock()
{
    OSFastMutex_Unlock(&mCriticalSectionInner);
}

long long TickSpan::toNanoSeconds() const
{
    return OSTicksToNanoseconds(mTicks);
}
void TickSpan::setNanoSeconds(long long nanoSeconds)
{
    mTicks = OSNanosecondsToTicks(nanoSeconds);
}

pead::Event::Event(bool manualReset) : IDisposer()
{
    initialize(manualReset);
}
Event::~Event()
{
}
void Event::initialize(bool manualReset)
{
    OSInitEvent(reinterpret_cast<OSEvent*>(mStorage), false, manualReset ? OS_EVENT_MODE_MANUAL : OS_EVENT_MODE_AUTO);
}
void Event::wait()
{
    OSWaitEvent(reinterpret_cast<OSEvent*>(mStorage));
}
bool Event::wait(TickSpan timeout)
{
    return OSWaitEventWithTimeout(reinterpret_cast<OSEvent*>(mStorage), timeout.toNanoSeconds());
}
void Event::setSignal()
{
    OSSignalEvent(reinterpret_cast<OSEvent*>(mStorage));
}
void Event::resetSignal()
{
    OSResetEvent(reinterpret_cast<OSEvent*>(mStorage));
}

MessageQueue::MessageQueue() : mBuffer(0), mQueue()
{
}
MessageQueue::~MessageQueue()
{
    free();
}
void MessageQueue::allocate(int size, Heap* heap)
{
    if (size > 0)
    {
        mBuffer = NewArray<OSMessage>(size, heap);
        OSInitMessageQueue(&mQueue, mBuffer, size);
    }
}
void MessageQueue::free()
{
    if (mBuffer)
    {
        DeleteArray(mBuffer);
        mBuffer = 0;
    }
}
bool MessageQueue::push(int message, BlockType block)
{
    OSMessage msg;
    msg.message = reinterpret_cast<void*>(message);
    return OSSendMessage(&mQueue, &msg, block == cBlock ? OS_MESSAGE_FLAGS_BLOCKING : OS_MESSAGE_FLAGS_NONE) != FALSE;
}
int MessageQueue::pop(BlockType block)
{
    OSMessage msg;
    if (OSReceiveMessage(&mQueue, &msg, block == cBlock ? OS_MESSAGE_FLAGS_BLOCKING : OS_MESSAGE_FLAGS_NONE))
        return reinterpret_cast<int>(msg.message);
    return 0;
}
bool MessageQueue::jam(int message, BlockType block)
{
    OSMessage msg;
    msg.message = reinterpret_cast<void*>(message);
    return OSSendMessage(
               &mQueue, &msg,
               static_cast<OSMessageFlags>((block == cBlock ? OS_MESSAGE_FLAGS_BLOCKING : OS_MESSAGE_FLAGS_NONE)
                                           | OS_MESSAGE_FLAGS_HIGH_PRIORITY))
           != FALSE;
}

Thread::Thread(Heap* heap, OSThread* thread)
    : IDisposer(),
      mName("pead::MainThread"),
      mMessageQueue(),
      mStackSize(0),
      mListNode(),
      mCurrentHeap(0),
      mFindContainHeapCache(),
      mBlockType(MessageQueue::cNoBlock),
      mQuitMsg(cDefaultQuitMsg),
      mId(reinterpret_cast<uintptr_t>(thread)),
      mState(cState_Running),
      mAffinity(1),
      mThread(thread),
      mStack(0),
      mPriority(OSGetThreadPriority(thread))
{
    mListNode.mData = this;
    mMessageQueue.allocate(cDefaultMessageQueueSize, heap);
    mStack = static_cast<unsigned char*>(thread->stackEnd);
    mStackSize = static_cast<unsigned char*>(thread->stackStart) - mStack;
}
Thread::Thread(const SafeStringBase<char>& name, Heap* heap, int priority, MessageQueue::BlockType blockType,
               int quitMsg, int stackSize, int messageQueueSize)
    : IDisposer(),
      mName(name),
      mMessageQueue(),
      mStackSize(stackSize),
      mListNode(),
      mCurrentHeap(0),
      mFindContainHeapCache(),
      mBlockType(blockType),
      mQuitMsg(quitMsg),
      mId(0),
      mState(cState_Initialized),
      mAffinity(1),
      mStack(0),
      mPriority(priority)
{
    mListNode.mData = this;
    mMessageQueue.allocate(messageQueueSize, heap);
    mStack = NewArray<unsigned char>(mStackSize, heap);
    mThread = new (allocateFromSeadHeap(sizeof(OSThread), heap, cThreadAlignment)) OSThread();
    mThread->stackEnd = reinterpret_cast<void*>(cUncreatedStackEnd);
    mThread->stackStart = 0;
    mId = reinterpret_cast<uintptr_t>(mThread);
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (threadMgr)
    {
        ScopedLock<CriticalSection> lock(&threadMgr->mListCS);
        threadMgr->mList.pushBack(&mListNode);
    }
}
Thread::~Thread()
{
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (threadMgr)
    {
        if (this != threadMgr->mMainThread)
        {
            {
                ScopedLock<CriticalSection> lock(&threadMgr->mListCS);
                mListNode.erase();
            }
            if (mState == cState_Running)
                quitAndWaitDoneSingleThread(false);
            else if (mState == cState_Quitting || mState == cState_Terminated)
                waitDone();
            if (mThread)
                freeToSeadHeap(mThread);
            DeleteArray(mStack);
        }
        mMessageQueue.free();
    }
}
void Thread::waitDone()
{
    if (mState != cState_Initialized && mState != cState_Released)
    {
        OSJoinThread(mThread, 0);
        mState = cState_Released;
    }
}
int Thread::cafeThreadFunc_(int, void* context)
{
    Thread* thread = static_cast<Thread*>(context);
    thread->run_();
    thread->mState = cState_Terminated;
    return 0;
}
unsigned int Thread::start()
{
    if (mState != cState_Initialized)
        return 0;
    if (!OSCreateThread(mThread, reinterpret_cast<OSThreadEntryPointFn>(cafeThreadFunc_), 0,
                        reinterpret_cast<char*>(this), mStack + mStackSize, mStackSize, mPriority, 0))
        return 0;
    OSSetThreadName(mThread, mName.cstr());
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < CoreInfo::sCoreCount; ++id)
    {
        if (mAffinity.isOn(CoreId(id)))
            platformMask |= CoreInfo::getPlatformMask(CoreId(id));
    }
    OSSetThreadAffinity(mThread, platformMask);
    mState = cState_Running;
    mId = reinterpret_cast<uintptr_t>(mThread);
    OSResumeThread(mThread);
    return 1;
}
void Thread::sleep(TickSpan span)
{
    OSSleepTicks(span.getTicks());
}
void Thread::yield()
{
    OSYieldThread();
}
void Thread::setPriority(int priority)
{
    mPriority = priority;
    if (mState == cState_Initialized || mState == cState_Released)
        return;
    OSSetThreadPriority(mThread, priority);
}
int Thread::getPriority() const
{
    return mPriority;
}
void Thread::setAffinity(const CoreIdMask& affinity)
{
    mAffinity = affinity;
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < CoreInfo::sCoreCount; ++id)
    {
        if (mAffinity.isOn(CoreId(id)))
            platformMask |= CoreInfo::getPlatformMask(CoreId(id));
    }
    OSSetThreadAffinity(mThread, platformMask);
}
void* Thread::getStackCheckStartAddress_() const
{
    return mStack ? mStack + 4 : 0;
}
void Thread::destroy()
{
    waitDone();
}
MessageQueue* Thread::getMessageQueue() const
{
    return const_cast<MessageQueue*>(&mMessageQueue);
}
bool Thread::sendMessage(int message, MessageQueue::BlockType block)
{
    if (!message || mState == cState_Terminated || mState == cState_Released || message == mQuitMsg)
        return false;
    return mMessageQueue.push(message, block);
}
int Thread::recvMessage(MessageQueue::BlockType block)
{
    if (mState == cState_Quitting)
        return 0;
    return mMessageQueue.pop(block);
}
void Thread::quit(bool isJam)
{
    if (isDone())
        return;
    if (mState == cState_Running)
        mState = cState_Quitting;
    if (isJam)
        mMessageQueue.jam(mQuitMsg, MessageQueue::cBlock);
    else
        mMessageQueue.push(mQuitMsg, MessageQueue::cBlock);
}
void Thread::quitAndDestroySingleThread(bool jam)
{
    quitAndWaitDoneSingleThread(jam);
    destroy();
}
void Thread::quitAndWaitDoneSingleThread(bool jam)
{
    quit(jam);
    waitDone();
}
MessageQueue::BlockType Thread::getBlockType() const
{
    return mBlockType;
}
int Thread::getStackSize() const
{
    return mStackSize;
}
void Thread::run_()
{
    int message;
    while ((message = mMessageQueue.pop(mBlockType)) != mQuitMsg)
        calc_(message);
}
int Thread::calcStackUsedSizePeak() const
{
    return 0;
}

MainThread::~MainThread()
{
    mState = cState_Terminated;
}
void MainThread::destroy()
{
}
void MainThread::quit(bool)
{
}
void MainThread::waitDone()
{
}
void MainThread::quitAndDestroySingleThread(bool)
{
}
void MainThread::setPriority(int)
{
}
void MainThread::calc_(int)
{
}

DelegateThread::DelegateThread(const SafeStringBase<char>& name, IDelegate2<Thread*, int>* delegate, Heap* heap,
                               int priority, MessageQueue::BlockType block, int quitMsg, int stackSize,
                               int messageQueueSize)
    : Thread(name, heap, priority, block, quitMsg, stackSize, messageQueueSize),
      mDelegate(delegate)
{
}
DelegateThread::~DelegateThread()
{
}
void DelegateThread::calc_(int message)
{
    mDelegate->invoke(this, message);
}

ThreadMgr* ThreadMgr::sInstance = 0;
ThreadMgr::SingletonDisposer_* ThreadMgr::SingletonDisposer_::sStaticDisposer = 0;

ThreadMgr::SingletonDisposer_::~SingletonDisposer_()
{
    if (this == sStaticDisposer)
    {
        sStaticDisposer = 0;
        sInstance->~ThreadMgr();
        sInstance = 0;
    }
}

void HeapMgr::removeFromFindContainHeapCache_(Heap* heap)
{
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (!threadMgr)
        return;
    Thread* mainThread = threadMgr->getMainThread();
    TickSpan wait = TickSpan::makeFromMicroSeconds(10);
    if (mainThread)
    {
        while (!mainThread->getFindContainHeapCache()->tryRemoveHeap(heap))
            Thread::sleep(wait);
    }
    for (;;)
    {
        threadMgr->mListCS.lock();
        bool removed = true;
        for (TListNode<Thread*>* node = threadMgr->mList.begin(); node != threadMgr->mList.end();
             node = ThreadMgr::ThreadList::next(node))
        {
            if (!node->mData->getFindContainHeapCache()->tryRemoveHeap(heap))
            {
                removed = false;
                break;
            }
        }
        if (removed)
            break;
        threadMgr->mListCS.unlock();
        Thread::sleep(wait);
    }
    threadMgr->mListCS.unlock();
}

ThreadMgr::ThreadMgr() : mList(), mListCS(), mMainThread(0)
{
}
ThreadMgr::~ThreadMgr()
{
    ScopedLock<CriticalSection> lock(&mListCS);
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
        node->mData->quit(false);
    bool allDone;
    do
    {
        allDone = true;
        for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
            allDone &= node->mData->isDone();
        Thread::yield();
    } while (!allDone);
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
        node->mData->waitDone();
    sInstance = 0;
}
ThreadMgr* ThreadMgr::createInstance(Heap* heap)
{
    if (!sInstance)
    {
        unsigned char* buffer = NewArray<unsigned char>(sizeof(ThreadMgr), heap);
        SingletonDisposer_::sStaticDisposer = new (buffer) SingletonDisposer_(heap);
        sInstance = new (buffer) ThreadMgr();
    }
    return sInstance;
}
void ThreadMgr::deleteInstance()
{
    SingletonDisposer_* disposer = SingletonDisposer_::sStaticDisposer;
    if (disposer)
    {
        SingletonDisposer_::sStaticDisposer = 0;
        disposer->~SingletonDisposer_();
        if (sInstance)
        {
            sInstance->~ThreadMgr();
            freeToSeadHeap(sInstance);
        }
        sInstance = 0;
    }
}
void ThreadMgr::initMainThread_(Heap* heap)
{
    OSThread* thread = OSGetCurrentThread();
    mMainThread = new (heap) MainThread(heap, thread);
}
unsigned int ThreadMgr::getCurrentThreadID_()
{
    return reinterpret_cast<uintptr_t>(OSGetCurrentThread());
}
void ThreadMgr::initialize(Heap* heap)
{
    initMainThread_(heap);
}
void ThreadMgr::destroyMainThread_()
{
    if (mMainThread)
    {
        mMainThread->~MainThread();
        freeToSeadHeap(mMainThread);
        mMainThread = 0;
    }
}
void ThreadMgr::destroy()
{
    destroyMainThread_();
}
Thread* ThreadMgr::getCurrentThread() const
{
    ScopedLock<CriticalSection> lock(&mListCS);
    const unsigned int id = getCurrentThreadID_();
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
    {
        if (node->mData->mId == id)
            return node->mData;
    }
    return mMainThread;
}
void ThreadMgr::waitDoneMultipleThread(Thread* const* threads, int count)
{
    bool allDone;
    do
    {
        allDone = true;
        for (int index = 0; index < count; ++index)
            allDone &= threads[index]->isDone();
        Thread::yield();
    } while (!allDone);
    for (int index = 0; index < count; ++index)
        threads[index]->waitDone();
}
void ThreadMgr::quitAndWaitDoneMultipleThread(Thread** threads, int count, bool jam)
{
    for (int index = 0; index < count; ++index)
        threads[index]->quit(jam);
    waitDoneMultipleThread(threads, count);
}

PrintOutput::~PrintOutput()
{
}
void PrintOutput::writeLineBreak()
{
    write("\n", 1);
}
BufferingPrintOutput::~BufferingPrintOutput()
{
    mStreamSrc.BufferMultiByteTextWriteStreamSrc::write(&SafeStringBase<char>::cNullChar, sizeof(char));
    mStreamSrc.BufferMultiByteNullTerminatedTextWriteStreamSrc::flush();
}
void BufferingPrintOutput::write(const char* text, int length)
{
    mStreamSrc.BufferMultiByteTextWriteStreamSrc::write(text, length);
}
StringPrintOutput::~StringPrintOutput()
{
}
void StringPrintOutput::write(const char* text, int length)
{
    BufferedSafeStringBase<char> buffer(mString, mPosition);
    mPosition += buffer.copy(SafeStringBase<char>(text), length);
}
StringCutOffPrintOutput::~StringCutOffPrintOutput()
{
}
void StringCutOffPrintOutput::write(const char* text, int length)
{
    BufferedSafeStringBase<char> buffer(mString, mPosition);
    mPosition += buffer.copy(SafeStringBase<char>(text), length);
}
StreamPrintOutput::~StreamPrintOutput()
{
    mSrc->flush();
}
void StreamPrintOutput::write(const char* text, int length)
{
    mSrc->write(text, length);
}

bool PrintFormatter::proceedToFormatMark_(char* specifier)
{
    specifier[0] = 0;
    if (!mFormat)
        return false;
    if (mPassThrough)
        return true;
    if (mPosition >= mLength)
        return false;
    const char* segment = mFormat + mPosition;
    int count = 0;
    while (segment[count] != 0)
    {
        if (segment[count] != '%')
        {
            ++count;
            continue;
        }
        if (segment[count + 1] == '%')
        {
            mOutput->write(segment, count + 1);
            segment += count + 2;
            mPosition += count + 2;
            count = 0;
            continue;
        }
        if (count > 0)
        {
            mOutput->write(segment, count);
            mPosition += count;
            segment += count;
        }
        if (segment[1] == '@')
        {
            mPosition += 2;
            return true;
        }
        if (segment[1] == '<')
        {
            mPassThrough = true;
            mPosition += 2;
            return true;
        }
        specifier[0] = '%';
        for (int index = 1;; ++index)
        {
            const char character = segment[index];
            if (character != '+' && character != '-' && character != ' ' && character != '#' && character != 'h'
                && character != 'l' && character != 'L' && character != '.' && !(character >= '0' && character <= '9'))
            {
                if (character != 0)
                {
                    specifier[index] = character;
                    specifier[index + 1] = 0;
                    mPosition += index + 1;
                    return true;
                }
                specifier[0] = 0;
                mPosition = mLength;
                return true;
            }
            if (index > cMaxSpecifierLength)
            {
                specifier[0] = 0;
                mPosition = mLength;
                return true;
            }
            specifier[index] = character;
        }
    }
    if (count > 0)
    {
        mOutput->write(segment, count);
        mPosition += count;
    }
    return false;
}
void PrintFormatter::outputString_(const char* option, PrintOutput* output, const char* text, int length)
{
    if (!option || option[1] == 's')
    {
        if (length == -1)
            output->write(text, strlen(text));
        else
            output->write(text, length);
        return;
    }
    if (option[strlen(option) - 1] == 's')
    {
        FixedSafeString128 converted;
        SafeStringBase<char> source(text);
        int convertedLength = StringUtil::convertUtf8ToSjis(converted.getBuffer(), converted.getBufferSize(),
                                                            source.cstr(), -1);
        if (option[1] == '-')
        {
            output->write(text, length == -1 ? strlen(text) : length);
            int width = StringUtil::parseS32(SafeStringBase<char>(option + 2), StringUtil::cDecimal);
            for (int index = 0; index < width - convertedLength; ++index)
                output->write(" ", 1);
        }
        else
        {
            int width = StringUtil::parseS32(SafeStringBase<char>(option + 1), StringUtil::cDecimal);
            for (int index = 0; index < width - convertedLength; ++index)
                output->write(" ", 1);
            if (length == -1)
                output->write(text, strlen(text));
            else
                output->write(text, length);
        }
    }
    else
    {
        FixedSafeString128 formatted;
        int formattedLength = formatted.format(option, text);
        output->write(formatted.cstr(), formattedLength);
    }
}
void PrintFormatter::outputPtr_(const char* option, PrintOutput* output, unsigned int pointer)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, pointer);
    else
        length = text.format("0x%08X", pointer);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned char>(const unsigned char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned short>(const unsigned short& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned int>(const unsigned int& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned long long>(const unsigned long long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%llu", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<signed char>(const signed char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<short>(const short& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<int>(const int& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<long long>(const long long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%lld", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<float>(const float& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%f", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<double>(const double& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%f", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<char>(const char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<long>(const long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<bool>(const bool& value, const char* option, PrintOutput* output)
{
    if (option)
        out<int>(value, option, output);
    else if (value)
        output->write("true", 4);
    else
        output->write("false", 5);
}

template <>
void PrintFormatter::out<char>(const char* value, const char* option, PrintOutput* output)
{
    bool isString = true;
    if (option)
        isString = SafeStringBase<char>(option).include('s');
    if (isString)
    {
        outputString_(option, output, value, -1);
        return;
    }
    FixedSafeString<32> text;
    int length = text.format(option, value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<wchar_t>(const wchar_t* value, const char* option, PrintOutput* output)
{
    bool isString = true;
    if (option)
        isString = SafeStringBase<char>(option).include('s');
    if (isString)
    {
        FixedSafeString<256> text;
        int length = StringUtil::convertUtf16ToUtf8(text.getBuffer(), text.getBufferSize(),
                                                    SafeStringBase<wchar_t>(value).cstr(), -1);
        outputString_(option, output, text.cstr(), length);
        return;
    }
    FixedSafeString<32> text;
    int length = text.format(option, value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<TickSpan>(const TickSpan& value, const char*, PrintOutput* output)
{
    out<long long>(value.toMicroSeconds(), "%lld", output);
    output->write("us", 2);
}

template <>
void PrintFormatter::OutImpl<char, SafeStringBase>::out(const SafeStringBase<char>& value, const char* option,
                                                        PrintOutput* output)
{
    outputString_(option, output, value.cstr(), value.calcLength());
}

template <>
void PrintFormatter::OutImpl<char, BufferedSafeStringBase>::out(const BufferedSafeStringBase<char>& value,
                                                                const char* option, PrintOutput* output)
{
    outputString_(option, output, value.cstr(), value.calcLength());
}

template <>
void PrintFormatter::out<Heap>(const Heap& heap, const char*, PrintOutput* output)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&heap.mCS), heap.isEnableLock());
    FixedSafeString128 text;
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>("\n"), 0, output);
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>("==================================================\n"), 0,
                                       output);
    text.format("              Name: %s\n", heap.mName.cstr());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("             Range: [0x%08X - 0x%08X)\n", heap.getStartAddress(), heap.getEndAddress());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    if (!heap.mParent)
        text.format("            Parent: %s (0x%08X)\n", "--", 0);
    else
        text.format("            Parent: %s (0x%08X)\n", heap.mParent->mName.cstr(), heap.mParent);
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("         Direction: %s\n", heap.mDirection == Heap::cHeapDirection_Forward ? "Forward" : "Reverse");
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("              Size: %u\n", heap.getSize());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("          FreeSize: %u\n", heap.getFreeSize());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("MaxAllocatableSize: %u\n", heap.getMaxAllocatableSize(4));
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>("--------------------------------------------------\n"), 0,
                                       output);
}

template <>
void PrintFormatter::out<ExpHeap>(const ExpHeap& heap, const char*, PrintOutput* output)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&heap.mCS), heap.isEnableLock());
    out<Heap>(heap, 0, output);
    FixedSafeString128 text;
    text.format("         AllocMode: %s\n",
                heap.getAllocMode() == ExpHeap::cAllocMode_FirstFit ? "First Fit" : "Best Fit");
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("      UseList size: %d\n", heap.mUseList.size());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    text.format("     FreeList size: %d\n", heap.mFreeList.size());
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>(text.cstr()), 0, output);
    OutImpl<char, SafeStringBase>::out(SafeStringBase<char>("==================================================\n"), 0,
                                       output);
}

template <>
void PrintFormatter::OutImpl<unsigned char, BitFlag>::out(const BitFlag<unsigned char>& value, const char* option,
                                                          PrintOutput* output)
{
    unsigned char bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned short, BitFlag>::out(const BitFlag<unsigned short>& value, const char* option,
                                                           PrintOutput* output)
{
    unsigned short bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned int, BitFlag>::out(const BitFlag<unsigned int>& value, const char* option,
                                                         PrintOutput* output)
{
    unsigned int bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned long long, BitFlag>::out(const BitFlag<unsigned long long>& value,
                                                               const char* option, PrintOutput* output)
{
    unsigned long long bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%llx", bits);
    output->write(text.cstr(), length);
}

void PrintConfig::registerFinalCallback(IDelegate1<const PrintEventArg&>* callback)
{
    sFinalPrintCallback = callback;
}
void PrintConfig::execCallbacks(const PrintEventArg& event)
{
    if (sPrintEventEnabled)
        sPrintEvent.invoke(event);
    if (sFinalPrintCallback)
        sFinalPrintCallback->invoke(event);
    else
        system::PrintStringImpl(event.text, event.level);
}

int StringUtil::__N_18_peadStringUtil_cpp_6714fef4::checkAndConvertDecChar_(char character)
{
    return character > '9' ? -1 : character - '0';
}

int StringUtil::__N_18_peadStringUtil_cpp_6714fef4::checkAndConvertHexChar_(char character)
{
    if (character >= 'A' && character <= 'Z')
        character = static_cast<char>(character + ('a' - 'A'));
    if (character >= '0' && character <= '9')
        return character - '0';
    if (character >= 'a' && character <= 'f')
        return character - 'a' + 10;
    return -1;
}

void StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseSignImpl_(int* sign,
                                                                       SafeStringBase<char>::iterator& current)
{
    *sign = 1;
    char character = *current;
    if (character == '+')
    {
        ++current;
    }
    else if (character == '-')
    {
        *sign = -1;
        ++current;
    }
}

bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParsePrefixImpl_(int* base,
                                                                         SafeStringBase<char>::iterator& current,
                                                                         const SafeStringBase<char>& text)
{
    char character = *current;
    if (character < '0' || character > '9')
        return false;
    if (character == '0')
    {
        char next = text.at(current.getIndex() + 1);
        if (next == 'b')
        {
            *base = 2;
            ++current;
            ++current;
        }
        else if (next == 'x')
        {
            *base = 16;
            ++current;
            ++current;
        }
        else
        {
            *base = 8;
        }
    }
    else
    {
        *base = 10;
    }
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseNumberImpl_(T* value,
                                                                         SafeStringBase<char>::iterator& current,
                                                                         int base)
{
    T result = 0;
    bool parsed = false;
    for (;;)
    {
        int digit = checkAndConvertHexChar_(*current);
        if (digit < 0 || digit >= base)
            break;
        if (result > MathCalcCommon<T>::maxNumber() / base)
            return false;
        if (result * base > MathCalcCommon<T>::maxNumber() - digit)
            return false;
        result = result * base + digit;
        parsed = true;
        ++current;
    }
    if (!parsed)
        return false;
    if (value)
        *value = result;
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseDecImpl_(T* value, SafeStringBase<char>::iterator& current,
                                                                      int sign)
{
    T result = 0;
    bool parsed = false;
    for (;;)
    {
        int digit = checkAndConvertDecChar_(*current);
        if (digit < 0)
            break;
        if (result < MathCalcCommon<T>::minNumber() / 10 || result > MathCalcCommon<T>::maxNumber() / 10)
            return false;
        result *= 10;
        if (sign < 0)
        {
            if (result < MathCalcCommon<T>::minNumber() + digit)
                return false;
        }
        else if (result > MathCalcCommon<T>::maxNumber() - digit)
        {
            return false;
        }
        result += sign * digit;
        parsed = true;
        ++current;
    }
    if (!parsed)
        return false;
    if (value)
        *value = result;
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseImpl_(T* value, const SafeStringBase<char>& text, int base)
{
    SafeStringBase<char>::iterator current(&text);
    int sign;
    tryParseSignImpl_(&sign, current);
    if (base > 0 || tryParsePrefixImpl_(&base, current, text))
    {
        if (base == 10)
            return tryParseDecImpl_(value, current, sign);
        typename UnsignedType<T>::Type result;
        if (tryParseNumberImpl_(&result, current, base))
        {
            *value = result * sign;
            return true;
        }
    }
    return false;
}

template <typename T>
void StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseUnderDecimalPointImpl_(
    T* value, SafeStringBase<char>::iterator& current, T sign, T integer)
{
    T numerator = 0;
    T denominator = 1;
    for (;;)
    {
        int digit = checkAndConvertDecChar_(*current);
        if (digit < 0)
            break;
        denominator *= 10;
        ++current;
        numerator = numerator * 10 + sign * digit;
    }
    if (denominator != 1 && value)
        *value = numerator / denominator + integer;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseDecimalFractionImpl_(T* value,
                                                                                  const SafeStringBase<char>& text)
{
    SafeStringBase<char>::iterator current(&text);
    int sign;
    tryParseSignImpl_(&sign, current);
    if (!tryParseDecImpl_(value, current, sign))
        return false;
    if (*current == '.')
    {
        ++current;
        tryParseUnderDecimalPointImpl_(value, current, static_cast<T>(sign), *value);
    }
    return true;
}

template <typename T>
T pead::StringUtil::__N_18_peadStringUtil_cpp_6714fef4::parseImpl_(const SafeStringBase<char>& text,
                                                                   CardinalNumber cardinal)
{
    T value = 0;
    tryParseImpl_(&value, text, cardinal);
    return value;
}

template <typename T>
T StringUtil::__N_18_peadStringUtil_cpp_6714fef4::parseDecimalFractionImpl_(const SafeStringBase<char>& text)
{
    T value = 0;
    tryParseDecimalFractionImpl_(&value, text);
    return value;
}

namespace
{
enum
{
    cByteBits = 8,
    cByteMask = 0xff,
    cAsciiEnd = 0x80,
    cHalfwidthKatakanaFirst = 0xff61,
    cHalfwidthKatakanaCount = 0x3f,
    cHalfwidthKatakanaToSjis = 0xfec0,
    cSurrogateFirst = 0xd800,
    cSurrogateToPrivateEndSpan = 0x20ff,
    cPrivateUseFirst = 0xe000,
    cPrivateUseCount = 0x758,
    cSjisRowLength = 0xbc,
    cSjisUserDefinedLeadFirst = 0xf0,
    cSjisTrailFirst = 0x40,
    cSjisTrailGap = 0x3f,
    cSjisSpecialA0Source = 0xf8f0,
    cSjisSpecialA0 = 0xa0,
    cSjisSpecialFdSource = 0xf8f1,
    cSjisSpecialFdCount = 3,
    cSjisSpecialFdOffset = 0xf7f4,
    cSjisEntryUtf16High = 0,
    cSjisEntryLead = 1,
    cSjisEntryTrail = 2,
    cSjisEntrySize = 3,
    cSjisCodeCount = 7415,
    cSjisIndexCount = 257,
    cSjisIndexSize = 2,
    cUtf8Lead3First = 0xe0,
    cUtf8Lead4First = 0xf0,
    cUtf8Lead2Payload = 0x1f,
    cUtf8TrailPayload = 0x3f,
    cUtf8TrailBits = 6,
    cUtf8ConvertBufferLength = 4,
    cDoubleMantissaBits = 52,
    cDoubleMantissaShift = 11,
    cDoubleExponentMask = 0x7ff,
    cFloatDigitRecordCount = 2098,
    cFloatDigitRecordSize = 24,
    cFloatDigitOffset = 4,
    cFloatAccumulatorLength = 21,
    cFloatMaxSignificantDigits = 17,
    cFloatMaxPrecision = 512,
    cFloatRoundHalf = 4,
    cFloatZeroExponent = 1,
    cFloatGeneralMinExponent = -3,
    cFloatExponentTextLength = 5,
    cFloatExponentDigits = 3
};

const unsigned long long cDoubleHiddenBit = 1ULL << cDoubleMantissaBits;
const unsigned long long cDoubleFractionMask = cDoubleHiddenBit - 1;
const unsigned long long cDoubleSignBit = 1ULL << (cDoubleMantissaBits + cDoubleMantissaShift);

const unsigned char sSjisCodeTable[cSjisCodeCount][cSjisEntrySize] = {
#include "platform/wiiu/shaders/PeadSjisCodeTable.inc"
};

const unsigned char sSjisIndexTable[cSjisIndexCount * cSjisIndexSize] = {
#include "platform/wiiu/shaders/PeadSjisIndexTable.inc"
};

const unsigned char sFloatDigitTable[cFloatDigitRecordCount][cFloatDigitRecordSize] = {
#include "platform/wiiu/shaders/PeadFloatDigitTable.inc"
};

}

namespace StringUtilImpl = StringUtil::__N_18_peadStringUtil_cpp_6714fef4;
typedef StringUtilImpl::FormatParser16 FormatParser16;
typedef StringUtilImpl::OutBuffer16 OutBuffer16;

FormatParser16::FormatParser16(va_list args)
{
    mArgs[0] = args[0];
    clear_();
}
void FormatParser16::clear_()
{
    mFlags = 0;
    mWidth = 0;
    mPrecision = 0;
    mHasPrecision = false;
    mSize = cSizeInt;
}
const wchar_t* FormatParser16::parseFlag_(const wchar_t* format)
{
    for (;; ++format)
    {
        if (*format == L'-')
            mFlags |= cFlagLeftJustify;
        else if (*format == L'+')
            mFlags |= cFlagForceSign;
        else if (*format == L' ')
            mFlags |= cFlagBlankSign;
        else if (*format == L'#')
            mFlags |= cFlagAlternate;
        else if (*format == L'0')
            mFlags |= cFlagZeroPad;
        else
            return format;
    }
}
const wchar_t* FormatParser16::parseWidth_(const wchar_t* format)
{
    if (*format == L'*')
    {
        mWidth = va_arg(mArgs, int);
        return format + 1;
    }
    if (*format >= L'1' && *format <= L'9')
    {
        mWidth = *format++ - L'0';
        while (*format >= L'0' && *format <= L'9')
            mWidth = mWidth * 10 + *format++ - L'0';
    }
    return format;
}
const wchar_t* FormatParser16::parsePrecision_(const wchar_t* format)
{
    if (*format != L'.')
        return format;
    mHasPrecision = true;
    ++format;
    if (*format == L'*')
    {
        mPrecision = va_arg(mArgs, int);
        return format + 1;
    }
    if (*format >= L'0' && *format <= L'9')
    {
        mPrecision = *format++ - L'0';
        while (*format >= L'0' && *format <= L'9')
            mPrecision = mPrecision * 10 + *format++ - L'0';
    }
    return format;
}
const wchar_t* FormatParser16::parseSize_(const wchar_t* format)
{
    if (*format == L'h')
    {
        ++format;
        if (*format == L'h')
        {
            ++format;
            mSize = cSizeChar;
        }
        else
            mSize = cSizeShort;
    }
    else if (*format == L'l')
    {
        ++format;
        if (*format == L'l')
        {
            ++format;
            mSize = cSizeLongLong;
        }
        else
            mSize = cSizeLong;
    }
    return format;
}

template <typename T>
void FormatParser16::printIntegerInverseString_(OutBuffer16* output, unsigned int base, bool upper, T value)
{
    if (value == 0)
    {
        output->put(L'0');
        return;
    }
    if (base == cBaseOctal)
    {
        while (value != 0)
        {
            output->put(static_cast<wchar_t>(L'0' + (value & cOctalDigitMask)));
            value >>= cOctalDigitBits;
        }
    }
    else if (base == cBaseDecimal)
    {
        while (value != 0)
        {
            output->put(static_cast<wchar_t>(L'0' + value % cBaseDecimal));
            value /= cBaseDecimal;
        }
    }
    else if (base == cBaseHexadecimal)
    {
        const wchar_t letter = upper ? L'A' : L'a';
        while (value != 0)
        {
            const unsigned int digit = static_cast<unsigned int>(value & cHexDigitMask);
            if (digit < cBaseDecimal)
                output->put(static_cast<wchar_t>(L'0' + digit));
            else
                output->put(static_cast<wchar_t>(letter - cBaseDecimal + digit));
            value >>= cHexDigitBits;
        }
    }
}

template <typename U, typename S, typename A>
void FormatParser16::printIntegerInverseStringFromArg_(OutBuffer16* output, bool* negative, bool signedValue,
                                                       unsigned int base, bool upper)
{
    if (!signedValue)
    {
        printIntegerInverseString_<U>(output, base, upper,
                                      static_cast<typename UnsignedType<S>::Type>(va_arg(mArgs, A)));
        return;
    }
    const S value = static_cast<S>(va_arg(mArgs, A));
    if (value >= 0)
    {
        printIntegerInverseString_<U>(output, base, upper, value);
        return;
    }
    printIntegerInverseString_<U>(output, base, upper, -value);
    *negative = true;
}

template <typename T>
void FormatParser16::printStringCore_(OutBuffer16* output, const T* text)
{
    static const T cEmpty[1] = {0};
    if (!text)
        text = cEmpty;
    unsigned int length = 0;
    if (mFlags & cFlagLeftJustify)
    {
        if (!mHasPrecision)
        {
            const T* current = text;
            while (*current != 0)
                output->put(static_cast<wchar_t>(static_cast<typename UnsignedType<T>::Type>(*current++)));
            length = current - text;
        }
        else
        {
            while (*text != 0 && length < static_cast<unsigned int>(mPrecision))
            {
                output->put(static_cast<wchar_t>(static_cast<typename UnsignedType<T>::Type>(*text++)));
                ++length;
            }
        }
        if (length < static_cast<unsigned int>(mWidth))
            output->put(L' ', mWidth - length);
        return;
    }
    if (!mHasPrecision)
    {
        const T* current = text;
        while (*current != 0)
            ++current;
        length = current - text;
    }
    else
    {
        while (text[length] != 0 && length < static_cast<unsigned int>(mPrecision))
            ++length;
    }
    if (length < static_cast<unsigned int>(mWidth))
        output->put((mFlags & cFlagZeroPad) ? L'0' : L' ', mWidth - length);
    for (unsigned int index = 0; index < length; ++index)
        output->put(static_cast<wchar_t>(static_cast<typename UnsignedType<T>::Type>(text[index])));
}

void FormatParser16::printInteger_(OutBuffer16* output, unsigned int base, bool signedValue, bool upper)
{
    wchar_t digitBuffer[cIntegerDigitBufferLength];
    OutBuffer16 digits(digitBuffer, cIntegerDigitCapacity);
    bool negative = false;
    switch (mSize)
    {
    case cSizeChar:
        printIntegerInverseStringFromArg_<unsigned int, signed char, int>(&digits, &negative, signedValue, base, upper);
        break;
    case cSizeShort:
        printIntegerInverseStringFromArg_<unsigned int, short, int>(&digits, &negative, signedValue, base, upper);
        break;
    case cSizeInt:
    case cSizeLong:
        printIntegerInverseStringFromArg_<unsigned int, int, int>(&digits, &negative, signedValue, base, upper);
        break;
    case cSizeLongLong:
        printIntegerInverseStringFromArg_<unsigned long long, long long, long long>(&digits, &negative, signedValue,
                                                                                    base, upper);
        break;
    }
    const unsigned int digitCount = digits.length;
    unsigned int totalLength = digitCount;
    unsigned int zeroCount = 0;
    if (mHasPrecision && digitCount < static_cast<unsigned int>(mPrecision))
    {
        zeroCount = mPrecision - digitCount;
        totalLength = mPrecision;
    }
    unsigned int leftSpaceCount = 0;
    unsigned int rightSpaceCount = 0;
    if (signedValue)
    {
        if (negative || (mFlags & (cFlagForceSign | cFlagBlankSign)))
            ++totalLength;
    }
    else if (mFlags & cFlagAlternate)
    {
        if (base == cBaseOctal)
            ++totalLength;
        else if (base == cBaseHexadecimal)
            totalLength += 2;
    }
    if (totalLength < static_cast<unsigned int>(mWidth))
    {
        if (mFlags & cFlagLeftJustify)
            rightSpaceCount = mWidth - totalLength;
        else if ((mFlags & cFlagZeroPad) && !mHasPrecision)
            zeroCount += mWidth - totalLength;
        else
            leftSpaceCount = mWidth - totalLength;
    }
    output->put(L' ', leftSpaceCount);
    if (signedValue)
    {
        if (negative)
            output->put(L'-');
        else if (mFlags & cFlagForceSign)
            output->put(L'+');
        else if (mFlags & cFlagBlankSign)
            output->put(L' ');
    }
    else if (mFlags & cFlagAlternate)
    {
        if (base == cBaseOctal)
        {
            output->put(L'0');
        }
        else if (base == cBaseHexadecimal)
        {
            output->put(L'0');
            output->put(upper ? L'X' : L'x');
        }
    }
    output->put(L'0', zeroCount);
    for (int index = static_cast<int>(digitCount) - 1; index >= 0; --index)
        output->put(digitBuffer[index]);
    output->put(L' ', rightSpaceCount);
    digitBuffer[digitCount] = 0;
}

void FormatParser16::printCharacter_(OutBuffer16* output)
{
    wchar_t character = 0;
    if (mSize < cSizeInt)
        character = static_cast<unsigned char>(va_arg(mArgs, int));
    else if (mSize <= cSizeLongLong)
        character = static_cast<unsigned short>(va_arg(mArgs, int));
    const unsigned int length = character != 0 ? 1 : 0;
    if (length >= static_cast<unsigned int>(mWidth))
    {
        if (character != 0)
            output->put(character);
        return;
    }
    const unsigned int padCount = mWidth - length;
    if (!(mFlags & cFlagLeftJustify))
        output->put((mFlags & cFlagZeroPad) ? L'0' : L' ', padCount);
    if (character != 0)
        output->put(character);
    if (mFlags & cFlagLeftJustify)
        output->put(L' ', padCount);
}

void FormatParser16::printString_(OutBuffer16* output)
{
    if (mSize < cSizeInt)
        printStringCore_(output, va_arg(mArgs, const char*));
    else if (mSize <= cSizeLongLong)
        printStringCore_(output, va_arg(mArgs, const wchar_t*));
}

void FormatParser16::printFloat_(OutBuffer16* output, wchar_t type, bool upper)
{
    union
    {
        double value;
        unsigned long long bits;
    } number;
    number.value = va_arg(mArgs, double);
    const bool positive = (number.bits & cDoubleSignBit) == 0;
    const unsigned int biasedExponent = static_cast<unsigned int>(number.bits >> cDoubleMantissaBits)
                                        & cDoubleExponentMask;
    const unsigned long long fraction = number.bits & cDoubleFractionMask;
    if (biasedExponent == cDoubleExponentMask)
    {
        mHasPrecision = false;
        if (fraction == 0)
        {
            if (!positive)
                printStringCore_(output, "-Inf");
            else if (mFlags & cFlagForceSign)
                printStringCore_(output, "+Inf");
            else if (mFlags & cFlagBlankSign)
                printStringCore_(output, " Inf");
            else
                printStringCore_(output, "Inf");
            return;
        }
        if (!positive)
            printStringCore_(output, "-NaN");
        else if (mFlags & cFlagForceSign)
            printStringCore_(output, "+NaN");
        else if (mFlags & cFlagBlankSign)
            printStringCore_(output, " NaN");
        else
            printStringCore_(output, "NaN");
        return;
    }
    unsigned int precision = cDefaultFloatPrecision;
    if (mHasPrecision)
    {
        precision = mPrecision;
        if (precision > cFloatMaxPrecision)
            precision = cFloatMaxPrecision;
    }
    unsigned int accumulator[cFloatAccumulatorLength + 1];
    unsigned int* const digitBase = accumulator + 1;
    int exponent = cFloatZeroExponent;
    unsigned long long mantissa = 0;
    int tableIndex = 0;
    if (biasedExponent != 0)
    {
        mantissa = (fraction | cDoubleHiddenBit) << cDoubleMantissaShift;
        tableIndex = biasedExponent + cDoubleMantissaBits - 1;
    }
    else if (fraction != 0)
    {
        mantissa = fraction << cDoubleMantissaShift;
        tableIndex = cDoubleMantissaBits;
        do
        {
            mantissa <<= 1;
            --tableIndex;
        } while ((mantissa & cDoubleSignBit) == 0);
    }
    const unsigned char* record = sFloatDigitTable[tableIndex];
    if (mantissa != 0)
        exponent = static_cast<int>(static_cast<unsigned int>(record[0]) << (cByteBits * 3)
                                    | record[1] << (cByteBits * 2) | record[2] << cByteBits | record[3]);
    MemUtil::fill(digitBase, 0, cFloatAccumulatorLength * sizeof(unsigned int));
    while (mantissa != 0)
    {
        if (mantissa & cDoubleSignBit)
        {
            const int recordExponent = static_cast<int>(static_cast<unsigned int>(record[0]) << (cByteBits * 3)
                                                        | record[1] << (cByteBits * 2) | record[2] << cByteBits
                                                        | record[3]);
            const int shift = exponent - recordExponent + 1;
            for (int position = shift; position < cFloatAccumulatorLength; ++position)
                digitBase[position] += record[cFloatDigitOffset + position - shift];
            if (shift > 0 && shift >= cFloatAccumulatorLength
                && record[cFloatDigitOffset + cFloatAccumulatorLength - shift] > cFloatRoundHalf)
                ++digitBase[cFloatAccumulatorLength - 1];
        }
        mantissa <<= 1;
        record -= cFloatDigitRecordSize;
    }
    for (int position = cFloatAccumulatorLength - 1; position > 0; --position)
    {
        digitBase[position - 1] += digitBase[position] / cBaseDecimal;
        digitBase[position] %= cBaseDecimal;
    }
    unsigned int* digits = digitBase + 1;
    if (digitBase[0] != 0)
    {
        digits = digitBase;
        ++exponent;
    }
    int significant = 0;
    bool round = false;
    if (type == L'e')
    {
        significant = precision + 1;
        round = true;
    }
    else if (type == L'f')
    {
        significant = precision + exponent;
        if (significant >= 0)
            round = true;
        else
            significant = 0;
    }
    else if (type == L'g')
    {
        significant = precision;
        if (significant < 1)
            significant = 1;
        round = true;
    }
    if (round)
    {
        if (significant > cFloatMaxSignificantDigits)
            significant = cFloatMaxSignificantDigits;
        if (digits[significant] > cFloatRoundHalf)
        {
            unsigned int* carry = digits + (significant - 1);
            while (*carry + 1 > cBaseDecimal - 1 && carry >= digitBase)
            {
                *carry -= cBaseDecimal - 1;
                --carry;
            }
            ++*carry;
            if (carry == digits - 1)
            {
                ++exponent;
                --digits;
            }
        }
    }
    unsigned int length = 0;
    if (!positive || (mFlags & (cFlagForceSign | cFlagBlankSign)))
        length = 1;
    unsigned int integerDigits = 0;
    unsigned int integerZeros = 0;
    unsigned int leadingZeros = 0;
    unsigned int fractionDigits = 0;
    unsigned int trailingZeros = 0;
    bool point = false;
    bool exponentForm = false;
    const bool alternate = (mFlags & cFlagAlternate) != 0;
    if (type == L'e')
    {
        exponentForm = true;
        integerDigits = 1;
        fractionDigits = significant - 1;
        trailingZeros = precision - fractionDigits;
        point = precision != 0 || alternate;
    }
    else if (type == L'f')
    {
        if (significant <= exponent)
        {
            integerDigits = significant;
            integerZeros = exponent - significant;
            trailingZeros = precision;
            point = precision != 0 || alternate;
        }
        else if (exponent >= 0)
        {
            integerDigits = exponent;
            fractionDigits = significant - exponent;
            trailingZeros = precision + exponent - significant;
            point = true;
        }
        else
        {
            integerZeros = 1;
            point = true;
            leadingZeros = -exponent;
            if (static_cast<int>(leadingZeros) < static_cast<int>(precision))
            {
                if (leadingZeros + significant < precision)
                {
                    fractionDigits = significant;
                    trailingZeros = precision - significant + exponent;
                }
                else
                {
                    fractionDigits = precision + exponent;
                }
            }
            else
            {
                leadingZeros = precision;
            }
        }
    }
    else if (type == L'g')
    {
        if (!alternate)
        {
            while (significant != 0 && digits[significant - 1] == 0)
                --significant;
        }
        if (exponent < cFloatGeneralMinExponent || static_cast<int>(precision) < exponent)
        {
            exponentForm = true;
            integerDigits = 1;
            point = precision >= 2 || alternate;
            fractionDigits = significant - 1;
            if (alternate)
                trailingZeros = precision - significant;
        }
        else if (exponent < significant)
        {
            point = true;
            if (exponent < 1)
            {
                leadingZeros = -exponent;
                integerZeros = 1;
                fractionDigits = significant;
                if (alternate)
                    trailingZeros = precision - significant;
            }
            else
            {
                integerDigits = exponent;
                fractionDigits = significant - exponent;
                if (alternate)
                    trailingZeros = precision - exponent - fractionDigits;
            }
        }
        else
        {
            integerDigits = significant;
            integerZeros = exponent - significant;
            if (static_cast<int>(precision) <= exponent)
            {
                point = alternate;
            }
            else if (alternate)
            {
                point = true;
                trailingZeros = precision - exponent;
            }
        }
    }
    length += integerDigits + integerZeros + (point ? 1 : 0) + leadingZeros + fractionDigits + trailingZeros;
    if (exponentForm)
        length += cFloatExponentTextLength;
    unsigned int padZeros = 0;
    unsigned int rightSpaces = 0;
    if (length < static_cast<unsigned int>(mWidth))
    {
        const unsigned int padding = mWidth - length;
        if (mFlags & cFlagLeftJustify)
            rightSpaces = padding;
        else if (mFlags & cFlagZeroPad)
            padZeros = padding;
        else
            output->put(L' ', padding);
    }
    if (!positive)
        output->put(L'-');
    else if (mFlags & cFlagForceSign)
        output->put(L'+');
    else if (mFlags & cFlagBlankSign)
        output->put(L' ');
    output->put(L'0', padZeros);
    for (unsigned int position = 0; position < integerDigits; ++position)
        output->put(static_cast<wchar_t>(L'0' + digits[position]));
    output->put(L'0', integerZeros);
    if (point)
        output->put(L'.');
    output->put(L'0', leadingZeros);
    for (unsigned int position = 0; position < fractionDigits; ++position)
        output->put(static_cast<wchar_t>(L'0' + digits[integerDigits + position]));
    output->put(L'0', trailingZeros);
    if (exponentForm)
    {
        output->put(upper ? L'E' : L'e');
        const bool positiveExponent = exponent >= 1;
        unsigned int value = positiveExponent ? exponent - 1 : 1 - exponent;
        unsigned int exponentDigits[cFloatExponentDigits];
        for (int position = 0; position < cFloatExponentDigits; ++position)
        {
            exponentDigits[position] = value % cBaseDecimal;
            value /= cBaseDecimal;
        }
        output->put(positiveExponent ? L'+' : L'-');
        for (int position = cFloatExponentDigits - 1; position >= 0; --position)
            output->put(static_cast<wchar_t>(L'0' + exponentDigits[position]));
    }
    output->put(L' ', rightSpaces);
}

int FormatParser16::execute(OutBuffer16* output, const wchar_t* format)
{
    clear_();
    const wchar_t* conversion = parseSize_(parsePrecision_(parseWidth_(parseFlag_(format))));
    switch (*conversion)
    {
    case L'%':
        output->put(L'%');
        break;
    case L'E':
        printFloat_(output, L'e', true);
        break;
    case L'G':
        printFloat_(output, L'g', true);
        break;
    case L'X':
        printInteger_(output, cBaseHexadecimal, false, true);
        break;
    case L'c':
        printCharacter_(output);
        break;
    case L'd':
    case L'i':
        printInteger_(output, cBaseDecimal, true, false);
        break;
    case L'e':
        printFloat_(output, L'e', false);
        break;
    case L'f':
        printFloat_(output, L'f', false);
        break;
    case L'g':
        printFloat_(output, L'g', false);
        break;
    case L'o':
        printInteger_(output, cBaseOctal, false, false);
        break;
    case L'p':
        mPrecision = cPointerPrecision;
        mSize = cSizeInt;
        mFlags &= ~cFlagZeroPad;
        printInteger_(output, cBaseHexadecimal, false, false);
        break;
    case L's':
        printString_(output);
        break;
    case L'u':
        printInteger_(output, cBaseDecimal, false, false);
        break;
    case L'x':
        printInteger_(output, cBaseHexadecimal, false, false);
        break;
    default:
        return 0;
    }
    return conversion - format + 1;
}

template bool StringUtilImpl::tryParseNumberImpl_<unsigned long long>(unsigned long long*,
                                                                      SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned int>(unsigned int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned short>(unsigned short*, SafeStringBase<char>::iterator&,
                                                                  int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned char>(unsigned char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned long long>(unsigned long long*, SafeStringBase<char>::iterator&,
                                                                   int);
template bool StringUtilImpl::tryParseDecImpl_<long long>(long long*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned int>(unsigned int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<int>(int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned short>(unsigned short*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<short>(short*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned char>(unsigned char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<signed char>(signed char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<float>(float*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<double>(double*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned long long>(unsigned long long*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<long long>(long long*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned int>(unsigned int*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<int>(int*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned short>(unsigned short*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<short>(short*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned char>(unsigned char*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<signed char>(signed char*, const SafeStringBase<char>&, int);
template void StringUtilImpl::tryParseUnderDecimalPointImpl_<float>(float*, SafeStringBase<char>::iterator&, float,
                                                                    float);
template void StringUtilImpl::tryParseUnderDecimalPointImpl_<double>(double*, SafeStringBase<char>::iterator&, double,
                                                                     double);
template bool StringUtilImpl::tryParseDecimalFractionImpl_<float>(float*, const SafeStringBase<char>&);
template bool StringUtilImpl::tryParseDecimalFractionImpl_<double>(double*, const SafeStringBase<char>&);
template int StringUtilImpl::parseImpl_<int>(const SafeStringBase<char>&, CardinalNumber);
template void FormatParser16::printIntegerInverseString_<unsigned int>(OutBuffer16*, unsigned int, bool, unsigned int);
template void FormatParser16::printIntegerInverseString_<unsigned long long>(OutBuffer16*, unsigned int, bool,
                                                                             unsigned long long);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned int, signed char, int>(OutBuffer16*, bool*,
                                                                                                bool, unsigned int,
                                                                                                bool);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned int, short, int>(OutBuffer16*, bool*, bool,
                                                                                          unsigned int, bool);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned int, int, int>(OutBuffer16*, bool*, bool,
                                                                                        unsigned int, bool);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned long long, long long, long long>(OutBuffer16*,
                                                                                                          bool*, bool,
                                                                                                          unsigned int,
                                                                                                          bool);
template void FormatParser16::printStringCore_<char>(OutBuffer16*, const char*);
template void FormatParser16::printStringCore_<wchar_t>(OutBuffer16*, const wchar_t*);

bool StringUtil::tryParseU64(unsigned long long* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseS64(long long* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseU32(unsigned int* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseS32(int* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseU16(unsigned short* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseS16(short* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseU8(unsigned char* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
bool StringUtil::tryParseS8(signed char* v, const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::tryParseImpl_(v, s, c);
}
int StringUtil::parseS32(const SafeStringBase<char>& s, CardinalNumber c)
{
    return StringUtilImpl::parseImpl_<int>(s, c);
}
int StringUtil::vsw16printf(wchar_t* buffer, unsigned int size, const wchar_t* format, va_list args)
{
    if (size == 0)
        return -1;
    OutBuffer16 output(buffer, size - 1);
    FormatParser16 parser(args);
    int index = 0;
    for (;;)
    {
        const wchar_t character = format[index++];
        if (character == 0)
        {
            output.buffer[output.length] = 0;
            return output.length;
        }
        if (character == L'%')
        {
            index += parser.execute(&output, format + index);
        }
        else
        {
            if (output.length >= output.capacity)
                break;
            output.buffer[output.length++] = character;
        }
        if (output.overflow)
            break;
    }
    output.buffer[output.length] = 0;
    return -1;
}
int StringUtil::vsnprintf(char* buffer, unsigned int size, const char* format, va_list args)
{
    if (size == 0)
        return -1;
    int result = ::vsnprintf(buffer, size, format, args);
    if (result < 0 || static_cast<unsigned int>(result) >= size)
        buffer[size - 1] = SafeStringBase<char>::cNullChar;
    if (static_cast<unsigned int>(result) >= size)
        result = size - 1;
    return result;
}
int StringUtil::convertUtf16ToSjis(char* destination, unsigned int capacity, const wchar_t* source, int length)
{
    if (capacity == 0 || length < -1)
        return 0;
    unsigned int written = 0;
    int index = 0;
    for (;;)
    {
        if (length != -1 && index >= length)
            break;
        const unsigned int character = static_cast<unsigned short>(source[index]);
        if (character == 0)
            break;
        ++index;
        if (written >= capacity - 1)
        {
            written = capacity - 1;
            break;
        }
        unsigned char lead = 0;
        unsigned char trail = 0;
        if (character < cAsciiEnd)
        {
            lead = static_cast<unsigned char>(character);
        }
        else if (character - cHalfwidthKatakanaFirst < cHalfwidthKatakanaCount)
        {
            lead = static_cast<unsigned char>(character - cHalfwidthKatakanaToSjis);
        }
        else if (character - cSurrogateFirst > cSurrogateToPrivateEndSpan)
        {
            const unsigned int lowByte = character & cByteMask;
            const unsigned char* range = sSjisIndexTable + lowByte * cSjisIndexSize;
            int first = range[0] << cByteBits | range[1];
            int last = (range[cSjisIndexSize] << cByteBits | range[cSjisIndexSize + 1]) - 1;
            const unsigned int highByte = character >> cByteBits;
            while (first <= last)
            {
                const int middle = (first + last) / 2;
                const unsigned char* entry = sSjisCodeTable[middle];
                if (highByte < entry[cSjisEntryUtf16High])
                {
                    last = middle - 1;
                }
                else if (highByte > entry[cSjisEntryUtf16High])
                {
                    first = middle + 1;
                }
                else
                {
                    lead = entry[cSjisEntryLead];
                    trail = entry[cSjisEntryTrail];
                    break;
                }
            }
        }
        else if (character - cPrivateUseFirst < cPrivateUseCount)
        {
            const unsigned int offset = character - cPrivateUseFirst;
            const unsigned int column = offset % cSjisRowLength;
            lead = static_cast<unsigned char>((offset - column) / cSjisRowLength + cSjisUserDefinedLeadFirst);
            trail = static_cast<unsigned char>(column
                                               + (column < cSjisTrailGap ? cSjisTrailFirst : cSjisTrailFirst + 1));
        }
        else if (character == cSjisSpecialA0Source)
        {
            destination[written++] = static_cast<char>(cSjisSpecialA0);
            continue;
        }
        else if (character - cSjisSpecialFdSource < cSjisSpecialFdCount)
        {
            lead = static_cast<unsigned char>(character - cSjisSpecialFdOffset);
        }
        if (lead == 0)
            break;
        if (trail == 0)
        {
            destination[written++] = static_cast<char>(lead);
            continue;
        }
        if (written >= capacity - 2)
        {
            written = capacity - 2;
            break;
        }
        destination[written] = static_cast<char>(lead);
        destination[written + 1] = static_cast<char>(trail);
        written += 2;
    }
    destination[written] = 0;
    return static_cast<int>(written);
}
int StringUtil::convertUtf16ToUtf8(char* destination, unsigned int capacity, const wchar_t* source, int length)
{
    if (capacity == 0 || length < -1)
        return 0;
    unsigned int written = 0;
    if (length == -1)
        length = INT_MAX;
    for (int index = 0; index < length && source[index] != 0; ++index)
    {
        const unsigned int character = static_cast<unsigned short>(source[index]);
        if (character < 0x80)
        {
            destination[written++] = static_cast<char>(character);
        }
        else if (character < 0x800)
        {
            if (written + 1 >= capacity)
            {
                destination[written - 1] = 0;
                return static_cast<int>(written - 1);
            }
            destination[written++] = static_cast<char>(0xc0 | character >> 6);
            destination[written++] = static_cast<char>(0x80 | character & 0x3f);
        }
        else
        {
            if (written + 2 >= capacity)
            {
                destination[written - 1] = 0;
                return static_cast<int>(written - 1);
            }
            destination[written++] = static_cast<char>(0xe0 | character >> 12);
            destination[written++] = static_cast<char>(0x80 | character >> 6 & 0x3f);
            destination[written++] = static_cast<char>(0x80 | character & 0x3f);
        }
        if (written >= capacity)
        {
            destination[written - 1] = 0;
            return static_cast<int>(written - 1);
        }
    }
    destination[written] = 0;
    return static_cast<int>(written);
}
int StringUtil::convertUtf8ToSjis(char* destination, unsigned int capacity, const char* source, int length)
{
    if (capacity == 0 || length < -1)
        return 0;
    unsigned int written = 0;
    int index = 0;
    for (;;)
    {
        if (length != -1 && index >= length)
            break;
        unsigned char bytes[cUtf8ConvertBufferLength];
        bytes[0] = source[index];
        if (bytes[0] == 0)
            break;
        if (written >= capacity - 1)
        {
            written = capacity - 1;
            break;
        }
        if (bytes[0] < cAsciiEnd)
        {
            destination[written++] = bytes[0];
            ++index;
            continue;
        }
        bytes[1] = source[index + 1];
        if (bytes[1] == 0)
            break;
        wchar_t utf16[2];
        int converted;
        if (bytes[0] < cUtf8Lead3First)
        {
            utf16[0] = static_cast<wchar_t>((bytes[0] & cUtf8Lead2Payload) << cUtf8TrailBits
                                            | (bytes[1] & cUtf8TrailPayload));
            utf16[1] = 0;
            converted = convertUtf16ToSjis(reinterpret_cast<char*>(bytes), cUtf8ConvertBufferLength, utf16, -1);
            index += 2;
        }
        else
        {
            if (bytes[0] >= cUtf8Lead4First)
                break;
            bytes[2] = source[index + 2];
            if (bytes[2] == 0)
                break;
            utf16[1] = 0;
            utf16[0] = static_cast<wchar_t>(bytes[0] << (cUtf8TrailBits * 2)
                                            | (bytes[1] & cUtf8TrailPayload) << cUtf8TrailBits
                                            | (bytes[2] & cUtf8TrailPayload));
            converted = convertUtf16ToSjis(reinterpret_cast<char*>(bytes), cUtf8ConvertBufferLength, utf16, -1);
            index += 3;
        }
        if (converted == 0)
            break;
        if (written >= capacity - converted)
        {
            written = capacity - converted;
            break;
        }
        for (int position = 0; position < converted; ++position)
            destination[written++] = bytes[position];
    }
    destination[written] = 0;
    return static_cast<int>(written);
}
namespace
{
const StringUtil::Char16Pair cToUpperCapitalPairs[] = {
    {L'a', L'A'},           {L'b', L'B'},           {L'c', L'C'},           {L'd', L'D'},
    {L'e', L'E'},           {L'f', L'F'},           {L'g', L'G'},           {L'h', L'H'},
    {L'i', L'I'},           {L'j', L'J'},           {L'k', L'K'},           {L'l', L'L'},
    {L'm', L'M'},           {L'n', L'N'},           {L'o', L'O'},           {L'p', L'P'},
    {L'q', L'Q'},           {L'r', L'R'},           {L's', L'S'},           {L't', L'T'},
    {L'u', L'U'},           {L'v', L'V'},           {L'w', L'W'},           {L'x', L'X'},
    {L'y', L'Y'},           {L'z', L'Z'},           {L'\x00e0', L'\x00c0'}, {L'\x00e1', L'\x00c1'},
    {L'\x00e2', L'\x00c2'}, {L'\x00e3', L'\x00c3'}, {L'\x00e4', L'\x00c4'}, {L'\x00e6', L'\x00c6'},
    {L'\x00e7', L'\x00c7'}, {L'\x00e8', L'\x00c8'}, {L'\x00e9', L'\x00c9'}, {L'\x00ea', L'\x00ca'},
    {L'\x00eb', L'\x00cb'}, {L'\x00ec', L'\x00cc'}, {L'\x00ed', L'\x00cd'}, {L'\x00ee', L'\x00ce'},
    {L'\x00ef', L'\x00cf'}, {L'\x00f1', L'\x00d1'}, {L'\x00f2', L'\x00d2'}, {L'\x00f3', L'\x00d3'},
    {L'\x00f4', L'\x00d4'}, {L'\x00f5', L'\x00d5'}, {L'\x00f6', L'\x00d6'}, {L'\x00f9', L'\x00d9'},
    {L'\x00fa', L'\x00da'}, {L'\x00fb', L'\x00db'}, {L'\x00fc', L'\x00dc'}, {L'\x00ff', L'\x0178'},
    {L'\x0153', L'\x0152'}, {L'\x0430', L'\x0410'}, {L'\x0431', L'\x0411'}, {L'\x0432', L'\x0412'},
    {L'\x0433', L'\x0413'}, {L'\x0434', L'\x0414'}, {L'\x0435', L'\x0415'}, {L'\x0436', L'\x0416'},
    {L'\x0437', L'\x0417'}, {L'\x0438', L'\x0418'}, {L'\x0439', L'\x0419'}, {L'\x043a', L'\x041a'},
    {L'\x043b', L'\x041b'}, {L'\x043c', L'\x041c'}, {L'\x043d', L'\x041d'}, {L'\x043e', L'\x041e'},
    {L'\x043f', L'\x041f'}, {L'\x0440', L'\x0420'}, {L'\x0441', L'\x0421'}, {L'\x0442', L'\x0422'},
    {L'\x0443', L'\x0423'}, {L'\x0444', L'\x0424'}, {L'\x0445', L'\x0425'}, {L'\x0446', L'\x0426'},
    {L'\x0447', L'\x0427'}, {L'\x0448', L'\x0428'}, {L'\x0449', L'\x0429'}, {L'\x044a', L'\x042a'},
    {L'\x044b', L'\x042b'}, {L'\x044c', L'\x042c'}, {L'\x044d', L'\x042d'}, {L'\x044e', L'\x042e'},
    {L'\x044f', L'\x042f'}, {L'\x0451', L'\x0401'}};
const StringUtil::Char16Pair cToLowerCapitalPairs[] = {
    {L'A', L'a'},           {L'B', L'b'},           {L'C', L'c'},           {L'D', L'd'},
    {L'E', L'e'},           {L'F', L'f'},           {L'G', L'g'},           {L'H', L'h'},
    {L'I', L'i'},           {L'J', L'j'},           {L'K', L'k'},           {L'L', L'l'},
    {L'M', L'm'},           {L'N', L'n'},           {L'O', L'o'},           {L'P', L'p'},
    {L'Q', L'q'},           {L'R', L'r'},           {L'S', L's'},           {L'T', L't'},
    {L'U', L'u'},           {L'V', L'v'},           {L'W', L'w'},           {L'X', L'x'},
    {L'Y', L'y'},           {L'Z', L'z'},           {L'\x00c0', L'\x00e0'}, {L'\x00c1', L'\x00e1'},
    {L'\x00c2', L'\x00e2'}, {L'\x00c3', L'\x00e3'}, {L'\x00c4', L'\x00e4'}, {L'\x00c6', L'\x00e6'},
    {L'\x00c7', L'\x00e7'}, {L'\x00c8', L'\x00e8'}, {L'\x00c9', L'\x00e9'}, {L'\x00ca', L'\x00ea'},
    {L'\x00cb', L'\x00eb'}, {L'\x00cc', L'\x00ec'}, {L'\x00cd', L'\x00ed'}, {L'\x00ce', L'\x00ee'},
    {L'\x00cf', L'\x00ef'}, {L'\x00d1', L'\x00f1'}, {L'\x00d2', L'\x00f2'}, {L'\x00d3', L'\x00f3'},
    {L'\x00d4', L'\x00f4'}, {L'\x00d5', L'\x00f5'}, {L'\x00d6', L'\x00f6'}, {L'\x00d9', L'\x00f9'},
    {L'\x00da', L'\x00fa'}, {L'\x00db', L'\x00fb'}, {L'\x00dc', L'\x00fc'}, {L'\x0152', L'\x0153'},
    {L'\x0178', L'\x00ff'}, {L'\x0401', L'\x0451'}, {L'\x0410', L'\x0430'}, {L'\x0411', L'\x0431'},
    {L'\x0412', L'\x0432'}, {L'\x0413', L'\x0433'}, {L'\x0414', L'\x0434'}, {L'\x0415', L'\x0435'},
    {L'\x0416', L'\x0436'}, {L'\x0417', L'\x0437'}, {L'\x0418', L'\x0438'}, {L'\x0419', L'\x0439'},
    {L'\x041a', L'\x043a'}, {L'\x041b', L'\x043b'}, {L'\x041c', L'\x043c'}, {L'\x041d', L'\x043d'},
    {L'\x041e', L'\x043e'}, {L'\x041f', L'\x043f'}, {L'\x0420', L'\x0440'}, {L'\x0421', L'\x0441'},
    {L'\x0422', L'\x0442'}, {L'\x0423', L'\x0443'}, {L'\x0424', L'\x0444'}, {L'\x0425', L'\x0445'},
    {L'\x0426', L'\x0446'}, {L'\x0427', L'\x0447'}, {L'\x0428', L'\x0448'}, {L'\x0429', L'\x0449'},
    {L'\x042a', L'\x044a'}, {L'\x042b', L'\x044b'}, {L'\x042c', L'\x044c'}, {L'\x042d', L'\x044d'},
    {L'\x042e', L'\x044e'}, {L'\x042f', L'\x044f'}};
const Buffer<const StringUtil::Char16Pair> cToUpperCapitalTable(sizeof(cToUpperCapitalPairs)
                                                                    / sizeof(cToUpperCapitalPairs[0]),
                                                                cToUpperCapitalPairs);
const Buffer<const StringUtil::Char16Pair> cToLowerCapitalTable(sizeof(cToLowerCapitalPairs)
                                                                    / sizeof(cToLowerCapitalPairs[0]),
                                                                cToLowerCapitalPairs);
}
int StringUtil::compareChar16Pair(const Char16Pair& pair, const wchar_t& value)
{
    return value < pair.from ? 1 : value > pair.from ? -1 : 0;
}
wchar_t StringUtil::replace(wchar_t value, const Buffer<const Char16Pair>& pairs)
{
    int first = 0;
    int last = pairs.size();
    while (first < last)
    {
        int middle = first + (last - first) / 2;
        int order = compareChar16Pair(pairs[middle], value);
        if (order == 0)
            return pairs[middle].to;
        if (order < 0)
            first = middle + 1;
        else
            last = middle;
    }
    return value;
}
wchar_t StringUtil::toUpperCapital(wchar_t value)
{
    return replace(value, cToUpperCapitalTable);
}
void StringUtil::toUpperCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>* text)
{
    wchar_t* buffer = text->getBuffer();
    if (buffer[0] != L' ')
    {
        buffer[0] = toUpperCapital(buffer[0]);
    }
}
wchar_t StringUtil::toLowerCapital(wchar_t value)
{
    return replace(value, cToLowerCapitalTable);
}
void StringUtil::toLowerCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>* text)
{
    wchar_t* buffer = text->getBuffer();
    if (buffer[0] != L' ')
    {
        buffer[0] = toLowerCapital(buffer[0]);
    }
}

namespace system
{
void PrintStringImpl(const char*, int)
{
}
void PrintString(const char* text, int level)
{
    PrintConfig::execCallbacks(PrintConfig::PrintEventArg(text, level));
}
}

template <typename T>
int replaceStringImpl_(T* dst, int* length, int dstSize, const T* src, int srcSize, const SafeStringBase<T>& oldString,
                       const SafeStringBase<T>& newString, bool* isBufferOverflow)
{
    *isBufferOverflow = false;
    const int dstMax = dstSize - 1;
    const T* oldText = oldString.cstr();
    const int oldLength = oldString.calcLength();
    if (oldLength == 0)
    {
        if (dst != src)
        {
            *isBufferOverflow = dstSize < srcSize + 1;
            if (*isBufferOverflow)
            {
                MemUtil::copy(dst, src, dstMax * sizeof(T));
                dst[dstMax] = 0;
                if (length)
                    *length = dstMax;
            }
            else
            {
                MemUtil::copy(dst, src, (srcSize + 1) * sizeof(T));
                if (length)
                    *length = srcSize;
            }
        }
        return 0;
    }
    const T* newText = newString.cstr();
    const int newLength = newString.calcLength();
    if (dst != src || newLength <= oldLength)
    {
        int replaceCount = 0;
        int srcIndex = 0;
        int dstIndex = 0;
        while (srcIndex < srcSize)
        {
            if (isMatchString_(src + srcIndex, oldText, oldLength))
            {
                int copyLength = dstMax - dstIndex;
                if (newLength < copyLength)
                    copyLength = newLength;
                if (copyLength > 0)
                {
                    MemUtil::copy(dst + dstIndex, newText, copyLength * sizeof(T));
                    ++replaceCount;
                }
                else if (newLength == 0)
                {
                    ++replaceCount;
                }
                if (copyLength < newLength)
                {
                    *isBufferOverflow = true;
                    dst[dstMax] = 0;
                    if (length)
                        *length = dstMax;
                    return replaceCount;
                }
                srcIndex += oldLength;
                dstIndex += newLength;
            }
            else
            {
                if (dstIndex >= dstMax)
                {
                    *isBufferOverflow = true;
                    dst[dstMax] = 0;
                    if (length)
                        *length = dstMax;
                    return replaceCount;
                }
                dst[dstIndex++] = src[srcIndex++];
            }
        }
        dst[dstIndex] = 0;
        if (length)
            *length = dstIndex;
        return replaceCount;
    }
    int srcIndex = 0;
    int dstIndex = 0;
    while (srcIndex < srcSize)
    {
        if (isMatchString_(src + srcIndex, oldText, oldLength))
        {
            dstIndex += newLength;
            srcIndex += oldLength;
        }
        else
        {
            ++srcIndex;
            ++dstIndex;
        }
        if (dstIndex > dstMax)
        {
            *isBufferOverflow = true;
            break;
        }
    }
    if (*isBufferOverflow)
    {
        dst[dstMax] = 0;
        if (length)
            *length = dstMax;
    }
    else
    {
        dst[dstIndex] = 0;
        if (length)
            *length = dstIndex;
    }
    int replaceCount = 0;
    --srcIndex;
    --dstIndex;
    while (srcIndex >= 0)
    {
        if (isMatchString_(src + srcIndex - oldLength + 1, oldText, oldLength))
        {
            int copyLength = dstMax - (dstIndex - newLength + 1);
            if (newLength < copyLength)
                copyLength = newLength;
            if (copyLength > 0)
            {
                MemUtil::copy(dst + dstIndex - newLength + 1, newText, copyLength * sizeof(T));
                ++replaceCount;
            }
            srcIndex -= oldLength;
            dstIndex -= newLength;
        }
        else
        {
            if (dstIndex < dstMax)
                dst[dstIndex] = src[srcIndex];
            --srcIndex;
            --dstIndex;
        }
    }
    return replaceCount;
}

template int replaceStringImpl_<char>(char*, int*, int, const char*, int, const SafeStringBase<char>&,
                                      const SafeStringBase<char>&, bool*);
template int replaceStringImpl_<wchar_t>(wchar_t*, int*, int, const wchar_t*, int, const SafeStringBase<wchar_t>&,
                                         const SafeStringBase<wchar_t>&, bool*);
}
