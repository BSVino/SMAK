#include <stdio.h>
#include <memory>

#include <tinker/shell.h>

#include <toys/toy_util.h>
#include <toys/toy.h>

void test_toy()
{
	CToyUtil t;

	t.AddMaterial("test.mat");
	t.AddMaterial("test2.mat");
	t.AddMaterial("test0.mat");
	t.AddMaterial("test3.mat");
	t.AddVertex(0, Vector(1, 2, 3), Vector2D(0, 0));
	t.AddVertex(0, Vector(1, 3, 3), Vector2D(0, 1));
	t.AddVertex(0, Vector(1, 2, 4), Vector2D(1, 1));
	t.AddVertex(0, Vector(2, 2, 3), Vector2D(1, 0));
	t.AddVertex(0, Vector(2, 3, 3), Vector2D(1, 1));
	t.AddVertex(0, Vector(2, 2, 4), Vector2D(1, 0));
	t.AddVertex(1, Vector(10, 20, 30), Vector2D(0, 0));
	t.AddVertex(1, Vector(10, 30, 30), Vector2D(0, 10));
	t.AddVertex(1, Vector(10, 20, 40), Vector2D(10, 10));
	t.AddVertex(1, Vector(20, 20, 30), Vector2D(10, 0));
	t.AddVertex(1, Vector(20, 30, 30), Vector2D(10, 10));
	t.AddVertex(1, Vector(20, 20, 40), Vector2D(10, 0));
	t.AddVertex(1, Vector(11, 12, 13), Vector2D(10, 10));
	t.AddVertex(1, Vector(11, 13, 13), Vector2D(10, 11));
	t.AddVertex(1, Vector(11, 12, 14), Vector2D(11, 11));
	// Skip the third, it has no verts and should get op'd out.
	t.AddVertex(3, Vector(12, 12, 13), Vector2D(11, 10));
	t.AddVertex(3, Vector(12, 13, 13), Vector2D(11, 11));
	t.AddVertex(3, Vector(12, 12, 14), Vector2D(11, 10));
	t.AddPhysVertex(Vector(0, 0, 1));
	t.AddPhysVertex(Vector(0, 1, 1));
	t.AddPhysVertex(Vector(0, 1, 0));
	t.AddPhysVertex(Vector(0, 0, 0));
	t.AddPhysTriangle(0, 1, 2);
	t.AddPhysTriangle(0, 2, 3);
	t.AddPhysBox(TRS(Vector(11, 12, 13), EAngle(0, 0, 0), Vector(7, 8, 9)));
	t.AddPhysBox(TRS(Vector(9, 8, 7), EAngle(0, 0, 0), Vector(3, 2, 1)));
	bool bWrite = t.Write("test.toy");
	TAssert(bWrite);

	CToy* pToy = new CToy();

	bool bRead = CToyUtil::Read("test.toy", pToy);
	TAssert(bRead);
	TAssert(strcmp(pToy->GetMaterialName(0), "test.mat") == 0);
	TAssert(strcmp(pToy->GetMaterialName(1), "test2.mat") == 0);
	TAssert(strcmp(pToy->GetMaterialName(2), "test3.mat") == 0);
	TAssert(Vector(pToy->GetMaterialVert(0, 0)) == Vector(1, 2, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 0)+3) == Vector2D(0, 0));
	TAssert(Vector(pToy->GetMaterialVert(0, 1)) == Vector(1, 3, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 1)+3) == Vector2D(0, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 2)) == Vector(1, 2, 4));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 2)+3) == Vector2D(1, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 3)) == Vector(2, 2, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 3)+3) == Vector2D(1, 0));
	TAssert(Vector(pToy->GetMaterialVert(0, 4)) == Vector(2, 3, 3));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 4)+3) == Vector2D(1, 1));
	TAssert(Vector(pToy->GetMaterialVert(0, 5)) == Vector(2, 2, 4));
	TAssert(Vector2D(pToy->GetMaterialVert(0, 5)+3) == Vector2D(1, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 0)) == Vector(10, 20, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 0)+3) == Vector2D(0, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 1)) == Vector(10, 30, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 1)+3) == Vector2D(0, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 2)) == Vector(10, 20, 40));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 2)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 3)) == Vector(20, 20, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 3)+3) == Vector2D(10, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 4)) == Vector(20, 30, 30));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 4)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 5)) == Vector(20, 20, 40));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 5)+3) == Vector2D(10, 0));
	TAssert(Vector(pToy->GetMaterialVert(1, 6)) == Vector(11, 12, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 6)+3) == Vector2D(10, 10));
	TAssert(Vector(pToy->GetMaterialVert(1, 7)) == Vector(11, 13, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 7)+3) == Vector2D(10, 11));
	TAssert(Vector(pToy->GetMaterialVert(1, 8)) == Vector(11, 12, 14));
	TAssert(Vector2D(pToy->GetMaterialVert(1, 8)+3) == Vector2D(11, 11));
	TAssert(Vector(pToy->GetMaterialVert(2, 0)) == Vector(12, 12, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 0)+3) == Vector2D(11, 10));
	TAssert(Vector(pToy->GetMaterialVert(2, 1)) == Vector(12, 13, 13));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 1)+3) == Vector2D(11, 11));
	TAssert(Vector(pToy->GetMaterialVert(2, 2)) == Vector(12, 12, 14));
	TAssert(Vector2D(pToy->GetMaterialVert(2, 2)+3) == Vector2D(11, 10));

	pToy->DeallocateMesh();

	TAssert(pToy->GetMaterialNumVerts(0) == 6);
	TAssert(pToy->GetMaterialNumVerts(1) == 9);
	TAssert(pToy->GetMaterialNumVerts(2) == 3);
	TAssert(pToy->GetAABB().m_vecMins == Vector(1, 2, 3));
	TAssert(pToy->GetAABB().m_vecMaxs == Vector(20, 30, 40));
	TAssert(pToy->GetNumMaterials() == 3);

	TAssert(pToy->GetPhysicsNumTris() == 2);
	TAssert(pToy->GetPhysicsNumVerts() == 4);
	TAssert(pToy->GetPhysicsTri(0)[0] == 0);
	TAssert(pToy->GetPhysicsTri(0)[1] == 1);
	TAssert(pToy->GetPhysicsTri(0)[2] == 2);
	TAssert(pToy->GetPhysicsTri(1)[0] == 0);
	TAssert(pToy->GetPhysicsTri(1)[1] == 2);
	TAssert(pToy->GetPhysicsTri(1)[2] == 3);
	TAssert(Vector(pToy->GetPhysicsVert(0)) == Vector(0, 0, 1));
	TAssert(Vector(pToy->GetPhysicsVert(1)) == Vector(0, 1, 1));
	TAssert(Vector(pToy->GetPhysicsVert(2)) == Vector(0, 1, 0));
	TAssert(Vector(pToy->GetPhysicsVert(3)) == Vector(0, 0, 0));

	TAssert(pToy->GetPhysicsNumBoxes() == 2);
	TAssert(pToy->GetPhysicsBox(0).m_vecTranslation == Vector(11, 12, 13));
	TAssert(pToy->GetPhysicsBox(0).m_angRotation == EAngle(0, 0, 0));
	TAssert(pToy->GetPhysicsBox(0).m_vecScaling == Vector(7, 8, 9));
	TAssert(pToy->GetPhysicsBox(1).m_vecTranslation == Vector(9, 8, 7));
	TAssert(pToy->GetPhysicsBox(1).m_angRotation == EAngle(0, 0, 0));
	TAssert(pToy->GetPhysicsBox(1).m_vecScaling == Vector(3, 2, 1));

	delete pToy;
}

