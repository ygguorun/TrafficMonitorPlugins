
// PluginTesterDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PluginTester.h"
#include "PluginTesterDlg.h"
#include "afxdialogex.h"
#include "../utilities/bass64/base64.h"
#include "../utilities/Common.h"
#include <fstream>
#include "PluginInfoDlg.h"
#include "DrawCommon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID 1548

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPluginTesterDlg �Ի���



CPluginTesterDlg::CPluginTesterDlg(CWnd* pParent /*=NULL*/)
    : CDialog(IDD_PLUGINTESTER_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPluginTesterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SELECT_PLUGIN_COMBO, m_select_plugin_combo);
}

PluginInfo CPluginTesterDlg::LoadPlugin(const std::wstring& plugin_file_name)
{
    PluginInfo plugin_info;
    if (plugin_file_name.empty())
        return plugin_info;

    if (plugin_info.plugin_module != NULL)
        FreeLibrary(plugin_info.plugin_module);

    //���dll��·��
    plugin_info.file_path = m_plugin_dir + plugin_file_name;
    //����ļ���
    std::wstring file_name{ plugin_file_name };
    if (!file_name.empty() && (file_name[0] == L'\\' || file_name[0] == L'/'))
        file_name = file_name.substr(1);

    //����dll
    plugin_info.plugin_module = LoadLibrary(plugin_info.file_path.c_str());
    if (plugin_info.plugin_module == NULL)
    {
        return plugin_info;
    }

    //��ȡ��������ڵ�ַ
    pfTMPluginGetInstance TMPluginGetInstance = (pfTMPluginGetInstance)::GetProcAddress(plugin_info.plugin_module, "TMPluginGetInstance");
    if (TMPluginGetInstance == NULL)
    {
        return plugin_info;
    }

    //�����������
    plugin_info.plugin = TMPluginGetInstance();
    if (plugin_info.plugin == nullptr)
        return plugin_info;

    ////������汾
    //int version = plugin_info.plugin->GetAPIVersion();
    //if (version <= PLUGIN_UNSUPPORT_VERSION)
    //{
    //    plugin_info.state = PluginState::PS_VERSION_NOT_SUPPORT;
    //    continue;
    //}

    //��ȡ�����Ϣ
    for (int i{}; i < ITMPlugin::TMI_MAX; i++)
    {
        ITMPlugin::PluginInfoIndex index{ static_cast<ITMPlugin::PluginInfoIndex>(i) };
        plugin_info.properties[index] = plugin_info.plugin->GetInfo(index);
    }

    //��ȡ�����ʾ��Ŀ
    int index = 0;
    for (int i = 0; i < 99; i++)
    {
        IPluginItem* item = plugin_info.plugin->GetItem(index);
        if (item == nullptr)
            break;
        plugin_info.plugin_items.push_back(item);
        index++;
    }

    return plugin_info;
}

void CPluginTesterDlg::EnableControl()
{
    CString str;
    m_select_plugin_combo.GetWindowText(str);
    bool enable = (!str.IsEmpty());
    EnableControl(enable);
}

void CPluginTesterDlg::EnableControl(bool enable)
{
    GetDlgItem(IDC_OPTION_BUTTON)->EnableWindow(enable);
    GetDlgItem(IDC_DETAIL_BUTTON)->EnableWindow(enable);
}

PluginInfo CPluginTesterDlg::GetCurrentPlugin()
{
    if (m_cur_index >= 0 && m_cur_index < m_plugins.size())
        return m_plugins[m_cur_index];
    else
        return PluginInfo();
}

