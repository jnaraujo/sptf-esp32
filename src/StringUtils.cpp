#include "StringUtils.hpp"


namespace StringUtils {
  String centerString(const String& text, int totalWidth) {
    int textLen = text.length();
    if (textLen >= totalWidth) {
      return text;
    }
    int totalPadding = totalWidth - textLen;
    int leftPadding = totalPadding / 2;
    int rightPadding = totalPadding - leftPadding;
    String paddedString = "";
    for (int i = 0; i < leftPadding; i++) {
      paddedString += " ";
    }
    paddedString += text;
    for (int i = 0; i < rightPadding; i++) {
      paddedString += " ";
    }
    return paddedString;
  }

  String formatString(const String& s, int numLines, int maxCharPerLine) {
    const int totalMaxChars = maxCharPerLine * numLines;
    const String ellipsis = "...";

    if(s.length() <= totalMaxChars) {
      return s;
    }

    String trimmed = s.substring(0, totalMaxChars - ellipsis.length());
    if (trimmed.endsWith(" ")) {
      trimmed.remove(trimmed.length() - 1);
    }
    return trimmed + ellipsis;
  }

  String wordWrap(String s, int limit) {
    int space = 0;
    int i = 0;
    int line = 0;
    while (i < s.length()) {
      if (s.substring(i, i + 1) == " ") {
        space = i;
      }
      if (line > limit - 1) {
        s = s.substring(0, space) + "~" + s.substring(space + 1);
        line = 0;
      }
      i++; line++;
    }
    s.replace("~", "\n");
    return s;
  }
}