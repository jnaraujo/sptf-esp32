#include "StringUtils.hpp"

namespace StringUtils {
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto centerString(const String& text, int totalWidth) -> String {
	int textLen = text.length();
	if (textLen >= totalWidth) {
		return text;
	}
	int totalPadding = totalWidth - textLen;
	int leftPadding = totalPadding / 2;
	int rightPadding = totalPadding - leftPadding;

	String paddedString = "";
	paddedString.reserve(totalWidth);

	for (int i = 0; i < leftPadding; i++) {
		paddedString += " ";
	}
	paddedString += text;
	for (int i = 0; i < rightPadding; i++) {
		paddedString += ' ';
	}
	return paddedString;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto formatString(const String& s, int numLines, int maxCharPerLine) -> String {
	const int totalMaxChars = maxCharPerLine * numLines;
	const String ellipsis = "...";

	if (s.length() <= totalMaxChars) {
		return s;
	}

	String trimmed = s.substring(0, totalMaxChars - ellipsis.length());
	if (trimmed.endsWith(" ")) {
		trimmed.remove(trimmed.length() - 1);
	}
	return trimmed + ellipsis;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto wordWrap(const String& s, int limit) -> String {
	String str = s;

	int space = 0;
	int index = 0;
	int line = 0;
	while (index < str.length()) {
		if (str.substring(index, index + 1) == " ") {
			space = index;
		}
		if (line > limit - 1) {
			str = str.substring(0, space) + "~" + str.substring(space + 1);
			line = 0;
		}
		index++;
		line++;
	}
	str.replace("~", "\n");
	return str;
}
}  // namespace StringUtils