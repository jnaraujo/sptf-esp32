#include "StringUtils.hpp"

namespace StringUtils {
std::string centerString(std::string_view text, int totalWidth) {
	if (text.size() >= totalWidth) {
		return std::string(text);
	}

	const size_t totalPadding = totalWidth - text.size();
	const size_t leftPadding = totalPadding / 2;
	const size_t rightPadding = totalPadding - leftPadding;

	std::string paddedString;

	paddedString.reserve(totalWidth);

	paddedString.append(leftPadding, ' ');
	paddedString.append(text);
	paddedString.append(rightPadding, ' ');

	return paddedString;
}

std::string formatString(std::string_view s, int numLines, int maxCharPerLine) {
	const size_t totalMaxChars = maxCharPerLine * numLines;
	const std::string ellipsis = "...";

	if (s.size() <= totalMaxChars) {
		return std::string(s);
	}

	// Calculate safe limit to avoid underflow
	size_t limit = (totalMaxChars > ellipsis.length()) ? (totalMaxChars - ellipsis.length()) : 0;

	std::string trimmed(s.substr(0, limit));

	if (!trimmed.empty() && trimmed.back() == ' ') {
		trimmed.pop_back();
	}

	// Remove trailing space if it exists (Replaces .ends_with and .remove)
	if (!trimmed.empty() && trimmed.back() == ' ') {
		trimmed.pop_back();
	}

	return trimmed + ellipsis;
}

std::string wordWrap(std::string_view s, int limit) {
	std::string str(s);

	int lastSpace = -1;
	int lineLength = 0;

	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == ' ') {
			lastSpace = i;
		}

		if (lineLength >= limit) {
			if (lastSpace != -1) {
				str[lastSpace] = '\n';

				lineLength = i - lastSpace;

				lastSpace = -1;
			}
		}

		lineLength++;
	}

	return str;
}
}  // namespace StringUtils