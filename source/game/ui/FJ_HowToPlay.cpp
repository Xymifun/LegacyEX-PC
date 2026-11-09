#include "game/ui/FJ_HowToPlay.hpp"

#include <cstdio>

#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_ProgressBar.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_HowToPlay::FJ_HowToPlay(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    InitScene();
}

void FJ_HowToPlay::_init(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
}

void FJ_HowToPlay::init(FJ_Event* pEvent)
{
}

bool FJ_HowToPlay::LoadHowToPlayPage(int page, std::vector<std::wstring>& pages)
{
    std::wstring text;
    if (pages.size() == 0)
        text = L"";
    else
        text = pages[0];

    if (page > 28)
        return false;
    if (m_page == page && !m_initialized)
        return false;

    m_initialized = false;
    char name[64];
    if (m_page == -1)
    {
        for (int i = 0; i < 29; ++i)
        {
            std::sprintf(name, "HowToPlayText_%d", i);
            FJ_FuiNode* textNode = getChildByName(name);
            if (textNode != NULL)
                textNode->setVisible(false);

            std::sprintf(name, "HowToPlayImage_%d", i);
            FJ_FuiNode* image = getChildByName(name);
            if (image != NULL)
                image->setVisible(false);

            std::sprintf(name, "HowToPlayContainer_%d", i);
            FJ_FuiNode* container = getChildByName(name);
            FJ_FuiNode* parent = this;
            if (container != NULL)
            {
                container->setVisible(false);
                parent = container;
            }

            for (int label = 1; label <= 4; ++label)
            {
                std::sprintf(name, "Label%d_%d", label, i);
                FJ_FuiNode* labelNode = getChildByName(name);
                if (labelNode == NULL)
                    break;
                FJ_Label* labelControl = labelNode->asFJ_Label();
                if (labelControl == NULL)
                    break;
                labelControl->setVisible(false);
            }

            for (int list = 0; list < 7; ++list)
            {
                std::sprintf(name, "HowToPlaySlotList_%d_%d", list, i);
                FJ_FuiNode* listNode = parent->getChildByName(name);
                if (listNode == NULL)
                    break;
                FJ_SlotList* slotList = listNode->asFJ_SlotList();
                if (slotList == NULL)
                    break;
                slotList->setVisible(false);
            }
        }
    }
    else
    {
        std::sprintf(name, "HowToPlayImage_%d", m_page);
        FJ_FuiNode* image = getChildByName(name);
        if (image != NULL)
            image->setVisible(false);

        std::sprintf(name, "HowToPlayContainer_%d", m_page);
        FJ_FuiNode* container = getChildByName(name);
        FJ_FuiNode* parent = this;
        if (container != NULL)
        {
            container->setVisible(false);
            parent = container;
        }

        for (int label = 1; label <= 4; ++label)
        {
            std::sprintf(name, "Label%d_%d", label, m_page);
            FJ_FuiNode* labelNode = getChildByName(name);
            if (labelNode == NULL)
                break;
            FJ_Label* labelControl = labelNode->asFJ_Label();
            if (labelControl == NULL)
                break;
            labelControl->setVisible(false);
        }

        for (int list = 0; list < 7; ++list)
        {
            std::sprintf(name, "HowToPlaySlotList_%d_%d", list, m_page);
            FJ_FuiNode* listNode = parent->getChildByName(name);
            if (listNode == NULL)
                break;
            FJ_SlotList* slotList = listNode->asFJ_SlotList();
            if (slotList == NULL)
                break;
            slotList->Clear();
            slotList->setVisible(false);
        }
    }

    std::sprintf(name, "HowToPlayText_%d", page);
    FJ_FuiNode* textNode = getChildByName(name);
    if (textNode != NULL)
    {
        m_dynamicHtmlText->setX(textNode->getX());
        m_dynamicHtmlText->setY(textNode->getY());
        m_dynamicHtmlText->setWidth(textNode->getWidth());
        m_dynamicHtmlText->setHeight(textNode->getHeight());
        m_dynamicHtmlText->Reload(Math::round(textNode->getWidth()), Math::round(textNode->getHeight()));
        if (pages.size() != 0)
        {
            bool last = false;
            for (unsigned int i = 0; i < pages.size(); ++i)
            {
                if (i == pages.size() - 1)
                    last = true;
                m_dynamicHtmlText->AddText(pages[i], last);
            }
        }
        getStage()->setFocus(m_dynamicHtmlText);
    }

    std::sprintf(name, "HowToPlayImage_%d", page);
    FJ_FuiNode* image = getChildByName(name);
    if (image != NULL)
        image->setVisible(true);

    std::sprintf(name, "HowToPlayContainer_%d", page);
    FJ_FuiNode* container = getChildByName(name);
    FJ_FuiNode* parent = this;
    if (container != NULL)
    {
        container->setVisible(true);
        parent = container;
    }

    for (int label = 1; label <= 4; ++label)
    {
        std::sprintf(name, "Label%d_%d", label, page);
        FJ_FuiNode* labelNode = getChildByName(name);
        if (labelNode == NULL)
            break;
        FJ_Label* labelControl = labelNode->asFJ_Label();
        if (labelControl == NULL)
            break;
        labelControl->setVisible(true);
    }

    int slot = 0;
    for (int list = 0; list < 7; ++list)
    {
        std::sprintf(name, "HowToPlaySlotList_%d_%d", list, page);
        FJ_FuiNode* listNode = parent->getChildByName(name);
        if (listNode == NULL)
            break;
        FJ_SlotList* slotList = listNode->asFJ_SlotList();
        if (slotList == NULL)
            break;
        slotList->setVisible(true);
        slotList->Init();
        for (int i = 0; i < slotList->getRows() * slotList->getColumns(); ++i)
            slotList->addSlot(slot++, false, false);
    }

    if (page == 3)
    {
        FJ_LabelOutline* level = parent->getChildByName("PlayerLevel")->asFJ_LabelOutline();
        if (level != NULL)
            level->SetLabel(L"3");
        FJ_ExperienceBar* experience = parent->getChildByName("ExpBar")->asFJ_ExperienceBar();
        if (experience != NULL)
            experience->setProgress(0.75f);
    }
    else if (page == 10)
    {
        FJ_ProgressBar* fire = parent->getChildByName("FurnaceFire")->asFJ_ProgressBar();
        if (fire != NULL)
        {
            fire->setProgress(0.5f);
            fire->setProgress(0.5f);
        }
    }
    else if (page == 12)
    {
        FJ_ProgressBar* bubbles = parent->getChildByName("BrewingBubbles")->asFJ_ProgressBar();
        FJ_ProgressBar* arrow = parent->getChildByName("BrewingArrow")->asFJ_ProgressBar();
        FJ_ProgressBar* plate = parent->getChildByName("BrewingPlateBar")->asFJ_ProgressBar();
        if (bubbles != NULL)
        {
            bubbles->setProgress(0.5f);
            arrow->setProgress(0.5f);
            plate->setProgress(0.7f);
        }
    }
    else if (page == 13)
    {
        FJ_FuiNode* label1 = parent->getChildByName("EnchantmentLabel01");
        FJ_FuiNode* label2 = parent->getChildByName("EnchantmentLabel02");
        FJ_FuiNode* label3 = parent->getChildByName("EnchantmentLabel03");
        if (label1 != NULL)
        {
            label1->getChildAt(0)->getRenderNodeChildAt(0)->asEditText()->setText(L"3");
            label2->getChildAt(0)->getRenderNodeChildAt(0)->asEditText()->setText(L"5");
            label3->getChildAt(0)->getRenderNodeChildAt(0)->asEditText()->setText(L"8");
        }
    }
    else if (page == 21)
    {
        FJ_FuiNode* input = parent->getChildByName("AnvilTextInput");
        if (input != NULL)
            input->getChildAt(0)->getRenderNodeChildAt(0)->asEditText()->setText(L"Excalibur");
    }
    else if (page == 22)
    {
        std::sprintf(name, "HowToPlaySlotList_%d_%d", 2, 22);
        FJ_FuiNode* listNode = parent->getChildByName(name);
        FJ_SlotList* slotList = NULL;
        if (listNode != NULL)
            slotList = listNode->asFJ_SlotList();
        if (slotList != NULL)
        {
            slotList->SetSlotRedBox(0, true);
            slotList->SetSlotRedBox(1, true);
            slotList->SetSlotRedBox(4, true);
        }
    }

    m_page = page;
    return true;
}

void FJ_HowToPlay::InitScene()
{
    addEventListener(eFJEventType_Init, this, _init, false);
    m_page = -1;
    m_text = L"";
    m_dynamicHtmlText = getChildByName("DynamicHtmlText")->asFJ_DynamicHtmlText();
    m_dynamicHtmlText->setX(0.0f);
    std::vector<std::wstring> pages;
    pages.push_back(L"");
    LoadHowToPlayPage(0, pages);
    m_initialized = true;
}

FJ_HowToPlay* FJ_HowToPlay::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HowToPlay(pRenderNode);
}

unsigned int FJ_HowToPlay::getType()
{
    return 88;
}

void FJ_HowToPlay::SetSafeZone(int top, int bottom, int left, int right)
{
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(left);
        setY(top);
    }
}