BEGIN_MESSAGE_MAP(CPluginTesterDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND(ID_BASS64_ENCODE, &CPluginTesterDlg::OnBass64Encode)
    ON_COMMAND(ID_BASE64_DECODE, &CPluginTesterDlg::OnBase64Decode)
    ON_COMMAND(ID_APP_ABOUT, &CPluginTesterDlg::OnAppAbout)
    ON_CBN_SELCHANGE(IDC_SELECT_PLUGIN_COMBO, &CPluginTesterDlg::OnCbnSelchangeSelectPluginCombo)
    ON_BN_CLICKED(IDC_OPTION_BUTTON, &CPluginTesterDlg::OnBnClickedOptionButton)
    ON_BN_CLICKED(IDC_DETAIL_BUTTON, &CPluginTesterDlg::OnBnClickedDetailButton)
    ON_WM_TIMER()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


// CPluginTesterDlg ��Ϣ��������

BOOL CPluginTesterDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ��������...���˵������ӵ�ϵͳ�˵��С�

    // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
#ifdef _DEBUG
            pSysMenu->AppendMenu(MF_STRING, IDM_TEST_CMD, _T("Test Command"));
#endif
        }
    }

    // ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // TODO: �ڴ����Ӷ���ĳ�ʼ������

    //�������в������ʼ�������б�
    std::vector<std::wstring> files;
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    size_t index;
    m_plugin_dir = path;
    index = m_plugin_dir.find_last_of(L'\\');
    m_plugin_dir = m_plugin_dir.substr(0, index + 1);
    CCommon::GetFiles((m_plugin_dir + L"*.dll").c_str(), files);
    for (const auto& file_name : files)
    {
        m_select_plugin_combo.AddString(file_name.c_str());
        m_plugins.push_back(LoadPlugin(file_name));
        ITMPlugin* plugin = m_plugins.back().plugin;
        if (plugin != nullptr)
            plugin->OnExtenedInfo(ITMPlugin::EI_CONFIG_DIR, m_plugin_dir.c_str());
    }

    m_select_plugin_combo.SetCurSel(0);
    OnCbnSelchangeSelectPluginCombo();

    SetTimer(TIMER_ID, 1000, nullptr);

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPluginTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if (nID == IDM_TEST_CMD)
    {
        std::string str_base64 = utilities::Base64Encode(CCommon::UnicodeToStr(L"�����ı�abcde", true));

        std::wstring str_ori = CCommon::StrToUnicode(utilities::Base64Decode(str_base64).c_str(), true);

        bool is_base64 = utilities::IsBase64Code(str_base64);

        int a = 0;
    }
    else if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

int CPluginTesterDlg::GetItemWidth(IPluginItem* pItem, CDC* pDC)
{
    int width = 0;
    ITMPlugin* plugin = GetCurrentPlugin().plugin;
    if (plugin != nullptr && plugin->GetAPIVersion() >= 3)
    {
        width = pItem->GetItemWidthEx(pDC->GetSafeHdc());       //����ʹ��GetItemWidthEx�ӿڻ�ȡ����
    }
    if (width == 0)
    {
        width = theApp.DPI(pItem->GetItemWidth());
    }
    return width;

}


void CPluginTesterDlg::SavePluginItemRect(IPluginItem* pItem, CRect rect)
{
    if (pItem != nullptr)
    {
        rect.MoveToX(rect.left + m_draw_rect.left);
        rect.MoveToY(rect.top + m_draw_rect.top);
        m_plugin_item_rect[pItem->GetItemId()] = rect;
    }
}

IPluginItem* CPluginTesterDlg::GetPluginItemByPoint(CPoint point)
{
    PluginInfo plugin_info = GetCurrentPlugin();
    for (IPluginItem* plugin : plugin_info.plugin_items)
    {
        if (plugin != nullptr)
        {
            CRect rc_item = m_plugin_item_rect[plugin->GetItemId()];
            if (rc_item.PtInRect(point))
                return plugin;
        }
    }
    return nullptr;
}

CPoint CPluginTesterDlg::GetMouseCursurPosition()
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    return point;
}

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPluginTesterDlg::OnPaint()
{
    CWindowDC dc(this);
    CRect draw_rect;
    GetClientRect(draw_rect);
    draw_rect.MoveToY(theApp.DPI(100));
    draw_rect.MoveToX(0);
    CDrawDoubleBuffer draw_double_buffer(&dc, draw_rect);
    m_draw_rect = draw_rect;

    CDrawCommon drawer;
    drawer.Create(draw_double_buffer.GetMemDC(), this);
    
    draw_rect.MoveToY(0);
    drawer.FillRect(draw_rect, GetSysColor(COLOR_WINDOW));

    //���Ʋ������ʾ��Ŀ
    int index = 0;
    PluginInfo cur_plugin = GetCurrentPlugin();
    for (const auto& item : cur_plugin.plugin_items)
    {
        if (item != nullptr)
        {
            //�����ı�
            CRect rc_text;
            rc_text.left = theApp.DPI(16);
            rc_text.top = theApp.DPI(12) + index * (theApp.DPI(56));
            rc_text.bottom = rc_text.top + theApp.DPI(24);
            rc_text.right = rc_text.left + theApp.DPI(120);
            drawer.DrawWindowText(rc_text, item->GetItemName(), RGB(0, 0, 0));

            //������ʾ��Ŀ
            CRect rc_item = rc_text;
            rc_item.MoveToY(rc_text.bottom);
            int item_width = GetItemWidth(item, drawer.GetDC());
            if (item_width > 0)
                rc_item.right = rc_item.left + item_width;
            drawer.FillRect(rc_item, RGB(205, 221, 234));

            //������ʾ��Ŀ��λ��
            SavePluginItemRect(item, rc_item);

            if (item->IsCustomDraw())
            {
                item->DrawItem(drawer.GetDC()->GetSafeHdc(), rc_item.left, rc_item.top, rc_item.Width(), rc_item.Height(), false);
            }
            else
            {
                CRect rc_label = rc_item;
                rc_label.right = rc_label.left + drawer.GetDC()->GetTextExtent(item->GetItemLableText()).cx;
                drawer.DrawWindowText(rc_label, item->GetItemLableText(), RGB(0, 0, 0));

                CRect rc_value = rc_item;
                rc_value.left = rc_label.right + theApp.DPI(6);
                drawer.DrawWindowText(rc_value, item->GetItemValueText(), RGB(0, 0, 0));
            }
            index++;
        }
    }
    CDialog::OnPaint();
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPluginTesterDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CPluginTesterDlg::OnBass64Encode()
{
    //��һ���ļ�
    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, _T("(*.*)|*.*||"), this);
    if (fileDlg.DoModal() == IDOK)
    {
        CString file_name = fileDlg.GetPathName();
        std::string file_contents;
        CCommon::GetFileContent(file_name.GetString(), file_contents);
        std::string base64_contents = utilities::Base64Encode(file_contents);

        //�����ļ�
        CString file_name_base64 = file_name += _T(".base64.txt");
        std::ofstream file_stream(file_name_base64.GetString());
        file_stream << base64_contents;
        file_stream.close();
    }
}


