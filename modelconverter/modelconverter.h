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

#ifndef Lw_MODELCONVERTER_H
#define LW_MODELCONVERTER_H

#include <iostream>
#include <fstream>

#include <tvector.h>
#include <worklistener.h>
#include "convmesh.h"

class CModelConverter
{
public:
						CModelConverter(CConversionScene* pScene);

public:
	bool				ReadModel(const tstring& sFilename);

	bool				ReadOBJ(const tstring& sFilename);
	void				ReadMTL(const tstring& sFilename);

	// SIA and its utility functions.
	bool				ReadSIA(const tstring& sFilename);
	const tchar*		ReadSIAMat(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, const tstring& sFilename);
	const tchar*		ReadSIAShape(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, bool bCare = true);

	bool				ReadDAE(const tstring& sFilename);
	void				ReadDAESceneTree(class FCDSceneNode* pNode, CConversionSceneNode* pScene);

	bool				ReadAssImp(const tstring& sFilename);
	void				ReadAssImpSceneTree(const struct aiScene* pAIScene, struct aiNode* pNode, CConversionSceneNode* pScene);

	bool				SaveModel(const tstring& sFilename);

	void				SaveOBJ(const tstring& sFilename);
	void				SaveSIA(const tstring& sFilename);
	void				SaveDAE(const tstring& sFilename);
	void				SaveDAEScene(class FCDSceneNode* pNode, CConversionSceneNode* pScene);

	void				WriteSMDs(const tstring& sFilename = "");
	void				WriteSMD(size_t iMesh, const tstring& sFilename = "");

	tstring				GetFilename(const tstring& sFilename);
	tstring				GetDirectory(const tstring& sFilename);
	bool				IsWhitespace(tstring::value_type cChar);
	tstring				StripWhitespace(tstring sLine);

	void				SetScene(CConversionScene* pScene) { m_pScene = pScene; };
	CConversionScene*	GetScene() { return m_pScene; };

	void				SetWorkListener(IWorkListener* pWorkListener) { m_pWorkListener = pWorkListener; }

	void				SetWantEdges(bool bWantEdges) { m_bWantEdges = bWantEdges; }

	static tvector<tstring>	GetReadFormats();

protected:
	CConversionScene*	m_pScene;

	IWorkListener*		m_pWorkListener;

	bool				m_bWantEdges;
};

#endif
