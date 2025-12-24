DIRS := src include lib

EXTENSIONS := -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h"

CMD := clang-format -i -style=file

all: format

format:
	@echo "🔍 Searching for files in: $(DIRS)..."
	@find $(DIRS) -type f \( $(EXTENSIONS) \) -exec $(CMD) {} +
	@echo "✅ Formatting completed successfully!"

.PHONY: all format