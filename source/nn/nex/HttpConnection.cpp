#include "nn/nex/HttpConnection.hpp"

#include <stdio.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/CurlHttpConnection.hpp"
#include "nn/nex/MD5Checksum.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int BOUNDARY_SEED_SIZE = 8;
const unsigned int BOUNDARY_DIGEST_SIZE = 16;
const unsigned int APPEND_AT_END = 0xFFFFFFFF;

}

int HttpConnection::s_iConnectionCount = 0;
CriticalSection* HttpConnection::s_pCriticalSection = 0;

const char* HttpConnection::GetBoundaryString() const
{
    if (m_aBoundary[0] == 0)
    {
        unsigned long long ullTime = Time::GetTime();
        Buffer seed(BOUNDARY_SEED_SIZE, 0);
        MD5Checksum checksum;
        Buffer digest(BOUNDARY_DIGEST_SIZE, 0);
        seed.AppendData(&ullTime, BOUNDARY_SEED_SIZE, APPEND_AT_END);
        checksum.ComputeChecksum(seed, &digest);
        char* pDigest = 0;
        digest.ToString().CreateCopy(&pDigest);
        snprintf(m_aBoundary, BOUNDARY_CAPACITY, "--------BOUNDARY--------%s", pDigest);
        m_aBoundary[BOUNDARY_CAPACITY - 1] = 0;
        String::ReleaseCopy(pDigest);
    }
    return m_aBoundary;
}

HttpConnection* HttpConnection::GetHttpConnection()
{
    if (s_pCriticalSection)
        s_pCriticalSection->Enter();
    ++s_iConnectionCount;
    return new CurlHttpConnection();
}

void HttpConnection::ReleaseHttpConnection(HttpConnection* pConnection)
{
    if (pConnection)
    {
        delete pConnection;
        --s_iConnectionCount;
        if (s_pCriticalSection)
            s_pCriticalSection->Leave();
    }
}

int HttpConnection::GetConnectionCount()
{
    return s_iConnectionCount;
}

}
}
