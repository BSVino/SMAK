#include <stdio.h>

#include <vector.h>
#include <common.h>
#include <tstring.h>

#include <tinker/shell.h>

void test_eangles()
{
	Vector v;
	EAngle r;

	TAssert(AngleDifference(0, 360) == 0);
	TAssert(AngleDifference(360, 0) == 0);
	TAssert(AngleDifference(0, -360) == 0);
	TAssert(AngleDifference(-360, 0) == 0);
	TAssert(AngleDifference(360, -360) == 0);
	TAssert(AngleDifference(-360, 360) == 0);
	TAssert(AngleDifference(360, 360) == 0);
	TAssert(AngleDifference(-360, -360) == 0);
	TAssert(AngleDifference(180, -180) == 0);
	TAssert(AngleDifference(-180, 180) == 0);
	TAssert(AngleDifference(0, 10) == -10);
	TAssert(AngleDifference(0, -10) == 10);
	TAssert(AngleDifference(360, 10) == -10);
	TAssert(AngleDifference(360, -10) == 10);
	TAssert(AngleDifference(45, -134) == 179);
	TAssert(AngleDifference(45, -136) == -179);
	TAssert(AngleApproach(0, 181, 1) == 182);
	TAssert(AngleApproach(0, 179, 1) == 178);
	TAssert(AngleApproach(180, 1, 1) == 2);
	TAssert(AngleApproach(180, -1, 1) == -2);
	TAssert(AngleApproach(0, -179, 1) == -178);
	TAssert(AngleApproach(0, -181, 1) == -182);
	TAssert(AngleApproach(-180, -1, 1) == -2);
	TAssert(AngleApproach(-180, 1, 1) == 2);
	TAssert(AngleApproach(10, 350, 1) == 351);
	TAssert(AngleApproach(350, 10, 1) == 9);
	TAssert(AngleApproach(45, -134, 1) == -133);
	TAssert(AngleApproach(45, -136, 1) == -137);
	TAssert(EAngle(0, 180, 0) == EAngle(0, -180, 0));
	TAssert(EAngle(0, 360, 0) == EAngle(0, 0, 0));
	TAssert(!EAngle(0, 0, 0).Equals(EAngle(0, 180, 0), 0.1f));
	TAssert(EAngle(90, 0, 180).EqualsExhaustive(EAngle(90, 180, 0)));	// Because of Gimbal lock
	TAssert(EAngle(180, 0, 0).EqualsExhaustive(EAngle(0, 180, 180)));	// Because of Gimbal lock

	TAssert(AngleVector(EAngle(0, 0, 0)) == Vector(1, 0, 0));
	TAssert(AngleVector(EAngle(0, 90, 0)) == Vector(0, 0, 1));
	TAssert(AngleVector(EAngle(0, -90, 0)) == Vector(0, 0, -1));
	TAssert(AngleVector(EAngle(90, 0, 0)) == Vector(0, 1, 0));
	TAssert(AngleVector(EAngle(-90, 0, 0)) == Vector(0, -1, 0));
	TAssert(AngleVector(EAngle(0, 0, 45)) == Vector(1, 0, 0));
	TAssert(AngleVector(EAngle(0, 0, -45)) == Vector(1, 0, 0));

	Vector vecForward, vecUp, vecRight;
	AngleVectors(EAngle(0, 0, 0), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(1, 0, 0));
	TAssert(vecUp == Vector(0, 1, 0));
	TAssert(vecRight == Vector(0, 0, 1));
	AngleVectors(EAngle(0, 90, 0), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(0, 0, 1));
	TAssert(vecUp == Vector(0, 1, 0));
	TAssert(vecRight == Vector(-1, 0, 0));
	AngleVectors(EAngle(0, -90, 0), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(0, 0, -1));
	TAssert(vecUp == Vector(0, 1, 0));
	TAssert(vecRight == Vector(1, 0, 0));
	AngleVectors(EAngle(90, 0, 0), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(0, 1, 0));
	TAssert(vecUp == Vector(-1, 0, 0));
	TAssert(vecRight == Vector(0, 0, 1));
	AngleVectors(EAngle(-90, 0, 0), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(0, -1, 0));
	TAssert(vecUp == Vector(1, 0, 0));
	TAssert(vecRight == Vector(0, 0, 1));
	AngleVectors(EAngle(0, 0, 45), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(1, 0, 0));
	TAssert(vecUp == Vector(0, 0.70710677f, 0.70710677f));
	TAssert(vecRight == Vector(0, -0.70710677f, 0.70710677f));
	AngleVectors(EAngle(0, 0, -45), &vecForward, &vecUp, &vecRight);
	TAssert(vecForward == Vector(1, 0, 0));
	TAssert(vecUp == Vector(0, 0.70710677f, -0.70710677f));
	TAssert(vecRight == Vector(0, 0.70710677f, 0.70710677f));

	TAssert(VectorAngles(Vector(1, 0, 0)) == EAngle(0, 0, 0));
	TAssert(VectorAngles(Vector(0, 0, 1)) == EAngle(0, 90, 0));
	TAssert(VectorAngles(Vector(0, 0, -1)) == EAngle(0, -90, 0));
	TAssert(VectorAngles(Vector(0, 1, 0)) == EAngle(90, 0, 0));
	TAssert(VectorAngles(Vector(0, -1, 0)) == EAngle(-90, 0, 0));

	TAssert(AngleVector(VectorAngles(Vector(1, 0, 0))) == Vector(1, 0, 0));
	TAssert(AngleVector(VectorAngles(Vector(-1, 0, 0))) == Vector(-1, 0, 0));
	TAssert(AngleVector(VectorAngles(Vector(0, 1, 0))) == Vector(0, 1, 0));
	TAssert(AngleVector(VectorAngles(Vector(0, -1, 0))) == Vector(0, -1, 0));
	TAssert(AngleVector(VectorAngles(Vector(0, 0, 1))) == Vector(0, 0, 1));
	TAssert(AngleVector(VectorAngles(Vector(0, 0, -1))) == Vector(0, 0, -1));
}
