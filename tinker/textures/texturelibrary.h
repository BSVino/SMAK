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

#ifndef DT_TEXTURE_LIBRARY_H
#define DT_TEXTURE_LIBRARY_H

#include <tmap.h>
#include <tstring.h>
#include <vector.h>

#include "texturehandle.h"

class CTexture
{
public:
	CTexture()
	{
		m_iReferences = 0; 
		m_iGLID = 0; 
		m_iWidth = 0; 
		m_iHeight = 0; 
	}

public:
	size_t					m_iReferences;

	size_t					m_iGLID;
	size_t					m_iWidth;
	size_t					m_iHeight;
};

class CTextureLibrary
{
public:
							CTextureLibrary();
							~CTextureLibrary();

public:
	static size_t			GetNumTextures() { return Get()->m_aTextures.size(); }

	static CTextureHandle	AddTexture(const tstring& sTexture, int iClamp = 0);
	static CTextureHandle	AddTexture(Vector* vecColors, size_t iWidth, size_t iHeight, bool bMipMaps=true);
	static CTexture*		AddAsset(const tstring& sTexture, int iClamp = 0);
	static CTexture*		AddAsset(const tstring& sTexture, Vector* vecColors, size_t iWidth, size_t iHeight, bool bMipMaps=true);
	static CTextureHandle	FindAsset(const tstring& sTexture);
	static size_t			FindTextureID(const tstring& sTexture);

	static size_t			GetTextureGLID(const tstring& sTexture);
	static size_t			GetTextureWidth(const tstring& sTexture);
	static size_t			GetTextureHeight(const tstring& sTexture);

	static size_t			GetNumTexturesLoaded() { return Get()->m_aTextures.size(); };
	static bool				IsAssetLoaded(const tstring& sTexture);

	static void				UnloadTexture(const tstring& sTexture);

	static void				ClearUnreferenced();

public:
	static CTextureLibrary*	Get() { return s_pTextureLibrary; };

protected:
	tmap<tstring, CTexture>	m_aTextures;

private:
	static CTextureLibrary*	s_pTextureLibrary;
};

#include <tinker/assethandle_functions.h>

#endif
