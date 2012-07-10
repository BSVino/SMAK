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

#include "renderer.h"

#include <GL3/gl3w.h>
#include <GL/glu.h>

#include <maths.h>
#include <tinker_platform.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <common/worklistener.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>

#include "renderingcontext.h"

CFrameBuffer::CFrameBuffer()
{
	m_iRB = m_iMap = m_iDepth = m_iDepthTexture = m_iFB = 0;
}

void CFrameBuffer::Destroy()
{
	if (m_iMap)
		glDeleteTextures(1, &m_iMap);

	if (m_iDepthTexture)
		glDeleteTextures(1, &m_iDepthTexture);

	if (m_iRB)
		glDeleteRenderbuffers(1, &m_iRB);

	if (m_iDepth)
		glDeleteRenderbuffers(1, &m_iDepth);

	if (m_iFB)
		glDeleteFramebuffers(1, &m_iFB);

	m_iRB = m_iMap = m_iDepth = m_iDepthTexture = m_iFB = 0;
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	TMsg("Initializing renderer\n");

	if (!HardwareSupported())
	{
		TError("Hardware not supported!");
		Alert("Your hardware does not support OpenGL 3.0. Please try updating your drivers.");
		exit(1);
	}

	m_bUseMultisampleTextures = !!glTexImage2DMultisample;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);

	SetSize(iWidth, iHeight);

	m_bFrustumOverride = false;
	m_bDrawBackground = true;
}

void CRenderer::Initialize()
{
	LoadShaders();
	CShaderLibrary::CompileShaders(m_iScreenSamples);

	WindowResize(m_iWidth, m_iHeight);

	if (!CShaderLibrary::IsCompiled())
	{
		TError("Shader compilation error!");
		Alert("There was a problem compiling shaders. Please send the files shaders.txt and glinfo.txt to jorge@lunarworkshop.com");
		OpenExplorer(GetAppDataDirectory(Application()->AppDirectory()));
		exit(1);
	}
}

void CRenderer::LoadShaders()
{
	tvector<tstring> asShaders = ListDirectory("shaders", false);

	for (size_t i = 0; i < asShaders.size(); i++)
	{
		tstring sShader = asShaders[i];
		if (!sShader.endswith(".txt"))
			continue;

		CShaderLibrary::AddShader("shaders/" + sShader);
	}
}

