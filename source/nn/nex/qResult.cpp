#include "nn/nex/qResult.hpp"
namespace nn
{
namespace nex
{
qResult::qResult()
    : m_code(0x80010001),
      m_file(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/Result.cpp"),
      m_line(106)
{
}
qResult::qResult(const int& code)
    : m_code(code),
      m_file(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/Result.cpp"),
      m_line(0x71)
{
}
qResult::qResult(const bool& ok)
    : m_code(ok ? 0x10001 : 0x80010001),
      m_file(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/Result.cpp"),
      m_line(0x7f)
{
}
bool qResult::Equals(const int& code) const
{
    return m_code == code;
}
bool qResult::Equals(const bool& ok) const
{
    return !IsError() == ok;
}
qResult::operator bool() const
{
    return IsError() == false;
}
qResult& qResult::operator=(const qResult& other)
{
    m_code = other.m_code;
    m_file = other.m_file;
    m_line = other.m_line;
    return *this;
}
qResult& qResult::operator=(const bool& ok)
{
    m_code = ok ? 0x10001 : 0x80010001;
    m_file = L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/Result.cpp";
    m_line = 0xa2;
    return *this;
}
int qResult::GetFacility() const
{
    return (m_code & 0x07fff000) >> 16;
}
}
}
