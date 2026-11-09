#ifndef SOURCE_PLATFORM_WIIU_TLSSTORAGECAFE_HPP
#define SOURCE_PLATFORM_WIIU_TLSSTORAGECAFE_HPP

class TLSStorageCafe
{
public:
    TLSStorageCafe();
    int Alloc();
    static TLSStorageCafe* Instance();
    void Free(unsigned int index);
    unsigned int GetValue(unsigned int index);
    unsigned int SetValue(unsigned int index, void* value);

private:
    static TLSStorageCafe* m_instance;
};

unsigned int TlsAlloc();
void* TlsGetValue(unsigned int index);
int TlsSetValue(unsigned int index, void* value);

#endif
