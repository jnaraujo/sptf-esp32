#pragma once

#include <Arduino.h>

namespace StringUtils {
  String centerString(const String& text, int totalWidth);

  String formatString(const String& s, int numLines, int maxCharPerLine);

  String wordWrap(String s, int limit);
}