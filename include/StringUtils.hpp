#pragma once

#include <Arduino.h>

namespace StringUtils {
auto centerString(const String& text, int totalWidth) -> String;

auto formatString(const String& s, int numLines, int maxCharPerLine) -> String;

auto wordWrap(String s, int limit) -> String;
}  // namespace StringUtils