void CRenderer::WindowResize(int w, int h)
{
	m_oSceneBuffer.Destroy();
	m_oSceneBuffer = CreateFrameBuffer(w, h, (fb_options_e)(FB_TEXTURE|FB_DEPTH|FB_MULTISAMPLE));

	if (m_iScreenSamples)
	{
		m_oResolvedSceneBuffer.Destroy();
		m_oResolvedSceneBuffer = CreateFrameBuffer(w, h, (fb_options_e)(FB_TEXTURE|FB_DEPTH));
	}

	size_t iWidth = m_oSceneBuffer.m_iWidth;
	size_t iHeight = m_oSceneBuffer.m_iHeight;
	for (size_t i = 0; i < BLOOM_FILTERS; i++)
	{
		m_oBloom1Buffers[i].Destroy();
		m_oBloom2Buffers[i].Destroy();
		m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR));
		m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_TEXTURE));
		iWidth /= 2;
		iHeight /= 2;
	}
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, fb_options_e eOptions)
{
	TAssert((eOptions&FB_TEXTURE) ^ (eOptions&FB_RENDERBUFFER));

	if (!(eOptions&(FB_TEXTURE|FB_RENDERBUFFER)))
		eOptions = (fb_options_e)(eOptions|FB_TEXTURE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);
	GLsizei iSamples = m_iScreenSamples;

	bool bUseMultisample = true;
	if (iSamples == 0)
		bUseMultisample = false;
	if (!(eOptions&FB_MULTISAMPLE))
		bUseMultisample = false;

	bool bMultisampleTexture = m_bUseMultisampleTextures && bUseMultisample;

	if ((eOptions&FB_TEXTURE) && !bMultisampleTexture)
		bUseMultisample = false;

	GLuint iTextureTarget = GL_TEXTURE_2D;
	if (bUseMultisample)
		iTextureTarget = GL_TEXTURE_2D_MULTISAMPLE;

	CFrameBuffer oBuffer;
	oBuffer.m_bMultiSample = bUseMultisample;

	if (eOptions&FB_TEXTURE)
	{
		glGenTextures(1, &oBuffer.m_iMap);
		glBindTexture(iTextureTarget, (GLuint)oBuffer.m_iMap);
		if (bUseMultisample)
			glTexImage2DMultisample(iTextureTarget, iSamples, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, GL_FALSE);
		else
		{
			glTexParameteri(iTextureTarget, GL_TEXTURE_MIN_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_MAG_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			if (eOptions&FB_TEXTURE_HALF_FLOAT)
				glTexImage2D(iTextureTarget, 0, GL_RGBA16F, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
			else
				glTexImage2D(iTextureTarget, 0, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		glBindTexture(iTextureTarget, 0);
	}
	else if (eOptions&FB_RENDERBUFFER)
	{
		glGenRenderbuffers(1, &oBuffer.m_iRB);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB );
		if (bUseMultisample)
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, iSamples, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	if (eOptions&FB_DEPTH)
	{
		glGenRenderbuffers(1, &oBuffer.m_iDepth);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		if (bUseMultisample)
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, iSamples, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
	else if (eOptions&FB_DEPTH_TEXTURE)
	{
		glGenTextures(1, &oBuffer.m_iDepthTexture);
		glBindTexture(GL_TEXTURE_2D, oBuffer.m_iDepthTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	if (eOptions&FB_TEXTURE)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iTextureTarget, (GLuint)oBuffer.m_iMap, 0);
	else if (eOptions&FB_RENDERBUFFER)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB);
	if (eOptions&FB_DEPTH)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
	else if (eOptions&FB_DEPTH_TEXTURE)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (GLuint)oBuffer.m_iDepthTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	TAssert(status == GL_FRAMEBUFFER_COMPLETE);

	GLint iFBSamples;
	glGetIntegerv(GL_SAMPLES, &iFBSamples);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	TAssert(iFBSamples == iSamples || iFBSamples == 0 || 0 == iSamples);

	oBuffer.m_iWidth = iWidth;
	oBuffer.m_iHeight = iHeight;

	oBuffer.m_vecTexCoords[0] = Vector2D(0, 1);
	oBuffer.m_vecTexCoords[1] = Vector2D(0, 0);
	oBuffer.m_vecTexCoords[2] = Vector2D(1, 0);
	oBuffer.m_vecTexCoords[3] = Vector2D(1, 1);

	oBuffer.m_vecVertices[0] = Vector2D(0, 0);
	oBuffer.m_vecVertices[1] = Vector2D(0, (float)iHeight);
	oBuffer.m_vecVertices[2] = Vector2D((float)iWidth, (float)iHeight);
	oBuffer.m_vecVertices[3] = Vector2D((float)iWidth, 0);

	return oBuffer;
}

void CRenderer::DestroyFrameBuffer(CFrameBuffer* pBuffer)
{
	if (!pBuffer)
		return;

	pBuffer->Destroy();
}

void CRenderer::PreFrame()
{
}

void CRenderer::PostFrame()
{
}

void CRenderer::PreRender()
{
	m_iWidth = Application()->GetWindowWidth();
	m_iHeight = Application()->GetWindowHeight();
}

void CRenderer::PostRender()
{
}

void CRenderer::ModifyContext(class CRenderingContext* pContext)
{
	pContext->UseFrameBuffer(&m_oSceneBuffer);
}

void CRenderer::SetupFrame(class CRenderingContext* pContext)
{
	TPROF("CRenderer::SetupFrame");

	pContext->ClearDepth();

	if (m_bDrawBackground)
		DrawBackground(pContext);
}

void CRenderer::DrawBackground(class CRenderingContext* pContext)
{
	pContext->ClearColor();
}

void CRenderer::StartRendering(class CRenderingContext* pContext)
{
	TPROF("CRenderer::StartRendering");

	float flAspectRatio = (float)m_iWidth/(float)m_iHeight;

	if (ShouldRenderOrthographic())
		pContext->SetProjection(Matrix4x4::ProjectOrthographic(
				-flAspectRatio*m_flCameraOrthoHeight, flAspectRatio*m_flCameraOrthoHeight,
				-m_flCameraOrthoHeight, m_flCameraOrthoHeight,
				-100, 100
			));
	else
		pContext->SetProjection(Matrix4x4::ProjectPerspective(
				m_flCameraFOV,
				flAspectRatio,
				m_flCameraNear,
				m_flCameraFar
			));

	pContext->SetView(Matrix4x4::ConstructCameraView(m_vecCameraPosition, m_vecCameraDirection, m_vecCameraUp));

	for (size_t i = 0; i < 16; i++)
	{
		m_aflModelView[i] = ((float*)pContext->GetView())[i];
		m_aflProjection[i] = ((float*)pContext->GetProjection())[i];
	}

	if (m_bFrustumOverride)
	{
		Matrix4x4 mProjection = Matrix4x4::ProjectPerspective(
				m_flFrustumFOV,
				(float)m_iWidth/(float)m_iHeight,
				m_flFrustumNear,
				m_flFrustumFar
			);

		Matrix4x4 mView = Matrix4x4::ConstructCameraView(m_vecFrustumPosition, m_vecFrustumDirection, m_vecCameraUp);

		m_oFrustum.CreateFrom(mProjection * mView);
	}
	else
		m_oFrustum.CreateFrom(pContext->GetProjection() * pContext->GetView());

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);

	if (m_iScreenSamples)
		glEnable(GL_MULTISAMPLE);
}

CVar show_frustum("debug_show_frustum", "no");

void CRenderer::FinishRendering(class CRenderingContext* pContext)
{
	if (m_iScreenSamples)
		glDisable(GL_MULTISAMPLE);

	TPROF("CRenderer::FinishRendering");

	if (show_frustum.GetBool())
	{
		TUnimplemented();

		for (size_t i = 0; i < 6; i++)
		{
			Vector vecForward = m_oFrustum.p[i].n;
			Vector vecRight = vecForward.Cross(Vector(0, 1, 0)).Normalized();
			Vector vecUp = vecRight.Cross(vecForward).Normalized();
			Vector vecCenter = vecForward * m_oFrustum.p[i].d;

			vecForward *= 100;
			vecRight *= 100;
			vecUp *= 100;

/*			glBegin(GL_QUADS);
				glVertex3fv(vecCenter + vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp - vecRight);
				glVertex3fv(vecCenter + vecUp - vecRight);
			glEnd();*/
		}
	}
}

void CRenderer::FinishFrame(class CRenderingContext* pContext)
{
	pContext->SetProjection(Matrix4x4());
	pContext->SetView(Matrix4x4());

	RenderOffscreenBuffers(pContext);

	RenderFullscreenBuffers(pContext);
}

CVar r_bloom("r_bloom", "1");

void CRenderer::RenderOffscreenBuffers(class CRenderingContext* pContext)
{
	if (r_bloom.GetBool())
	{
		TPROF("Bloom");

		if (m_iScreenSamples)
			RenderBufferToBuffer(&m_oSceneBuffer, &m_oResolvedSceneBuffer);

		CRenderingContext c(this);

		// Use a bright-pass filter to catch only the bright areas of the image
		c.UseProgram("brightpass");

		c.SetUniform("iSource", 0);
		c.SetUniform("flScale", (float)1/BLOOM_FILTERS);

		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			c.SetUniform("flBrightness", BloomBrightnessCutoff() - 0.1f*i);
			if (m_iScreenSamples)
				RenderMapToBuffer(m_oResolvedSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
			else
				RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
		}

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
	}
}

CVar r_bloom_buffer("r_bloom_buffer", "-1");

void CRenderer::RenderFullscreenBuffers(class CRenderingContext* pContext)
{
	TPROF("CRenderer::RenderFullscreenBuffers");

	if (m_iScreenSamples)
	{
		RenderBufferToBuffer(&m_oSceneBuffer, &m_oResolvedSceneBuffer);
		RenderFrameBufferFullscreen(&m_oResolvedSceneBuffer);
	}
	else
		RenderFrameBufferFullscreen(&m_oSceneBuffer);

	if (r_bloom.GetBool())
	{
		if (r_bloom_buffer.GetInt() >= 0 && r_bloom_buffer.GetInt() < BLOOM_FILTERS)
		{
			CRenderingContext c(this);
			RenderFrameBufferFullscreen(&m_oBloom1Buffers[r_bloom_buffer.GetInt()]);
		}
		else
		{
			CRenderingContext c(this);
			c.SetBlend(BLEND_ADDITIVE);
			for (size_t i = 0; i < BLOOM_FILTERS; i++)
				RenderFrameBufferFullscreen(&m_oBloom1Buffers[i]);
		}
	}
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	CRenderingContext c(this);

	c.UseProgram("blur");

	c.SetUniform("iSource", 0);
	c.SetUniform("aflCoefficients", KERNEL_SIZE, &aflKernel[0]);
	c.SetUniform("flOffsetX", 0.0f);
	c.SetUniform("flOffsetY", 0.0f);

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		if (bHorizontal)
			c.SetUniform("flOffsetX", 1.2f / apSources[i].m_iWidth);
		else
			c.SetUniform("flOffsetY", 1.2f / apSources[i].m_iWidth);

		RenderMapToBuffer(apSources[i].m_iMap, &apTargets[i]);
    }
}

void CRenderer::RenderFrameBufferFullscreen(CFrameBuffer* pBuffer)
{
	if (pBuffer->m_iMap)
		RenderMapFullscreen(pBuffer->m_iMap, pBuffer->m_bMultiSample);
	else if (pBuffer->m_iRB)
		RenderRBFullscreen(pBuffer);
}

void CRenderer::RenderRBFullscreen(CFrameBuffer* pSource)
{
	TAssert(false);		// ATI cards don't like this at all. Never do it.

	glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderBufferToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)pDestination->m_iFB);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, pDestination->m_iWidth, pDestination->m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderMapFullscreen(size_t iMap, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(0);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.BindTexture(iMap, 0, bMapIsMultisample);

	c.EndRenderVertexArray(6);
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(pBuffer);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.BindTexture(iMap, 0, bMapIsMultisample);

	c.EndRenderVertexArray(6);
}

void CRenderer::FrustumOverride(Vector vecPosition, Vector vecDirection, float flFOV, float flNear, float flFar)
{
	m_bFrustumOverride = true;
	m_vecFrustumPosition = vecPosition;
	m_vecFrustumDirection = vecDirection;
	m_flFrustumFOV = flFOV;
	m_flFrustumNear = flNear;
	m_flFrustumFar = flFar;
}

void CRenderer::CancelFrustumOverride()
{
	m_bFrustumOverride = false;
}

Vector CRenderer::GetCameraVector()
{
	return m_vecCameraDirection;
}

void CRenderer::GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp)
{
	Vector vecForward = GetCameraVector();
	Vector vecRight;

	if (pvecForward)
		(*pvecForward) = vecForward;

	if (pvecRight || pvecUp)
		vecRight = vecForward.Cross(m_vecCameraUp).Normalized();

	if (pvecRight)
		(*pvecRight) = vecRight;

	if (pvecUp)
		(*pvecUp) = vecRight.Cross(vecForward).Normalized();
}

bool CRenderer::IsSphereInFrustum(const Vector& vecCenter, float flRadius)
{
	return m_oFrustum.TouchesSphere(vecCenter, flRadius);
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;

	m_vecFullscreenTexCoords[0] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[1] = Vector2D(1, 0);
	m_vecFullscreenTexCoords[2] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[3] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[4] = Vector2D(1, 1);
	m_vecFullscreenTexCoords[5] = Vector2D(1, 0);

	m_vecFullscreenVertices[0] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[1] = Vector(1, 1, 0);
	m_vecFullscreenVertices[2] = Vector(-1, 1, 0);
	m_vecFullscreenVertices[3] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[4] = Vector(1, -1, 0);
	m_vecFullscreenVertices[5] = Vector(1, 1, 0);
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aflModelView, (GLdouble*)m_aflProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)m_iHeight - (float)y, (float)z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)m_iHeight - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aflModelView, (GLdouble*)m_aflProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
}

