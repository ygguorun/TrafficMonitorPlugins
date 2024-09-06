#pragma once

#include "PluginInterface.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>

struct CacheContent {
    bool multi_line = false;
    bool special_type = false;
    std::wstring first_line;
    std::wstring second_line;
    int type = 0;
    time_t last_update_time = 0;

    CFont* regular_font = nullptr;  // 使用指针而不是直接对象
    CFont* smaller_font = nullptr;  // 使用指针而不是直接对象

    // 构造函数
    CacheContent() = default;

    // 析构函数：确保指针资源在对象销毁时释放
    ~CacheContent() {
        delete regular_font;
        delete smaller_font;
    }

    // 禁止复制操作，避免浅拷贝的问题
    CacheContent(const CacheContent&) = delete;
    CacheContent& operator=(const CacheContent&) = delete;

    // 允许移动操作，优化资源管理
    CacheContent(CacheContent&& other) noexcept {
        *this = std::move(other);
    }

    CacheContent& operator=(CacheContent&& other) noexcept {
        if (this != &other) {
            // 释放当前资源
            delete regular_font;
            delete smaller_font;

            // 转移所有权
            regular_font = other.regular_font;
            smaller_font = other.smaller_font;
            multi_line = other.multi_line;
            special_type = other.special_type;
            first_line = std::move(other.first_line);
            second_line = std::move(other.second_line);
            type = other.type;
            last_update_time = other.last_update_time;

            // 清空其他对象的指针，避免重复释放
            other.regular_font = nullptr;
            other.smaller_font = nullptr;
        }
        return *this;
    }
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

private:
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
};
