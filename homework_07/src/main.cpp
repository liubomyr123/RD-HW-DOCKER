#include "../include/logger.hpp"
#include "../include/ballistics.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#define CONFIG_JSON_FILE_NAME "data/config.json"
#define AMMO_JSON_FILE_NAME "data/ammo.json"
#define TARGETS_JSON_FILE_NAME "data/targets.json"
#define SIMULATION_JSON_FILE_NAME "data/simulation.json"

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Simulation sim;
    sim.ammoParam = new AmmoParams();
    sim.outputData.steps = new SimStep*[SIM_MAX_STEPS+1]();
    sim.droneConfig = new DroneConfig();

    if (!getJSONDroneConfig(sim.droneConfig, CONFIG_JSON_FILE_NAME))
    {
        return 1;
    }

    if (!getTargetsJSONData(sim.targetsData, TARGETS_JSON_FILE_NAME))
    {
        return 1;
    }

    if (!getJSONAmmoParamByType(sim.droneConfig->ammoName, sim.ammoParam, AMMO_JSON_FILE_NAME)) 
    {
        return 1;
    }

    float ammoTimeOfFlight = 0.0f;
    if (!getAmmoTimeOfFlight(ammoTimeOfFlight, sim.droneConfig, sim.ammoParam)) 
    {
        return 1;
    }

    float horizontalFlightRange = 0.0f;
    if (!getHorizontalFlightRange(horizontalFlightRange, sim.droneConfig, sim.ammoParam, ammoTimeOfFlight)) 
    {
        return 1;
    }

    float acceleration = 0.0f;
    if (!getAcceleration(acceleration, sim.droneConfig)) 
    {
        return 1;
    }

    sim.state.dronePosition = sim.droneConfig->startPos;
    sim.state.droneZ = sim.droneConfig->altitude;

    sim.state.droneDir = sim.droneConfig->initialDir;
    sim.state.dropPointDir = sim.droneConfig->initialDir;
    sim.state.droneState = STOPPED;

    const size_t targets_number = sim.targetsData.targetCount;

    int count = 0;
    while (count <= SIM_MAX_STEPS)
    {
        LOG_DEBUG("------ FRAME " << count << " ------");
        LOG_INFO("Total simulation time elapsed: " << sim.state.totalSimTime << "[s]");

        // Знайшли координати цілей на даний момент
        std::vector<Coord> interpolatedPosition(targets_number);
        for (size_t i = 0; i < targets_number; i++)
        {
            if (!interpolate(interpolatedPosition[i], i, sim.state.totalSimTime, sim.droneConfig, sim.targetsData))
            {
                return 1;
            }
        }

        // Шукаємо найкращу ціль на даний момент
        // Ітераційно пройтися та:
        // 1) розрахувати для кожної цілі точку скиду
        // 2) розрахувати час який треба дрону щоб долетіти до точки скиду + час польоту снаряду
        // 3) розрахувати саму позицію цілі
        // 4) вибрати ту ціль до якої треба найменше часу летіти
        float smallestTime = std::numeric_limits<float>::max();
        int bestTargetIndex = -1;
        Coord bestDropPoint{};
        Coord bestTarget{};
        for (size_t targetIterator = 0; targetIterator < targets_number; targetIterator++) 
        {
            LOG_DEBUG("-----------: target # " << targetIterator);

            Coord initialTarget{
                interpolatedPosition[targetIterator].x, 
                interpolatedPosition[targetIterator].y
            };

            // Нехай перше припущення цілі - там де ціль зараз стоїть
            Coord predictedTarget = initialTarget;
            Coord predictedDropPoint = initialTarget;

            for (size_t j = 0; j < 6; j++)
            {
                LOG_DEBUG("-----------: prediction # " << j);

                // ✅ 1) Треба знайти точку скиду 
                // ✅ 2) Скільки часу дрон буде до неї летіти 
                // ✅ 3) Час польоту снаряду ми уже маємо - ammoTimeOfFlight 
                // ✅ 4) І знаючи час долітання до точки скиду та політ снаряду можемо знайти 
                //                      де буде ціль від поточного моменту через цей час
                // ✅ 5) Оновити прогнозоване місце цілі і повторити ітерацію
                // ✅ 6) В кінці ми отримаємо для цієї цілі нове місце цілі та точку скиду

                float distanceToTarget = 0.0f;
                if (!getDistanceToTarget(distanceToTarget, 
                    sim.state.dronePosition, 
                    predictedTarget)) 
                {
                    return 1;
                }

                Coord ammoDropPoint{};
                if (!getAmmoDropPoint(ammoDropPoint,
                    sim.state.dronePosition, 
                    distanceToTarget, 
                    horizontalFlightRange, 
                    predictedTarget)) 
                {
                    return 1;
                }
                predictedDropPoint = ammoDropPoint;

                float distanceToDropPoint = 0.0f;
                if (!getDistanceToTarget(distanceToDropPoint, 
                    sim.state.dronePosition, 
                    ammoDropPoint)) 
                {
                    return 1;
                }

                float timeToDropPoint = 0.0f;
                if (!getTimeToTarget(timeToDropPoint, 
                    distanceToDropPoint, 
                    sim.droneConfig->attackSpeed)) 
                {
                    return 1;
                }

                float totalTime = timeToDropPoint + ammoTimeOfFlight;

                Coord targetVelocity{};
                if (!getTargetVelocity(targetVelocity, 
                    targetIterator, 
                    sim.state, 
                    sim.droneConfig, 
                    sim.targetsData))
                {
                    return 1;
                }
                Coord predictedPosition{};
                if (!getPredictedPosition(predictedPosition, 
                    targetVelocity, 
                    initialTarget,
                    totalTime)) 
                {
                    return 1;
                }

                predictedTarget = predictedPosition;
            }
        
            // Далі треба знайти timeToStop, тобто додатковий час необхідний якщо ми хочемо змінити ціль
            // Будемо рахувати штрафний час до зупинки
            float timeToStop = 0.0f;
            switch (sim.state.droneState)
            {
                case STOPPED:
                    // Ми уже стоїмо і нам не треба додаткового часу
                    break;
                case ACCELERATING:
                    // Швидкість дрона >=0, тому нам треба врахувати скільки часу
                    // дрон уже розганяється, як штрафний час необхідний щоб зупинитися
                    // 
                    // Наприклад:
                    //      - поточна швидкість = 8 м/с
                    //      - прискорення - 5 м/с
                    // 
                    // Тоді час який дрон уже затратив на досягнення швидкості 8 м/с буде:
                    // 5 м/с - 1 с
                    // 8 м/с - х с
                    // х = (8 м/с) / (5 м/с) = 1.6 c
                    timeToStop = sim.state.droneVelocity / acceleration;
                    break;
                case MOVING:
                    // Дрон уже рухається зі стабільною макс швидкістю, отже треба врахувати
                    // час на гальмування = attackSpeed / acceleration, ми беремо 
                    // attackSpeed бо ми уже на цій стабільній швидкості
                    timeToStop = sim.droneConfig->attackSpeed / acceleration;
                    break;
                case TURNING:
                    // Дрон зараз розвертається, і він все ще в процесі
                    // Не важливо чи він почав, чи ще в процесі, залишається ще певний кут 
                    // який йому залишилося розвернутися до попередньої активної цілі
                    // Отже, нам треба знати скільки по часу ще залишилося йому розвертатися і зупинитися
                    // 
                    // Наприклад, нехай залишилося ще розвернути 1.2 радіани
                    // І маємо максимальну швидкість повороту - angularSpeed рад/с, нехай 1 рад/с
                    // Тоді час необхідний щоб довернути решту 1.2 радіанів буде:
                    // 1 рад    - 1 с
                    // 1.2 рад  - х с
                    // х = (1.2 рад) / (1 рад) = 1.2 с
                    timeToStop = sim.state.angleTurnLeft / sim.droneConfig->angularSpeed;
                    break;
                case DECELERATING:
                    // Час необхідний щоб повністю зупинитися, тобто скільки ще часу він буде тормозити
                    // Ми маємо його поточну швидкість, яка падає
                    // І можемо знайти скільки ще часу він буде сповільнюватися до 0
                    // За умовою, швидкість прискорення та гальмування є однаковим
                    // Тому робимо так само як і в ACCELERATING
                    timeToStop = sim.state.droneVelocity / acceleration;
                    break;
                default:
                    break;
            }

            float distanceToDropPoint = 0.0f;
            if (!getDistanceToTarget(distanceToDropPoint, 
                sim.state.dronePosition, 
                predictedDropPoint)) 
            {
                return 1;
            }

            float timeToDropPoint = 0.0f;
            if (!getTimeToTarget(timeToDropPoint, 
                distanceToDropPoint, 
                sim.droneConfig->attackSpeed)) 
            {
                return 1;
            }

            float totalTime = 0.0f;

            float distanceToTarget = 0.0f;
            if (!getDistanceToTarget(distanceToTarget, 
                sim.state.dronePosition, 
                predictedTarget)) 
            {
                return 1;
            }

            if (distanceToTarget >= horizontalFlightRange)
            {
                // Все добре, дрон є на достатній відстані від цілі щоб попасти снарядом
                totalTime = timeToDropPoint + ammoTimeOfFlight;
            }
            else
            {
                // Дрон перетнув межу horizontalFlightRange, треба врахувати час на маневр
                // Тому треба знайти скільки часу займе дрону на маневр: 
                //      - тобто пролетіти якусь додаткову дистанцію щоб мати можливість скинути
                //      - і можливо треба буде розвернутися до цілі, беремо найгірший випадок - розворот на 180 градусів
                // 
                // Нехай відстань від дрона до цілі - 5 метрів, а дистанція на скид - 7 метрів
                // Це означає, що дрон уже як 2 метри пропустив точку скиду
                // Тому йому треба відлетіти на ще на 5 метрів і ще як мінімум на 7 метрів, потім ще й розвернутися
                // 
                // 1 радіан = (180 градусів / M_PI)
                // 
                // Нехай у нас angularSpeed = 3 рад/с, тобто за 1 секунду ми повертаємось на 3 радіани або 3 * (180 градусів / M_PI)
                // Тоді щоб знайти час скільки дрон буде розвертатися на 180 градусів:
                // 
                // 1 сек - 3 * (180 градусів / M_PI)
                // х сек - 180 градусів
                // 
                // x сек = (180 градусів * 1 сек) / (3 * (180 градусів / M_PI))
                // x сек = ((180 градусів * 1 сек) / 1) * (M_PI / (3 * 180 градусів)) | скорочуємо на 180 градусів
                // x сек = ((1 сек) / 1) * (M_PI / (3))
                // x сек = M_PI / 3
                // Тобто час розвороту на 180 градусів - це M_PI / droneConfig->angularSpeed
                float timeOfFullTurn = (float)M_PI / sim.droneConfig->angularSpeed;
                
                float restoreDistance = distanceToTarget + horizontalFlightRange;

                float restoreTime = 0.0f;
                if (!getTimeToTarget(restoreTime, restoreDistance, sim.droneConfig->attackSpeed)) 
                {
                    return 1;
                }

                totalTime = restoreTime + timeOfFullTurn + ammoTimeOfFlight;
            }

            // Якщо це якась інша ціль, не та до якої ми зараз летимо, будемо враховувати штраф на зміну цілі
            if (sim.state.currentTargetIndex != -1 && static_cast<int>(targetIterator) != sim.state.currentTargetIndex)
            {
                totalTime += timeToStop;
            }

            if (totalTime < smallestTime)
            {
                smallestTime = totalTime;
                bestTargetIndex = targetIterator;
                bestTarget = predictedTarget;
                bestDropPoint = predictedDropPoint;
            }
        }
        LOG_DEBUG("Best target -> " << bestTargetIndex);
        sim.state.currentTargetIndex = bestTargetIndex;

        LOG_DEBUG("bestTargetX -> " << bestTarget.x);
        LOG_DEBUG("bestTargetY -> " << bestTarget.y);

        float bestDistanceToTarget = 0.0f;
        if (!getDistanceToTarget(bestDistanceToTarget, 
            sim.state.dronePosition, 
            bestTarget)) 
        {
            return 1;
        }

        if (bestDistanceToTarget >= horizontalFlightRange)
        {
            // Все добре, дрон є на достатній відстані від цілі щоб попасти снарядом
            // Точка скиду залишається без змін
        }
        else
        {
            // Дрон перетнув межу horizontalFlightRange, треба знову врахувати час на маневр
            // Треба знайти нову точку скиду, треба знайти точку на відстані horizontalFlightRange від цілі
            // Тобто у мене є координати цілі та дрона і відстань horizontalFlightRange
            // 
            // 1) Знайти вектор від дрона до цілі
            // 2) Знаходимо одиничний вектор
            // 3) Рухаємося на певну відстань по цьому 1-ому вектору
            // 
            // Наприклад,
            //      - дрон (x1 = 1, y1 = 2) 
            //      - ціль (x2 = 5, y2 = 4) 
            //      - треба відступити від цілі на 3 метри
            // 
            // 1) Знадемо векток від дрона до цілі:
            // Вектор рахуємо (куди - звідки):
            //      drone -> target => (targetX - droneX; targetY - droneY)
            //      target -> drone => (droneX - targetX; droneY - targetY)
            // 
            //      (x2 - x1; y2 - y1)
            //      (5-1; 4-2) = (4; 2)
            // 
            // 2) Знадемо одиничний вектор:
            //      а) Знайти довжину ветора: √((x2 - x1)² + (y2 - y1)²)
            //          √(4² + 2²) = √(16 + 4) = √(20) ~ 4.47
            //      б) Поділити координати на довжину:
            //          (4 / 4.47; 2 / 4.47) = (0.89; 0.44) = (nx; ny)
            // 3) Рухаємося на 3 метри від цілі:
            //          (x2 - nx * 3; y2 - ny * 3)
            //          (5 - 0.89 * 3; 4 - 0.44 * 3)
            //          (5 - 2.67; 4 - 1.32)
            //          (2.33; 2.68)
            // Отже, (2.33; 2.68) - це координати точки на відстані в 3 метри від цілі в напрямку дрона

            Coord direction = bestTarget - sim.state.dronePosition;
            Coord offset = (direction / direction.getVectorLength()) * horizontalFlightRange;
            bestDropPoint = bestTarget - offset;
        }

        // Тепер треба знайти чи треба нам скидутаи снаряд прямо зараз, щоб виконати місію
        // Для цього дрон має рухатися, тобто мати швидкість attackSpeed - швидкість при якій він скидає
        // І через те що і ціль рухається також нам треба знати де буде ціль якщо ми
        // прямо зараз в поточкому фреймі скинемо снаряд і воно пролетить ammoTimeOfFlight секунд часу
        // І порівняти 2 точки - де впаде снаряд і де буде ціль
        // В ідеалі якщо це однакові точки - ми фактично фіксуємо попадання
        // Але за умовою ми маємо hitRadius - це допустима похибка попадання в метрах
        // Отже, у нас буде певний запас на попадання цілі
        // Тому будемо шукати точку де буде ціль 
        // Шукаємо відстань від точки де впаде снаряд до точки де буде ціль і ця відстань
        // має бути меншою-рівною hitRadius
        // 1) Знаходимо одиничний вектор напрямку дрона:
        // 2) Рахуємо зміщення по координатах на horizontalFlightRange по знайдемону вектору
        // 3) Знаходимо точку де буде снаряд 
        Coord vecDirection = {std::cos(sim.state.droneDir), std::sin(sim.state.droneDir)};
        Coord hit = sim.state.dronePosition + vecDirection * horizontalFlightRange;
        // 4) Шукаємо точку де буде ціль ціль через ammoTimeOfFlight
        Coord targetVelocity{};
        if (!getTargetVelocity(targetVelocity, 
            sim.state.currentTargetIndex, 
            sim.state, 
            sim.droneConfig, 
            sim.targetsData))
        {
            return 1;
        }
        Coord predictedPosition{};
        if (!getPredictedPosition(predictedPosition, 
            targetVelocity, 
            bestTarget,
            ammoTimeOfFlight)) 
        {
            return 1;
        }


        // 5) Шукаємо відстань між цими точками
        float hitDistance = 0.0f;
        if (!getDistanceToTarget(hitDistance, 
            hit, 
            predictedPosition)) 
        {
            return 1;
        }

        SimStep *step = new SimStep;
        step->pos = sim.state.dronePosition;
        step->direction = sim.state.droneDir;
        step->state = sim.state.droneState;
        step->targetIdx = sim.state.currentTargetIndex;
        step->dropPoint = bestDropPoint;
        step->aimPoint = hit;
        step->predictedTarget = predictedPosition;

        sim.outputData.steps[count] = step;
        sim.outputData.totalSteps = count + 1;
        if (sim.state.droneState == MOVING && hitDistance <= sim.droneConfig->hitRadius)
        {
            // Ура, реєструємо ураження цілі, зупиняємо симуляцію
            break;
        }

        // Тепр коли ми маємо нову ціль + її координати + нову точку скиду
        // треба зрозуміти чи має дрон розвернутися до точки скиду
        // 1) Для цього знайдемо спочатку напрямок куди дрон має дивитися для нової цілі
        Coord maneuverVector = bestDropPoint - sim.state.dronePosition;
        // 2) Далі треба знайти кут між Х і точкою скиду
        float angleToDropPoint = std::atan2(maneuverVector.y, maneuverVector.x);
        // angleToDropPoint - це значення в радіанах, і воно обмежене [-M_PI; M_PI] або [-180 градусів; 180 градуів]
        // 3) І тепер ми можемо знайти різницю між тим куди зараз дивитися дрон і тим куди треба
        float deltaAngle = angleToDropPoint - sim.state.droneDir;
        if (!getNormalizedAngle(deltaAngle)) 
        {
            return 1;
        }
        // І тепер якщо deltaAngle = 0, це означає що дрон дивитися прямо на точку скиду
        // Якщо < 0, тоді треба розвертатися ліворуч, якщо > 0, тоді праворуч
        LOG_DEBUG("deltaAngle -> " << deltaAngle);

        /**
         * STOPPED          -> TURNING / ACCELERATING
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в DECELERATING - ні, ми уже стоїмо
         * 
         * ACCELERATING     -> MOVING / DECELERATING
         *      чи можемо ми перейти в STOPPED - ні, спочатку треба гальмувати
         *      чи можемо ми перейти в TURNING - ні, спочатку треба перейти зупинитися
         * 
         * MOVING           -> DECELERATING
         *      чи можемо ми перейти в STOPPED - ні, спочатку треба гальмувати
         *      чи можемо ми перейти в ACCELERATING - ні, бо ми уже набрали максимальну швидкість
         *      чи можемо ми перейти в TURNING - ні, бо спочатку треба зупинитися
         * 
         * TURNING          -> ACCELERATING
         *      чи можемо ми перейти в STOPPED - ні, немає сенсу, ми досягнули етапу STOPPED щоб почати розвертатися 
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в DECELERATING - ні, у нас уже швидкість = 0
         * 
         * DECELERATING     -> STOPPED
         *      чи можемо ми перейти в ACCELERATING - ні, ціль гальмування це повністю зупинитися
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в TURNING - ні, спочатку треба повністю зупинитися
         */
        // І тепер нарешті коли нам усе відомо про ціль, напрямок ми тепер можемо почати керувати дроном
        switch (sim.state.droneState)
        {
            // Якщо дрон стоїть, то він або на початку симуляції, або зупинився 
            // щоб перейти в стан TURNING якщо змінився напрямок, або в стан ACCELERATING
            // якщо напрямок ок
            case STOPPED:
            {
                // Якщо кут між напрямками дрона і цілі більше за порогове значення - turnThreshold,
                // ми переходимо в стан TURNING
                if (std::fabs(deltaAngle) > sim.droneConfig->turnThreshold)
                {
                    // Коли ми вирішили розвертатися, ми зберігаємо скільки ще нам залишається 
                    // градусів або радіанів щоб точно дивитися на ціль
                    // Ми уже маємо це значення - deltaAngle
                    sim.state.angleTurnLeft = std::fabs(deltaAngle);
                    // Далі нам треба оновити напрямок цілі куди ми маємо 
                    // дивитися щоб на кожному кроці в TURNING розуміти скільки ще залишилося
                    sim.state.dropPointDir = angleToDropPoint;
                    sim.state.droneState = TURNING;
                }
                else
                {
                    // За умовою, ми можемо продовжити рух дрона і поступово повертатися дугою
                    // до цілі якщо ми кут менше turnThreshold
                    // Тому ми просто оновлюємо напрямок дрона на ціль і починаємо його рух
                    sim.state.droneDir = angleToDropPoint;
                    sim.state.droneState = ACCELERATING;
                }
                break;
            }
            // Стан при якому дрон починає гальмувати до швидкості в 0 м/с
            // Після досягнення нульової швидкості - переходить в стан STOPPED
            case DECELERATING:
            {
                // На кожному фреймі коли ми гальмуємо, нам треба знати яку 
                // швидкість ми обрізаємо від поточної швидкісті дрона
                // За умовою швидкість прискорення та швидкість гальмування однакові
                // Тому треба знайти яку швидкість розвиває дрон за simTimeStep з певним прискоренням
                // 
                // Наприклад
                //      - прискорення 5 м/с
                //      - крок симуляції 0.5 с
                // 
                // Прискорення означає як швидкість змінюється з часом
                // Наприкла прискорення в 5 м/с означає що за 1 секунду ми змінюємо швидкість на 5 м/с
                // тоді за 0.5 с прискорення буде на половину меншим
                float oldDroneVelocity = sim.state.droneVelocity;
                float decelerationSpeed = acceleration * sim.droneConfig->simTimeStep;
                sim.state.droneVelocity -= decelerationSpeed;
                if (sim.state.droneVelocity <= 0.0f)
                {
                    sim.state.droneVelocity = 0.0f; // Вирівнюємо якщо швидкість відємна
                    sim.state.droneState = STOPPED;
                }
                // Щоб гальмування відбувалося нам треба не лише зменшувати швидкість дрона 
                // як значення але і поступово зупиняти в просторі
                // Для цього ми маємо рухати дрон з новою швидкістю в певному напрямку з кроком в simTimeStep
                // Отже, ми маємо одиничний вектор який буде показувати напрямок 
                // Зважаючи на те що дрон змінює свою швидкість не моментально, нам треба знайти середню
                // швидкість з якою він буде летіти протягом simTimeStep часу
                // Наприклад, на попередньому фреймі дрон летів зі швидкістю 10 м/с, на цьому фреймі він 
                // зменшить її на 2 м/с, тобто нова швидкість в кінці фрейму = 8 м/с
                // Тоді середня буде (10 + 8) / 2 = 9 м/с
                float avgVelocity = (oldDroneVelocity + sim.state.droneVelocity) * 0.5f;
                // І маємо знайти відстань на яку пролетить дрон зі швидкістю avgVelocity за simTimeStep час
                // Відстань знайти просто - знаючи швидкість та час, можна знайти яку відстань дрон
                // пролетить за цей час: якщо швидкість 5 м/с, а час 0.5 с, тоді відстань = 5 м/с * 0.5 с = 2.5 м
                float shift = avgVelocity * sim.droneConfig->simTimeStep;
                sim.state.dronePosition.x += std::cos(sim.state.droneDir) * shift; 
                sim.state.dronePosition.y += std::sin(sim.state.droneDir) * shift; 
                break;
            }
            // Дрон перебуває в стані прискорення якщо він перед цим стояв
            // і зараз починає розганятися в напрямку цілі. Розгін відбувається максимум
            // до швидкості attackSpeed, після досягнення якої дрон переходить в стан MOVING
            // Якщо дрону треба розвертатися, тоді в цьому стані він так само може перейти в DECELERATING
            case ACCELERATING:
            {
                if (std::fabs(deltaAngle) > sim.droneConfig->turnThreshold)
                {
                    sim.state.droneState = DECELERATING;
                }
                else
                {
                    // Аналогічно до того як ми гальмуємо, ми і розганяємося
                    // Тобто знаходимо швидкість яку задтний набрати дрон за simTimeStep
                    float oldDroneVelocity = sim.state.droneVelocity;
                    float accelerationSpeed = acceleration * sim.droneConfig->simTimeStep;
                    sim.state.droneVelocity += accelerationSpeed;
                    // Якщо ми досягаємо макс швидкості в attackSpeed, то переходимо в MOVING
                    if (sim.state.droneVelocity >= sim.droneConfig->attackSpeed)
                    {
                        sim.state.droneVelocity = sim.droneConfig->attackSpeed; // Стабілізуємо якщо перестирнбемо attackSpeed
                        sim.state.droneState = MOVING;
                    }

                    // Перед тим як змінювати розташування дрона портібно постійно уточнювати 
                    // його напрямок, бо може бути таке що ми дивимося на нову ціль, але
                    // ми не переступили turnThreshold
                    sim.state.droneDir = angleToDropPoint;

                    // І тепер аналогічним чином до DECELERATING, рухаємо дрон в просторі вперед
                    float avgVelocity = (oldDroneVelocity + sim.state.droneVelocity) * 0.5f;
                    float shift = avgVelocity * sim.droneConfig->simTimeStep;
                    Coord direction {
                        std::cos(sim.state.droneDir),
                        std::sin(sim.state.droneDir)
                    };
                    sim.state.dronePosition = sim.state.dronePosition + direction * shift;
                }
                break;
            }
            // Цей стан дрона означає що дрон набрав свою максимальну швидкість - attackSpeed
            // Тобто на цьому етапі швидкість дрона не перевищує attackSpeed
            // Якщо дрону треба розвертатися, тоді в цьому стані він так само може перейти в DECELERATING
            case MOVING:
            {
                if (std::fabs(deltaAngle) > sim.droneConfig->turnThreshold)
                {
                    sim.state.droneState = DECELERATING;
                }
                else
                {
                    // Якщо ми не перестрибнули turnThreshold, ми всеодно 
                    // можемо поступово повертатися до іншої цілі
                    sim.state.droneDir = angleToDropPoint;

                    // І тепер аналогічним чином до DECELERATING, рухаємо дрон в просторі вперед
                    float shift = sim.state.droneVelocity * sim.droneConfig->simTimeStep;
                    Coord direction {
                        std::cos(sim.state.droneDir),
                        std::sin(sim.state.droneDir)
                    };
                    sim.state.dronePosition = sim.state.dronePosition + direction * shift;
                }
                break;
            }
            // Дрон попадає в цей стан після того як він повністю зупинився
            // В цьому стані він розвертається в певному напрямку зі 
            // швидкістю angularSpeed - значення в радіанах за секунду
            // Коли завершив розвертатися до цілі, переходить в стан ACCELERATING
            case TURNING:
            {
                // Аналогічно до того як ми шукали яку швидкість дрона може скинути чи набрати за simTimeStep
                // Так само треба знайти на скільки радіан він може розвернутися за simTimeStep
                // Але ще треба знайти в яку сторону. Для цього шукаємо нову різницю між напрямком дрона і цілі
                float newDeltaAngle = sim.state.dropPointDir - sim.state.droneDir;
                if (!getNormalizedAngle(newDeltaAngle)) 
                {
                    return 1;
                }
                // І перевіряємо значення на знак, можливо нам треба буде почати розворот в протилежну сторону
                float turnDirection = newDeltaAngle > 0 ? 1.0f : -1.0f;
                // Щоб знайти цей кут ми беремо швидкість обертання angularSpeed і дивимося на скільки розвернемося 
                // за simTimeStep
                // 
                // Наприклад
                //      - angularSpeed - 1 рад/с
                //      - simTimeStep - 0.1 с
                // 
                // Тоді за 0.1 с дрон розвернеться на одну десяту від angularSpeed
                float turnShift = sim.droneConfig->angularSpeed * sim.droneConfig->simTimeStep;
                // Тепер коли ми знаємо на скільки дрон здатний повернутися за simTimeStep
                // Треба змінити його напрямок щоб він тепер почав дивитися з цим зсувом
                float newDroneDir = sim.state.droneDir + (turnShift * turnDirection);
                if (!getNormalizedAngle(newDroneDir)) 
                {
                    return 1;
                }
                sim.state.droneDir = newDroneDir;
                // І оновити скільки залишилося довернути
                sim.state.angleTurnLeft -= turnShift;
                
                // Якщо дрон уже достаньо розвернувся
                if (sim.state.angleTurnLeft <= 0.0f)
                {
                    sim.state.angleTurnLeft = 0.0f; // Знову таки зрізаємо ті зайві радіани які могли залишитися
                    sim.state.droneDir = sim.state.dropPointDir; // І тут тоже вирівнюємо рівно на ціль
                    sim.state.droneState = ACCELERATING;
                }
                break;
            }
            default:
                break;
        }

        count++;
        sim.state.totalSimTime = count * sim.droneConfig->simTimeStep;
        LOG_INFO("");
    }
    sim.outputData.totalSteps = count;

    if (!writeOutputToFile(sim.outputData, SIMULATION_JSON_FILE_NAME)) 
    {
        return 1;
    }

    return 0;
}