bool CRenderer::HardwareSupported()
{
	if (!gl3wIsSupported(3, 0))
		return false;

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &oBuffer.m_iDepth);
	glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512 );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		TError("Test framebuffer compile failed.\n");
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffers(1, &oBuffer.m_iDepth);
		glDeleteFramebuffers(1, &oBuffer.m_iFB);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	oBuffer.Destroy();

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"#version 130\n"
		"void main()"
		"{"
		"	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);"
		"}";

	const char* pszFragmentShader =
		"#version 130\n"
		"out vec4 vecFragColor;"
		"void main()"
		"{"
		"	vecFragColor = vec4(1.0, 1.0, 1.0, 1.0);"
		"}";

	GLuint iVShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint iProgram = glCreateProgram();

	glShaderSource(iVShader, 1, &pszVertexShader, NULL);
	glCompileShader(iVShader);

	int iVertexCompiled;
	glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	glShaderSource(iFShader, 1, &pszFragmentShader, NULL);
	glCompileShader(iFShader);

	int iFragmentCompiled;
	glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	glAttachShader(iProgram, iVShader);
	glAttachShader(iProgram, iFShader);
	glLinkProgram(iProgram);

	int iProgramLinked;
	glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked);

	if (!(iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE))
		TError("Test shader compile failed.\n");

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE;
}

