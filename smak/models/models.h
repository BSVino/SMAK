/*
SMAK - The Super Model Army Knife
Copyright (C) 2012, Lunar Workshop, Inc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include <tvector.h>
#include <color.h>
#include <geometry.h>
#include <tstring.h>

#include <textures/materialhandle.h>

class CModel
{
public:
							CModel();
							~CModel();

public:
	bool					Load(class CConversionScene* pScene, size_t iMesh);
	size_t					LoadBufferIntoGL(size_t iMaterial);

	void					Unload();
	void					UnloadBufferFromGL(size_t iBuffer);

	size_t					FloatsPerVertex() { return 14; }
	size_t					Stride() { return FloatsPerVertex()*sizeof(float); }
	size_t					PositionOffset() { return 0; }
	size_t					NormalsOffset() { return 3*sizeof(float); }
	size_t					TangentsOffset() { return 6*sizeof(float); }
	size_t					BitangentsOffset() { return 9*sizeof(float); }
	size_t					TexCoordOffset() { return 12*sizeof(float); }

	size_t					FloatsPerWireframeVertex() { return 6; }
	size_t					WireframeStride() { return FloatsPerWireframeVertex()*sizeof(float); }
	size_t					WireframePositionOffset() { return 0; }
	size_t					WireframeNormalsOffset() { return 3*sizeof(float); }

	size_t					FloatsPerUVVertex() { return 2; }
	size_t					UVStride() { return FloatsPerUVVertex()*sizeof(float); }
	size_t					UVPositionOffset() { return 0; }

public:
	size_t					m_iReferences;

	tvector<tstring>		m_asMaterialStubs;
	tvector<size_t>			m_aiVertexBuffers;
	tvector<size_t>			m_aiVertexBufferSizes;	// How many vertices in this vertex buffer?
	size_t					m_iVertexWireframeBuffer;
	size_t					m_iVertexWireframeBufferSize;
	size_t					m_iVertexUVBuffer;
	size_t					m_iVertexUVBufferSize;

	AABB					m_aabbBoundingBox;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	static size_t			GetNumModelsLoaded() { return Get()->m_iModelsLoaded; }

	static void				AddModel(class CConversionScene* pScene, size_t iMesh);
	static CModel*			GetModel(size_t i);
	static void				ReleaseModel(size_t i);
	static void				UnloadModel(size_t i);

	static void				ResetReferenceCounts();
	static void				ClearUnreferenced();

	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	tvector<CModel*>		m_apModels;
	size_t					m_iModelsLoaded;

private:
	static CModelLibrary*	s_pModelLibrary;
};
