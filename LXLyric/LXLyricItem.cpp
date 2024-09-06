#include "pch.h"
#include "LXLyricItem.h"
#include "Common.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include "DataManager.h"

CLXLyricItem::CLXLyricItem() {
    // 初始化缓存内容
    m_cache_content = CacheContent();

    // 启动独立线程获取歌词
    m_fetch_thread = std::thread([this]() {
        while (m_running) {
            fetch_content();  // 周期性地获取歌词
            std::this_thread::sleep_for(std::chrono::microseconds(100));  // 每10秒更新一次
        }
        });
}

CLXLyricItem::~CLXLyricItem() {
    // 停止线程
    m_running = false;
    if (m_fetch_thread.joinable()) {
        m_fetch_thread.join();
    }
}

std::wstring CLXLyricItem::Utf8ToWstring(const std::string& utf8Str) {
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wideCharLen == 0) {
        return L"";
    }

    std::wstring wideStr(wideCharLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideCharLen);
    return wideStr;
}

void CLXLyricItem::prepare_fonts(CDC* pDC) {
    if (!m_cache_content.regular_font) {
        // 分配内存用于常规字体
        m_cache_content.regular_font = new CFont();
        m_cache_content.smaller_font = new CFont();

        // 获取当前字体
        CFont* pOldFont = pDC->GetCurrentFont();
        LOGFONT logfont;
        pOldFont->GetLogFont(&logfont);

        // 创建常规字体
        m_cache_content.regular_font->CreateFontIndirect(&logfont);

        // 创建小字号字体
        logfont.lfHeight += 2;  // 将字号减小2
        m_cache_content.smaller_font->CreateFontIndirect(&logfont);
    }
}


void CLXLyricItem::fetch_content() {
    std::wstring url = L"http://127.0.0.1:23330/status?filter=lyricLineAllText";
    std::string responseStr;

    // 使用 CCommon::GetURL 获取歌词数据
    if (CCommon::GetURL(url, responseStr, true, L"", L"", 0)) {
        size_t pos1 = responseStr.find("\"lyricLineAllText\":\"");
        size_t pos2 = responseStr.find("\"", pos1 + 20);
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            std::string lyricText = responseStr.substr(pos1 + 20, pos2 - (pos1 + 20));
            std::wstring lyric = CCommon::StrToUnicode(lyricText.c_str(), true);
            if (!lyric.empty()) {
                std::lock_guard<std::mutex> lock(m_cache_mutex);
                m_cache_content.type = 0;  // 歌词类型
                handel_lyric(lyric);
                return;
            }
            
        }
    }
    if (should_update_dict()) {
        // 如果无法获取歌词，则从词典中选择随机词语
        if (select_random_line_from_dict()) {
            std::lock_guard<std::mutex> lock(m_cache_mutex);
            m_cache_content.type = 1;  // word 类型
            m_cache_content.multi_line = true;
            m_cache_content.special_type = false; 
            m_cache_content.last_update_time = std::time(nullptr);
        }
    }
}

void CLXLyricItem::handel_lyric(std::wstring& text) {
    m_cache_content.type = 0;
    std::vector<std::wstring> lines;
    size_t pos = 0, found;
    while ((found = text.find(L"\\n", pos)) != std::wstring::npos) {
        std::wstring tmp_str = text.substr(pos, found - pos);
        lines.push_back(tmp_str);
        pos = found + 2;
    }
    lines.push_back(text.substr(pos)); // 最后一段

    // 确保最多两行
    if (lines.size() > 2) {
        lines.resize(2);
    }

    m_cache_content.first_line = lines[0];
    if (lines.size() == 2) {
        m_cache_content.multi_line = true;
        m_cache_content.special_type = true;
        m_cache_content.second_line = lines[1];
    }
    else {
        m_cache_content.multi_line = false;
        m_cache_content.special_type = false;
    }
}

bool CLXLyricItem::select_random_line_from_dict() {
    if (lines.empty()) {
        std::ifstream file("C:\\Users\\admin\\scoop\\persist\\trafficmonitor\\plugins\\dict.txt");
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open file 'dict.txt'" << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(Utf8ToWstring(line));
        }
        file.close();
    }

    if (lines.empty()) return false;

    srand(time(NULL));
    int random_index = rand() % lines.size();
    std::wstring selected_line = lines[random_index];

    size_t pos = selected_line.find(L'   ');
    if (pos != std::wstring::npos) {
        m_cache_content.first_line= selected_line.substr(0, pos);
        m_cache_content.second_line = selected_line.substr(pos + 3);
        return true;
    }
    return false;
}

