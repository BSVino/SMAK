#include <stdio.h>

#include <matrix.h>
#include <quaternion.h>
#include <common.h>
#include <maths.h>
#include <tstring.h>

#include <tinker/shell.h>

#define FULL_TEST

void test_matrix()
{
	Matrix4x4 m;

	// Test identity
	TAssert(m.IsIdentity());

	// Test translation
	m.SetTranslation(Vector(4, 5, 6));
	TAssert(m.GetTranslation() == Vector(4, 5, 6));

	// Test resetting identity
	m.Identity();
	TAssert(m.IsIdentity());

	m.Identity();
	m.SetTranslation(Vector(0, 0, 0));
	TAssert(m.GetTranslation() == Vector(0, 0, 0));

	m.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	TAssert(m.GetTranslation() == Vector(1, 2, 3));

	// Test setting angles and getting back the same angles
	m.Identity();
	m.SetAngles(EAngle(0, 0, 0));
	TAssert(m.GetAngles() == EAngle(0, 0, 0));
	TAssert(m.IsIdentity());
	m.SetAngles(EAngle(4, 5, 6));
	TAssert(m.GetAngles() == EAngle(4, 5, 6));
	m.SetAngles(EAngle(90, 0, 0));
	TAssert(m.GetAngles() == EAngle(90, 0, 0));
	m.SetAngles(EAngle(0, 90, 0));
	TAssert(m.GetAngles() == EAngle(0, 90, 0));
	m.SetAngles(EAngle(0, 0, 90));
	TAssert(m.GetAngles() == EAngle(0, 0, 90));
	m.SetAngles(EAngle(-90, 0, 0));
	TAssert(m.GetAngles() == EAngle(-90, 0, 0));
	m.SetAngles(EAngle(0, -90, 0));
	TAssert(m.GetAngles() == EAngle(0, -90, 0));
	m.SetAngles(EAngle(0, 0, -90));
	TAssert(m.GetAngles() == EAngle(0, 0, -90));
	m.SetAngles(EAngle(0, 90, 90));					// Let's test some Gimbal lock situations!
	TAssert(m.GetAngles() == EAngle(0, 90, 90));
	m.SetAngles(EAngle(90, 0, 90));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(90, 0, 90)));
	m.SetAngles(EAngle(90, 90, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(90, 90, 0)));
	m.SetAngles(EAngle(90, 90, 90));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(90, 90, 90)));
	m.SetAngles(EAngle(0, -90, -90));
	TAssert(m.GetAngles() == EAngle(0, -90, -90));
	m.SetAngles(EAngle(-90, 0, -90));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(-90, 0, -90)));
	m.SetAngles(EAngle(-90, -90, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(-90, -90, 0)));
	m.SetAngles(EAngle(-90, -90, -90));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(-90, -90, -90)));
	m.SetAngles(EAngle(0, 180, 0));
	TAssert(m.GetAngles() == EAngle(0, 180, 0));
	m.SetAngles(EAngle(0, -180, 0));
	TAssert(m.GetAngles() == EAngle(0, -180, 0));
	m.SetAngles(EAngle(90, 180, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(90, 180, 0)));
	m.SetAngles(EAngle(-90, -180, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(-90, -180, 0)));
	m.SetAngles(EAngle(0, 360, 0));
	TAssert(m.GetAngles() == EAngle(0, 360, 0));
	m.SetAngles(EAngle(0, -360, 0));
	TAssert(m.GetAngles() == EAngle(0, -360, 0));
	m.SetAngles(EAngle(180, 0, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(180, 0, 0)));
	m.SetAngles(EAngle(-180, 0, 0));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(-180, 0, 0)));
	m.SetAngles(EAngle(0, 0, 180));
	TAssert(m.GetAngles() == EAngle(0, 0, 180));
	m.SetAngles(EAngle(0, 0, -180));
	TAssert(m.GetAngles() == EAngle(0, 0, -180));
	m.SetAngles(EAngle(135, 45, 45));
	TAssert(m.GetAngles().EqualsExhaustive(EAngle(135, 45, 45)));

#ifdef FULL_TEST
	float flSin89 = sin(89.0f*M_PI/180);

	// See how GetAngles() reacts as it approaches 90 and passes degrees.
	for (int i = 0; i < 25000; i++)
	{
		float f = 89.0f + 0.0001f*i;
		m.SetAngles(EAngle(f, 12, 34));

		float flEp = 0.0001f;
		if (m.m[0][1] >= 0.999999f)
			flEp = 0.01f;
		TAssert(m.GetAngles().EqualsExhaustive(EAngle(f, 12, 34), flEp));
	}

	flSin89 = sin(-89.0f*M_PI/180);

	for (int i = 0; i < 25000; i++)
	{
		float f = -89.0f - 0.0001f*i;
		m.SetAngles(EAngle(f, 12, 34));

		float flEp = 0.0001f;
		if (m.m[0][1] <= -0.999999f)
			flEp = 0.01f;
		TAssert(m.GetAngles().EqualsExhaustive(EAngle(f, 12, 34), flEp));
	}
