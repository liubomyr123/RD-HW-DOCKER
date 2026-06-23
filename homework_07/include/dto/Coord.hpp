#pragma once

#include <cmath>

struct Coord {
	float x;
	float y;

    Coord operator+(const Coord& other) const 
    {
        Coord result{};
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    Coord operator-(const Coord& other) const 
    {
        Coord result{};
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Coord operator*(float scalar) const 
    {
        Coord result{};
        result.x = x * scalar;
        result.y = y * scalar;
        return result;
    }

    Coord operator/(float s) const {
        return {x / s, y / s};
    }

    float getVectorLength() const
    {
        return std::hypot(x, y);
    }
};
