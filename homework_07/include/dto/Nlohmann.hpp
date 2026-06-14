#pragma once

#include "third_party/json.hpp"
#include "dto/Coord.hpp"

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, 
    x, 
    y)

namespace config {
    struct Drone_Data {
        Coord position;
        float altitude;
        float initialDirection;
        float attackSpeed;
        float accelerationPath;
        float angularSpeed;
        float turnThreshold;
    };

    struct Simulation_Data {
        float timeStep;
        float hitRadius;
    };

    struct Data {
        Drone_Data drone;
        std::string ammo;
        Simulation_Data simulation;
        int targetArrayTimeStep;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Drone_Data,
        position,
        altitude,
        initialDirection,
        attackSpeed,
        accelerationPath,
        angularSpeed,
        turnThreshold)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Simulation_Data,
        timeStep,
        hitRadius)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data,
        drone,
        ammo,
        simulation,
        targetArrayTimeStep)
}

namespace simulation {
    /**
     *     | Назва              | Опис  |
     *     ------------------------------
     *     | dropPoint          | Точка скиду (firePoint) — куди летить дрон, щоб скинути бомбу. Розраховується з lead targeting. |
     *     | aimPoint           | Точка влучання бомби = позиція дрона + hDist у напрямку direction. Куди впаде бомба, якщо скинути просто зараз. |
     *     | predictedTarget    | Прогнозована позиція цілі через flightTime. Ідеальна точка — коли aimPoint == predictedTarget. |
     */
    struct Step_Data {
        Coord position;          	
        float direction;    	
        int   state;        	
        int   targetIdx;    	
        Coord dropPoint;    	
        Coord aimPoint;     	
        Coord predictedTarget;  
    };

    struct Data {
        int totalSteps;
        std::vector<Step_Data> steps;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Step_Data,
        position,
        direction,
        state,
        targetIdx,
        dropPoint,
        aimPoint,
        predictedTarget)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data,
        totalSteps,
        steps)
}

namespace targets {
    struct Target_Item_Data {
        std::vector<Coord> positions;
    };

    struct Data {
        int targetCount;
        int timeSteps;
        std::vector<Target_Item_Data> targets;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Target_Item_Data, 
        positions)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, 
        targetCount,
        timeSteps,
        targets)
}

namespace ammo {
    struct Data {
        std::string name;
        float mass;
        float drag;
        float lift;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, 
        name, 
        mass, 
        drag, 
        lift)
}
