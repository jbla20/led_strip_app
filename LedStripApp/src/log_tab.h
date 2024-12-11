#pragma once

#include <iostream>
#include <streambuf>
#include <string>
#include <chrono>

#include "app_tab.h"

// Logger widget based on https://github.com/ocornut/imgui/issues/300
namespace ImGui
{
    struct LoggerWidget
    {
        ImGuiTextBuffer Buf;
        ImGuiTextFilter Filter;
        ImVector<int> LineOffsets;  // Index to lines offset. We maintain this with AddLog() calls.
        bool AutoScroll;            // Keep scrolling if already at the bottom.
        int LogLevel = 1;

        LoggerWidget()
        {
            AutoScroll = true;
            Clear();
        }

        void Clear()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();

            // Extract seconds and fractional milliseconds
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

            int old_size = Buf.size();

            // Format the timestamp
            Buf.appendf("[%lld.%03lld]", seconds, milliseconds); // Include milliseconds

            // Handle variable arguments
            va_list args;
            va_start(args, fmt);

            // Use a temporary buffer to ensure stability
            char temp_buf[1024];
            vsnprintf(temp_buf, sizeof(temp_buf), fmt, args);
            Buf.append(temp_buf); // Append the formatted string

            va_end(args);

            // Update line offsets
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n') LineOffsets.push_back(old_size + 1);
        }

        void SetLogLevel(int log_level)
        {
            if (log_level >= 0 && log_level <= 4)
            {
                LogLevel = log_level;
            }
        }

    private:
        void text_formatted(const char* line_start, const char* line_end)
        {
            // Skip timestamp: Find the first ']' to locate the start of the actual message
            const char* log_start = strstr(line_start, "]");
            log_start = (log_start != nullptr) ? log_start + 1 : line_start;

            bool skip_render = false;
            ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (line_end - log_start >= 2)
            {
                if (*(log_start + 1) == 'D')
                {
                    if (LogLevel > 0) skip_render = true;
                    col = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                }
                else if (*(log_start + 1) == 'I')
                {
                    if (LogLevel > 1) skip_render = true;
                    col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                }
                else if (*(log_start + 1) == 'W')
                {
                    if (LogLevel > 2) skip_render = true;
                    col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                }
                else if (*(log_start + 1) == 'E')
                {
                    if (LogLevel > 3) skip_render = true;
                    col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                }
                else if (*(log_start + 1) == 'F')
                {
                    col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                }
            }

            if (skip_render)
            {
                ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight())); // Advance cursor by one line
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted(line_start, line_end);
                ImGui::PopStyleColor();
            }
        }

    public:
        void Draw()
        {
            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options")) ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (clear) Clear();
                if (copy) ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                const char* buf = Buf.begin();
                const char* buf_end = Buf.end();
                if (Filter.IsActive())
                {
                    // In this example we don't use the clipper when Filter is enabled.
                    // This is because we don't have random access to the result of our filter.
                    // A real application processing logs with ten of thousands of entries may want to store the result of
                    // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                    for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        if (Filter.PassFilter(line_start, line_end))
                        {
                            text_formatted(line_start, line_end);
                        }
                    }
                }
                else
                {
                    // The simplest and easy way to display the entire buffer:
                    //   ImGui::TextUnformatted(buf_begin, buf_end);
                    // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                    // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                    // within the visible area.
                    // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                    // on your side is recommended. Using ImGuiListClipper requires
                    // - A) random access into your data
                    // - B) items all being the same height,
                    // both of which we can handle since we have an array pointing to the beginning of each line of text.
                    // When using the filter (in the block of code above) we don't have random access into the data to display
                    // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                    // it possible (and would be recommended if you want to search through tens of thousands of entries).
                    ImGuiListClipper clipper;
                    clipper.Begin(LineOffsets.Size);
                    while (clipper.Step())
                    {
                        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                        {
                            const char* line_start = buf + LineOffsets[line_no];
                            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                            text_formatted(line_start, line_end);
                        }
                    }
                    clipper.End();
                }
                ImGui::PopStyleVar();

                // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
                // Using a scrollbar or mouse-wheel will take away from the bottom edge.
                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }
    };
}  // namespace ImGui

class LogStreamBuffer : public std::streambuf {
public:
    explicit LogStreamBuffer(ImGui::LoggerWidget& logger) : logger(logger) {}

protected:
    virtual int overflow(int c) override {
        if (c == '\n') {
            logger.AddLog("%s\n", buffer.c_str());
            buffer.clear();
        }
        else {
            buffer += static_cast<char>(c);
        }
        return c;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize count) override {
        buffer.append(s, count);
        size_t pos = 0;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            logger.AddLog("%s\n", buffer.substr(0, pos).c_str());
            buffer.erase(0, pos + 1);
        }
        return count;
    }

private:
    std::string buffer;
    ImGui::LoggerWidget& logger;
};


class App;

class LogTab : public AppTab
{
public:
    explicit LogTab(App* app, std::string name);

    void render() override;
    void on_open() override;

private:
    // Data used across windows in this tab
    ImGui::LoggerWidget m_logger_widget;
    int m_log_level = 1;

    static inline const char* log_levels[] = { "Debug", "Info", "Warning", "Error", "Fatal" };
};