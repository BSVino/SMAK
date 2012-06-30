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

#include "renderingcontext.h"

#include <GL3/gl3w.h>
#include <GL/glu.h>
#include <FTGL/ftgl.h>

#include <maths.h>
#include <simplex.h>

#include <glgui/label.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>
#include <textures/materiallibrary.h>
#include <renderer/renderer.h>

tvector<CRenderingContext::CRenderContext> CRenderingContext::s_aContexts;

CRenderingContext::CRenderingContext(CRenderer* pRenderer, bool bInherit)
{
	m_pRenderer = pRenderer;

	m_clrRender = ::Color(255, 255, 255, 255);

	s_aContexts.push_back();

	if (bInherit && s_aContexts.size() > 1)
	{
		CRenderContext& oLastContext = s_aContexts[s_aContexts.size()-2];

		GetContext().m_mProjection = oLastContext.m_mProjection;
		GetContext().m_mView = oLastContext.m_mView;
		GetContext().m_mTransformations = oLastContext.m_mTransformations;

		GetContext().m_hMaterial = oLastContext.m_hMaterial;
		GetContext().m_pFrameBuffer = oLastContext.m_pFrameBuffer;
		tstrncpy(GetContext().m_szProgram, PROGRAM_LEN, oLastContext.m_szProgram, PROGRAM_LEN);
		GetContext().m_pShader = oLastContext.m_pShader;

		GetContext().m_rViewport = oLastContext.m_rViewport;
		GetContext().m_eBlend = oLastContext.m_eBlend;
		GetContext().m_flAlpha = oLastContext.m_flAlpha;
		GetContext().m_bDepthMask = oLastContext.m_bDepthMask;
		GetContext().m_bDepthTest = oLastContext.m_bDepthTest;
		GetContext().m_eDepthFunction = oLastContext.m_eDepthFunction;
		GetContext().m_bCull = oLastContext.m_bCull;
		GetContext().m_bWinding = oLastContext.m_bWinding;

		m_pShader = GetContext().m_pShader;

		if (m_pShader)
			m_iProgram = m_pShader->m_iProgram;
		else
			m_iProgram = 0;
	}
	else
	{
		m_pShader = NULL;

		BindTexture(0);
		UseMaterial(CMaterialHandle());
		UseFrameBuffer(NULL);
		UseProgram("");

		SetViewport(Rect(0, 0, Application()->GetWindowWidth(), Application()->GetWindowHeight()));
		SetBlend(BLEND_NONE);
		SetAlpha(1);
		SetDepthMask(true);
		SetDepthTest(true);
		SetDepthFunction(DF_LESS);
		SetBackCulling(true);
		SetWinding(true);
	}
}

CRenderingContext::~CRenderingContext()
{
	TAssert(s_aContexts.size());

	s_aContexts.pop_back();

	if (s_aContexts.size())
	{
		UseMaterial(GetContext().m_hMaterial);
		UseFrameBuffer(GetContext().m_pFrameBuffer);
		UseProgram(GetContext().m_pShader);

		if (*GetContext().m_szProgram)
		{
			SetUniform("mProjection", GetContext().m_mProjection);
			SetUniform("mView", GetContext().m_mView);
			SetUniform("mGlobal", GetContext().m_mTransformations);
		}

		SetViewport(GetContext().m_rViewport);
		SetBlend(GetContext().m_eBlend);
		SetAlpha(GetContext().m_flAlpha);
		SetDepthMask(GetContext().m_bDepthMask);
		SetDepthTest(GetContext().m_bDepthTest);
		SetDepthFunction(GetContext().m_eDepthFunction);
		SetBackCulling(GetContext().m_bCull);
		SetWinding(GetContext().m_bWinding);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_pRenderer)
			glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight);
		else
			glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight());

		glUseProgram(0);

		glDisablei(GL_BLEND, 0);

		glDepthMask(true);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);

		glFrontFace(GL_CCW);
	}
}

void CRenderingContext::SetProjection(const Matrix4x4& m)
{
	GetContext().m_mProjection = m;

	if (m_pShader)
		SetUniform("mProjection", m);
}

void CRenderingContext::SetView(const Matrix4x4& m)
{
	GetContext().m_mView = m;

	if (m_pShader)
		SetUniform("mView", m);
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	GetContext().m_mTransformations *= m;
}