void test_scene()
{
	CToyUtil t1;
	t1.AddMaterial("test.mat");
	t1.AddVertex(0, Vector(1, 2, 3), Vector2D(0, 0));
	t1.AddVertex(0, Vector(1, 3, 3), Vector2D(0, 1));
	t1.AddVertex(0, Vector(1, 2, 4), Vector2D(1, 1));
	t1.AddVertex(0, Vector(2, 2, 3), Vector2D(1, 0));
	t1.AddVertex(0, Vector(2, 3, 3), Vector2D(1, 1));
	t1.AddVertex(0, Vector(2, 2, 4), Vector2D(1, 0));
	bool bWrite = t1.Write("testscene1.toy");
	TAssert(bWrite);

	CToyUtil t2;
	t2.AddMaterial("test.mat");
	t2.AddVertex(0, Vector(10, 2, 3), Vector2D(0, 0));
	t2.AddVertex(0, Vector(10, 3, 3), Vector2D(0, 1));
	t2.AddVertex(0, Vector(10, 2, 4), Vector2D(1, 1));
	t2.AddVertex(0, Vector(20, 2, 3), Vector2D(1, 0));
	t2.AddVertex(0, Vector(20, 3, 3), Vector2D(1, 1));
	t2.AddVertex(0, Vector(20, 2, 4), Vector2D(1, 0));
	bWrite = t2.Write("testscene2.toy");
	TAssert(bWrite);

	CToyUtil t3;
	t3.AddMaterial("test.mat");
	t3.AddVertex(0, Vector(1, 20, 3), Vector2D(0, 0));
	t3.AddVertex(0, Vector(1, 30, 3), Vector2D(0, 1));
	t3.AddVertex(0, Vector(1, 20, 4), Vector2D(1, 1));
	t3.AddVertex(0, Vector(2, 20, 3), Vector2D(1, 0));
	t3.AddVertex(0, Vector(2, 30, 3), Vector2D(1, 1));
	t3.AddVertex(0, Vector(2, 20, 4), Vector2D(1, 0));
	bWrite = t3.Write("testscene3.toy");
	TAssert(bWrite);

	CToyUtil ts;
	ts.SetGameDirectory(".");
	size_t iSceneArea1 = ts.AddSceneArea("testscene1.toy");
	size_t iSceneArea2 = ts.AddSceneArea("testscene2.toy");
	size_t iSceneArea3 = ts.AddSceneArea("testscene3.toy");
	ts.AddSceneAreaNeighbor(iSceneArea1, iSceneArea2);
	ts.AddSceneAreaNeighbor(iSceneArea2, iSceneArea1);
	ts.AddSceneAreaNeighbor(iSceneArea2, iSceneArea3);
	ts.AddSceneAreaNeighbor(iSceneArea3, iSceneArea2);
	bWrite = ts.Write("testscenes.toy");
	TAssert(bWrite);

	CToy* pToy = new CToy();

	bool bRead = CToyUtil::Read("testscenes.toy", pToy);
	TAssert(bRead);
	TAssert(pToy->GetNumSceneAreas() == 3);

	TAssert(tstring(pToy->GetSceneAreaFileName(0)) == tstring("testscene1.toy"));
	TAssert(tstring(pToy->GetSceneAreaFileName(1)) == tstring("testscene2.toy"));
	TAssert(tstring(pToy->GetSceneAreaFileName(2)) == tstring("testscene3.toy"));
	TAssert(pToy->GetSceneAreaAABB(0) == t1.GetBounds());
	TAssert(pToy->GetSceneAreaAABB(1) == t2.GetBounds());
	TAssert(pToy->GetSceneAreaAABB(2) == t3.GetBounds());
	TAssert(pToy->GetSceneAreaNumVisible(0) == 2);
	TAssert(pToy->GetSceneAreaNumVisible(1) == 3);
	TAssert(pToy->GetSceneAreaNumVisible(2) == 2);
	TAssert(pToy->GetSceneAreasVisible(0, 0) == 0);	// Is visible to itself
	TAssert(pToy->GetSceneAreasVisible(0, 1) == 1);	// Is visible to its neighbor
	TAssert(pToy->GetSceneAreasVisible(1, 0) == 0);	// Is visible to its neighbor
	TAssert(pToy->GetSceneAreasVisible(1, 1) == 1);	// Is visible to itself
	TAssert(pToy->GetSceneAreasVisible(1, 2) == 2);	// Is visible to its neighbor
	TAssert(pToy->GetSceneAreasVisible(2, 0) == 1);	// Is visible to its neighbor
	TAssert(pToy->GetSceneAreasVisible(2, 1) == 2);	// Is visible to itself

	delete pToy;
}

void test_toys()
{
	test_toy();
	test_scene();
}
