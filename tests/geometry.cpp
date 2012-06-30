#include <stdio.h>

#include <common.h>
#include <tstring.h>
#include <mesh.h>
#include <tvector.h>

#include <tinker/shell.h>

void test_mesh()
{
	{
		// Triangular base
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, 0));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(0, 1, 0));

		CConvexHullGenerator c(avecPoints);
		auto aiTriangles = c.GetConvexTriangles();
		TAssert(aiTriangles.size() % 3 == 0);
		TAssert(aiTriangles.size()/3 == 4);
	}

	{
		// Square base
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, 1, 0));

		CConvexHullGenerator c(avecPoints);
		auto aiTriangles = c.GetConvexTriangles();
		TAssert(aiTriangles.size() % 3 == 0);
		TAssert(aiTriangles.size()/3 == 6);
	}

	{
		// Extra vert on the inside
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, 1, 0));
		avecPoints.push_back(Vector(0, 0, 0));

		CConvexHullGenerator c(avecPoints);
		auto aiTriangles = c.GetConvexTriangles();
		TAssert(aiTriangles.size() % 3 == 0);
		TAssert(aiTriangles.size()/3 == 6);
	}

	{
		// Adding an extra vert
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, 1, 0));
		avecPoints.push_back(Vector(0, -1, -1.5f));

		CConvexHullGenerator c(avecPoints);
		auto aiTriangles = c.GetConvexTriangles();
		TAssert(aiTriangles.size() % 3 == 0);
		TAssert(aiTriangles.size()/3 == 8);
	}

	{
		// Single tri. Nothing to remove.
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size() == 3);
		TAssert(aiTriangles[0] != aiTriangles[1]);
		TAssert(aiTriangles[0] != aiTriangles[2]);
		TAssert(aiTriangles[1] != aiTriangles[2]);
	}

	{
		// Single quad. Nothing to remove.
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(0);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size() == 6);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Three tri strip. Nothing to remove.
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 2));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(0);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		aiTriangles.push_back(3);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 3);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Four tri strip. Nothing to remove.
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 2));
		avecPoints.push_back(Vector(1, -1, 2));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(0);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		aiTriangles.push_back(3);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		aiTriangles.push_back(3);
		aiTriangles.push_back(4);
		aiTriangles.push_back(5);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 4);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Four tri fan, not continuous. Nothing to remove.
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 2));
		avecPoints.push_back(Vector(1, -1, 2));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);

		aiTriangles.push_back(3);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(3);
		aiTriangles.push_back(2);
		aiTriangles.push_back(5);

		aiTriangles.push_back(3);
		aiTriangles.push_back(5);
		aiTriangles.push_back(4);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 4);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Three tris, one removable coplanar point #1 (Removable point third in winding)
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(0, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(3);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		aiTriangles.push_back(2);
		aiTriangles.push_back(0);
		aiTriangles.push_back(3);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Three tris, one removable coplanar point #2 (Removable point first in winding)
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(0, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);

		aiTriangles.push_back(3);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(3);
		aiTriangles.push_back(2);
		aiTriangles.push_back(0);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Three tris, one removable coplanar point #3 (Removable point second in winding)
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(0, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(1);
		aiTriangles.push_back(3);
		aiTriangles.push_back(0);

		aiTriangles.push_back(2);
		aiTriangles.push_back(3);
		aiTriangles.push_back(1);

		aiTriangles.push_back(0);
		aiTriangles.push_back(3);
		aiTriangles.push_back(2);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Three tris, one removable coplanar point #4 (Removable point altenating in winding)
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(0, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(1);
		aiTriangles.push_back(3);
		aiTriangles.push_back(0);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		aiTriangles.push_back(3);
		aiTriangles.push_back(2);
		aiTriangles.push_back(0);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// One removable coplanar point
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(4);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		aiTriangles.push_back(2);
		aiTriangles.push_back(3);
		aiTriangles.push_back(4);

		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(4);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 2);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// One non-coplanar point, none removable
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, 1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		aiTriangles.push_back(0);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(4);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		aiTriangles.push_back(2);
		aiTriangles.push_back(3);
		aiTriangles.push_back(4);

		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(4);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 6);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// One coplanar and one non coplanar point, one removable
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(-1, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));
		avecPoints.push_back(Vector(0, 1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(4);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		aiTriangles.push_back(2);
		aiTriangles.push_back(3);
		aiTriangles.push_back(4);

		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(4);

		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(5);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(5);

		aiTriangles.push_back(2);
		aiTriangles.push_back(3);
		aiTriangles.push_back(5);

		aiTriangles.push_back(3);
		aiTriangles.push_back(0);
		aiTriangles.push_back(5);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 6);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// Five tris, two removable coplanar points
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(0, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));
		avecPoints.push_back(Vector(0, -1, 0.5f));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(1);
		aiTriangles.push_back(3);
		aiTriangles.push_back(0);

		aiTriangles.push_back(1);
		aiTriangles.push_back(4);
		aiTriangles.push_back(3);

		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(4);

		aiTriangles.push_back(2);
		aiTriangles.push_back(0);
		aiTriangles.push_back(4);

		aiTriangles.push_back(4);
		aiTriangles.push_back(0);
		aiTriangles.push_back(3);

		CCoplanarPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// One unused vertex
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(0);
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);

		CUnusedPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		TAssert(avecPoints.size() == 3);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}

	{
		// One unused vertex
		tvector<Vector> avecPoints;
		avecPoints.push_back(Vector(-1, -1, -1));
		avecPoints.push_back(Vector(1, -1, -1));
		avecPoints.push_back(Vector(1, -1, 1));
		avecPoints.push_back(Vector(0, -1, 0));

		tvector<size_t> aiTriangles;
		aiTriangles.push_back(1);
		aiTriangles.push_back(2);
		aiTriangles.push_back(3);

		CUnusedPointOptimizer::OptimizeMesh(avecPoints, aiTriangles);

		TAssert(aiTriangles.size()/3 == 1);
		TAssert(avecPoints.size() == 3);
		for (size_t i = 0; i < aiTriangles.size(); i += 3)
		{
			TAssert(aiTriangles[i+0] != aiTriangles[i+1]);
			TAssert(aiTriangles[i+0] != aiTriangles[i+2]);
			TAssert(aiTriangles[i+1] != aiTriangles[i+2]);
		}
	}
}
