#pragma once
#ifdef ENABLE_RENDER_TARGET
#include "GrpBase.h"
#include "GrpTexture.h"

class CGraphicRenderTargetTexture : public CGraphicTexture
{
public:
	CGraphicRenderTargetTexture();
	virtual ~CGraphicRenderTargetTexture();

public:
	bool Create(int width, int height, D3DFORMAT texFormat, D3DFORMAT depthFormat);
	void CreateTextures();
	bool CreateRenderTexture(int width, int height, D3DFORMAT format);
	bool CreateRenderDepthStencil(int width, int height, D3DFORMAT format);

	void SetRenderTarget();
	void ResetRenderTarget();

	void SetRenderingRect(RECT* rect) { m_renderRect = *rect; }
	void SetRenderingBox(RECT* rect) { m_renderBox = *rect; }
	RECT* GetRenderingRect() { return &m_renderRect; }
	RECT* GetRenderingBox() { return &m_renderBox; }

	LPDIRECT3DTEXTURE8 GetRenderTargetTexture() const { return m_lpd3dRenderTexture; }

	void ReleaseTextures();
	static void Clear();

	void Render() const;

protected:
	void Reset();
	LPDIRECT3DTEXTURE8 m_lpd3dRenderTexture{};
	LPDIRECT3DSURFACE8 m_lpd3dRenderTargetSurface{};
	LPDIRECT3DSURFACE8 m_lpd3dDepthSurface{};

	LPDIRECT3DSURFACE8 m_lpd3dOriginalRenderTarget{};
	LPDIRECT3DSURFACE8 m_lpd3dOldDepthBufferSurface{};
	D3DFORMAT m_d3dFormat;
	D3DFORMAT m_depthStencilFormat;
	RECT m_renderRect{};
	RECT m_renderBox{};
};
#endif
