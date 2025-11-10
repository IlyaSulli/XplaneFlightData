# Makefile for X-Plane MFD and Flight Calculators

CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra
TARGETS = wind_calculator flight_calculator turn_calculator vnav_calculator density_altitude_calculator

.PHONY: all clean test run install-fonts

all: $(TARGETS)

wind_calculator: wind_calculator.cpp
	@echo "Compiling wind calculator..."
	$(CXX) $(CXXFLAGS) -o wind_calculator wind_calculator.cpp
	@echo "Wind calculator built!"

flight_calculator: flight_calculator.cpp
	@echo "Compiling flight calculator..."
	$(CXX) $(CXXFLAGS) -o flight_calculator flight_calculator.cpp
	@echo "Flight calculator built!"

turn_calculator: turn_calculator.cpp
	@echo "Compiling turn calculator..."
	$(CXX) $(CXXFLAGS) -o turn_calculator turn_calculator.cpp
	@echo "Turn calculator built!"

vnav_calculator: vnav_calculator.cpp
	@echo "Compiling VNAV calculator..."
	$(CXX) $(CXXFLAGS) -o vnav_calculator vnav_calculator.cpp
	@echo "VNAV calculator built!"

density_altitude_calculator: density_altitude_calculator.cpp
	@echo "Compiling density altitude calculator..."
	$(CXX) $(CXXFLAGS) -o density_altitude_calculator density_altitude_calculator.cpp
	@echo "Density altitude calculator built!"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGETS)
	rm -rf __pycache__
	rm -f *.pyc
	@echo "Clean complete!"

test: $(TARGETS)
	@echo "Running calculator tests..."
	@python3 test_wind_calc.py

install-fonts:
	@echo "Installing B612 Mono fonts..."
	@cp fonts/B612Mono-Regular.ttf ~/Library/Fonts/ 2>/dev/null || true
	@cp fonts/B612Mono-Bold.ttf ~/Library/Fonts/ 2>/dev/null || true
	@echo "Fonts installed!"

run: $(TARGETS)
	@echo "Launching X-Plane MFD..."
	@./run_mfd.sh

help:
	@echo "X-Plane MFD Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make               - Build all C++ calculators"
	@echo "  make clean         - Remove build artifacts"
	@echo "  make test          - Run calculator tests"
	@echo "  make run           - Build and launch MFD"
	@echo "  make install-fonts - Install B612 Mono fonts"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Built executables:"
	@echo "  • flight_calculator          - Comprehensive flight calculations (wind, envelope, energy)"
	@echo "  • turn_calculator            - Turn performance (radius, rate, lead distance)"
	@echo "  • vnav_calculator            - VNAV helpers (TOD, required VS)"
	@echo "  • density_altitude_calculator - Density altitude & performance"
	@echo "  • wind_calculator            - Legacy wind calculator"

