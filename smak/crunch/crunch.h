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

#include <parallelize.h>
#include <worklistener.h>
#include <common.h>

#include <modelconverter/convmesh.h>
#include <textures/texturehandle.h>
#include <tinker/renderer/renderer.h>

#include "ui/smakwindow.h"

namespace raytrace
{
	class CRaytracer;
	class CTraceResult;
};

class CTexelMethod
{
public:
							CTexelMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			PreGenerate() {};

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer) {};

	virtual void			PostGenerate() {};

	virtual CTextureHandle	GenerateDiffuse(bool bInMedias = false) { return CTextureHandle(); };
	virtual CTextureHandle	GenerateNormal(bool bInMedias = false) { return CTextureHandle(); };
	virtual CTextureHandle	GenerateAO(bool bInMedias = false) { return CTextureHandle(); };

	virtual void			SaveToFile(const tstring& sFilename);

	virtual tstring			FileSuffix() { return ""; };
	virtual Vector*			GetData() { return NULL; };

protected:
	CTexelGenerator*		m_pGenerator;

	size_t					m_iWidth;
	size_t					m_iHeight;
};

class CTexelDiffuseMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelDiffuseMethod, CTexelMethod);

public:
							CTexelDiffuseMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelDiffuseMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			PreGenerate();

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	virtual CTextureHandle	GenerateDiffuse(bool bInMedias = false);

	virtual tstring			FileSuffix() { return "diffuse"; };
	virtual Vector*			GetData();

protected:
	Vector*					m_avecDiffuseValues;
	Vector*					m_avecDiffuseGeneratedValues;
	size_t*					m_aiDiffuseReads;

	class CTexture
	{
	public:
		Color*				m_pclrData;
		int					m_iWidth;
		int					m_iHeight;
	};

	tvector<CTexture>		m_aTextures;
};

class CTexelAOMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelAOMethod, CTexelMethod);

public:
							CTexelAOMethod(class CTexelGenerator* pGenerator, size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed);
	virtual 				~CTexelAOMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	virtual CTextureHandle	GenerateAO(bool bInMedias = false);

	virtual tstring	FileSuffix() { return "ao"; };
	virtual Vector*			GetData();

protected:
	size_t					m_iSamples;
	bool					m_bRandomize;
	float					m_flRayFalloff;
	bool					m_bGroundOcclusion;
	size_t					m_iBleed;

	Vector*					m_avecShadowValues;
	Vector*					m_avecShadowGeneratedValues;
	size_t*					m_aiShadowReads;
};

class CTexelNormalMethod : public CTexelMethod
{
	DECLARE_CLASS(CTexelNormalMethod, CTexelMethod);

public:
							CTexelNormalMethod(class CTexelGenerator* pGenerator);
	virtual 				~CTexelNormalMethod();

public:
	virtual void			SetSize(size_t iWidth, size_t iHeight);

	virtual void			GenerateTexel(size_t iTexel, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, class raytrace::CTraceResult* tr, const Vector& vecUVPosition, class raytrace::CRaytracer* pTracer);

	virtual void			PostGenerate();
	void					Bleed();

	void					TexturizeValues(Vector* avecTexture);
	virtual CTextureHandle	GenerateNormal(bool bInMedias = false);

	virtual void			SaveToFile(const tstring& sFilename);

	virtual tstring	FileSuffix() { return "normal"; };
	virtual Vector*			GetData();

protected:
	Vector*					m_avecNormalValues;
	Vector*					m_avecNormalGeneratedValues;
};

// Accepts hi and lo res models, bakes the hi to the low res using any of a multitude of mix-and-matchable generators.
// For example, can make an ao + normal or just ao or just normal from hi to lo in one pass.
// All generation in CTexelGenerator works on a texel basis, each triangle is broken down into texels and computed one texel at a time
// by tracing to find the hi res mesh and then doing whatever calculations are needed for that method.
class CTexelGenerator
{
public:
							CTexelGenerator(CConversionScene* pScene);
							~CTexelGenerator();

public:
	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetModels(const tvector<CConversionMeshInstance*>& apHiRes, const tvector<CConversionMeshInstance*>& apLoRes);

	void					ClearMethods();
	void					AddDiffuse();
	void					AddAO(size_t iSamples, bool bRandomize, float flRayFalloff, bool bGroundOcclusion, size_t iBleed);
	void					AddNormal();

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };
	IWorkListener*			GetWorkListener() { return m_pWorkListener; };

	void					Generate();
	void					GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, class raytrace::CRaytracer* pTracer, size_t& iRendered);
	void					FindHiResMeshLocation(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, size_t i, size_t j, raytrace::CRaytracer* pTracer);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);
	bool					Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask = NULL);

	CParallelizer*			GetParallelizer() { return m_pWorkParallelizer; }
	CConversionScene*		GetScene() { return m_pScene; }

	void					MarkTexelUsed(size_t iTexel) { m_abTexelMask[iTexel] = true; }

	CTextureHandle			GenerateDiffuse(bool bInMedias = false);
	CTextureHandle			GenerateAO(bool bInMedias = false);
	CTextureHandle			GenerateNormal(bool bInMedias = false);

	void					SaveAll(const tstring& sFilename);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }
	bool					IsStopped() { return m_bStopGenerating; }

	const tvector<CConversionMeshInstance*>&	GetHiResMeshInstances() { return m_apHiRes; }
	const tvector<CConversionMeshInstance*>&	GetLoResMeshInstances() { return m_apLoRes; }

protected:
	CConversionScene*		m_pScene;

	tvector<CConversionMeshInstance*>	m_apHiRes;
	tvector<CConversionMeshInstance*>	m_apLoRes;

	tvector<CTexelMethod*>	m_apMethods;

	size_t					m_iWidth;
	size_t					m_iHeight;

	IWorkListener*			m_pWorkListener;

	bool*					m_abTexelMask;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;

	CParallelizer*			m_pWorkParallelizer;
};
