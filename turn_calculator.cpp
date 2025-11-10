/**
 * Turn Performance Calculator for X-Plane MFD
 * 
 * Calculates turn performance metrics:
 * - Turn radius
 * - Turn rate (degrees per second)
 * - Lead turn distance for course changes
 * - Standard rate bank angle
 * - Time to turn
 * 
 * Compile: g++ -std=c++20 -O3 -o turn_calculator turn_calculator.cpp
 * 
 * Usage: ./turn_calculator <tas_kts> <bank_deg> <course_change_deg>
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
constexpr double GRAVITY = 9.80665;  // m/s²
constexpr double KTS_TO_MS = 0.514444;  // knots to m/s
constexpr double NM_TO_FT = 6076.12;    // nautical miles to feet
constexpr double STANDARD_RATE = 3.0;   // degrees per second

[[nodiscard]] double parse_double(std::string_view sv) {
    return std::stod(std::string(sv));
}

struct TurnData {
    double radius_nm;           // Turn radius in nautical miles
    double radius_ft;           // Turn radius in feet
    double turn_rate_dps;       // Turn rate in degrees per second
    double lead_distance_nm;    // Lead distance to roll out
    double lead_distance_ft;    // Lead distance in feet
    double time_to_turn_sec;    // Time to complete the turn
    double load_factor;         // G-loading in the turn
    double standard_rate_bank;  // Bank angle for standard rate turn
};

/**
 * Calculate comprehensive turn performance
 * 
 * Formulas:
 * - Turn radius: R = V² / (g * tan φ)
 * - Turn rate: ω = (g * tan φ) / V
 * - Lead distance: L = R * tan(Δψ/2)
 * - Load factor: n = 1 / cos φ
 * - Standard rate bank: φ = atan(ω * V / g) where ω = 3°/s
 */
[[nodiscard]] TurnData calculate_turn_performance(double tas_kts, double bank_deg, double course_change_deg) {
    TurnData result;
    
    // Convert inputs
    double v_ms = tas_kts * KTS_TO_MS;  // TAS in m/s
    double phi_rad = bank_deg * DEG_TO_RAD;  // Bank angle in radians
    double delta_psi_rad = course_change_deg * DEG_TO_RAD;  // Course change in radians
    
    // Calculate load factor
    result.load_factor = 1.0 / std::cos(phi_rad);
    
    // Turn radius: R = V² / (g * tan φ)
    double tan_phi = std::tan(phi_rad);
    if (std::abs(tan_phi) < 0.001) {
        // Essentially wings level - infinite radius
        result.radius_nm = 999.9;
        result.radius_ft = 999900.0;
        result.turn_rate_dps = 0.0;
        result.lead_distance_nm = 0.0;
        result.lead_distance_ft = 0.0;
        result.time_to_turn_sec = 999.9;
    } else {
        double radius_m = (v_ms * v_ms) / (GRAVITY * tan_phi);
        result.radius_ft = radius_m * 3.28084;  // Convert m to ft
        result.radius_nm = result.radius_ft / NM_TO_FT;  // Convert ft to nm
        
        // Turn rate: ω = (g * tan φ) / V (in rad/s)
        double omega_rad_s = (GRAVITY * tan_phi) / v_ms;
        result.turn_rate_dps = omega_rad_s * RAD_TO_DEG;  // Convert to deg/s
        
        // Lead distance: L = R * tan(Δψ/2)
        double lead_m = radius_m * std::tan(delta_psi_rad / 2.0);
        result.lead_distance_ft = lead_m * 3.28084;
        result.lead_distance_nm = result.lead_distance_ft / NM_TO_FT;
        
        // Time to complete turn
        if (result.turn_rate_dps > 0.01) {
            result.time_to_turn_sec = std::abs(course_change_deg) / result.turn_rate_dps;
        } else {
            result.time_to_turn_sec = 999.9;
        }
    }
    
    // Standard rate turn bank angle (3°/s turn rate)
    // φ = atan(ω * V / g) where ω = 3°/s = 0.0524 rad/s
    double omega_std_rad_s = STANDARD_RATE * DEG_TO_RAD;
    double phi_std_rad = std::atan((omega_std_rad_s * v_ms) / GRAVITY);
    result.standard_rate_bank = phi_std_rad * RAD_TO_DEG;
    
    return result;
}

/**
 * Output results as JSON
 */
void print_json(const TurnData& turn) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "{\n";
    std::cout << "  \"radius_nm\": " << turn.radius_nm << ",\n";
    std::cout << "  \"radius_ft\": " << turn.radius_ft << ",\n";
    std::cout << "  \"turn_rate_dps\": " << turn.turn_rate_dps << ",\n";
    std::cout << "  \"lead_distance_nm\": " << turn.lead_distance_nm << ",\n";
    std::cout << "  \"lead_distance_ft\": " << turn.lead_distance_ft << ",\n";
    std::cout << "  \"time_to_turn_sec\": " << turn.time_to_turn_sec << ",\n";
    std::cout << "  \"load_factor\": " << turn.load_factor << ",\n";
    std::cout << "  \"standard_rate_bank\": " << turn.standard_rate_bank << "\n";
    std::cout << "}\n";
}

} // namespace xplane_mfd::calc

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " <tas_kts> <bank_deg> <course_change_deg>\n\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  tas_kts          : True airspeed (knots)\n";
    std::cerr << "  bank_deg         : Bank angle (degrees)\n";
    std::cerr << "  course_change_deg: Course change required (degrees)\n\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << program_name << " 250 25 90\n";
    std::cerr << "  (250 knots TAS, 25° bank, 90° turn)\n";
}

int main(int argc, char* argv[]) {
    using namespace xplane_mfd::calc;
    
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    
    if (args.size() != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        double tas_kts = parse_double(args[0]);
        double bank_deg = parse_double(args[1]);
        double course_change_deg = parse_double(args[2]);
        
        // Validate inputs
        if (tas_kts <= 0) {
            std::cerr << "Error: TAS must be positive\n";
            return 1;
        }
        
        if (std::abs(bank_deg) > 85) {
            std::cerr << "Error: Bank angle must be between -85 and 85 degrees\n";
            return 1;
        }
        
        TurnData turn = calculate_turn_performance(tas_kts, bank_deg, course_change_deg);
        print_json(turn);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        print_usage(argv[0]);
        return 1;
    }
}