void CRenderingContext::Translate(const Vector& vecTranslate)
{
	GetContext().m_mTransformations.AddTranslation(vecTranslate);
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	Matrix4x4 mRotation;
	mRotation.SetRotation(flAngle, vecAxis);

	GetContext().m_mTransformations *= mRotation;
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	GetContext().m_mTransformations.AddScale(Vector(flX, flY, flZ));
}

void CRenderingContext::ResetTransformations()
{
	GetContext().m_mTransformations.Identity();
}

void CRenderingContext::LoadTransform(const Matrix4x4& m)
{
	GetContext().m_mTransformations = m;
}

void CRenderingContext::SetViewport(const Rect& rViewport)
{
	glViewport(rViewport.x, rViewport.y, rViewport.w, rViewport.h);

	GetContext().m_rViewport = rViewport;
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (eBlend)
	{
		glEnablei(GL_BLEND, 0);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (eBlend == BLEND_ADDITIVE)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		else if (eBlend == BLEND_BOTH)
			glBlendFunc(GL_ONE, GL_ONE);
		else
			TUnimplemented();
	}
	else
		glDisablei(GL_BLEND, 0);

	GetContext().m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	glDepthMask(bDepthMask);
	GetContext().m_bDepthMask = bDepthMask;
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (bDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	GetContext().m_bDepthTest = bDepthTest;
}

void CRenderingContext::SetDepthFunction(depth_function_t eDepthFunction)
{
	if (eDepthFunction == DF_LEQUAL)
		glDepthFunc(GL_LEQUAL);
	else if (eDepthFunction == DF_LESS)
		glDepthFunc(GL_LESS);
	else
		TUnimplemented();

	GetContext().m_eDepthFunction = eDepthFunction;
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	GetContext().m_bCull = bCull;
}

void CRenderingContext::SetWinding(bool bWinding)
{
	GetContext().m_bWinding = bWinding;
	glFrontFace(bWinding?GL_CCW:GL_CW);
}

void CRenderingContext::ClearColor(const ::Color& clrClear)
{
	glClearColor((float)(clrClear.r())/255, (float)(clrClear.g())/255, (float)(clrClear.b())/255, (float)(clrClear.a())/255);
	glClear(GL_COLOR_BUFFER_BIT);
}

void CRenderingContext::ClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void CRenderingContext::RenderSphere()
{
	static GLUquadricObj* pQuadric = nullptr;

	if (pQuadric == nullptr)
		pQuadric = gluNewQuadric();

	gluSphere(pQuadric, 1, 20, 10);
}

void CRenderingContext::RenderWireBox(const AABB& aabbBounds)
{
	BeginRenderLines();
		Vertex(aabbBounds.m_vecMaxs);
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(aabbBounds.m_vecMaxs);

		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
	EndRender();
}

void CRenderingContext::RenderBillboard(const CMaterialHandle& hMaterial, float flRadius, Vector vecUp, Vector vecRight)
{
	TAssert(hMaterial.IsValid());
	if (!hMaterial.IsValid())
		return;

	vecUp *= flRadius;
	vecRight *= flRadius;

	UseMaterial(hMaterial);
	BeginRenderTriFan();
		TexCoord(0.0f, 1.0f);
		Vertex(-vecRight + vecUp);
		TexCoord(0.0f, 0.0f);
		Vertex(-vecRight - vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecRight - vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecRight + vecUp);
	EndRender();
}

void CRenderingContext::UseFrameBuffer(const CFrameBuffer* pBuffer)
{
	GetContext().m_pFrameBuffer = pBuffer;

	if (pBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)pBuffer->m_iFB);
		glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_pRenderer)
			glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight);
		else
			glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight());
	}
}

void CRenderingContext::UseProgram(const tchar* pszProgram)
{
	tstrncpy(GetContext().m_szProgram, PROGRAM_LEN, pszProgram, PROGRAM_LEN);

	GetContext().m_pShader = m_pShader = CShaderLibrary::GetShader(pszProgram);

	if (*pszProgram)
		TAssert(m_pShader);

	UseProgram(m_pShader);
}

void CRenderingContext::UseProgram(class CShader* pShader)
{
	GetContext().m_pShader = m_pShader = pShader;

	if (!m_pShader)
	{
		GetContext().m_szProgram[0] = '\0';
		m_iProgram = 0;
		glUseProgram(0);
		return;
	}

	tstrncpy(GetContext().m_szProgram, PROGRAM_LEN, pShader->m_sName.c_str(), PROGRAM_LEN);

	m_iProgram = m_pShader->m_iProgram;
	glUseProgram((GLuint)m_pShader->m_iProgram);

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
}

