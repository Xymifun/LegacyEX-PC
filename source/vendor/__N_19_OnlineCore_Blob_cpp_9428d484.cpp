#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"
#include <string.h>
#include <wchar.h>

extern "C"
{
int shutdown(int handle, int how);
int socketclose(int handle);
}

namespace
{

const int UTF8_ONE_BYTE_LIMIT = 128;
const int UTF8_TWO_BYTE_LIMIT = 2048;
const int UTF8_THREE_BYTE_LIMIT = 65536;

}

void closesocket(int handle)
{
    shutdown(handle, 2);
    socketclose(handle);
}

nn::nex::String __N_19_OnlineCore_Blob_cpp_9428d484::_Copy(const wchar_t* first, unsigned int firstLength,
                                                           const wchar_t* second, unsigned int secondLength)
{
    nn::nex::String result;
    result.Reserve(firstLength + secondLength + 1);
    wchar_t* data = result;
    wcscpy(data, first);
    wcscpy(data + firstLength, second);
    return result;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::UnicodeToUtf8(const wchar_t* input, char* output, unsigned int capacity)
{
    int written = 0;
    int remaining = static_cast<int>(capacity) - 1;
    bool invalid = false;
    while (*input != 0 && remaining != 0 && !invalid)
    {
        const unsigned int value = static_cast<unsigned int>(*input++);
        if (value < 0x80)
        {
            *output++ = static_cast<char>(value);
            --remaining;
            ++written;
        }
        else if (value < 0x800)
        {
            *output++ = static_cast<char>((value >> 6) | 0xc0);
            *output++ = static_cast<char>((value & 0x3f) | 0x80);
            remaining -= 2;
            written += 2;
        }
        else if (value < 0x10000)
        {
            *output++ = static_cast<char>((value >> 12) | 0xe0);
            *output++ = static_cast<char>(((value >> 6) & 0x3f) | 0x80);
            *output++ = static_cast<char>((value & 0x3f) | 0x80);
            remaining -= 3;
            written += 3;
        }
        else
            invalid = true;
    }
    *output = 0;
    return written;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::Utf8ToUnicode(const char* input, wchar_t* output, unsigned int capacity)
{
    int written = 0;
    int remaining = static_cast<int>(capacity) - 1;
    bool invalid = false;
    wchar_t* cursor = output;
    do
    {
        const unsigned char lead = *reinterpret_cast<const unsigned char*>(input);
        if (lead == 0 || remaining == 0)
        {
            *cursor = 0;
            return written + 1;
        }
        const unsigned int value = lead;
        if (value < 0x80)
        {
            --remaining;
            ++written;
            *cursor++ = static_cast<wchar_t>(static_cast<signed char>(lead));
        }
        else if (value - 0xc2 < 0x1e)
        {
            const unsigned char first = *reinterpret_cast<const unsigned char*>(input);
            ++input;
            --remaining;
            ++written;
            *cursor++ = static_cast<wchar_t>(first * 0x40 + *reinterpret_cast<const unsigned char*>(input) - 0x3080);
        }
        else if (value - 0xe0 < 0x10)
        {
            const unsigned char* second = reinterpret_cast<const unsigned char*>(input) + 1;
            const unsigned char first = *reinterpret_cast<const unsigned char*>(input);
            input += 2;
            --remaining;
            ++written;
            *cursor++ = static_cast<wchar_t>(first * 0x1000 + *second * 0x40
                                             + *reinterpret_cast<const unsigned char*>(input) - 0x2080);
        }
        else
            invalid = true;
        ++input;
    } while (!invalid);
    *output = 0;
    return -1;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::GetNeededUtf8BufferSize(const wchar_t* input)
{
    int size = 1;
    while (*input != 0)
    {
        const wchar_t value = *input;
        if (value < UTF8_ONE_BYTE_LIMIT)
            size += 1;
        else if (value < UTF8_TWO_BYTE_LIMIT)
            size += 2;
        else if (value < UTF8_THREE_BYTE_LIMIT)
            size += 3;
        ++input;
    }
    return size;
}

void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvFromCallback(nn::nex::SocketDriver::_Result, const unsigned char*,
                                                                  unsigned int,
                                                                  const nn::nex::SocketDriver::InetAddress*,
                                                                  const unsigned int*)
{
}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendToCallback(nn::nex::SocketDriver::_Result, const unsigned char*,
                                                                unsigned int, const nn::nex::SocketDriver::InetAddress&,
                                                                const unsigned int*)
{
}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvCallback(nn::nex::SocketDriver::_Result, const unsigned char*,
                                                              unsigned int, const unsigned int*)
{
}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendCallback(nn::nex::SocketDriver::_Result, const unsigned char*,
                                                              unsigned int, const unsigned int*)
{
}
