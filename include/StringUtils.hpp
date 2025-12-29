#pragma once

#include <string>
#include <string_view>

namespace StringUtils {
std::string centerString(std::string_view text, int totalWidth);

std::string formatString(std::string_view s, int numLines, int maxCharPerLine);

std::string wordWrap(std::string_view s, int limit);
}  // namespace StringUtils