#ifndef SOURCE_NN_NEX_QRESULT_HPP
#define SOURCE_NN_NEX_QRESULT_HPP
#include <stdint.h>
#include "nn/nex/RootObject.hpp"
namespace nn
{
namespace nex
{
class qResult : public RootObject
{
public:
    qResult();
    explicit qResult(const int&);
    explicit qResult(const bool&);
    ~qResult() {}
    bool Equals(const int&) const;
    bool Equals(const bool&) const;
    operator bool() const;
    unsigned int GetReturnCode() const { return m_code; }
    bool IsError() const { return (m_code & ERROR_BIT) != 0; }
    bool operator==(const qResult& rOther) const { return m_code == rOther.m_code; }
    qResult& operator=(const qResult&);
    qResult& operator=(const bool&);
    int GetFacility() const;

private:
    static const uint32_t ERROR_BIT = 0x80000000u;

    int32_t m_code;
    const wchar_t* m_file;
    int32_t m_line;
};
}
}

template <class T>
bool operator==(const nn::nex::qResult& rResult, const T& rValue)
{
    return rResult.Equals(rValue);
}

template <class T>
bool operator!=(const nn::nex::qResult& rResult, const T& rValue)
{
    return !rResult.Equals(rValue);
}

#endif
