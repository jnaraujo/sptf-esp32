#include "StringUtils.hpp"

namespace StringUtils {
std::string centerString(const std::string& text, int totalWidth) {
	int textLen = text.length();
	if (textLen >= totalWidth) {
		return text;
	}
	int totalPadding = totalWidth - textLen;
	int leftPadding = totalPadding / 2;
	int rightPadding = totalPadding - leftPadding;

	std::string paddedString = "";
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

std::string formatString(const std::string& s, int numLines, int maxCharPerLine) {
	const size_t totalMaxChars = (size_t)maxCharPerLine * numLines;
	const std::string ellipsis = "...";

	if (s.length() <= totalMaxChars) {
		return s;
	}

	// Calculate safe limit to avoid underflow
	size_t limit = (totalMaxChars > ellipsis.length()) ? (totalMaxChars - ellipsis.length()) : 0;

	std::string trimmed = s.substr(0, limit);

	// Remove trailing space if it exists (Replaces .ends_with and .remove)
	if (!trimmed.empty() && trimmed.back() == ' ') {
		trimmed.pop_back();
	}

	return trimmed + ellipsis;
}

std::string wordWrap(const std::string& s, int limit) {
	std::string str = s;
	int lastSpace = -1;
	int lineCount = 0;

	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == ' ') {
			lastSpace = i;
		}

		if (lineCount >= limit) {
			if (lastSpace != -1) {
				// Replace space with newline at the last found space
				str[lastSpace] = '\n';
				// Reset lineCount to the distance from the new newline to current index
				lineCount = i - lastSpace;
				lastSpace = -1;
			}
		}
		lineCount++;
	}

	return str;
}
}  // namespace StringUtils