#pragma once
#ifndef IMGUIANSICOLOR_HPP
#define IMGUIANSICOLOR_HPP
//https://gist.github.com/ddovod/be210315f285becc6b0e455b775286e1
#include <vector>
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>
#include <sstream>

namespace jet
{
	std::vector<std::string> split(const std::string& str, const std::string& delim);
}

namespace ImGui
{
	static const int kMaxChar = 10000;
	static ImU32 col_buf[kMaxChar];
	static bool char_skip[kMaxChar];

	bool ParseColor(const char* s, ImU32* col, int* skipChars);

	void ImFont_RenderAnsiText(const ImFont* font,
			ImDrawList* draw_list,
			float size,
			ImVec2 pos,
			ImU32 col,
			const ImVec4& clip_rect,
			const char* text_begin,
			const char* text_end,
			float wrap_width = 0.0f,
			bool cpu_fine_clip = false);

	void ImDrawList_AddAnsiText(ImDrawList* drawList,
			const ImFont* font,
			float font_size,
			const ImVec2& pos,
			ImU32 col,
			const char* text_begin,
			const char* text_end = NULL,
			float wrap_width = 0.0f,
			const ImVec4* cpu_fine_clip_rect = NULL);

	void RenderAnsiText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash);

	void RenderAnsiTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width);

	void TextAnsiUnformatted(const char* text, const char* text_end);

	void TextAnsiV(const char* fmt, va_list args);

	void TextAnsi(const char* fmt, ...);

	void TextAnsiColoredV(const ImVec4& col, const char* fmt, va_list args);

	void TextAnsiColored(const ImVec4& col, const char* fmt, ...);
}
#endif //IMGUIANSICOLOR_HPP
