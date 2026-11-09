#ifndef GAME_DATA_CONSOLESAVEPATH_HPP
#define GAME_DATA_CONSOLESAVEPATH_HPP

#include <string>

class ConsoleSavePath
{
public:
    ConsoleSavePath(const std::wstring& path) : m_path(path) {}

    std::wstring getPath() const { return m_path; }

private:
    std::wstring m_path;
};

#endif
