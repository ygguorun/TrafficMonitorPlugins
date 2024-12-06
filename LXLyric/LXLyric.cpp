#include "pch.h"
#include "LXLyric.h"
#include "DataManager.h"
#include "OptionsDlg.h"

CLXLyric CLXLyric::m_instance;

CLXLyric::CLXLyric()
{
}

CLXLyric& CLXLyric::Instance()
{
    return m_instance;
}

IPluginItem* CLXLyric::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CLXLyric::GetTooltipInfo()
{
    switch (m_item.m_cache_content.type) {
    case ContentType::Lyric:
        break;
    case ContentType::Word: {
        // 拼接 first_line 和 second_line，并加上换行符
        m_tooltip_info = m_item.m_cache_content.first_line + L"\r\n" + m_item.m_cache_content.second_line;
        return m_tooltip_info.c_str();
    }
    }
    return m_tooltip_info.c_str();
}


void CLXLyric::DataRequired()
{

}

ITMPlugin::OptionReturn CLXLyric::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);
    COptionsDlg dlg(pParent);
    dlg.m_data = g_data.m_setting_data;
    if (dlg.DoModal() == IDOK)
    {
        g_data.m_setting_data = dlg.m_data;
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

const wchar_t* CLXLyric::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return L"洛雪歌词插件";
    case TMI_DESCRIPTION:
        return L"调用洛雪开放 API，通过 TrafficMonitor 显示实时歌词";
    case TMI_AUTHOR:
        //TODO: 在此返回作者的名字
        return L"";
    case TMI_COPYRIGHT:
        //TODO: 在此返回版权信息
        return L"Copyright (C) by XXX 2021";
    case ITMPlugin::TMI_URL:
        //TODO: 在此返回URL
        return L"";
        break;
    case TMI_VERSION:
        //TODO: 在此修改插件的版本
        return L"1.00";
    default:
        break;
    }
    return L"";
}

void CLXLyric::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));
        break;
    default:
        break;
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CLXLyric::Instance();
}
