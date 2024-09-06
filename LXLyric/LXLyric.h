#pragma once
#include "PluginInterface.h"
#include "LXLyricItem.h"
#include <string>

class CLXLyric : public ITMPlugin
{
private:
    CLXLyric();

public:
    static CLXLyric& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

private:

private:
    static CLXLyric m_instance;
    CLXLyricItem m_item;
    std::wstring m_tooltip_info;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
