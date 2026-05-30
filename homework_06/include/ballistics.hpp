#pragma once
#include <iostream>

#define LOG_INFO(x)     std::cout << x << "\n"
#define LOG_PROCESS(x)  std::cout << "🔄 " << x << "\n"
#define LOG_ERROR(x)    std::cerr << "❌ " << x << "\n"
#define LOG_WARN(x)     std::cout << "⚠️  " << x << "\n"
#define LOG_SUCCESS(x)  std::cout << "✅ " << x << "\n"

#define GRAVITATIONAL_ACCELERATION 9.81f

struct InputData {
    float xd; 
    float yd; 
    float zd;
    float targetX; 
    float targetY;
    float attackSpeed;
    float accelerationPath;
    std::string ammo_name;
};

struct OutputData {
    float fireX;
    float fireY;
    float postManeuverX;
    float postManeuverY;
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

struct AmmoInfo {
    float m;
    float d;
    float l;
    bool isFreeFall;
};

bool getInputData(const std::string& file_name, InputData& inputData);

bool getAmmoInfoByType(const std::string ammo_name, AmmoInfo& outAmmo);

bool getAmmoTimeOfFlight(float& result, const InputData& inputData, const AmmoInfo& outAmmo);

bool getHorizontalFlightRange(float& result, const InputData& inputData, const AmmoInfo& outAmmo, const float& ammoTimeOfFlight);

bool getDistanceToTarget(float& result, const InputData& inputData);

bool isManeuverRequired(const float& h, const float& accelerationPath, const float& D);

bool getNewDroneCoordinatesForManeuver(float& newX, float& newY, const InputData& inputData, const float& D, const float& h);

bool getAmmoDropPoint(OutputData& outputData, const InputData& inputData, const AmmoInfo& outAmmo, const float& D, const float& h);

bool writeOutputToFile(const std::string& file_name, const OutputData& outputData);



 