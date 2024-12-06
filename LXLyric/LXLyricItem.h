#pragma once

#include "PluginInterface.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>

enum class ContentType {
    Lyric,  // 歌词
    Word    // 单词
};

struct CacheContent {
    bool multi_line = false;
    bool special_type = false;
    std::wstring first_line;
    std::wstring second_line;
    ContentType type = ContentType::Lyric;
    time_t last_update_time = 0;
    bool force_update = false;
};



class CLXLyricItem : public IPluginItem
{
public:
    CLXLyricItem();
    ~CLXLyricItem();

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    virtual int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;


public:
    void fetch_content();  // 线程函数，获取歌词
    void handel_lyric(std::wstring& text);
    std::wstring Utf8ToWstring(const std::string& utf8Str);
    bool select_random_line_from_dict();
    bool should_update_dict();
    void prepare_fonts(CDC* pDC); // 准备字体

    CacheContent m_cache_content;         // 缓存待绘制的内容
    std::thread m_fetch_thread;           // 独立线程用于获取歌词
    std::mutex m_cache_mutex;             // 用于保护缓存内容的互斥锁
    bool m_running = true;                // 控制线程的运行状态
    std::vector<std::wstring> lines;      // 存储词典的行

        // 添加字体为类的成员
    CFont regular_font;
    CFont smaller_font;
};