void CPluginTesterDlg::OnBase64Decode()
{
    // TODO: �ڴ�����������������
}


void CPluginTesterDlg::OnAppAbout()
{
    // TODO: �ڴ�����������������
    CAboutDlg dlg;
    dlg.DoModal();
}


void CPluginTesterDlg::OnCbnSelchangeSelectPluginCombo()
{
    m_cur_index = m_select_plugin_combo.GetCurSel();
    EnableControl();
    Invalidate();
}


void CPluginTesterDlg::OnBnClickedOptionButton()
{
    PluginInfo cur_plugin = GetCurrentPlugin();
    if (cur_plugin.plugin != nullptr)
    {
        ITMPlugin::OptionReturn rtn = cur_plugin.plugin->ShowOptionsDialog(m_hWnd);
    }
}


void CPluginTesterDlg::OnBnClickedDetailButton()
{
    CPluginInfoDlg dlg(GetCurrentPlugin());
    dlg.DoModal();
}


void CPluginTesterDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
    if (nIDEvent == TIMER_ID)
    {
        PluginInfo cur_plugin = GetCurrentPlugin();
        if (cur_plugin.plugin != nullptr)
        {
            cur_plugin.plugin->DataRequired();
            InvalidateRect(m_draw_rect, FALSE);
        }
    }

    CDialog::OnTimer(nIDEvent);
}


void CPluginTesterDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
    point = GetMouseCursurPosition();
    IPluginItem* plugin_item = GetPluginItemByPoint(point);
    if (plugin_item != nullptr)
    {
        CRect rc_item = m_plugin_item_rect[plugin_item->GetItemId()];
        plugin_item->OnMouseEvent(IPluginItem::MT_RCLICKED, point.x - rc_item.left, point.y - rc_item.top, GetSafeHwnd(), IPluginItem::MF_TASKBAR_WND);
    }
    else
    {
        CDialog::OnLButtonUp(nFlags, point);
    }
}


void CPluginTesterDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
    point = GetMouseCursurPosition();
    IPluginItem* plugin_item = GetPluginItemByPoint(point);
    if (plugin_item != nullptr)
    {
        CRect rc_item = m_plugin_item_rect[plugin_item->GetItemId()];
        plugin_item->OnMouseEvent(IPluginItem::MT_DBCLICKED, point.x - rc_item.left, point.y - rc_item.top, GetSafeHwnd(), IPluginItem::MF_TASKBAR_WND);
    }
    else
    {
        CDialog::OnLButtonDblClk(nFlags, point);
    }
}


void CPluginTesterDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
    // TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
    point = GetMouseCursurPosition();
    IPluginItem* plugin_item = GetPluginItemByPoint(point);
    if (plugin_item != nullptr)
    {
        CRect rc_item = m_plugin_item_rect[plugin_item->GetItemId()];
        plugin_item->OnMouseEvent(IPluginItem::MT_RCLICKED, point.x - rc_item.left, point.y - rc_item.top, GetSafeHwnd(), IPluginItem::MF_TASKBAR_WND);
    }
    else
    {
        CDialog::OnRButtonUp(nFlags, point);
    }
}