bool CLXLyricItem::should_update_dict() {
    time_t current_time = std::time(nullptr);
    return difftime(current_time, m_cache_content.last_update_time) > 10;  // 10秒更新周期
}

const wchar_t* CLXLyricItem::GetItemName() const {
    return g_data.StringRes(IDS_PLUGIN_ITEM_NAME);
}

const wchar_t* CLXLyricItem::GetItemId() const {
    return L"";
}

const wchar_t* CLXLyricItem::GetItemLableText() const {
    return L"";
}

const wchar_t* CLXLyricItem::GetItemValueText() const {
    return L"";
}

const wchar_t* CLXLyricItem::GetItemValueSampleText() const {
    return L"";
}

bool CLXLyricItem::IsCustomDraw() const {
    return true;
}

int CLXLyricItem::GetItemWidthEx(void* hDC) const {
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    return 121;
}

void CLXLyricItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) {
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CRect rect(CPoint(x, y), CSize(w, h));

    std::lock_guard<std::mutex> lock(m_cache_mutex);  // 加锁确保安全访问缓存

    // 准备字体
    prepare_fonts(pDC);

    // 判断是否是多行内容
    if (m_cache_content.multi_line) {
        CRect firstRect = rect;
        CRect secondRect = rect;
        int lineHeight = pDC->GetTextExtent(L"围").cy;
        firstRect.bottom -= lineHeight;
        secondRect.top += lineHeight;

        // 第一行文本处理
        std::wstring firstLine = m_cache_content.first_line;
        if (pDC->GetTextExtent(firstLine.c_str()).cx > w) {
            // 截断第一行并添加 "..."
            while (pDC->GetTextExtent(firstLine.c_str()).cx + pDC->GetTextExtent(L"...").cx > w) {
                firstLine.pop_back();  // 删除最后一个字符
            }
            firstLine += L"...";  // 添加 "..."
        }

        pDC->SelectObject(&m_cache_content.regular_font);
        pDC->DrawText(firstLine.c_str(), firstRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

        // 第二行文本处理
        std::wstring secondLine = m_cache_content.second_line;
        if (m_cache_content.special_type) {
            pDC->SelectObject(&m_cache_content.smaller_font);
        }

        if (pDC->GetTextExtent(secondLine.c_str()).cx > w) {
            // 截断第二行并添加 "..."
            while (pDC->GetTextExtent(secondLine.c_str()).cx + pDC->GetTextExtent(L"...").cx > w) {
                secondLine.pop_back();  // 删除最后一个字符
            }
            secondLine += L"...";  // 添加 "..."
        }

        pDC->DrawText(secondLine.c_str(), secondRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

        // 恢复原字体
        pDC->SelectObject(&m_cache_content.regular_font);
    }
    else {
        // 单行情况
        std::wstring firstLine = m_cache_content.first_line;
        int textWidth = pDC->GetTextExtent(firstLine.c_str()).cx;

        if (textWidth > w) {
            std::wstring secondLine;
            // 如果宽度超出，且文本中没有空格
            size_t breakPos = firstLine.find(L' '); // 通过空格分割为两行
            if (breakPos == std::wstring::npos) {
                size_t i = 0;
                int currentWidth = 0;
                // 根据rect宽度自动换行
                for (; i < firstLine.size(); ++i) {
                    currentWidth += pDC->GetTextExtent(firstLine.substr(i, 1).c_str()).cx;
                    if (currentWidth > w) {
                        secondLine = firstLine.substr(i); // 剩余部分成为第二行
                        firstLine = firstLine.substr(0, i); // 第一行
                        break;
                    }
                }
            }
            else {
                    secondLine = firstLine.substr(breakPos + 1);  // 剩余部分成为第二行
                    firstLine = firstLine.substr(0, breakPos);
            }
            // 截断第二行并添加 "..."
            int secondLineWidth = pDC->GetTextExtent(secondLine.c_str()).cx;
            if (secondLineWidth > w) {
                while (secondLineWidth + pDC->GetTextExtent(L"...").cx > w) {
                    secondLine.pop_back();
                    secondLineWidth = pDC->GetTextExtent(secondLine.c_str()).cx;
                }
                secondLine += L"...";
            }

            // 绘制两行
            int lineHeight = pDC->GetTextExtent(L"围").cy;
            CRect firstRect = rect;
            CRect secondRect = rect;
            firstRect.bottom -= lineHeight;
            secondRect.top += lineHeight;
            pDC->DrawText(firstLine.c_str(), firstRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            pDC->DrawText(secondLine.c_str(), secondRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }
        // 宽度未超出，水平垂直居中
        else {
            pDC->DrawText(firstLine.c_str(), rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }
    }
}