#endif

	m.Identity();
	// Test that this rotation setting function doesn't modify anything outside the center 3x3
	m.Init(0, 0, 0, 123, 0, 0, 0, 123, 0, 0, 0, 123, 123, 123, 123, 123);
	m.SetAngles(EAngle(1, 2, 3));
	TAssert(m.GetRow(0).w == 123);
	TAssert(m.GetRow(1).w == 123);
	TAssert(m.GetRow(2).w == 123);
	TAssert(m.GetColumn(0).w == 123);
	TAssert(m.GetColumn(1).w == 123);
	TAssert(m.GetColumn(2).w == 123);
	TAssert(m.GetColumn(3).w == 123);

	m.Identity();
	m.SetRotation(0, Vector(1, 0, 0));
	TAssert(m.IsIdentity());

	m.Identity();
	m.SetRotation(90, Vector(0, 1, 0));
	TAssert(m.GetAngles() == EAngle(0, -90, 0));
	TAssert(m.GetForwardVector() == Vector(0, 0, -1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));

	m.Identity();
	// Test that this rotation setting function doesn't modify anything outside the center 3x3
	m.Init(0, 0, 0, 123, 0, 0, 0, 123, 0, 0, 0, 123, 123, 123, 123, 123);
	m.SetRotation(1, Vector(2, 3, 4).Normalized());
	TAssert(m.GetRow(0).w == 123);
	TAssert(m.GetRow(1).w == 123);
	TAssert(m.GetRow(2).w == 123);
	TAssert(m.GetColumn(0).w == 123);
	TAssert(m.GetColumn(1).w == 123);
	TAssert(m.GetColumn(2).w == 123);
	TAssert(m.GetColumn(3).w == 123);

	m.Identity();
	Quaternion q;
	q.SetAngles(EAngle(4, 5, 6));
	m.SetRotation(q);
	TAssert(m.GetAngles() == EAngle(4, 5, 6));

	m.Identity();
	m.SetRotation(Quaternion());
	TAssert(m.IsIdentity());

	m.Identity();
	// Create a quaternion to test with
	m.SetAngles(EAngle(10, 90, 45));
	q = Quaternion(m);
	m.Identity();
	m.SetRotation(q);
	TAssert(m.GetAngles().Equals(EAngle(10, 90, 45), 0.00001f));

	m.Identity();
	// Test that this rotation setting function doesn't modify anything outside the center 3x3
	m.Init(0, 0, 0, 123, 0, 0, 0, 123, 0, 0, 0, 123, 123, 123, 123, 123);
	m.SetRotation(q);
	TAssert(m.GetRow(0).w == 123);
	TAssert(m.GetRow(1).w == 123);
	TAssert(m.GetRow(2).w == 123);
	TAssert(m.GetColumn(0).w == 123);
	TAssert(m.GetColumn(1).w == 123);
	TAssert(m.GetColumn(2).w == 123);
	TAssert(m.GetColumn(3).w == 123);

	m.Identity();
	m.SetOrientation(Vector(1, 0, 0));
	TAssert(m.IsIdentity());

	m.Identity();
	m.SetOrientation(Vector(-1, 0, 0));
	TAssert(m.GetAngles() == EAngle(0, 180, 0));
	TAssert(m.GetForwardVector() == Vector(-1, 0, 0));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, -1));

	m.Identity();
	m.SetOrientation(Vector(0, 0, 1));
	TAssert(m.GetAngles() == EAngle(0, 90, 0));
	TAssert(m.GetForwardVector() == Vector(0, 0, 1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(-1, 0, 0));

	m.Identity();
	m.SetOrientation(Vector(0, 0, -1));
	TAssert(m.GetAngles() == EAngle(0, -90, 0));
	TAssert(m.GetForwardVector() == Vector(0, 0, -1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(1, 0, 0));

	m.Identity();
	m.SetOrientation(Vector(0, 1, 0));
	TAssert(m.GetAngles() == EAngle(90, 0, 0));
	TAssert(m.GetForwardVector() == Vector(0, 1, 0));
	TAssert(m.GetUpVector() == Vector(-1, 0, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));

	m.Identity();
	m.SetOrientation(Vector(0, -1, 0));
	TAssert(m.GetAngles() == EAngle(-90, 0, 0));
	TAssert(m.GetForwardVector() == Vector(0, -1, 0));
	TAssert(m.GetUpVector() == Vector(1, 0, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));

	m.Identity();
	m.SetOrientation(Vector(1, 1, 1));
	TAssert(m.GetAngles() == EAngle(35.264389f, 45, 0));
	TAssert(m.GetForwardVector() == Vector(1, 1, 1).Normalized());
	TAssert(m.GetUpVector() == Vector(-1, 2, -1).Normalized());
	TAssert(m.GetRightVector() == Vector(-1, 0, 1).Normalized());

	m.Identity();
	// Test that this rotation setting function doesn't modify anything outside the center 3x3
	m.Init(0, 0, 0, 123, 0, 0, 0, 123, 0, 0, 0, 123, 123, 123, 123, 123);
	m.SetOrientation(Vector(1, 2, 3));
	TAssert(m.GetRow(0).w == 123);
	TAssert(m.GetRow(1).w == 123);
	TAssert(m.GetRow(2).w == 123);
	TAssert(m.GetColumn(0).w == 123);
	TAssert(m.GetColumn(1).w == 123);
	TAssert(m.GetColumn(2).w == 123);
	TAssert(m.GetColumn(3).w == 123);

	// Test scaling, and transforming a vector with scaled matrix
	m.Identity();
	m.SetScale(Vector(2, 2, 2));
	TAssert(m * Vector(1, 1, 1) == Vector(2, 2, 2));

	// Test reflection
	m.Identity();
	m.SetReflection(Vector(1, 0, 0));
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(-1, 0, 0));
	TAssert(m * Vector(-1, 0, 0) == Vector(1, 0, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, 1, 1));

	m.Identity();
	m.SetReflection(Vector(-1, 0, 0));	// Kind of a redundant test...
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(-1, 0, 0));
	TAssert(m * Vector(-1, 0, 0) == Vector(1, 0, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, 1, 1));

	m.Identity();
	m.SetReflection(Vector(0, 1, 0));
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(0, 1, 0) == Vector(0, -1, 0));
	TAssert(m * Vector(0, -1, 0) == Vector(0, 1, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(1, -1, 1));

	m.Identity();
	m.SetReflection(Vector(0, 0, 1));
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(0, 0, 1) == Vector(0, 0, -1));
	TAssert(m * Vector(0, 0, -1) == Vector(0, 0, 1));
	TAssert(m * Vector(1, 1, 1) == Vector(1, 1, -1));

	m.Identity();
	m.SetReflection(Vector(1, 0, 1).Normalized());
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(0, 0, -1));
	TAssert(m * Vector(-1, 0, 0) == Vector(0, 0, 1));
	TAssert(m * Vector(0, 0, 1) == Vector(-1, 0, 0));
	TAssert(m * Vector(0, 0, -1) == Vector(1, 0, 0));
	TAssert(m * Vector(1, 0, 1) == Vector(-1, 0, -1));
	TAssert(m * Vector(-1, 0, -1) == Vector(1, 0, 1));
	TAssert(m * Vector(-1, 0, 1) == Vector(-1, 0, 1));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, 1, -1));
	TAssert(m * Vector(-1, -1, -1) == Vector(1, -1, 1));

	m.Identity();
	m.SetReflection(Vector(-1, 0, -1).Normalized());	// Another redundant test...
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(0, 0, -1));
	TAssert(m * Vector(-1, 0, 0) == Vector(0, 0, 1));
	TAssert(m * Vector(0, 0, 1) == Vector(-1, 0, 0));
	TAssert(m * Vector(0, 0, -1) == Vector(1, 0, 0));
	TAssert(m * Vector(1, 0, 1) == Vector(-1, 0, -1));
	TAssert(m * Vector(-1, 0, -1) == Vector(1, 0, 1));
	TAssert(m * Vector(-1, 0, 1) == Vector(-1, 0, 1));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, 1, -1));
	TAssert(m * Vector(-1, -1, -1) == Vector(1, -1, 1));

	m.Identity();
	m.SetReflection(Vector(1, 1, 0).Normalized());
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(0, -1, 0));
	TAssert(m * Vector(-1, 0, 0) == Vector(0, 1, 0));
	TAssert(m * Vector(0, 1, 0) == Vector(-1, 0, 0));
	TAssert(m * Vector(0, -1, 0) == Vector(1, 0, 0));
	TAssert(m * Vector(1, 1, 0) == Vector(-1, -1, 0));
	TAssert(m * Vector(-1, -1, 0) == Vector(1, 1, 0));
	TAssert(m * Vector(-1, 1, 0) == Vector(-1, 1, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, -1, 1));

	m.Identity();
	m.SetReflection(Vector(1, 1, 1).Normalized());
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(-1, -1, -1));
	TAssert(m * Vector(-1, -1, -1) == Vector(1, 1, 1));
	TAssert(m * Vector(1, 0, 0) == Vector(1, -2, -2).Normalized());
	TAssert(m * Vector(0, 1, 0) == Vector(-2, 1, -2).Normalized());
	TAssert(m * Vector(0, 0, 1) == Vector(-2, -2, 1).Normalized());
	TAssert(m * Vector(-1, 0, 0) == Vector(-1, 2, 2).Normalized());
	TAssert(m * Vector(0, -1, 0) == Vector(2, -1, 2).Normalized());
	TAssert(m * Vector(0, 0, -1) == Vector(2, 2, -1).Normalized());
	TAssert(m * (Vector(0, 1, 1).Normalized()) == Vector(-4, -1, -1).Normalized());	// This is an odd relationship isn't it?
	TAssert(m * (Vector(1, 0, 1).Normalized()) == Vector(-1, -4, -1).Normalized());
	TAssert(m * (Vector(1, 1, 0).Normalized()) == Vector(-1, -1, -4).Normalized());
	TAssert(m * (Vector(0, -1, -1).Normalized()) == Vector(4, 1, 1).Normalized());
	TAssert(m * (Vector(-1, 0, -1).Normalized()) == Vector(1, 4, 1).Normalized());
	TAssert(m * (Vector(-1, -1, 0).Normalized()) == Vector(1, 1, 4).Normalized());

	m.Identity();
	// Test that this rotation setting function doesn't modify anything outside the center 3x3
	m.Init(0, 0, 0, 123, 0, 0, 0, 123, 0, 0, 0, 123, 123, 123, 123, 123);
	m.SetReflection(Vector(1, 2, 3).Normalized());
	TAssert(m.GetRow(0).w == 123);
	TAssert(m.GetRow(1).w == 123);
	TAssert(m.GetRow(2).w == 123);
	TAssert(m.GetColumn(0).w == 123);
	TAssert(m.GetColumn(1).w == 123);
	TAssert(m.GetColumn(2).w == 123);
	TAssert(m.GetColumn(3).w == 123);

	// Test translation
	m.Identity();
	m.SetTranslation(Vector(4, 5, 6));
	m += Vector(1, 2, 3);
	TAssert(m.GetTranslation() == Vector(5, 7, 9));

	// Test adding angles
	m.Identity();
	m.SetAngles(EAngle(0, 10, 0));
	m += EAngle(0, 20, 0);
	TAssert(m.GetAngles() == EAngle(0, 30, 0));

	m.Identity();
	m.SetAngles(EAngle(10, 0, 0));
	m += EAngle(-20, 0, 0);
	TAssert(m.GetAngles() == EAngle(-10, 0, 0));

	m.Identity();
	m.SetAngles(EAngle(0, 10, 0));	// Order is important here!
	m += EAngle(10, 0, 0);
	TAssert(m.GetAngles() == EAngle(10, 10, 0));

	// Test adding different transformations
	m.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	m.AddTranslation(Vector(-1, -1, -1));
	TAssert(m.GetTranslation() == Vector(0, 1, 2));

	m.Identity();
	m.SetAngles(EAngle(0, 90, 0));	// Order is important here too!
	m.AddAngles(EAngle(0, 0, 45));
	TAssert(m.GetAngles() == EAngle(0, 90, 45));

	m.Identity();
	m.SetAngles(EAngle(0, 90, 0));	// Order is very important!
	m += EAngle(45, 0, 0);
	m.AddAngles(EAngle(0, 0, 45));
	TAssert(m.GetAngles().Equals(EAngle(45, 90, 45), 0.00001f));

	m.Identity();
	m.SetScale(Vector(2, 2, 2));
	m.AddScale(Vector(-1, -1, -1));
	TAssert(m * Vector(1, 1, 1) == Vector(-2, -2, -2));

	// Test adding a reflection into an existing transformation
	m.Identity();
	m.SetReflection(Vector(1, 0, 0));
	m.AddReflection(Vector(0, 1, 0));
	TAssert(m == Matrix4x4(Vector(-1, 0, 0), Vector(0, -1, 0), Vector(0, 0, 1)));
	TAssert(m * Vector(1, 1, 0) == Vector(-1, -1, 0));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	m.AddScale(Vector(2, 2, 2));
	m.AddReflection(Vector(0, 0, 1));
	TAssert(m == Matrix4x4(Vector(0, 0, -2), Vector(0, 2, 0), Vector(-2, 0, 0)));
	TAssert(m * Vector(1, 0, 0) == Vector(0, 0, -2));
	TAssert(m * Vector(-1, 0, 0) == Vector(0, 0, 2));
	TAssert(m * Vector(0, 1, 0) == Vector(0, 2, 0));
	TAssert(m * Vector(0, -1, 0) == Vector(0, -2, 0));
	TAssert(m * Vector(0, 0, 1) == Vector(-2, 0, 0));
	TAssert(m * Vector(0, 0, -1) == Vector(2, 0, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(-2, 2, -2));
	TAssert(m * Vector(-1, -1, -1) == Vector(2, -2, 2));

	Matrix4x4 n;

	// Test matrix multiplication for n = identity (More mult tests at the end)
	m.Identity();
	m.SetAngles(EAngle(4, 5, 6));
	m.SetTranslation(Vector(4, 5, 6));
	n = n * m;
	TAssert(n.GetAngles() == EAngle(4, 5, 6));
	TAssert(n.GetTranslation() == Vector(4, 5, 6));

	m.Identity();
	n.Identity();
	m.SetAngles(EAngle(4, 5, 6));
	m.SetTranslation(Vector(4, 5, 6));
	n *= m;
	TAssert(n.GetAngles() == EAngle(4, 5, 6));
	TAssert(n.GetTranslation() == Vector(4, 5, 6));

	// Test transforming a vector
	m.Identity();
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(1, 0, 0));
	TAssert(m * Vector(0, 1, 0) == Vector(0, 1, 0));
	TAssert(m * Vector(0, 0, 1) == Vector(0, 0, 1));
	TAssert(m * Vector(1, 1, 1) == Vector(1, 1, 1));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	TAssert(m * Vector(0, 0, 0) == Vector(0, 0, 0));
	TAssert(m * Vector(1, 0, 0) == Vector(0, 0, -1));
	TAssert(m * Vector(-1, 0, 0) == Vector(0, 0, 1));
	TAssert(m * Vector(0, 1, 0) == Vector(0, 1, 0));
	TAssert(m * Vector(0, -1, 0) == Vector(0, -1, 0));
	TAssert(m * Vector(0, 0, 1) == Vector(1, 0, 0));
	TAssert(m * Vector(0, 0, -1) == Vector(-1, 0, 0));
	TAssert(m * Vector(1, 1, 1) == Vector(1, 1, -1));

	m.Identity();
	m.SetTranslation(Vector(4, 5, 6));
	TAssert(m * Vector(0, 0, 0) == Vector(4, 5, 6));
	TAssert(m * Vector(-4, -5, -6) == Vector(0, 0, 0));

	m.Identity();
	// SetAngles() and SetTranslation() makes a matrix with rotation and then translation
	m.SetAngles(EAngle(0, -90, 0));
	m.SetTranslation(Vector(4, 5, 6));
	TAssert(m * Vector(0, 0, 0) == Vector(4, 5, 6));
	TAssert(m * Vector(1, 1, 1) == Vector(5, 6, 5));	// (1, 1, 1) rotates to (1, 1, -1) and then translates to (5, 6, 5)
	TAssert(m.TransformVector(Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m.TransformVector(Vector(1, 0, 0)) == Vector(0, 0, -1));
	TAssert(m.TransformVector(Vector(0, 1, 0)) == Vector(0, 1, 0));
	TAssert(m.TransformVector(Vector(0, 0, 1)) == Vector(1, 0, 0));
	TAssert(m.TransformVector(Vector(1, 1, 1).Normalized()) == Vector(1, 1, -1).Normalized());
	TAssert(m.TransformVector(Vector(10, 0, 0)) == Vector(0, 0, -10));
	TAssert(m.TransformVector(Vector(0, 10, 0)) == Vector(0, 10, 0));
	TAssert(m.TransformVector(Vector(0, 0, 10)) == Vector(10, 0, 0));
	TAssert(m.TransformVector(Vector(10, 10, 10)) == Vector(10, 10, -10));
	TAssert(m * Vector4D(0, 0, 0, 0) == Vector4D(0, 0, 0, 0));
	TAssert(m * Vector4D(1, 1, 1, 0) == Vector4D(1, 1, -1, 0));
	TAssert(m * Vector4D(0, 0, 0, 1) == Vector4D(4, 5, 6, 1));
	TAssert(m * Vector4D(1, 1, 1, 1) == Vector4D(5, 6, 5, 1));

	m.Identity();
	// SetAngles() and AddTranslation() makes a matrix with translation and then rotation. (Reverse order.)
	m.SetAngles(EAngle(0, -90, 0));
	m.AddTranslation(Vector(4, 5, 6));
	TAssert(m * Vector(0, 0, 0) == Vector(6, 5, -4));
	TAssert(m * Vector(1, 1, 1) == Vector(7, 6, -5));	// (1, 1, 1) translates to (5, 6, 7) and then rotates to (7, 6, -5)
	TAssert(m.TransformVector(Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m.TransformVector(Vector(1, 0, 0)) == Vector(0, 0, -1));
	TAssert(m.TransformVector(Vector(0, 1, 0)) == Vector(0, 1, 0));
	TAssert(m.TransformVector(Vector(0, 0, 1)) == Vector(1, 0, 0));
	TAssert(m.TransformVector(Vector(1, 1, 1).Normalized()) == Vector(1, 1, -1).Normalized());
	TAssert(m.TransformVector(Vector(10, 0, 0)) == Vector(0, 0, -10));
	TAssert(m.TransformVector(Vector(0, 10, 0)) == Vector(0, 10, 0));
	TAssert(m.TransformVector(Vector(0, 0, 10)) == Vector(10, 0, 0));
	TAssert(m.TransformVector(Vector(10, 10, 10)) == Vector(10, 10, -10));
	TAssert(m * Vector4D(0, 0, 0, 0) == Vector4D(0, 0, 0, 0));
	TAssert(m * Vector4D(1, 1, 1, 0) == Vector4D(1, 1, -1, 0));
	TAssert(m * Vector4D(0, 0, 0, 1) == Vector4D(6, 5, -4, 1));
	TAssert(m * Vector4D(1, 1, 1, 1) == Vector4D(7, 6, -5, 1));

	m.Identity();
	// SetTranslation() and AddAngles() makes a matrix with rotation and then translation
	m.SetTranslation(Vector(4, 5, 6));
	m.AddAngles(EAngle(0, -90, 0));
	TAssert(m * Vector(0, 0, 0) == Vector(4, 5, 6));
	TAssert(m * Vector(1, 1, 1) == Vector(5, 6, 5));	// (1, 1, 1) rotates to (1, 1, -1) and then translates to (5, 6, 5)
	TAssert(m.TransformVector(Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m.TransformVector(Vector(1, 0, 0)) == Vector(0, 0, -1));
	TAssert(m.TransformVector(Vector(0, 1, 0)) == Vector(0, 1, 0));
	TAssert(m.TransformVector(Vector(0, 0, 1)) == Vector(1, 0, 0));
	TAssert(m.TransformVector(Vector(1, 1, 1).Normalized()) == Vector(1, 1, -1).Normalized());
	TAssert(m.TransformVector(Vector(10, 0, 0)) == Vector(0, 0, -10));
	TAssert(m.TransformVector(Vector(0, 10, 0)) == Vector(0, 10, 0));
	TAssert(m.TransformVector(Vector(0, 0, 10)) == Vector(10, 0, 0));
	TAssert(m.TransformVector(Vector(10, 10, 10)) == Vector(10, 10, -10));
	TAssert(m * Vector4D(0, 0, 0, 0) == Vector4D(0, 0, 0, 0));
	TAssert(m * Vector4D(1, 1, 1, 0) == Vector4D(1, 1, -1, 0));
	TAssert(m * Vector4D(0, 0, 0, 1) == Vector4D(4, 5, 6, 1));
	TAssert(m * Vector4D(1, 1, 1, 1) == Vector4D(5, 6, 5, 1));

	// Test proper results for GetRow() and GetColumn()
	m.Identity();
	TAssert(m.GetRow(0) == Vector4D(1, 0, 0, 0));
	TAssert(m.GetRow(1) == Vector4D(0, 1, 0, 0));
	TAssert(m.GetRow(2) == Vector4D(0, 0, 1, 0));
	TAssert(m.GetRow(3) == Vector4D(0, 0, 0, 1));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	TAssert(m.GetColumn(0) == Vector4D(0, 0, 1, 0));
	TAssert(m.GetColumn(1) == Vector4D(0, 1, 0, 0));
	TAssert(m.GetColumn(2) == Vector4D(-1, 0, 0, 0));
	TAssert(m.GetColumn(3) == Vector4D(0, 0, 0, 1));

	m.Identity();
	TAssert(m.GetColumn(0) == Vector4D(1, 0, 0, 0));
	TAssert(m.GetColumn(1) == Vector4D(0, 1, 0, 0));
	TAssert(m.GetColumn(2) == Vector4D(0, 0, 1, 0));
	TAssert(m.GetColumn(3) == Vector4D(0, 0, 0, 1));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	TAssert(m.GetRow(0) == Vector4D(0, 0, -1, 0));
	TAssert(m.GetRow(1) == Vector4D(0, 1, 0, 0));
	TAssert(m.GetRow(2) == Vector4D(1, 0, 0, 0));
	TAssert(m.GetRow(3) == Vector4D(0, 0, 0, 1));

	m.Identity();
	m.SetColumn(0, Vector(2, 3, 4));
	TAssert(m.GetColumn(0) == Vector4D(2, 3, 4, 0));

	m.Identity();
	m.SetColumn(0, Vector4D(2, 3, 4, 5));
	TAssert(m.GetColumn(0) == Vector4D(2, 3, 4, 5));

	// Test proper results for GetXXXXVector()
	m.Identity();
	TAssert(m.GetForwardVector() == Vector(1, 0, 0));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	TAssert(m.GetForwardVector() == Vector(0, 0, -1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(1, 0, 0));

	m.Identity();
	m.SetAngles(EAngle(90, 0, 0));
	TAssert(m.GetForwardVector() == Vector(0, 1, 0));
	TAssert(m.GetUpVector() == Vector(-1, 0, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));

	// Test Translation/Rotation inversion
	m = Matrix4x4().AddAngles(EAngle(0, -90, 0)).InvertedRT();
	TAssert(m.GetForwardVector() == Vector(0, 0, 1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(-1, 0, 0));
	TAssert(m.GetTranslation() == Vector(0, 0, 0));

	m.Identity();
	m.SetTranslation(Vector(2, 3, 4));
	TAssert(m * Vector(0, 0, 0) == Vector(2, 3, 4));
	m.InvertRT();
	TAssert(m * Vector(2, 3, 4) == Vector(0, 0, 0));
	TAssert(m.GetForwardVector() == Vector(1, 0, 0));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));
	TAssert(m.GetTranslation() == Vector(-2, -3, -4));

	m.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	m.SetTranslation(Vector(2, 3, 4));
	TAssert(m * Vector(0, 0, 0) == Vector(2, 3, 4));
	m.InvertRT();
	TAssert(m * Vector(2, 3, 4) == Vector(0, 0, 0));
	TAssert(m.GetForwardVector() == Vector(0, 0, 1));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(-1, 0, 0));
	TAssert(m.GetTranslation() == Vector(4, -3, -2));

	m.Identity();
	m.SetReflection(Vector(1, 2, 3).Normalized());
	n = m;
	m.InvertRT();
	TAssert(m == n);	// The inversion of a reflection is always itself.

	m.Identity();
	m.SetReflection(Vector(1, 0, 0));
	m.AddTranslation(Vector(1, 2, 3));
	TAssert(m * Vector(0, 0, 0) == Vector(-1, 2, 3));
	m.InvertRT();
	TAssert(m * Vector(-1, 2, 3) == Vector(0, 0, 0));
	TAssert(m.GetForwardVector() == Vector(-1, 0, 0));
	TAssert(m.GetUpVector() == Vector(0, 1, 0));
	TAssert(m.GetRightVector() == Vector(0, 0, 1));
	TAssert(m.GetTranslation() == Vector(-1, -2, -3));

	m.Identity();
	m.SetReflection(Vector(1, 1, 1).Normalized());
	m.AddTranslation(Vector(1, 2, 3));
	TAssert(m * Vector(0, 0, 0) == Vector(-3, -2, -1));
	m.InvertRT();
	TAssert(m * Vector(-3, -2, -1) == Vector(0, 0, 0));
	TAssert(m.GetForwardVector() == Vector(1, -2, -2)/3);
	TAssert(m.GetUpVector() == Vector(-2, 1, -2)/3);
	TAssert(m.GetRightVector() == Vector(-2, -2, 1)/3);
	TAssert(m.GetTranslation() == Vector(-1, -2, -3));

	m.Identity();
	m.AddAngles(EAngle(0, 45, 0));
	m.AddReflection(Vector(1, 1, 1).Normalized());
	m.AddTranslation(Vector(1, 2, 3));
	TAssert(m * Vector(0, 0, 0) == Vector(-1.4142135f, -2, -2.8284268f));
	m.InvertRT();
	TAssert(m * Vector(-1.4142135f, -2, -2.8284268f) == Vector(0, 0, 0));
	TAssert(m.GetForwardVector() == Vector(0.70710677f, 0, -0.70710677f));
	TAssert(m.GetUpVector() == Vector(-2, 1, -2)/3);
	TAssert(m.GetRightVector() == Vector(-0.23570222f, -0.94280899f, -0.23570222f));
	TAssert(m.GetTranslation() == Vector(-1, -2, -3));

	// Can't really test SetReflection() with SetAngles() since they both set the same shit.
	// Whatever, we'll catch it below.

	// Test more matrix mult by doing local/world space conversions.
	m.Identity();
	n.Identity();
	m.SetTranslation(Vector(2, 3, 4));
	n.SetTranslation(Vector(1, 2, 3));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1), Vector(3, 5, 7)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(3, 5, 7));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(4, 6, 8));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(1, 1, 1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(10, 10, 10));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(1, 1, 1, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(3, 5, 7, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(4, 6, 8, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(3, 5, 7)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(4, 6, 8)) == Vector(1, 1, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(1, 1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(10, 10, 10));
	TAssert(n * (m * Vector4D(3, 5, 7, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(4, 6, 8, 1)) == Vector4D(1, 1, 1, 1));
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(-3, -5, -7));
	TAssert(n * (m * Vector(1, 1, 1)) == Vector(-2, -4, -6));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(1, 1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(10, 10, 10));
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(1, 1, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(-3, -5, -7, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(-2, -4, -6, 1));

	m.Identity();
	n.Identity();
	m.SetAngles(EAngle(0, -90, 0));
	n.SetAngles(EAngle(45, 0, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(0, 1, -1).Normalized(), Vector(0, 1, 1).Normalized(), Vector(1, 0, 0)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(0, 1, -1).Normalized());
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(0, 1, 1).Normalized());
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(1, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(0, 1, -1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0, 1, 1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(1, 0, 0).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(0.57735026f, 0.81649655f, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(0, 1, -1).Normalized()*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0, 1, 1).Normalized()*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(10, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(10, 14.142136f, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(1, 1.4142136f, 0, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(1, 1.4142136f, 0, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(0, 1, -1).Normalized()) == Vector(1, 0, 0));
	TAssert(n * (m * Vector(0, 1, 1).Normalized()) == Vector(0, 1, 0));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, -1).Normalized())) == Vector(1, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 1).Normalized())) == Vector(0, 1, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0.57735026f, 0.81649655f, 0))) == Vector(1, 1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, -1).Normalized()*10)) == Vector(10, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 1).Normalized()*10)) == Vector(0, 10, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(0, 0, 10));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 14.142136f, 0))) == Vector(10, 10, 10));
	TAssert(n * (m * Vector4D(1, 1.4142136f, 0, 0)) == Vector4D(1, 1, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(1, 1.4142136f, 0, 1)) == Vector4D(1, 1, 1, 1));
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(0, 0, 1));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(1, 1, 0).Normalized());
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(-1, 1, 0).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 0, 0))) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 0))) == Vector(1, 1, 0).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 1))) == Vector(-1, 1, 0).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(0, 0.81649655f, 0.57735026f));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(0, 0, 10));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 10, 0))) == Vector(1, 1, 0).Normalized()*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 10))) == Vector(-1, 1, 0).Normalized()*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(0, 14.142136f, 10));
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(0, 1.4142135f, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(0, 1.4142135f, 1, 1));

	m.Identity();
	n.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	n.SetTranslation(Vector(4, 5, 6));
	m.SetAngles(EAngle(0, -90, 0));
	n.SetAngles(EAngle(45, 0, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(0, 1, -1).Normalized(), Vector(0, 1, 1).Normalized(), Vector(1, 0, 0), Vector(7, 7, -1)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(7, 7, -1));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(7, 7.7071066f, -1.7071068f));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(7, 7.7071066f, -0.29289344f));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(8, 7, -1));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(0, 1, -1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0, 1, 1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(1, 0, 0).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(0.57735026f, 0.81649655f, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(0, 1, -1).Normalized()*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0, 1, 1).Normalized()*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(10, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(10, 14.142136f, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(1, 1.4142136f, 0, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(7, 7, -1, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(8, 8.4142132f, -1, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(7, 7, -1)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(7, 7.7071066f, -1.7071068f)) == Vector(1, 0, 0));
	TAssert(n * (m * Vector(7, 7.7071066f, -0.29289344f)) == Vector(0, 1, 0));
	TAssert(n * (m * Vector(8, 7, -1)) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, -1).Normalized())) == Vector(1, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 1).Normalized())) == Vector(0, 1, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0.57735026f, 0.81649655f, 0))) == Vector(1, 1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, -1).Normalized()*10)) == Vector(10, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 1).Normalized()*10)) == Vector(0, 10, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 14.142136f, 0))) == Vector(10, 10, 10));
	TAssert(n * (m * Vector4D(1, 1.4142136f, 0, 0)) == Vector4D(1, 1, 1, 0));
	TAssert(n * (m * Vector4D(7, 7, -1, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(8, 8.4142132f, -1, 1)) == Vector4D(1, 1, 1, 1));
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(-5.6568542f, -4.2426405f, -7));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(-5.6568542f, -4.2426405f, -6));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(-4.9497471f, -3.5355339f, -7));
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(-6.3639607f, -3.5355339f, -7));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 0, 0))) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 0))) == Vector(1, 1, 0).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 1))) == Vector(-1, 1, 0).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(0, 0.81649655f, 0.57735026f));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(0, 0, 10));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 10, 0))) == Vector(1, 1, 0).Normalized()*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 10))) == Vector(-1, 1, 0).Normalized()*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(0, 14.142136f, 10));
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(0, 1.4142135f, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(-5.6568542f, -4.2426405f, -7, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(-5.6568542f, -4.2426405f+1.4142135f, -6, 1));

	m.Identity();
	n.Identity();
	m.SetReflection(Vector(1, 0, 0));	// Double identical reflection should cancel out
	n.SetReflection(Vector(1, 0, 0));
	// First test that the result matrix is correct
	TAssert((m * n).IsIdentity());
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(1, 0, 0));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(0, 1, 0));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(0, 0, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(1, 0, 0));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(0, 1, 0));
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(0, 0, 1));

	m.Identity();
	n.Identity();
	m.SetReflection(Vector(1, 0, 0));
	n.SetReflection(Vector(0, 1, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(-1, 0, 0), Vector(0, -1, 0), Vector(0, 0, 1)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(-1, 0, 0));
	TAssert(m * (n * Vector(-1, 0, 0)) == Vector(1, 0, 0));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(0, -1, 0));
	TAssert(m * (n * Vector(0, -1, 0)) == Vector(0, 1, 0));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(0, 0, 1));
	TAssert(m * (n * Vector(0, 0, -1)) == Vector(0, 0, -1));
	TAssert(m * (n * Vector(0, 1, 1)) == Vector(0, -1, 1));
	TAssert(m * (n * Vector(0, -1, -1)) == Vector(0, 1, -1));
	TAssert(m * (n * Vector(1, 0, 1)) == Vector(-1, 0, 1));
	TAssert(m * (n * Vector(-1, 0, -1)) == Vector(1, 0, -1));
	TAssert(m * (n * Vector(1, 1, 0)) == Vector(-1, -1, 0));
	TAssert(m * (n * Vector(-1, -1, 0)) == Vector(1, 1, 0));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(-1, -1, 1));
	TAssert(m * (n * Vector(-1, -1, -1)) == Vector(1, 1, -1));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(-1, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0, -1, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(0, 0, 1));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-1, -1, 1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(-10, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0, -10, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(0, 0, 10));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(-10, -10, 10));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(-1, -1, 1, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(-1, -1, 1, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(-1, 0, 0));
	TAssert(n * (m * Vector(-1, 0, 0)) == Vector(1, 0, 0));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(0, -1, 0));
	TAssert(n * (m * Vector(0, -1, 0)) == Vector(0, 1, 0));
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(0, 0, 1));
	TAssert(n * (m * Vector(0, 0, -1)) == Vector(0, 0, -1));
	TAssert(n * (m * Vector(0, 1, 1)) == Vector(0, -1, 1));
	TAssert(n * (m * Vector(0, -1, -1)) == Vector(0, 1, -1));
	TAssert(n * (m * Vector(1, 0, 1)) == Vector(-1, 0, 1));
	TAssert(n * (m * Vector(-1, 0, -1)) == Vector(1, 0, -1));
	TAssert(n * (m * Vector(1, 1, 0)) == Vector(-1, -1, 0));
	TAssert(n * (m * Vector(-1, -1, 0)) == Vector(1, 1, 0));
	TAssert(n * (m * Vector(1, 1, 1)) == Vector(-1, -1, 1));
	TAssert(n * (m * Vector(-1, -1, -1)) == Vector(1, 1, -1));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 0, 0))) == Vector(-1, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 0))) == Vector(0, -1, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 1))) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-1, -1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(-10, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 10, 0))) == Vector(0, -10, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 10))) == Vector(0, 0, 10));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(-10, -10, 10));
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(-1, -1, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(-1, -1, 1, 1));

	m.Identity();
	n.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	m.SetReflection(Vector(1, 0, 0));
	n.SetTranslation(Vector(2, 3, 4));
	n.SetReflection(Vector(0, 1, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(-1, 0, 0), Vector(0, -1, 0), Vector(0, 0, 1), Vector(-1, 5, 7)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(-1, 5, 7));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(-2, 5, 7));
	TAssert(m * (n * Vector(-1, 0, 0)) == Vector(0, 5, 7));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(-1, 4, 7));
	TAssert(m * (n * Vector(0, -1, 0)) == Vector(-1, 6, 7));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(-1, 5, 8));
	TAssert(m * (n * Vector(0, 0, -1)) == Vector(-1, 5, 6));
	TAssert(m * (n * Vector(0, 1, 1)) == Vector(-1, 4, 8));
	TAssert(m * (n * Vector(0, -1, -1)) == Vector(-1, 6, 6));
	TAssert(m * (n * Vector(1, 0, 1)) == Vector(-2, 5, 8));
	TAssert(m * (n * Vector(-1, 0, -1)) == Vector(0, 5, 6));
	TAssert(m * (n * Vector(1, 1, 0)) == Vector(-2, 4, 7));
	TAssert(m * (n * Vector(-1, -1, 0)) == Vector(0, 6, 7));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(-2, 4, 8));
	TAssert(m * (n * Vector(-1, -1, -1)) == Vector(0, 6, 6));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(-1, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0, -1, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(0, 0, 1));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-1, -1, 1).Normalized());
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(-10, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0, -10, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(0, 0, 10));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(-10, -10, 10));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(-1, -1, 1, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(-1, 5, 7, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(-2, 4, 8, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(-1, 5, -7));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(-2, 5, -7));
	TAssert(n * (m * Vector(-1, 0, 0)) == Vector(0, 5, -7));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(-1, 4, -7));
	TAssert(n * (m * Vector(0, -1, 0)) == Vector(-1, 6, -7));
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(-1, 5, -6));
	TAssert(n * (m * Vector(0, 0, -1)) == Vector(-1, 5, -8));
	TAssert(n * (m * Vector(0, 1, 1)) == Vector(-1, 4, -6));
	TAssert(n * (m * Vector(0, -1, -1)) == Vector(-1, 6, -8));
	TAssert(n * (m * Vector(1, 0, 1)) == Vector(-2, 5, -6));
	TAssert(n * (m * Vector(-1, 0, -1)) == Vector(0, 5, -8));
	TAssert(n * (m * Vector(1, 1, 0)) == Vector(-2, 4, -7));
	TAssert(n * (m * Vector(-1, -1, 0)) == Vector(0, 6, -7));
	TAssert(n * (m * Vector(1, 1, 1)) == Vector(-2, 4, -6));
	TAssert(n * (m * Vector(-1, -1, -1)) == Vector(0, 6, -8));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 0, 0))) == Vector(-1, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 0))) == Vector(0, -1, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 1))) == Vector(0, 0, 1));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-1, -1, 1).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(-10, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 10, 0))) == Vector(0, -10, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 10))) == Vector(0, 0, 10));
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10))) == Vector(-10, -10, 10));
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(-1, -1, 1, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(-1, 5, -7, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(-2, 4, -6, 1));

	m.Identity();
	n.Identity();
	m.SetAngles(EAngle(0, 0, 45));
	m.AddReflection(Vector(1, 0, 0));
	n.SetAngles(EAngle(0, -45, 4));
	n.AddReflection(Vector(0, 1, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(-0.70710677f, 0.49999997f, -0.49999997f), Vector(0.049325272f, -0.67050606f, -0.74026257f), Vector(-0.70538431f, -0.54810727f, 0.44945675f)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(-0.70710677f, 0.49999997f, -0.49999997f));
	TAssert(m * (n * Vector(-1, 0, 0)) == Vector(0.70710677f, -0.49999997f, 0.49999997f));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(0.049325272f, -0.67050606f, -0.74026257f));
	TAssert(m * (n * Vector(0, -1, 0)) == Vector(-0.049325272f, 0.67050606f, 0.74026257f));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(-0.70538431f, -0.54810727f, 0.44945675f));
	TAssert(m * (n * Vector(0, 0, -1)) == Vector(0.70538431f, 0.54810727f, -0.44945675f));
	TAssert(m * (n * Vector(0, 1, 1)) == Vector(-0.65605903f, -1.2186134f, -0.29080585f));
	TAssert(m * (n * Vector(0, -1, -1)) == Vector(0.65605903f, 1.2186134f, 0.29080585f));
	TAssert(m * (n * Vector(1, 0, 1)) == Vector(-1.4124911f, -0.048107311f, -0.050543234f));
	TAssert(m * (n * Vector(-1, 0, -1)) == Vector(1.4124911f, 0.048107311f, 0.050543234f));
	TAssert(m * (n * Vector(1, 1, 0)) == Vector(-0.65778148f, -0.17050609f, -1.2402625f));
	TAssert(m * (n * Vector(-1, -1, 0)) == Vector(0.65778148f, 0.17050609f, 1.2402625f));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(-1.3631659f, -0.71861345f, -0.79080582f));
	TAssert(m * (n * Vector(-1, -1, -1)) == Vector(1.3631659f, 0.71861345f, 0.79080582f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(-0.70710677f, 0.49999997f, -0.49999997f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0.049325272f, -0.67050606f, -0.74026257f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(-0.70538431f, -0.54810727f, 0.44945675f));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-0.78702414f, -0.41489163f, -0.45657191f));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(-0.70710677f, 0.49999997f, -0.49999997f)*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0.049325272f, -0.67050606f, -0.74026257f)*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(-0.70538431f, -0.54810727f, 0.44945675f)*10);
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10).Normalized()*10)) == Vector(-0.78702414f, -0.41489163f, -0.45657191f)*10);
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(-1.3631659f, -0.71861345f, -0.79080582f, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(-1.3631659f, -0.71861345f, -0.79080582f, 1));

	m.InvertRT();
	n.InvertRT();
	// Global to local
	TAssert(n * (m * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(n * (m * Vector(1, 0, 0)) == Vector(-0.70710677f, 0.049325272f, -0.70538431f));
	TAssert(n * (m * Vector(0, 1, 0)) == Vector(0.49999997f, -0.67050606f, -0.54810727f));
	TAssert(n * (m * Vector(0, 0, 1)) == Vector(-0.49999997f, -0.74026257f, 0.44945675f));
	TAssert(n * (m * Vector(1, 1, 1)) == Vector(-0.70710677f, -1.3614434f, -0.80403489f));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 0, 0).Normalized())) == Vector(-0.70710677f, 0.049325272f, -0.70538431f));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 1, 0).Normalized())) == Vector(0.49999997f, -0.67050606f, -0.54810727f));
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 1).Normalized())) == Vector(-0.49999997f, -0.74026257f, 0.44945675f));
	TAssert(n.TransformVector(m.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-0.70710677f, -1.3614434f, -0.80403489f).Normalized());
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 0, 0))) == Vector(-0.70710677f, 0.049325272f, -0.70538431f)*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 10, 0))) == Vector(0.49999997f, -0.67050606f, -0.54810727f)*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(0, 0, 10))) == Vector(-0.49999997f, -0.74026257f, 0.44945675f)*10);
	TAssert(n.TransformVector(m.TransformVector(Vector(10, 10, 10).Normalized()*10)) == Vector(-0.70710677f, -1.3614434f, -0.80403489f).Normalized()*10);
	TAssert(n * (m * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(n * (m * Vector4D(1, 1, 1, 0)) == Vector4D(-0.70710677f, -1.3614434f, -0.80403489f, 0));
	TAssert(n * (m * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(n * (m * Vector4D(1, 1, 1, 1)) == Vector4D(-0.70710677f, -1.3614434f, -0.80403489f, 1));

	m.Identity();
	n.Identity();
	m.SetScale(Vector(2, 2, 2));
	m.AddReflection(Vector(1, 0, 0));
	n.SetScale(Vector(3, 1, 1));
	n.AddReflection(Vector(0, 1, 0));
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(-6, 0, 0), Vector(0, -2, 0), Vector(0, 0, 2)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(0, 0, 0));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(-6, 0, 0));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(0, -2, 0));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(0, 0, 2));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(-6, -2, 2));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(-6, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(0, -2, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(0, 0, 2));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(-3.4641016f, -1.1547005f, 1.1547005f));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(-60, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(0, -20, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(0, 0, 20));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10))) == Vector(-60, -20, 20));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(-6, -2, 2, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(0, 0, 0, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(-6, -2, 2, 1));
	// Can't test inverse since there's no valid matrix inversion function

	m.Identity();
	n.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	m.SetAngles(EAngle(45, -90, 0));
	m.AddScale(Vector(2, 2, 2));
	m.AddReflection(Vector(1, 0, 0));
	n.SetTranslation(Vector(2, 3, 4));
	n.SetAngles(EAngle(40, -180, 0));
	n.AddScale(Vector(1, 0.5f, 1));
	n.AddReflection(Vector(0, 1, 1).Normalized());
	// First test that the result matrix is correct
	TAssert((m * n) == Matrix4x4(Vector(0, 1.9923894f, -0.17431144f), Vector(2, 0, 0), Vector(0, -0.087155722f, -0.99619460f), Vector(9, 3.4142137f, 10.071068f)));
	// Local to global
	TAssert(m * (n * Vector(0, 0, 0)) == Vector(9, 3.4142137f, 10.071068f));
	TAssert(m * (n * Vector(1, 0, 0)) == Vector(9, 5.4066029f, 9.8967562f));
	TAssert(m * (n * Vector(0, 1, 0)) == Vector(11, 3.4142137f, 10.071067f));
	TAssert(m * (n * Vector(0, 0, 1)) == Vector(9, 3.3270576f, 9.0748730f));
	TAssert(m * (n * Vector(1, 1, 1)) == Vector(11, 5.3194470f, 8.9005613f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 0))) == Vector(0, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 0, 0))) == Vector(0, 1.9923894f, -0.17431144f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 1, 0))) == Vector(2, 0, 0));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 1))) == Vector(0, -0.087155722f, -0.99619460f));
	TAssert(m.TransformVector(n.TransformVector(Vector(1, 1, 1).Normalized())) == Vector(1.1547005f, 1.0999871f, -0.67579198f));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 0, 0))) == Vector(.0000012632007f, 19.923893f, -1.7431140f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 10, 0))) == Vector(19.999998f, -0.0000012065125f, 0.00000070309989f));
	TAssert(m.TransformVector(n.TransformVector(Vector(0, 0, 10))) == Vector(-5.5807857e-007f, -0.87155735f, -9.9619455f));
	TAssert(m.TransformVector(n.TransformVector(Vector(10, 10, 10).Normalized()*10)) == Vector(11.547006f, 10.999871f, -6.7579193f));
	TAssert(m * (n * Vector4D(0, 0, 0, 0)) == Vector4D(0, 0, 0, 0));
	TAssert(m * (n * Vector4D(1, 1, 1, 0)) == Vector4D(2, 1.9052335f, -1.1705060f, 0));
	TAssert(m * (n * Vector4D(0, 0, 0, 1)) == Vector4D(9, 3.4142137f, 10.071068f, 1));
	TAssert(m * (n * Vector4D(1, 1, 1, 1)) == Vector4D(11, 5.3194470f, 8.9005613f, 1));
	// Can't test inverse since there's no valid matrix inversion function

	// Check that TransformNormal() doesn't interfere with the vector length.
	m.Identity();
	n.Identity();
	m.SetTranslation(Vector(1, 2, 3));
	m.SetAngles(EAngle(45, -90, 0));
	m.AddReflection(Vector(1, 0, 0));
	n.SetTranslation(Vector(2, 3, 4));
	n.SetAngles(EAngle(40, -180, 0));
	n.AddReflection(Vector(0, 1, 1).Normalized());
	TAssert(fabs(n.TransformVector(Vector(10, 0, 0)).Length() - 10) < 0.00001f);
	TAssert(fabs(n.TransformVector(Vector(1, 1, 0).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs(n.TransformVector(Vector(1, 1, 1).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(Vector(10, 0, 0)).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(Vector(1, 1, 0).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(Vector(1, 1, 1).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs((m*n).TransformVector(Vector(10, 0, 0)).Length() - 10) < 0.00001f);
	TAssert(fabs((m*n).TransformVector(Vector(1, 1, 0).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs((m*n).TransformVector(Vector(1, 1, 1).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(n.TransformVector(Vector(10, 0, 0))).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(n.TransformVector(Vector(1, 1, 0)).Normalized()*10).Length() - 10) < 0.00001f);
	TAssert(fabs(m.TransformVector(n.TransformVector(Vector(1, 1, 1)).Normalized()*10).Length() - 10) < 0.00001f);
}
