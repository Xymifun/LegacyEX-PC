#include "game/client/render/Tesselator.hpp"

#include "game/core/BufferBuilder.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int g_tesselatorTlsIndex = TlsAlloc();

Tesselator::Tesselator(int bufferSize)
{
    mIsDrawing = new bool;
    mBuilder = new BufferBuilder(bufferSize);
}

Tesselator* Tesselator::getInstance()
{
    return (Tesselator*)TlsGetValue(g_tesselatorTlsIndex);
}

void Tesselator::CreateNewThreadStorage(int bufferSizeInBytes)
{
    TlsSetValue(g_tesselatorTlsIndex, new Tesselator(bufferSizeInBytes / 4));
}

void Tesselator::ReleaseThreadStorage()
{
    delete (Tesselator*)TlsGetValue(g_tesselatorTlsIndex);
}

BufferBuilder* Tesselator::getBuilder()
{
    return mBuilder;
}

void Tesselator::end()
{
    mBuilder->end();
}
