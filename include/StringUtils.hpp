#pragma once

#include <Arduino.h>

#include <string>

namespace StringUtils {
std::string centerString(const std::string& text, int totalWidth);

std::string formatString(const std::string& s, int numLines, int maxCharPerLine);

std::string wordWrap(const std::string& s, int limit);
}  // namespace StringUtils