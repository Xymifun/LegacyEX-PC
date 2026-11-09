#include <nn/ffl/FFLiFsClient.h>

FFLiFsClient::FFLiFsClient() : m_IsValid(false)

{
}

FFLiFsClient::~FFLiFsClient()
{
}

namespace
{

void StateChangeCallback(FSClient* pClient, FSVolumeState state, void* pContext)
{
    OSReport("client = %p state = %u context = %p lastError = %d\n", pClient, u32(state), pContext,
             FSGetLastError(pClient));
}

FSStateChangeParams s_Param = {StateChangeCallback};

}

FSStatus FFLiFsClient::Init(const FSStateChangeParams* pChangeParams, u32 errHandling)
{
    FSStatus status = FSAddClient(&m_FSClient, static_cast<FSErrorFlag>(errHandling));
    if (status == FS_STATUS_OK)
    {
        if (pChangeParams == NULL)
            pChangeParams = &s_Param;

        SetStateChangeNotification(const_cast<FSStateChangeParams*>(pChangeParams));
        m_IsValid = true;
    }

    return status;
}

void FFLiFsClient::SetStateChangeNotification(FSStateChangeParams* pChangeParams)
{
    FSSetStateChangeNotification(&m_FSClient, pChangeParams);
}

bool FFLiFsClient::IsValid() const
{
    return m_IsValid;
}

FSStatus FFLiFsClient::Shutdown(u32 errHandling)
{
    FSStatus status = FSDelClient(&m_FSClient, static_cast<FSErrorFlag>(errHandling));
    m_IsValid = false;
    return status;
}

FSStatus FFLiFsClient::ShutdownIfValid(u32 errHandling)
{
    FSStatus status = FS_STATUS_OK;
    if (IsValid())
        status = Shutdown(errHandling);
    return status;
}
