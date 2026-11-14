# Convenience front-end for the CMake presets. Run `make help`.
#
# The build itself lives in CMakeLists.txt; nothing here is required. These
# targets only save typing and document the everyday commands in one place.

PRESET  ?= dev
BUILD    = build/$(PRESET)
JOBS    ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
SOURCES  = $(shell git ls-files '*.c' '*.h')

.DEFAULT_GOAL := help
.PHONY: help configure build test run play simulate format format-check lint asan coverage \
        report docs package clean

help: ## Show this help
	@awk 'BEGIN {FS = ":.*## "} /^[a-z-]+:.*## / {printf "  \033[36m%-14s\033[0m %s\n", $$1, $$2}' \
		$(MAKEFILE_LIST)
	@echo
	@echo "  Variables: PRESET=dev|release|asan|coverage|headless (default: dev)"

configure: ## Configure the build tree
	cmake --preset $(PRESET)

build: configure ## Compile everything
	cmake --build --preset $(PRESET) -j $(JOBS)

test: build ## Run the test-suite
	ctest --preset $(PRESET) -j $(JOBS)

run: build ## Launch the GTK game
	$(BUILD)/src/ui/battleship

play: build ## Play in the terminal
	$(BUILD)/src/cli/battleship-cli play

simulate: ## Benchmark the three AI levels (optimised build)
	$(MAKE) build PRESET=release
	build/release/src/cli/battleship-cli simulate --games 10000 --seed 1

format: ## Reformat the sources in place
	clang-format -i $(SOURCES)

format-check: ## Fail if a source file is not formatted
	clang-format --dry-run --Werror $(SOURCES)

lint: build ## Static analysis with clang-tidy and cppcheck
	clang-tidy -p $(BUILD) --quiet $(shell git ls-files 'src/*.c')
	cppcheck --std=c11 --enable=warning,performance,portability --inline-suppr \
		--error-exitcode=1 --quiet -I include -I $(BUILD)/generated src/core src/cli

asan: ## Build and test under AddressSanitizer and UBSan
	$(MAKE) test PRESET=asan

coverage: ## Test coverage of the engine (needs gcovr)
	$(MAKE) test PRESET=coverage
	gcovr --root . --filter src/core --filter include build/coverage --print-summary \
		--fail-under-line 95

report: ## Build the LaTeX report (docs/report/build/battleship-report.pdf)
	cd docs/report && latexmk

docs: ## Generate the API reference with Doxygen (docs/api/)
	cmake --preset $(PRESET) -DBATTLESHIP_BUILD_DOCS=ON
	cmake --build --preset $(PRESET) --target docs

package: ## Build the release archives (.tar.gz, and .deb on Linux)
	$(MAKE) build PRESET=release
	cd build/release && cpack

clean: ## Remove every build product
	rm -rf build docs/report/build docs/api
