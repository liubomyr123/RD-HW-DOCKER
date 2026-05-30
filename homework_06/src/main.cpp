#include "../include/ballistics.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: <input_path>\n";
    return 1;
  }

  std::string input_file_name = argv[1];
  std::string output_file_name = "output.txt";

  InputData inputData{};
  if (!getInputData(input_file_name, inputData))
  {
    return 1;
  }

  AmmoInfo ammoInfo{};
  if (!getAmmoInfoByType(inputData.ammo_name, ammoInfo))
  {
    return 1;
  }

  double ammoTimeOfFlight = 0.0;
  if (!getAmmoTimeOfFlight(ammoTimeOfFlight, inputData, ammoInfo))
  {
    return 1;
  }

  double horizontalFlightRange = 0.0;
  if (!getHorizontalFlightRange(horizontalFlightRange, inputData, ammoInfo, ammoTimeOfFlight))
  {
    return 1;
  }

  double distanceToTarget = 0.0;
  if (!getDistanceToTarget(distanceToTarget, inputData))
  {
    return 1;
  }

  OutputData outputData{};
  if (!getAmmoDropPoint(outputData, inputData, ammoInfo, distanceToTarget, horizontalFlightRange))
  {
    return 1;
  }

  if (!writeOutputToFile(output_file_name, outputData))
  {
    return 1;
  }

  return 0;
}
