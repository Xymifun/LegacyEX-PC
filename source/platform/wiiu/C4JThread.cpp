#include "platform/wiiu/C4JThread.hpp"

#include "platform/wiiu/C4JThreadImpl.hpp"

void C4JThread::Sleep(int milliseconds)
{
    C4JThreadImpl::Sleep(milliseconds);
}

uint32_t C4JThread::getCurrentThread()
{
    return C4JThreadImpl::getCurrentThread();
}

bool C4JThread::isMainThread()
{
    return C4JThreadImpl::isMainThread();
}

const char* C4JThread::getCurrentThreadName()
{
    return C4JThreadImpl::getCurrentThreadName();
}
