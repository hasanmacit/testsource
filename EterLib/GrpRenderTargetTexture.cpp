#include "StdAfx.h"
#ifdef ENABLE_RENDER_TARGET
#include "../EterBase/Stl.h"
#include "GrpRenderTargetTexture.h"
#include "StateManager.h"

CGraphicRenderTargetTexture::CGraphicRenderTargetTexture() : m_d3dFormat{ D3DFMT_A8R8G8B8 }, m_depthStencilFormat{ D3DFMT_UNKNOWN }
{
	Initialize();
	memset(&m_renderRect, 0, sizeof(m_renderRect));
}

CGraphicRenderTargetTexture::~CGraphicRenderTargetTexture()
{
	Reset();
}

void CGraphicRenderTargetTexture::ReleaseTextures()
{
	safe_release(m_lpd3dRenderTexture);
	safe_release(m_lpd3dRenderTargetSurface);
	safe_release(m_lpd3dDepthSurface);
	safe_release(m_lpd3dOriginalRenderTarget);
	safe_release(m_lpd3dOldDepthBufferSurface);
	memset(&m_renderRect, 0, sizeof(m_renderRect));
	memset(&m_renderBox, 0, sizeof(m_renderBox));
}

bool CGraphicRenderTargetTexture::Create(const int width, const int height, const D3DFORMAT texFormat, const D3DFORMAT depthFormat)
{
	Reset();
	m_height = height;
	m_width = width;
	if (!CreateRenderTexture(width, height, texFormat))
		return false;
	if (!CreateRenderDepthStencil(width, height, depthFormat))
		return false;
	return true;
}

void CGraphicRenderTargetTexture::CreateTextures()
{
	if (CreateRenderTexture(m_width, m_height, m_d3dFormat))
		CreateRenderDepthStencil(m_width, m_height, m_depthStencilFormat);
}

bool CGraphicRenderTargetTexture::CreateRenderTexture(const int width, const int height, const D3DFORMAT format)
{
	m_d3dFormat = format;
	if (FAILED(ms_lpd3dDevice->CreateTexture(width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lpd3dRenderTexture, NULL)))
		return false;
	if (FAILED(m_lpd3dRenderTexture->GetSurfaceLevel(0, &m_lpd3dRenderTargetSurface)))
		return false;
	return true;
}

bool CGraphicRenderTargetTexture::CreateRenderDepthStencil(const int width, const int height, const D3DFORMAT format)
{
	m_depthStencilFormat = format;

#ifdef DIRECTX9
	return (ms_lpd3dDevice->CreateDepthStencilSurface(width, height, m_depthStencilFormat, D3DMULTISAMPLE_NONE, 0, true, &m_lpd3dDepthSurface, NULL)) == D3D_OK;
#else
	return (ms_lpd3dDevice->CreateDepthStencilSurface(width, height, m_depthStencilFormat, D3DMULTISAMPLE_NONE, &m_lpd3dDepthSurface)) == D3D_OK;
#endif
}

void CGraphicRenderTargetTexture::SetRenderTarget()
{
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpd3dOriginalRenderTarget)))
	{
		TraceError("FAILED GET TARGET");
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpd3dOldDepthBufferSurface)))
	{
		TraceError("FAILED GET DEPTH");
	}

	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dRenderTargetSurface);
}

void CGraphicRenderTargetTexture::ResetRenderTarget()
{
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dOldDepthBufferSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOriginalRenderTarget);

	safe_release(m_lpd3dOriginalRenderTarget);
	safe_release(m_lpd3dOldDepthBufferSurface);
}

void CGraphicRenderTargetTexture::Clear()
{
	ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
}

void CGraphicRenderTargetTexture::Render() const
{
	STATEMANAGER.SaveRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHABLENDENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);

	const float sx = static_cast<float>(m_renderRect.left) - 0.5f + static_cast<float>(m_renderBox.left);
	const float sy = static_cast<float>(m_renderRect.top) - 0.5f + static_cast<float>(m_renderBox.top);
	const float ex = static_cast<float>(m_renderRect.left) + (static_cast<float>(m_renderRect.right) - static_cast<float>(m_renderRect.left)) - 0.5f - static_cast<float>(m_renderBox.right);
	const float ey = static_cast<float>(m_renderRect.top) + (static_cast<float>(m_renderRect.bottom) - static_cast<float>(m_renderRect.top)) - 0.5f - static_cast<float>(m_renderBox.bottom);

	const float texReverseWidth = 1.0f / (static_cast<float>(m_renderRect.right) - static_cast<float>(m_renderRect.left));
	const float texReverseHeight = 1.0f / (static_cast<float>(m_renderRect.bottom) - static_cast<float>(m_renderRect.top));

	const float su = m_renderBox.left * texReverseWidth;
	const float sv = m_renderBox.top * texReverseHeight;
	const float eu = ((m_renderRect.right - m_renderRect.left) - m_renderBox.right) * texReverseWidth;
	const float ev = ((m_renderRect.bottom - m_renderRect.top) - m_renderBox.bottom) * texReverseHeight;

	TPDTVertex pVertices[4];
	pVertices[0].position = TPosition(sx, sy, 0.0f);
	pVertices[0].texCoord = TTextureCoordinate(su, sv);
	pVertices[0].diffuse = 0xffffffff;

	pVertices[1].position = TPosition(ex, sy, 0.0f);
	pVertices[1].texCoord = TTextureCoordinate(eu, sv);
	pVertices[1].diffuse = 0xffffffff;

	pVertices[2].position = TPosition(sx, ey, 0.0f);
	pVertices[2].texCoord = TTextureCoordinate(su, ev);
	pVertices[2].diffuse = 0xffffffff;

	pVertices[3].position = TPosition(ex, ey, 0.0f);
	pVertices[3].texCoord = TTextureCoordinate(eu, ev);
	pVertices[3].diffuse = 0xffffffff;

	if (SetPDTStream(pVertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, GetRenderTargetTexture());
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
	STATEMANAGER.RestoreRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND);
}

void CGraphicRenderTargetTexture::Reset()
{
	Destroy();
	ReleaseTextures();
	m_d3dFormat = D3DFMT_A8R8G8B8;

	m_depthStencilFormat = D3DFMT_UNKNOWN;
}
#endif

