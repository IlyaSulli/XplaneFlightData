/**
 * Wind Calculator for X-Plane MFD
 * 
 * Calculates headwind, crosswind, and wind correction angle
 * from aircraft position and wind data.
 * 
 * Compile: g++ -std=c++20 -O3 -o wind_calculator wind_calculator.cpp
 * 
 * Usage: ./wind_calculator <track> <heading> <wind_dir> <wind_speed>
 *   track:      Ground track (degrees true)
 *   heading:    Aircraft heading (degrees magnetic/true)
 *   wind_dir:   Wind direction (degrees, direction wind is FROM)
 *   wind_speed: Wind speed (knots)
 * 
 * Output: JSON format
 * {
 *   "headwind": <float>,      // positive = headwind, negative = tailwind (knots)
 *   "crosswind": <float>,     // positive = right, negative = left (knots)
 *   "total_wind": <float>,    // total wind speed (knots)
 *   "wca": <float>,           // wind correction angle (degrees)
 *   "drift": <float>          // drift angle (degrees)
 * }
 */

#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <numbers>
#include <vector>
#include <string_view>

namespace xplane_mfd::calc {

constexpr double DEG_TO_RAD = std::numbers::pi / 180.0;

[[nodiscard]] double parse_double(std::string_view sv) {
    return std::stod(std::string(sv));
}

struct WindComponents {
    double headwind;      // Positive = headwind, negative = tailwind
    double crosswind;     // Positive = from right, negative = from left
    double total_wind;    // Total wind speed
    double wca;          // Wind correction angle
    double drift;        // Drift angle (track - heading)
};

/**
 * Normalize angle to 0-360 range
 */
double normalize_angle(double angle) {
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

/**
 * Calculate wind components relative to aircraft track
 */
[[nodiscard]] WindComponents calculate_wind(double track, double heading, double wind_dir, double wind_speed) {
    WindComponents result;
    
    // Normalize all angles
    track = normalize_angle(track);
    heading = normalize_angle(heading);
    wind_dir = normalize_angle(wind_dir);
    
    // Calculate drift angle
    result.drift = normalize_angle(track - heading);
    if (result.drift > 180.0) result.drift -= 360.0;
    
    // Wind direction is where wind comes FROM
    // Calculate angle of wind-from relative to track
    double wind_from_relative = normalize_angle(wind_dir - track);
    if (wind_from_relative > 180.0) wind_from_relative -= 360.0;
    
    // Convert to radians for trig
    double wind_from_rad = wind_from_relative * DEG_TO_RAD;
    
    // Calculate components using wind-from angle
    // Headwind: positive when wind opposes motion (headwind), negative when assisting (tailwind)
    // When wind-from = 180° (wind from directly ahead), that's a headwind → cos(180°) = -1 → negate to get positive
    // When wind-from = 0° (wind from directly behind), that's a tailwind → cos(0°) = 1 → negate to get negative
    result.headwind = -wind_speed * std::cos(wind_from_rad);
    
    // Crosswind: positive when wind is from the right, negative when from left
    // When wind-from = 90° (wind from the right), crosswind from right → sin(90°) = 1 → positive ✓
    // When wind-from = -90° (wind from the left), crosswind from left → sin(-90°) = -1 → negative ✓
    result.crosswind = wind_speed * std::sin(wind_from_rad);
    
    result.total_wind = wind_speed;
    
    // Wind correction angle: WCA = asin(crosswind / TAS)
    // Since we don't have TAS, we cannot calculate the true WCA
    // Setting to 0 as a placeholder - this value should not be used
    result.wca = 0.0;  // Cannot calculate without TAS
    
    return result;
}

/**
 * Output results as JSON
 */
void print_json(const WindComponents& wind) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "{\n";
    std::cout << "  \"headwind\": " << wind.headwind << ",\n";
    std::cout << "  \"crosswind\": " << wind.crosswind << ",\n";
    std::cout << "  \"total_wind\": " << wind.total_wind << ",\n";
    std::cout << "  \"wca\": " << wind.wca << ",\n";
    std::cout << "  \"drift\": " << wind.drift << "\n";
    std::cout << "}\n";
}

} // namespace xplane_mfd::calc

/**
 * Print usage information
 */
void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name 
              << " <track> <heading> <wind_dir> <wind_speed>\n\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  track      : Ground track (degrees true)\n";
    std::cerr << "  heading    : Aircraft heading (degrees)\n";
    std::cerr << "  wind_dir   : Wind direction FROM (degrees)\n";
    std::cerr << "  wind_speed : Wind speed (knots)\n\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << program_name << " 90 85 270 15\n";
    std::cerr << "  (Track 90°, Heading 85°, Wind from 270° at 15 knots)\n";
}

int main(int argc, char* argv[]) {
    using namespace xplane_mfd::calc;
    
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    
    if (args.size() != 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        // Parse arguments
        double track = parse_double(args[0]);
        double heading = parse_double(args[1]);
        double wind_dir = parse_double(args[2]);
        double wind_speed = parse_double(args[3]);
        
        // Validate inputs
        if (wind_speed < 0) {
            std::cerr << "Error: Wind speed cannot be negative\n";
            return 1;
        }
        
        // Calculate wind components
        WindComponents wind = calculate_wind(track, heading, wind_dir, wind_speed);
        
        // Output JSON
        print_json(wind);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        print_usage(argv[0]);
        return 1;
    }
}

