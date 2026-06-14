#pragma once
#include <iostream>

#define LOG_INFO(x) std::cout << x << "\n"              // NOLINT(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
#define LOG_PROCESS(x) std::cout << "🔄 " << x << "\n"  // NOLINT(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
#define LOG_ERROR(x) std::cerr << "❌ " << x << "\n"    // NOLINT(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
#define LOG_WARN(x) std::cout << "⚠️  " << x << "\n"     // NOLINT(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
#define LOG_SUCCESS(x) std::cout << "✅ " << x << "\n"  // NOLINT(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

constexpr double GRAVITATIONAL_ACCELERATION = 9.81F;

struct InputData
{
  double xd;
  double yd;
  double zd;
  double targetX;
  double targetY;
  double attackSpeed;
  double accelerationPath;
  std::string ammo_name;
};

struct OutputData
{
  double fireX;
  double fireY;
  double postManeuverX;
  double postManeuverY;
  bool isRecalculated;
};

/**
 *     | Назва       | m (кг) | d (drag) | l (lift) | Тип               |
 *     ------------------------------------------------------------------
 *     | VOG-17      | 0.35   | 0.07     | 0.0      | Вільне падіння    |
 *     | M67         | 0.6    | 0.10     | 0.0      | Вільне падіння    |
 *     | RKG-3       | 1.2    | 0.10     | 0.0      | Вільне падіння    |
 *     | GLIDING-VOG | 0.45   | 0.10     | 1.0      | Планеруючий       |
 *     | GLIDING-RKG | 1.4    | 0.10     | 1.0      | Планеруючий       |
 *
 *     m — маса боєприпасу (кг)
 *     d — коефіцієнт аеродинамічного опору
 *     l — коефіцієнт підйомної сили (0 = вільне падіння, 1 = планерування).
 */

struct AmmoInfo
{
  double m;
  double d;
  double l;
  bool isFreeFall;
};

bool getInputData(const std::string& file_name, InputData& inputData);

bool getAmmoInfoByType(const std::string& ammo_name, AmmoInfo& outAmmo);

bool getAmmoTimeOfFlight(double& result, const InputData& inputData, const AmmoInfo& outAmmo);

bool getHorizontalFlightRange(double& result, const InputData& inputData, const AmmoInfo& outAmmo, const double& ammoTimeOfFlight);

bool getDistanceToTarget(double& result, const InputData& inputData);

bool isManeuverRequired(const double& h, const double& accelerationPath, const double& D);  // NOLINT(readability-identifier-length)

bool getNewDroneCoordinatesForManeuver(
  double& newX, double& newY, const InputData& inputData, const double& D, const double& h);  // NOLINT(readability-identifier-length)

bool getAmmoDropPoint(OutputData& outputData,
                      const InputData& inputData,
                      const AmmoInfo& outAmmo,
                      const double& D,
                      const double& h);  // NOLINT(readability-identifier-length)

bool writeOutputToFile(const std::string& file_name, const OutputData& outputData);