size_t CRenderer::LoadVertexDataIntoGL(size_t iSizeInBytes, float* aflVertices)
{
	GLuint iVBO;
	glGenBuffers(1, &iVBO);
	glBindBuffer(GL_ARRAY_BUFFER, iVBO);

	glBufferData(GL_ARRAY_BUFFER, iSizeInBytes, 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, iSizeInBytes, aflVertices);

    int iSize = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize);
    if(iSizeInBytes != iSize)
    {
        glDeleteBuffers(1, &iVBO);
		TAssert(false);
        TError("CRenderer::LoadVertexDataIntoGL(): Data size is mismatch with input array\n");
		return 0;
    }

	return iVBO;
}

void CRenderer::UnloadVertexDataFromGL(size_t iBuffer)
{
	glDeleteBuffers(1, (GLuint*)&iBuffer);
}

size_t CRenderer::LoadTextureIntoGL(tstring sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	int x, y, n;
    unsigned char *pData = stbi_load(sFilename.c_str(), &x, &y, &n, 4);

	if (!pData)
		return 0;

	if (!s_bNPO2TextureLoads)
	{
		if (x & (x-1))
		{
			TError("Image width is not power of 2.");
			stbi_image_free(pData);
			return 0;
		}

		if (y & (y-1))
		{
			TError("Image height is not power of 2.");
			stbi_image_free(pData);
			return 0;
		}
	}

	size_t iGLId = LoadTextureIntoGL((Color*)pData, x, y, iClamp);

	stbi_image_free(pData);

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Color* pclrData, int x, int y, int iClamp)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		4,
		x,
		y,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Vector* pvecData, int x, int y, int iClamp, bool bMipMaps)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (bMipMaps)
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, x, y, GL_RGB, GL_FLOAT, pvecData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_FLOAT, pvecData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

