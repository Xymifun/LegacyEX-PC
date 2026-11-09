#ifndef GAME_UI_SUPERFLATMENU_HPP
#define GAME_UI_SUPERFLATMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include "game/item/not_null_ptr.hpp"

class Block;
class FlatGeneratorInfo;
class ItemInstance;
class LaunchMoreOptionsMenuInitData;
class SuperflatConfig;
class UIControl_MultiList;

namespace SuperflatMenu
{
class SliderLabelBehaviour
{
public:
    SliderLabelBehaviour(int name, unsigned int minimum, unsigned int maximum);
    virtual std::wstring GetLabel(int value) = 0;
    unsigned int mMinimum;
    unsigned int mMaximum;
    int mName;
};
class CategoricalLabelBehaviour : public SliderLabelBehaviour
{
public:
    CategoricalLabelBehaviour(int name, int* labels);
    int getLength(int* labels);
    virtual std::wstring GetLabel(int value);
    int* mLabels;
};
class NumericalLabelBehaviour : public SliderLabelBehaviour
{
public:
    NumericalLabelBehaviour(int name, unsigned int minimum, unsigned int maximum);
    NumericalLabelBehaviour(int name, const std::wstring& label, int type, unsigned int minimum, unsigned int maximum);
    NumericalLabelBehaviour(int name, const std::wstring& label, double scale, unsigned int minimum,
                            unsigned int maximum);
    NumericalLabelBehaviour(int name, const std::wstring& label, unsigned int minimum, unsigned int maximum);
    virtual std::wstring GetLabel(int value);
    enum EType
    {
        eType_Integer,
        eType_Decimal
    };
    int mType;
    double mScale;
    std::wstring mFormat;
};
class Mapping;
class Param
{
public:
    typedef Mapping* (*MappingFactory)(int, UIControl_MultiList*);
    Param(bool terminal, bool parent, int name, int desc, int minimum, int maximum, MappingFactory* factory,
          SliderLabelBehaviour* behaviour);
    std::wstring getLabel(int value) const;
    int getName() const;
    bool isTerminal() const;
    bool isParent() const;
    bool isChild() const;
    int getDesc() const;
    int getMinValue() const;
    int getMaxValue() const;
    MappingFactory getFunc() const;
    static Param Parent(MappingFactory* factory, int name, int desc);
    static Param Child(SliderLabelBehaviour* behaviour, int name);
    static Param Terminal();
    bool mTerminal;
    bool mParent;
    int mName;
    int mDesc;
    int mMinValue;
    int mMaxValue;
    MappingFactory* mFactory;
    SliderLabelBehaviour* mBehaviour;
};
class Mapping
{
public:
    class Sub
    {
    public:
        Sub(Mapping* mapping);
        virtual ~Sub();
        void setEnabled(bool enabled);
        void update();
        void setPosition(int position);
        int getPosition();
        int mIndex;
        Mapping* mMapping;
        int mPosition;
        bool mEnabled;
    };
    Mapping(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info) = 0;
    virtual void read(FlatGeneratorInfo& info) = 0;
    virtual ~Mapping() {}
    bool isChecked();
    void updateChildren();
    void setChecked(bool checked);
    void writeConfig(FlatGeneratorInfo& info);
    void readConfig(FlatGeneratorInfo& info);
    template <class T>
    static Mapping* make(int control, UIControl_MultiList* list);
    int mControl;
    UIControl_MultiList* mList;
    std::vector<Sub*> mChildren;
};
class MapCanyons : public Mapping
{
public:
    MapCanyons(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapCaves : public Mapping
{
public:
    MapCaves(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapDecorations : public Mapping
{
public:
    MapDecorations(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapDungeon : public Mapping
{
public:
    MapDungeon(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapLake : public Mapping
{
public:
    MapLake(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapLavaLake : public Mapping
{
public:
    MapLavaLake(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
};
class MapBiomeSpecific : public Mapping
{
public:
    MapBiomeSpecific(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
};
class MapMineshaft : public Mapping
{
public:
    MapMineshaft(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
};
class MapOceanMonument : public Mapping
{
public:
    MapOceanMonument(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
    Mapping::Sub mSub2;
};
class MapShipwrecks : public Mapping
{
public:
    MapShipwrecks(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
    Mapping::Sub mSub2;
};
class MapStronghold : public Mapping
{
public:
    MapStronghold(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
    Mapping::Sub mSub2;
};
class MapVillage : public Mapping
{
public:
    MapVillage(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
    Mapping::Sub mSub2;
};
class MapWoodlandMansion : public Mapping
{
public:
    MapWoodlandMansion(int control, UIControl_MultiList* list);
    virtual void write(FlatGeneratorInfo& info);
    virtual void read(FlatGeneratorInfo& info);
    Mapping::Sub mSub;
    Mapping::Sub mSub2;
};
class BiomeInfo
{
public:
    std::wstring getName() const;
    int mBiome;
    int mDescription;
    not_null_ptr<ItemInstance> mItem;
};
class PresetCache
{
public:
    static void GetCachedPreset(int index, SuperflatConfig& config);
    static void SetCachedPreset(int index, const SuperflatConfig& config);

private:
    static SuperflatConfig sPresets[4];
    static bool sPresetStored[4];
};
class CustomSuperflatBlocks : public boost::unordered_map<int, not_null_ptr<ItemInstance> >
{
public:
    CustomSuperflatBlocks();
    static bool inlist(Block** list, Block* block);
    int addItem(int id, not_null_ptr<ItemInstance> item);
    int addItem(int id, ItemInstance* item);
    not_null_ptr<ItemInstance> item(int id) const;
    static bool hasRestrictedPlacement(Block* block);
    static bool hasRestrictedPlacement(int id);
    static bool sanitiseLayer(int* ids, int* data, int layer);
    static void sanitise(FlatGeneratorInfo& info);
    static uint32_t verify(LaunchMoreOptionsMenuInitData* data, SuperflatConfig* config);
    static bool inlist(const int* list, int value);
    ~CustomSuperflatBlocks();
    int itemcount() const;
    int mMaxId;
};
BiomeInfo* getBiomeInfoA();

const int PARAM_COUNT = 26;
extern Param params[PARAM_COUNT];
extern std::wstring labelSeparator;
}

#endif
