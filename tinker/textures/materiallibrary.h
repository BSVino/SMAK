/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <tmap.h>

#include <tstring.h>
#include <vector.h>
#include <matrix.h>

#include <tinker/renderer/shaders.h>

#include "materialhandle.h"
#include "texturehandle.h"
#include "texturelibrary.h"

class CMaterial
{
public:
	CMaterial()
	{
		m_iReferences = 0; 
	}

public:
	void Clear()
	{
		m_ahTextures.clear();
		m_aParameters.clear();

		// Don't clear references, there's probably open handles to this material.
	}

	void			Save() const;
	void			Reload();

	size_t			FindParameter(const tstring& sParameterName, bool bCreate = false);
	void			SetParameter(const tstring& sParameterName, const CTextureHandle& hTexture);
	void			SetParameter(const tstring& sParameterName, const tstring& sValue);
	void			SetParameter(const tstring& sParameterName, const Vector& vecValue);
	void			SetParameter(const tstring& sParameterName, float flValue);
	void			FillParameter(size_t iParameter, const tstring& sData, class CShader* pShader=nullptr);

public:
	tstring			m_sFile;
	tstring			m_sTextureDirectory;

	size_t			m_iReferences;

	tstring			m_sShader;
	tstring			m_sBlend;

	class CParameter
	{
	public:
		CParameter()
		{
			m_iValue = 0;
			m_pShaderParameter = nullptr;
		}

	public:
		tstring		m_sName;
		union
		{
			float		m_flValue;
			int			m_iValue;
			bool		m_bValue;
		};
		Vector2D	m_vec2Value;
		Vector		m_vecValue;
		Vector4D	m_vec4Value;
		tstring		m_sValue;

		tstring		m_sType;

		CShader::CParameter*	m_pShaderParameter;

	public:
		void		SetValue(const tstring& sValue, class CShader* pShader);
	};

	tvector<CParameter>		m_aParameters;
	tvector<CTextureHandle>	m_ahTextures;

	class CShader*			m_pShader;
};

class CMaterialLibrary
{
public:
							CMaterialLibrary();
							~CMaterialLibrary();

public:
	static size_t			GetNumMaterials() { return Get()->m_aMaterials.size(); }

	static CMaterialHandle	AddMaterial(const tstring& sMaterial, int iClamp = 0);
	static CMaterialHandle	AddMaterial(const class CData* pData, const tstring& sMaterial="");
	static CMaterial*		AddAsset(const tstring& sMaterial, int iClamp = 0);
	static CMaterial*		CreateMaterial(const class CData* pData, const tstring& sMaterial="");
	static CMaterialHandle	FindAsset(const tstring& sMaterial);

	static size_t			GetNumMaterialsLoaded() { return Get()->m_aMaterials.size(); };
	static bool				IsAssetLoaded(const tstring& sMaterial);

	static void				ClearUnreferenced();

	static CMaterialLibrary*	Get() { return s_pMaterialLibrary; };

protected:
	tmap<tstring, CMaterial>	m_aMaterials;

private:
	static CMaterialLibrary*	s_pMaterialLibrary;
};

#include <tinker/assethandle_functions.h>