void CRenderingContext::UseMaterial(const CMaterialHandle& hMaterial)
{
	if (!hMaterial.IsValid())
		return;

	GetContext().m_hMaterial = hMaterial;

	UseProgram(hMaterial->m_pShader);

	SetupMaterial();
}

void CRenderingContext::UseMaterial(const tstring& sName)
{
	UseMaterial(CMaterialLibrary::FindAsset(sName));
}

void CRenderingContext::SetupMaterial()
{
	if (!GetContext().m_hMaterial.IsValid())
		return;

	if (!m_pShader)
		return;

	const tstring& sMaterialBlend = GetContext().m_hMaterial->m_sBlend;
	if (sMaterialBlend == "alpha")
		SetBlend(BLEND_ALPHA);
	else if (sMaterialBlend == "additive")
		SetBlend(BLEND_ADDITIVE);
	else
	{
		TAssert(!sMaterialBlend.length());
		SetBlend(BLEND_NONE);
	}

	for (auto it = m_pShader->m_asUniforms.begin(); it != m_pShader->m_asUniforms.end(); it++)
	{
		CShader::CUniform& pUniformName = it->second;
		CShader::CParameter::CUniform* pUniform = it->second.m_pDefault;

		if (pUniform)
		{
			if (pUniformName.m_sUniformType == "float")
				SetUniform(it->first.c_str(), pUniform->m_flValue);
			else if (pUniformName.m_sUniformType == "vec2")
				SetUniform(it->first.c_str(), pUniform->m_vec2Value);
			else if (pUniformName.m_sUniformType == "vec3")
				SetUniform(it->first.c_str(), pUniform->m_vecValue);
			else if (pUniformName.m_sUniformType == "vec4")
				SetUniform(it->first.c_str(), pUniform->m_vec4Value);
			else if (pUniformName.m_sUniformType == "int")
				SetUniform(it->first.c_str(), pUniform->m_iValue);
			else if (pUniformName.m_sUniformType == "bool")
				SetUniform(it->first.c_str(), pUniform->m_bValue);
			else if (pUniformName.m_sUniformType == "mat4")
			{
				TUnimplemented();
			}
			else if (pUniformName.m_sUniformType == "sampler2D")
			{
				TUnimplemented();
			}
			else
				TUnimplemented();
		}
		else
		{
			if (pUniformName.m_sUniformType == "float")
				SetUniform(it->first.c_str(), 0.0f);
			else if (pUniformName.m_sUniformType == "vec2")
				SetUniform(it->first.c_str(), Vector2D());
			else if (pUniformName.m_sUniformType == "vec3")
				SetUniform(it->first.c_str(), Vector());
			else if (pUniformName.m_sUniformType == "vec4")
				SetUniform(it->first.c_str(), Vector4D());
			else if (pUniformName.m_sUniformType == "int")
				SetUniform(it->first.c_str(), 0);
			else if (pUniformName.m_sUniformType == "bool")
				SetUniform(it->first.c_str(), false);
			else if (pUniformName.m_sUniformType == "mat4")
				SetUniform(it->first.c_str(), Matrix4x4());
			else if (pUniformName.m_sUniformType == "sampler2D")
				SetUniform(it->first.c_str(), 0);
			else
				TUnimplemented();
		}
	}

	for (size_t i = 0; i < GetContext().m_hMaterial->m_aParameters.size(); i++)
	{
		auto& oParameter = GetContext().m_hMaterial->m_aParameters[i];
		CShader::CParameter* pShaderParameter = oParameter.m_pShaderParameter;

		TAssert(pShaderParameter);
		if (!pShaderParameter)
			continue;

		for (size_t j = 0; j < pShaderParameter->m_aActions.size(); j++)
		{
			auto& oAction = pShaderParameter->m_aActions[j];
			tstring& sName = oAction.m_sName;
			tstring& sValue = oAction.m_sValue;
			tstring& sType = m_pShader->m_asUniforms[sName].m_sUniformType;
			if (sValue == "[value]")
			{
				if (sType == "float")
					SetUniform(sName.c_str(), oParameter.m_flValue);
				else if (sType == "vec2")
					SetUniform(sName.c_str(), oParameter.m_vec2Value);
				else if (sType == "vec3")
					SetUniform(sName.c_str(), oParameter.m_vecValue);
				else if (sType == "vec4")
					SetUniform(sName.c_str(), oParameter.m_vec4Value);
				else if (sType == "int")
					SetUniform(sName.c_str(), oParameter.m_iValue);
				else if (sType == "bool")
					SetUniform(sName.c_str(), oParameter.m_bValue);
				else if (sType == "mat4")
				{
					TUnimplemented();
				}
				else if (sType == "sampler2D")
				{
					// No op, handled below.
				}
				else
					TUnimplemented();
			}
			else
			{
				if (sType == "float")
					SetUniform(sName.c_str(), oAction.m_flValue);
				else if (sType == "vec2")
					SetUniform(sName.c_str(), oAction.m_vec2Value);
				else if (sType == "vec3")
					SetUniform(sName.c_str(), oAction.m_vecValue);
				else if (sType == "vec4")
					SetUniform(sName.c_str(), oAction.m_vec4Value);
				else if (sType == "int")
					SetUniform(sName.c_str(), oAction.m_iValue);
				else if (sType == "bool")
					SetUniform(sName.c_str(), oAction.m_bValue);
				else if (sType == "mat4")
				{
					TUnimplemented();
				}
				else if (sType == "sampler2D")
				{
					TUnimplemented();
					SetUniform(sName.c_str(), 0);
				}
				else
					TUnimplemented();
			}
		}	
	}

	for (size_t i = 0; i < m_pShader->m_asTextures.size(); i++)
	{
		if (!GetContext().m_hMaterial->m_ahTextures[i].IsValid())
			continue;

		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, (GLuint)GetContext().m_hMaterial->m_ahTextures[i]->m_iGLID);
		SetUniform(m_pShader->m_asTextures[i].c_str(), (int)i);
	}
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector4D& vecValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const ::Color& clrValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, Vector4D(clrValue));
}

