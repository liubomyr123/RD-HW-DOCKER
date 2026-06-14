#pragma once

/**
 *      | Значення  | Назва         | Опис                                          |
 *      -----------------------------------------------------------------------------
 *      | 0         | STOPPED       | Повна зупинка (v = 0))                        |
 *      | 1         | ACCELERATING  | Розгін від 0 до attackSpeed                   |
 *      | 2         | DECELERATING  | Гальмування від attackSpeed до 0              |
 *      | 3         | TURNING       | Поворот на місці (v \= 0, зміна напрямку)     |
 *      | 4         | MOVING        | Рух з крейсерською швидкістю attackSpeed      |
 */
enum DroneState
{
    STOPPED = 0,
    ACCELERATING,
    DECELERATING,
    TURNING,
    MOVING,
};