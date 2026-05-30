#include <gtest/gtest.h>
#include "../include/ballistics.hpp"

TEST(Ballistics, DistanceCalculation)
{
    InputData input{};

    input.xd = 0;
    input.yd = 0;
    input.targetX = 3;
    input.targetY = 4;

    float result = 0.0f;
    bool ok = getDistanceToTarget(result, input);

    EXPECT_TRUE(ok);
    EXPECT_FLOAT_EQ(result, 5.0f);
}

TEST(Ballistics, AmmoInfoValidType)
{
    AmmoInfo ammo{};
    bool ok = getAmmoInfoByType("VOG-17", ammo);

    EXPECT_TRUE(ok);

    EXPECT_FLOAT_EQ(ammo.m, 0.35f);
    EXPECT_FLOAT_EQ(ammo.d, 0.07f);
    EXPECT_FLOAT_EQ(ammo.l, 0.0f);
    EXPECT_TRUE(ammo.isFreeFall);
}

TEST(Ballistics, UnknownAmmoType)
{
    AmmoInfo ammo{};
    bool ok = getAmmoInfoByType("UNKNOWN_AMMO", ammo);

    EXPECT_FALSE(ok);
}


TEST(Ballistics, ReferenceFullPipeline)
{
    InputData input{};

    input.xd = 100;
    input.yd = 100;
    input.zd = 100;

    input.targetX = 200;
    input.targetY = 200;

    input.attackSpeed = 10;
    input.accelerationPath = 10;

    input.ammo_name = "VOG-17";

    AmmoInfo ammo{};
    ASSERT_TRUE(getAmmoInfoByType(input.ammo_name, ammo));

    float tof = 0.0f;
    ASSERT_TRUE(getAmmoTimeOfFlight(tof, input, ammo));

    float range = 0.0f;
    ASSERT_TRUE(getHorizontalFlightRange(range, input, ammo, tof));

    float dist = 0.0f;
    ASSERT_TRUE(getDistanceToTarget(dist, input));

    OutputData out{};
    ASSERT_TRUE(getAmmoDropPoint(out, input, ammo, dist, range));

    EXPECT_NEAR(out.fireX, out.fireY, 1e-3);
    EXPECT_GT(out.fireX, 0.0f);
    EXPECT_GT(out.fireY, 0.0f);
}

TEST(Ballistics, UnknownAmmoIsHandledSafely)
{
    InputData input{};

    input.ammo_name = "THIS_DOES_NOT_EXIST";

    AmmoInfo ammo{};
    bool ok = getAmmoInfoByType(input.ammo_name, ammo);

    EXPECT_FALSE(ok);

    EXPECT_FLOAT_EQ(ammo.m, 0.0f);
    EXPECT_FLOAT_EQ(ammo.d, 0.0f);
    EXPECT_FLOAT_EQ(ammo.l, 0.0f);
}
