#include "catch.hpp"

#include "../im3d_math.h"

using namespace Im3d;

TEST_CASE("Validate math type sizes", "[math]")
{
	REQUIRE(sizeof(Vec2)  == sizeof(float) * 2);
	REQUIRE(sizeof(Vec3)  == sizeof(float) * 3);
	REQUIRE(sizeof(Vec4)  == sizeof(float) * 4);
	REQUIRE(sizeof(Mat4)  == sizeof(float) * 16);
	REQUIRE(sizeof(Color) == sizeof(U32));
}
TEST_CASE("Vec2 ops", "[math]")
{
	Vec2 a(2.0f), b(1.0f);
	Vec2 c;

	c = a + b;
	REQUIRE(c.x == Approx(3.0));
	REQUIRE(c.y == Approx(3.0));
	c = a - b;
	REQUIRE(c.x == Approx(1.0));
	REQUIRE(c.y == Approx(1.0));
	c = a * b;
	REQUIRE(c.x == Approx(2.0));
	REQUIRE(c.y == Approx(2.0));
	c = a / b;
	REQUIRE(c.x == Approx(2.0));
	REQUIRE(c.y == Approx(2.0));

	c = a * 10.0f;
	REQUIRE(c.x == Approx(20.0));
	REQUIRE(c.y == Approx(20.0));
	c = a / 10.0f;
	REQUIRE(c.x == Approx(0.2));
	REQUIRE(c.y == Approx(0.2));
}

TEST_CASE("Color", "[types]")
{
	Color c;
	
	c = Color(1.0f, 0.0f, 0.0f, 0.0f);
	REQUIRE(c.getR() == Approx(1.0));
	c = Color(0.0f, 1.0f, 0.0f, 0.0f);
	REQUIRE(c.getG() == Approx(1.0));
	c = Color(0.0f, 0.0f, 1.0f, 0.0f);
	REQUIRE(c.getB() == Approx(1.0));
	c = Color(0.0f, 0.0f, 0.0f, 1.0f);
	REQUIRE(c.getA() == Approx(1.0));
}