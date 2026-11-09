#ifndef GAME_CORE_JOINER_HPP
#define GAME_CORE_JOINER_HPP

#include <string>
#include <vector>

class Joiner
{
public:
    static Joiner on(const std::wstring& separator);
    static Joiner on(wchar_t separator);

    ~Joiner() {}

    void appendTo(std::wstring& out, const std::vector<std::wstring>& parts) const;

private:
    Joiner(const std::wstring& separator);
    Joiner(wchar_t separator);

    std::wstring mSeparator;
};

#endif
