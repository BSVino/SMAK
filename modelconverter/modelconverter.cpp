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

#include <stdio.h>
#include <string.h>

#include "modelconverter.h"
#include "strutils.h"

#ifdef WITH_ASSIMP
#include <assimp.h>
#endif

CModelConverter::CModelConverter(CConversionScene* pScene)
{
	m_pScene = pScene;
	m_pWorkListener = NULL;

	m_bWantEdges = true;
}

bool CModelConverter::ReadModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.tolower();

	if (sExtension == ".obj")
		return ReadOBJ(sFilename);
	else if (sExtension == ".sia")
		return ReadSIA(sFilename);
	else if (sExtension == ".dae")
		return ReadDAE(sFilename);
	else
		return ReadAssImp(sFilename);
}

bool CModelConverter::SaveModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.tolower();

	if (sExtension == ".obj")
		SaveOBJ(sFilename);
	else if (sExtension == ".sia")
		SaveSIA(sFilename);
	else if (sExtension == ".dae")
		SaveDAE(sFilename);
	else
		return false;

	return true;
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
tstring CModelConverter::GetFilename(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == '\\' || sFilename[i] == '/')
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == '.')
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

tstring CModelConverter::GetDirectory(const tstring& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	tstring sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == '\\' || sResult[i] == '/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		sResult[iLastSlash] = '\0';
	else
		return ".";

	return sResult;
}

bool CModelConverter::IsWhitespace(tstring::value_type cChar)
{
	return (cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n');
}

tstring CModelConverter::StripWhitespace(tstring sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != '\0')
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = '\0';

	return sLine.substr(i);
}

tvector<tstring> CModelConverter::GetReadFormats()
{
	tvector<tstring> asFormats;

#ifdef WITH_ASSIMP
	aiString sFormats;
	aiGetExtensionList(&sFormats);

	explode(sFormats.data, asFormats, ";");

	for (size_t i = 0; i < asFormats.size(); i++)	// Get rid of the * that assimp puts at the front
		asFormats[i] = asFormats[i].substr(1);
#else
	asFormats.push_back(".obj");
	asFormats.push_back(".dae");
#endif

	asFormats.push_back(".sia");

	return asFormats;
}