void CRenderer::UnloadTextureFromGL(size_t iGLId)
{
	glDeleteTextures(1, (GLuint*)&iGLId);
	s_iTexturesLoaded--;
}

size_t CRenderer::s_iTexturesLoaded = 0;
bool CRenderer::s_bNPO2TextureLoads = false;

Color* CRenderer::LoadTextureData(tstring sFilename, int& x, int& y)
{
	if (!sFilename.length())
		return nullptr;

	int n;
    unsigned char *pData = stbi_load(sFilename.c_str(), &x, &y, &n, 4);

	if (!pData)
		return 0;

	if (!s_bNPO2TextureLoads)
	{
		if (x & (x-1))
		{
			TError("Image width is not power of 2.");
			stbi_image_free(pData);
			return 0;
		}

		if (y & (y-1))
		{
			TError("Image height is not power of 2.");
			stbi_image_free(pData);
			return 0;
		}
	}

	return (Color*)pData;
}

void CRenderer::UnloadTextureData(Color* pData)
{
	stbi_image_free((char*)pData);
}

void CRenderer::ReadTextureFromGL(CTextureHandle hTexture, Vector* pvecData)
{
	TAssert(hTexture.IsValid());
	if (!hTexture.IsValid())
		return;

	glBindTexture(GL_TEXTURE_2D, hTexture->m_iGLID);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pvecData);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void CRenderer::ReadTextureFromGL(CTextureHandle hTexture, Color* pclrData)
{
	TAssert(hTexture.IsValid());
	if (!hTexture.IsValid())
		return;

	glBindTexture(GL_TEXTURE_2D, hTexture->m_iGLID);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void CRenderer::WriteTextureToFile(size_t iTexture, tstring sFilename)
{
	glBindTexture(GL_TEXTURE_2D, iTexture);

	int iWidth, iHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);

	tvector<Color> aclrPixels;
	aclrPixels.resize(iWidth*iHeight);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, aclrPixels.data());

	if (sFilename.endswith(".png"))
		stbi_write_png(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data(), 0);
	else if (sFilename.endswith(".tga"))
		stbi_write_tga(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data());
	else if (sFilename.endswith(".bmp"))
		stbi_write_bmp(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data());
}

