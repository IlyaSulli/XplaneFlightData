/**
 * VNAV Calculator for X-Plane MFD
 * 
 * Calculates vertical navigation parameters:
 * - Top of Descent (TOD) distance
 * - Required vertical speed for path
 * - Idle descent path check
 * - Flight path angle
 * - Time to altitude constraint
 * 
 * Compile: g++ -std=c++20 -O3 -o vnav_calculator vnav_calculator.cpp
 * 
 * Usage: ./vnav_calculator <current_alt_ft> <target_alt_ft> <distance_nm> <groundspeed_kts>
 */

#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <numbers>
#include <vector>
#include <string_view>

namespace xplane_mfd::calc {

constexpr double DEG_TO_RAD = std::numbers::pi / 180.0;
constexpr double RAD_TO_DEG = 180.0 / std::numbers::pi;
constexpr double NM_TO_FT = 6076.12;    // nautical miles to feet
constexpr double THREE_DEG_RAD = 3.0 * DEG_TO_RAD;
constexpr double FIVE_DEG_RAD = 5.0 * DEG_TO_RAD;

[[nodiscard]] double parse_double(std::string_view sv) {
    return std::stod(std::string(sv));
}

struct VNAVData {
    double altitude_to_lose_ft;      // Altitude change required
    double flight_path_angle_deg;    // Flight path angle (negative = descent)
    double required_vs_fpm;          // Required vertical speed
    double tod_distance_nm;          // Top of descent distance (for 3° path)
    double time_to_constraint_min;   // Time to reach altitude at current VS
    double distance_per_1000ft;      // Distance traveled per 1000 ft altitude change
    bool is_descent;                 // True if descending, false if climbing
    bool on_idle_path;               // True if within idle descent capability
};

/**
 * Calculate VNAV parameters
 * 
 * Key formulas:
 * - Flight path angle: γ = atan(Δh / distance)
 * - Required VS: VS_fpm = 101.27 * GS_kts * tan(γ)
 * - TOD for 3°: D_nm = Δh_ft / 319  (simplified: Δh / (6076 * tan(3°)))
 * - TOD for any angle: D_nm = Δh_ft / (6076 * tan(γ))
 * - Standard 3° descent: VS ≈ 5 * GS_kts
 */
[[nodiscard]] VNAVData calculate_vnav(double current_alt_ft, double target_alt_ft, 
                        double distance_nm, double groundspeed_kts) {
    VNAVData result;
    
    // Calculate altitude change (positive = need to climb, negative = need to descend)
    double altitude_change_ft = target_alt_ft - current_alt_ft;
    result.altitude_to_lose_ft = -altitude_change_ft;  // For display (legacy field name)
    result.is_descent = altitude_change_ft < 0;
    
    // Avoid division by zero
    if (distance_nm < 0.01) distance_nm = 0.01;
    if (groundspeed_kts < 1.0) groundspeed_kts = 1.0;
    
    // Calculate flight path angle (positive = climb, negative = descent)
    double distance_ft = distance_nm * NM_TO_FT;
    double gamma_rad = std::atan(altitude_change_ft / distance_ft);
    result.flight_path_angle_deg = gamma_rad * RAD_TO_DEG;
    
    // Required vertical speed to meet constraint at current groundspeed
    // VS = 101.27 * GS * tan(γ)  [where 101.27 = 60 / (6076 * (π/180))]
    // Positive VS = climb, negative VS = descent
    result.required_vs_fpm = 101.27 * groundspeed_kts * std::tan(gamma_rad);
    
    // Calculate TOD for standard 3° descent path
    // For 3°: D = h / (6076 * tan(3°)) ≈ h / 319
    // Only meaningful for descents
    const double THREE_DEG_RAD = 3.0 * DEG_TO_RAD;
    if (result.is_descent) {
        result.tod_distance_nm = std::abs(altitude_change_ft) / 
                                 (NM_TO_FT * std::tan(THREE_DEG_RAD));
    } else {
        result.tod_distance_nm = 0.0;  // TOD not applicable for climbs
    }
    
    // Time to constraint at current groundspeed
    result.time_to_constraint_min = (distance_nm / groundspeed_kts) * 60.0;
    
    // Distance traveled per 1000 ft altitude change
    if (std::abs(altitude_change_ft) > 10.0) {
        result.distance_per_1000ft = (distance_nm * 1000.0) / std::abs(altitude_change_ft);
    } else {
        result.distance_per_1000ft = 999.9;
    }
    
    // Check if on idle descent path (typical idle descent is 2.5° to 3.5°)
    // For safety and passenger comfort, we use 2° to 4° as acceptable range
    if (result.is_descent) {
        double abs_fpa = std::abs(result.flight_path_angle_deg);
        result.on_idle_path = (abs_fpa >= 2.0 && abs_fpa <= 4.0);
    } else {
        // For climbs, any reasonable positive angle is acceptable
        result.on_idle_path = (result.flight_path_angle_deg >= 0.5 && 
                               result.flight_path_angle_deg <= 15.0);
    }
    
    return result;
}

/**
 * Calculate additional useful metrics
 */
struct VNAVHelpers {
    double vs_for_3deg;      // VS needed for 3° path at current GS
    double vs_for_5deg;      // VS needed for steeper 5° emergency descent
    double distance_remaining_at_current_vs;  // If maintaining current VS
};

[[nodiscard]] VNAVHelpers calculate_vnav_helpers(double groundspeed_kts, double current_vs_fpm, 
                                   double altitude_change_ft) {
    VNAVHelpers result;
    
    // VS for 3° descent: VS ≈ -5.31 * GS (derived from 101.27 * tan(3°))
    result.vs_for_3deg = -101.27 * groundspeed_kts * std::tan(THREE_DEG_RAD);
    
    // VS for 5° descent (emergency/high drag)
    result.vs_for_5deg = -101.27 * groundspeed_kts * std::tan(FIVE_DEG_RAD);
    
    // Distance if maintaining current VS
    if (std::abs(current_vs_fpm) > 10.0 && groundspeed_kts > 1.0) {
        // How long to achieve the altitude change at current VS
        double time_min = altitude_change_ft / current_vs_fpm;  // Can be negative
        result.distance_remaining_at_current_vs = (time_min * groundspeed_kts) / 60.0;
        
        // If negative time (wrong direction), set to a large value
        if (result.distance_remaining_at_current_vs < 0) {
            result.distance_remaining_at_current_vs = 999.9;
        }
    } else {
        result.distance_remaining_at_current_vs = 999.9;
    }
    
    return result;
}

/**
 * Output results as JSON
 */
void print_json(const VNAVData& vnav, const VNAVHelpers& helpers) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "{\n";
    std::cout << "  \"altitude_to_lose_ft\": " << vnav.altitude_to_lose_ft << ",\n";
    std::cout << "  \"flight_path_angle_deg\": " << vnav.flight_path_angle_deg << ",\n";
    std::cout << "  \"required_vs_fpm\": " << vnav.required_vs_fpm << ",\n";
    std::cout << "  \"tod_distance_nm\": " << vnav.tod_distance_nm << ",\n";
    std::cout << "  \"time_to_constraint_min\": " << vnav.time_to_constraint_min << ",\n";
    std::cout << "  \"distance_per_1000ft\": " << vnav.distance_per_1000ft << ",\n";
    std::cout << "  \"is_descent\": " << (vnav.is_descent ? "true" : "false") << ",\n";
    std::cout << "  \"on_idle_path\": " << (vnav.on_idle_path ? "true" : "false") << ",\n";
    std::cout << "  \"vs_for_3deg\": " << helpers.vs_for_3deg << ",\n";
    std::cout << "  \"vs_for_5deg\": " << helpers.vs_for_5deg << ",\n";
    std::cout << "  \"distance_at_current_vs_nm\": " << helpers.distance_remaining_at_current_vs << "\n";
    std::cout << "}\n";
}

} // namespace xplane_mfd::calc

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name 
              << " <current_alt_ft> <target_alt_ft> <distance_nm> <groundspeed_kts> [current_vs_fpm]\n\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  current_alt_ft  : Current altitude (feet)\n";
    std::cerr << "  target_alt_ft   : Target altitude at constraint (feet)\n";
    std::cerr << "  distance_nm     : Distance to constraint (nautical miles)\n";
    std::cerr << "  groundspeed_kts : Current groundspeed (knots)\n";
    std::cerr << "  current_vs_fpm  : Current vertical speed (optional, ft/min)\n\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << program_name << " 35000 10000 100 450 -1500\n";
    std::cerr << "  (Descend from FL350 to 10000 ft, 100 nm away, GS 450 kts, VS -1500 fpm)\n";
}

int main(int argc, char* argv[]) {
    using namespace xplane_mfd::calc;
    
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    
    if (args.size() < 4 || args.size() > 5) {
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        double current_alt_ft = parse_double(args[0]);
        double target_alt_ft = parse_double(args[1]);
        double distance_nm = parse_double(args[2]);
        double groundspeed_kts = parse_double(args[3]);
        double current_vs_fpm = (args.size() == 5) ? parse_double(args[4]) : 0.0;
        
        // Validate inputs
        if (distance_nm < 0) {
            std::cerr << "Error: Distance cannot be negative\n";
            return 1;
        }
        
        if (groundspeed_kts <= 0) {
            std::cerr << "Error: Groundspeed must be positive\n";
            return 1;
        }
        
        VNAVData vnav = calculate_vnav(current_alt_ft, target_alt_ft, distance_nm, groundspeed_kts);
        
        // Calculate altitude change for helpers (target - current)
        double altitude_change_ft = target_alt_ft - current_alt_ft;
        VNAVHelpers helpers = calculate_vnav_helpers(groundspeed_kts, current_vs_fpm, 
                                                     altitude_change_ft);
        
        print_json(vnav, helpers);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        print_usage(argv[0]);
        return 1;
    }
}

