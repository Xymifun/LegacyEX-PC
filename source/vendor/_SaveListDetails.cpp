#include "vendor/_SaveListDetails.hpp"

#include <cstring>

_SaveListDetails::_SaveListDetails()
{
    m_saveIndex = 0;
    m_thumbnail = 0;
    m_thumbnailSize = 0;
    std::memset(m_title, 0, sizeof(m_title));
    std::memset(m_filename, 0, sizeof(m_filename));
}
