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

#include "modelconverter.h"

#include <direct.h>

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("I need a file to convert!\n");
		return 0;
	}

	size_t iFileLength = strlen(argv[1]);
	const char* pszExtension = argv[1]+iFileLength-4;

	CConversionScene s;
	CModelConverter c(&s);

	tstring sFile = argv[1];

	if (strcmp(pszExtension, ".obj") == 0)
	{
		printf("Reading the OBJ... ");
		c.ReadOBJ(sFile);
	}
	else if (strcmp(pszExtension, ".sia") == 0)
	{
		printf("Reading the Silo ASCII file... ");
		c.ReadSIA(sFile);
	}
	else if (strcmp(pszExtension, ".dae") == 0)
	{
		printf("Reading the Collada .dae file... ");
		c.ReadDAE(sFile);
	}

	printf("Done.\n");
	printf("\n");

	printf("-------------\n");
	printf("Materials   : %d\n", s.GetNumMaterials());
	printf("Meshes      : %d\n", s.GetNumMeshes());
	printf("\n");

	for (size_t i = 0; i < s.GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = s.GetMesh(i);

		printf("-------------\n");
		printf("Mesh: %s\n", pMesh->GetBoneName(0));
		printf("Vertices    : %d\n", pMesh->GetNumVertices());
		printf("Normals     : %d\n", pMesh->GetNumNormals());
		printf("UVs         : %d\n", pMesh->GetNumUVs());
		printf("Bones       : %d\n", pMesh->GetNumBones());
		printf("Faces       : %d\n", pMesh->GetNumFaces());
		printf("\n");
	}

	printf("Writing the SMD... ");
	c.WriteSMDs();
	printf("Done.\n");
	printf("\n");

	printf("Press enter to continue...\n");
	getchar();

	return 0;
}
