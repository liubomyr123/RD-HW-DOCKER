#pragma once

#define GRAVITATIONAL_ACCELERATION 9.81f
#define SIM_MAX_STEPS 10000

#define CONFIG_JSON_FILE_NAME "data/config.json"
#define AMMO_JSON_FILE_NAME "data/ammo.json"
#define TARGETS_JSON_FILE_NAME "data/targets.json"
#define SIMULATION_JSON_FILE_NAME "data/simulation.json"
#define LOGGER_FILE_NAME "data/app.log"

enum class SolverType   { ANALYTICAL };
enum class ProviderType { JSON };
enum class LoaderType   { FILE };
