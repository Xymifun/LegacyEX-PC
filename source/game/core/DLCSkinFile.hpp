#ifndef GAME_CORE_DLCSKINFILE_HPP
#define GAME_CORE_DLCSKINFILE_HPP

#include <string>
#include <vector>

#include "game/core/DLCFile.hpp"
#include "vendor/_SkinAdjustments.hpp"

struct SKIN_BOX;
class DLCLocalisationFile;

class DLCSkinFile : public DLCFile
{
public:
    DLCSkinFile(const std::wstring& path);
    bool isFiltered();
    virtual void addData(unsigned char* data, unsigned int size);
    virtual void addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value);
    virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring& name);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType parameter);
    DLCLocalisationFile* getLocalisationFile();
    std::wstring lookupString(const std::wstring& key, const std::wstring& fallback);
    std::vector<SKIN_BOX*>* getAdditionalBoxes();
    int getAdditionalBoxesCount();
    _SkinAdjustments getSkinAdjustments();
    bool isFree();

private:
    std::wstring mDisplayName;
    std::wstring mThemeName;
    bool mLocKeySet;
    std::wstring mLocKey;
    std::wstring mDisplayNameID;
    std::wstring mThemeNameID;
    std::wstring mCapePath;
    _SkinAdjustments mSkinAdjustments;
    bool mFree;
    std::vector<SKIN_BOX*> mAdditionalBoxes;
};

#endif
