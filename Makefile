# Makefile for X-Plane MFD and Flight Calculators
# Single non-compliant calculator build

CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra
SRC_DIR = calculators

# Calculator names (built in root directory)
TARGETS = wind_calculator flight_calculator turn_calculator vnav_calculator density_altitude_calculator

.PHONY: all clean test run install-fonts jsf-check help status

# Default target: build all calculators
all: build-all

# Internal target to build all calculators from specified directory
build-all: wind_calculator flight_calculator turn_calculator vnav_calculator density_altitude_calculator

wind_calculator:
	@echo "Compiling wind calculator from $(SRC_DIR)..."
	$(CXX) $(CXXFLAGS) -o wind_calculator $(SRC_DIR)/wind_calculator.cpp
	@echo "✓ Wind calculator built!"

flight_calculator:
	@echo "Compiling flight calculator from $(SRC_DIR)..."
	$(CXX) $(CXXFLAGS) -o flight_calculator $(SRC_DIR)/flight_calculator.cpp
	@echo "✓ Flight calculator built!"

turn_calculator:
	@echo "Compiling turn calculator from $(SRC_DIR)..."
	$(CXX) $(CXXFLAGS) -o turn_calculator $(SRC_DIR)/turn_calculator.cpp
	@echo "✓ Turn calculator built!"

vnav_calculator:
	@echo "Compiling VNAV calculator from $(SRC_DIR)..."
	$(CXX) $(CXXFLAGS) -o vnav_calculator $(SRC_DIR)/vnav_calculator.cpp
	@echo "✓ VNAV calculator built!"

density_altitude_calculator:
	@echo "Compiling density altitude calculator from $(SRC_DIR)..."
	$(CXX) $(CXXFLAGS) -o density_altitude_calculator $(SRC_DIR)/density_altitude_calculator.cpp
	@echo "✓ Density altitude calculator built!"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGETS)
	rm -rf __pycache__
	rm -f *.pyc
	@echo "Clean complete!"

test: $(TARGETS)
	@echo "Running calculator tests..."
	@./test_calculators.sh

install-fonts:
	@echo "Installing B612 Mono fonts..."
	@cp fonts/B612Mono-Regular.ttf ~/Library/Fonts/ 2>/dev/null || true
	@cp fonts/B612Mono-Bold.ttf ~/Library/Fonts/ 2>/dev/null || true
	@echo "Fonts installed!"

run: $(TARGETS)
	@echo "Launching X-Plane MFD..."
	@./run_mfd.sh

jsf-check:
	@echo "JSF compliance checks are not applicable for this project."

status:
	@echo "========================================"
	@echo "X-Plane Calculator Build Status"
	@echo "========================================"
	@echo "Source directory: $(SRC_DIR)/"
	@echo ""
	@echo "Built executables:"
	@for calc in $(TARGETS); do \
		if [ -f $$calc ]; then \
			echo "  ✓ $$calc"; \
		else \
			echo "  ✗ $$calc (not built)"; \
		fi \
	done

help:
	@echo "========================================"
	@echo "X-Plane MFD Makefile"
	@echo "========================================"
	@echo ""
	@echo "Build Targets:"
	@echo "  make                    - Build all calculators (default)"
	@echo "  make clean              - Remove build artifacts"
	@echo ""
	@echo "Run Targets:"
	@echo "  make test               - Run calculator tests"
	@echo "  make run                - Build and launch MFD"
	@echo "  make status             - Show current build status"
	@echo ""
	@echo "Utility Targets:"
	@echo "  make install-fonts      - Install B612 Mono fonts"
	@echo "  make jsf-check          - Verify JSF compliance status"
	@echo "  make help               - Show this help"
	@echo ""
	@echo "Built executables (in root directory):"
	@echo "  • flight_calculator          - Comprehensive flight calculations"
	@echo "  • turn_calculator            - Turn performance calculations"
	@echo "  • vnav_calculator            - VNAV helpers (TOD, required VS)"
	@echo "  • density_altitude_calculator - Density altitude & performance"
	@echo "  • wind_calculator            - Wind vector calculations"
	@echo ""
	@echo "Source directories:"
	@echo "  • calculators/          - Calculator source code"
	@echo ""
	@echo "Examples:"
	@echo "  make                      - Build all calculators"
	@echo "  make clean                - Clean build artifacts"
	@echo "========================================"