void CRenderingContext::SetUniform(const char* pszName, const Matrix4x4& mValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniformMatrix4fv(iUniform, 1, false, mValue);
}

void CRenderingContext::SetUniform(const char* pszName, size_t iSize, const float* aflValues)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1fv(iUniform, iSize, aflValues);
}

void CRenderingContext::BindTexture(size_t iTexture, int iChannel, bool bMultisample)
{
	// Not tested since the move to a stack
	TAssert(iChannel == 0);

	glActiveTexture(GL_TEXTURE0+iChannel);

	if (bMultisample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)iTexture);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
}

void CRenderingContext::BindBufferTexture(const CFrameBuffer& oBuffer, int iChannel)
{
	glActiveTexture(GL_TEXTURE0+iChannel);

	if (oBuffer.m_bMultiSample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)oBuffer.m_iMap);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);

	// Not ported to texture handle code.
//	GetContext().m_hTexture = oBuffer.m_iMap;
}

void CRenderingContext::SetColor(const ::Color& c)
{
	m_clrRender = c;
}

void CRenderingContext::BeginRenderTris()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_aclrColors.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_TRIANGLES;
}

void CRenderingContext::BeginRenderTriFan()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_aclrColors.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_TRIANGLE_FAN;
}

void CRenderingContext::BeginRenderQuads()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_aclrColors.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	TUnimplemented();
	//m_iDrawMode = GL_QUADS;
}

void CRenderingContext::BeginRenderLines(float flWidth)
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_aclrColors.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	glLineWidth( flWidth );
	m_iDrawMode = GL_LINE_LOOP;
}

void CRenderingContext::BeginRenderDebugLines()
{
	BeginRenderLines(3);
}

void CRenderingContext::BeginRenderPoints(float flSize)
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_aclrColors.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	glPointSize( flSize );
	m_iDrawMode = GL_POINTS;
}

void CRenderingContext::TexCoord(float s, float t, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = Vector2D(s, t);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector2D& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const DoubleVector2D& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = Vector2D(v);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const DoubleVector& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = DoubleVector2D(v);

	m_bTexCoord = true;
}

void CRenderingContext::Normal(const Vector& v)
{
	m_vecNormal = v;
	m_bNormal = true;
}

void CRenderingContext::Color(const ::Color& c)
{
	m_clrColor = c;
	m_bColor = true;
}

