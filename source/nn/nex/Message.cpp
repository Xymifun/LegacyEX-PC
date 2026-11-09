#include "nn/nex/Message.hpp"

#include "nn/nex/EndPoint.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    MESSAGE_INITIAL_RESULT = 0x00010001
};

const unsigned int MESSAGE_SIZE_FIELD_LENGTH = 4;

}

Message::Message() : ByteStream(), m_Result()
{
    m_uiBufferState = 0;
    m_uiCommonStateC = 0;

    InitCommon();
    InitBuffer();
    SetToBegining();
}

Message::Message(Buffer* pBuffer) : ByteStream(pBuffer), m_Result()
{
    m_uiBufferState = 0;
    m_uiCommonStateC = 0;

    InitCommon();

    m_uiBufferState = 0;
    m_uiBufferFlag = 0;

    SetToBegining();
}

Message::~Message()
{
    if (m_pSourceEndPoint != 0)
    {
        m_pSourceEndPoint->Close();
    }
}

void Message::InitCommon()
{
    m_uiCommonFlag = 0;
    m_bCompactFormat = 0;
    m_uiCommonStateD = 0;
    m_uiCommonStateA = 0;
    m_uiCommonStateB = 0;
    m_pSourceEndPoint = 0;
}

void Message::InitBuffer()
{
    Clear();

    m_uiBufferFlag = 0;
    m_uiBufferState = 0;

    SetLength(MESSAGE_SIZE_FIELD_LENGTH);
    SetSize();

    if (m_pBuffer->GetLength() >= MESSAGE_SIZE_FIELD_LENGTH)
    {
        m_uiPosition = MESSAGE_SIZE_FIELD_LENGTH;
    }
}

void Message::SetSize()
{
    const unsigned int uiLength = m_pBuffer->GetLength();

    SetLength(0);

    const unsigned int uiSize = uiLength - MESSAGE_SIZE_FIELD_LENGTH;
    unsigned char aSize[MESSAGE_SIZE_FIELD_LENGTH];

    aSize[0] = static_cast<unsigned char>(uiSize);
    aSize[1] = static_cast<unsigned char>(uiSize >> 8);
    aSize[2] = static_cast<unsigned char>(uiSize >> 16);
    aSize[3] = static_cast<unsigned char>(uiSize >> 24);

    m_pBuffer->AppendData(aSize, MESSAGE_SIZE_FIELD_LENGTH, 0xFFFFFFFFu);
    SetLength(uiLength);
}

void Message::SetToBegining()
{
    m_Result = qResult(MESSAGE_INITIAL_RESULT);

    if (m_pBuffer->GetLength() >= MESSAGE_SIZE_FIELD_LENGTH)
    {
        m_uiPosition = MESSAGE_SIZE_FIELD_LENGTH;
    }
}

Buffer* Message::GetBuffer()
{
    SetSize();
    return m_pBuffer;
}

void Message::AddDataFromMessage(const Message& rMessage)
{
    const Buffer* pBuffer = rMessage.m_pBuffer;

    AppendRaw(pBuffer->GetData() + MESSAGE_SIZE_FIELD_LENGTH, pBuffer->GetLength() - MESSAGE_SIZE_FIELD_LENGTH);
}

}
}
