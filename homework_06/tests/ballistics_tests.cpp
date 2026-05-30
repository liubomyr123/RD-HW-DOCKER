#include <gtest/gtest.h>
#include "../include/ballistics.hpp"

TEST(Ballistics, DistanceCalculation)
{
  InputData input{};

  input.xd = 0;
  input.yd = 0;
  input.targetX = 3;
  input.targetY = 4;

  double result = 0.0F;
  bool success = getDistanceToTarget(result, input);

  EXPECT_TRUE(success);
  EXPECT_DOUBLE_EQ(result, 5.0F);
}

TEST(Ballistics, AmmoInfoValidType)
{
  AmmoInfo ammo{};
  bool success = getAmmoInfoByType("VOG-17", ammo);

  EXPECT_TRUE(success);

  EXPECT_DOUBLE_EQ(ammo.m, 0.35);
  EXPECT_DOUBLE_EQ(ammo.d, 0.07);
  EXPECT_DOUBLE_EQ(ammo.l, 0.0);
  EXPECT_TRUE(ammo.isFreeFall);
}

TEST(Ballistics, UnknownAmmoType)
{
  AmmoInfo ammo{};
  bool success = getAmmoInfoByType("UNKNOWN_AMMO", ammo);

  EXPECT_FALSE(success);
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

  double tof = 0.0F;
  ASSERT_TRUE(getAmmoTimeOfFlight(tof, input, ammo));

  double range = 0.0F;
  ASSERT_TRUE(getHorizontalFlightRange(range, input, ammo, tof));

  double dist = 0.0F;
  ASSERT_TRUE(getDistanceToTarget(dist, input));

  OutputData out{};
  ASSERT_TRUE(getAmmoDropPoint(out, input, ammo, dist, range));

  EXPECT_NEAR(out.fireX, out.fireY, 1e-3);
  EXPECT_GT(out.fireX, 0.0);
  EXPECT_GT(out.fireY, 0.0);
}

TEST(Ballistics, UnknownAmmoIsHandledSafely)
{
  InputData input{};

  input.ammo_name = "THIS_DOES_NOT_EXIST";

  AmmoInfo ammo{};
  bool success = getAmmoInfoByType(input.ammo_name, ammo);

  EXPECT_FALSE(success);

  EXPECT_DOUBLE_EQ(ammo.m, 0.0);
  EXPECT_DOUBLE_EQ(ammo.d, 0.0);
  EXPECT_DOUBLE_EQ(ammo.l, 0.0);
}