void CRenderingContext::Vertex(const Vector& v)
{
	if (m_bTexCoord)
	{
		if (m_aavecTexCoords.size() < m_avecTexCoord.size())
			m_aavecTexCoords.resize(m_avecTexCoord.size());

		for (size_t i = 0; i < m_aavecTexCoords.size(); i++)
			m_aavecTexCoords[i].push_back(m_avecTexCoord[i]);
	}

	if (m_bNormal)
		m_avecNormals.push_back(m_vecNormal);

	if (m_bColor)
		m_aclrColors.push_back(m_clrColor);

	m_avecVertices.push_back(v);
}

void CRenderingContext::EndRender()
{
	TAssert(m_pShader);
	if (!m_pShader)
	{
		UseProgram("model");
		if (!m_pShader)
			return;
	}

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

	if (m_bTexCoord && m_pShader->m_iTexCoordAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
		glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, 0, m_aavecTexCoords[0].data());
	}

	if (m_bNormal && m_pShader->m_iNormalAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
		glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, 0, m_avecNormals.data());
	}

	if (m_bColor && m_pShader->m_iColorAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iColorAttribute);
		glVertexAttribPointer(m_pShader->m_iColorAttribute, 3, GL_UNSIGNED_BYTE, true, sizeof(::Color), m_aclrColors.data());
	}

	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, 0, m_avecVertices.data());

	glDrawArrays(m_iDrawMode, 0, m_avecVertices.size());

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);
}

void CRenderingContext::BeginRenderVertexArray(size_t iBuffer)
{
	if (iBuffer)
		glBindBuffer(GL_ARRAY_BUFFER, iBuffer);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void CRenderingContext::SetPositionBuffer(float* pflBuffer, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetPositionBuffer(size_t iOffset, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetNormalsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
	glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetNormalsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
	glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetTangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTangentAttribute);
	glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetTangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTangentAttribute);
	glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetBitangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetBitangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetTexCoordBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iTexCoordAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetTexCoordBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iTexCoordAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride)
{
	int iAttribute = glGetAttribLocation(m_iProgram, pszName);

	TAssert(iAttribute != ~0);
	if (iAttribute == ~0)
		return;

	glEnableVertexAttribArray(iAttribute);
	glVertexAttribIPointer(iAttribute, iSize, GL_INT, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::EndRenderVertexArray(size_t iVertices, bool bWireframe)
{
	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

	if (bWireframe)
	{
		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, iVertices);
	}
	else
		glDrawArrays(GL_TRIANGLES, 0, iVertices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iTangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTangentAttribute);
	if (m_pShader->m_iBitangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CRenderingContext::EndRenderVertexArrayTriangles(size_t iTriangles, int* piIndices)
{
	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

	glDrawElements(GL_TRIANGLES, iTriangles*3, GL_UNSIGNED_INT, piIndices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iTangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTangentAttribute);
	if (m_pShader->m_iBitangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CRenderingContext::RenderText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize)
{
	FTFont* pFont = glgui::CLabel::GetFont(sFontName, iFontFaceSize);

	if (!pFont)
	{
		glgui::CLabel::AddFontSize(sFontName, iFontFaceSize);
		pFont = glgui::CLabel::GetFont(sFontName, iFontFaceSize);
	}

	RenderText(sText, iLength, pFont);
}

void CRenderingContext::RenderText(const tstring& sText, unsigned iLength, FTFont* pFont)
{
	TAssert(m_pShader);
	if (!m_pShader)
		return;

	TAssert(m_pShader->m_iPositionAttribute >= 0);
	TAssert(m_pShader->m_iTexCoordAttribute >= 0);

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);

	// Take the position out and let FTGL do it. It looks sharper that way.
	Matrix4x4 mTransformations = GetContext().m_mTransformations;
	Vector vecPosition = mTransformations.GetTranslation();
	mTransformations.SetTranslation(Vector());
	SetUniform("mGlobal", mTransformations);

	ftglSetAttributeLocations(m_pShader->m_iPositionAttribute, m_pShader->m_iTexCoordAttribute);
	pFont->Render(sText.c_str(), iLength, FTPoint(vecPosition.x, vecPosition.y, vecPosition.z));
}

void CRenderingContext::ReadPixels(size_t x, size_t y, size_t w, size_t h, Vector4D* pvecPixels)
{
	glReadPixels(x, y, w, h, GL_RGBA, GL_FLOAT, pvecPixels);
}

void CRenderingContext::Finish()
{
	glFinish();
}