void CRenderer::WriteTextureToFile(Color* pclrData, int w, int h, tstring sFilename)
{
	if (sFilename.endswith(".png"))
		stbi_write_png(sFilename.c_str(), w, h, 4, pclrData, 0);
	else if (sFilename.endswith(".tga"))
		stbi_write_tga(sFilename.c_str(), w, h, 4, pclrData);
	else if (sFilename.endswith(".bmp"))
		stbi_write_bmp(sFilename.c_str(), w, h, 4, pclrData);
}

void R_DumpFBO(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	size_t iFBO = 0;
	if (asTokens.size() > 1)
		iFBO = stoi(asTokens[1]);

	int aiViewport[4];
	glGetIntegerv( GL_VIEWPORT, aiViewport );

	int iWidth = aiViewport[2];
	int iHeight = aiViewport[3];

	tvector<Color> aclrPixels;
	aclrPixels.resize(iWidth*iHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)iFBO);
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	// In case it's multisampled, blit it over to a normal one first.
	CFrameBuffer oResolvedBuffer = Application()->GetRenderer()->CreateFrameBuffer(iWidth, iHeight, (fb_options_e)(FB_RENDERBUFFER|FB_DEPTH));

	glBindFramebuffer(GL_READ_FRAMEBUFFER, iFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oResolvedBuffer.m_iFB);
	glBlitFramebuffer(
		0, 0, iWidth, iHeight, 
		0, 0, oResolvedBuffer.m_iWidth, oResolvedBuffer.m_iHeight, 
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oResolvedBuffer.m_iFB);
	glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, aclrPixels.data());

	oResolvedBuffer.Destroy();

	for (int i = 0; i < iWidth; i++)
	{
		for (int j = 0; j < iHeight/2; j++)
			std::swap(aclrPixels[j*iWidth + i], aclrPixels[iWidth*(iHeight-j-1) + i]);
	}

	CRenderer::WriteTextureToFile(aclrPixels.data(), iWidth, iHeight, sprintf("fbo-%d.png", iFBO));
}

CCommand r_dumpfbo(tstring("r_dumpfbo"), ::R_DumpFBO);
