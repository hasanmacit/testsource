#include "StdAfx.h"
#include "SnowEnvironment.h"

#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "../EffectLib/EffectManager.h"
#include "SnowParticle.h"
#include "MapOutdoor.h"
#include "../gamelib/FlyingObjectManager.h"
#include "MapManager.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "../EterLib/GrpBase.h"
#include "Area.h"
#include "AreaTerrain.h"
#include "MapBase.h"
#include "TerrainDecal.h"
#include "../UserInterface/PythonBackground.h"

void CSnowEnvironment::Enable()
{
	if (!m_bSnowEnable)
	{
		Create();
	}

	m_bSnowEnable = TRUE;
}

void CSnowEnvironment::Disable()
{
	m_bSnowEnable = FALSE;
}

void CSnowEnvironment::Update(const D3DXVECTOR3& c_rv3Pos)
{
	if (!m_bSnowEnable)
	{
		if (m_kVct_pkParticleSnow.empty())
			return;
	}

	m_v3Center = c_rv3Pos;
}

void CSnowEnvironment::Deform()
{
	if (!m_bSnowEnable)
	{
		if (m_kVct_pkParticleSnow.empty())
			return;
	}

	const D3DXVECTOR3& c_rv3Pos = m_v3Center;

	static long s_lLastTime = CTimer::Instance().GetCurrentMillisecond();
	long lcurTime = CTimer::Instance().GetCurrentMillisecond();
	float fElapsedTime = float(lcurTime - s_lLastTime) / 1000.0f;
	s_lLastTime = lcurTime;

	CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3& c_rv3View = pCamera->GetView();

	D3DXVECTOR3 v3ChangedPos = c_rv3View * 3500.0f + c_rv3Pos;
	v3ChangedPos.z = c_rv3Pos.z;

	std::vector<CSnowParticle*>::iterator itor = m_kVct_pkParticleSnow.begin();
	for (; itor != m_kVct_pkParticleSnow.end();)
	{
		CSnowParticle* pSnow = *itor;
		pSnow->Update(fElapsedTime, v3ChangedPos);

		if (!pSnow->IsActivate())
		{
			CSnowParticle::Delete(pSnow);

			itor = m_kVct_pkParticleSnow.erase(itor);
		}
		else
		{
			++itor;
		}
	}

	if (m_bSnowEnable)
	{
		for (int p = 0; p < min(CPythonBackground::Instance().SelectRain, m_dwParticleMaxNum - m_kVct_pkParticleSnow.size()); ++p)
		{
			CSnowParticle* pSnowParticle = CSnowParticle::New();
			pSnowParticle->Init(v3ChangedPos);
			m_kVct_pkParticleSnow.push_back(pSnowParticle);
		}
	}
}

void CSnowEnvironment::__BeginBlur()
{
	if (!m_bBlurEnable)
		return;

#ifdef DIRECTX9
	ms_lpd3dDevice->GetRenderTarget(0, &m_lpOldSurface);
#else
	ms_lpd3dDevice->GetRenderTarget(&m_lpOldSurface);
#endif
	ms_lpd3dDevice->GetDepthStencilSurface(&m_lpOldDepthStencilSurface);
#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpSnowDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpSnowRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpSnowRenderTargetSurface, m_lpSnowDepthSurface);
#endif
	ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
}

void CSnowEnvironment::__ApplyBlur()
{
	if (!m_bBlurEnable)
		return;
	{
#ifdef DIRECTX9
		ms_lpd3dDevice->SetDepthStencilSurface(m_lpOldDepthStencilSurface);
		ms_lpd3dDevice->SetRenderTarget(0, m_lpOldSurface);
#else
		ms_lpd3dDevice->SetRenderTarget(m_lpOldSurface, m_lpOldDepthStencilSurface);
#endif

		STATEMANAGER.SetTexture(0, m_lpSnowTexture);
		STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		D3DSURFACE_DESC	desc;
		m_lpOldSurface->GetDesc(&desc);
		float sx = (float)desc.Width;
		float sy = (float)desc.Height;
		SAFE_RELEASE(m_lpOldSurface);
		SAFE_RELEASE(m_lpOldDepthStencilSurface);

		BlurVertex V[4] = { BlurVertex(D3DXVECTOR3(0.0f,0.0f,0.0f),1.0f	,0xFFFFFF, 0,0) ,
							BlurVertex(D3DXVECTOR3(sx,0.0f,0.0f),1.0f	,0xFFFFFF, 1,0) ,
							BlurVertex(D3DXVECTOR3(0.0f,sy,0.0f),1.0f	,0xFFFFFF, 0,1) ,
							BlurVertex(D3DXVECTOR3(sx,sy,0.0f),1.0f		,0xFFFFFF, 1,1) };

#ifdef DIRECTX9
		STATEMANAGER.SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#else
		STATEMANAGER.SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#endif
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(BlurVertex));
	}
}

void CSnowEnvironment::Render()
{
	if (!m_bSnowEnable)
	{
		if (m_kVct_pkParticleSnow.empty())
			return;
	}

	__BeginBlur();

	DWORD dwParticleCount = min(m_dwParticleMaxNum, m_kVct_pkParticleSnow.size());

	CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3& c_rv3Up = pCamera->GetUp();
	const D3DXVECTOR3& c_rv3Cross = pCamera->GetCross();

	SParticleVertex* pv3Verticies;
	if (SUCCEEDED(m_pVB->Lock(0, sizeof(SParticleVertex) * dwParticleCount * 4, DXLOCKTYPE & pv3Verticies, D3DLOCK_DISCARD)))
	{
		int i = 0;
		std::vector<CSnowParticle*>::iterator itor = m_kVct_pkParticleSnow.begin();
		for (; i < dwParticleCount && itor != m_kVct_pkParticleSnow.end(); ++i, ++itor)
		{
			CSnowParticle* pSnow = *itor;
			pSnow->SetCameraVertex(c_rv3Up, c_rv3Cross);
			pSnow->GetVerticies(pv3Verticies[i * 4 + 0],
				pv3Verticies[i * 4 + 1],
				pv3Verticies[i * 4 + 2],
				pv3Verticies[i * 4 + 3]);

		}
		m_pVB->Unlock();
	}

	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pImageInstance->GetGraphicImagePointer()->GetTextureReference().SetTextureStage(0);
	STATEMANAGER.SetIndices(m_pIB, 0);
	STATEMANAGER.SetStreamSource(0, m_pVB, sizeof(SParticleVertex));
#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
#endif
	STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwParticleCount * 4, 0, dwParticleCount * 2);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	__ApplyBlur();
}

bool CSnowEnvironment::__CreateBlurTexture()
{
	if (!m_bBlurEnable)
		return true;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpSnowTexture, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpSnowTexture)))
#endif
		return false;
	if (FAILED(m_lpSnowTexture->GetSurfaceLevel(0, &m_lpSnowRenderTargetSurface)))
		return false;
#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &m_lpSnowDepthSurface, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, &m_lpSnowDepthSurface)))
#endif
		return false;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpAccumTexture, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpAccumTexture)))
#endif
		return false;
	if (FAILED(m_lpAccumTexture->GetSurfaceLevel(0, &m_lpAccumRenderTargetSurface)))
		return false;
#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &m_lpAccumDepthSurface, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, &m_lpAccumDepthSurface)))
#endif
		return false;

	return true;
}

bool CSnowEnvironment::__CreateGeometry()
{
#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(SParticleVertex) * m_dwParticleMaxNum * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_SYSTEMMEM, &m_pVB, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(SParticleVertex) * m_dwParticleMaxNum * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_SYSTEMMEM, &m_pVB)))
#endif
		return false;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(WORD) * m_dwParticleMaxNum * 6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
#else
	if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(WORD) * m_dwParticleMaxNum * 6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB)))
#endif
		return false;

	WORD* dstIndices;
	if (FAILED(m_pIB->Lock(0, sizeof(WORD) * m_dwParticleMaxNum * 6, DXLOCKTYPE & dstIndices, 0)))
		return false;

	const WORD c_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };
	for (int i = 0; i < m_dwParticleMaxNum; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{
			dstIndices[i * 6 + j] = i * 4 + c_awFillRectIndices[j];
		}
	}

	m_pIB->Unlock();
	return true;
}

bool CSnowEnvironment::Create()
{
	Destroy();

	if (!__CreateGeometry())
		return false;


	if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") != 0)
	{
		CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/rain.dds");
		m_pImageInstance = CGraphicImageInstance::New();
		m_pImageInstance->SetImagePointer(pImage);
	}
	else
	{
		CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/snow.dds");
		m_pImageInstance = CGraphicImageInstance::New();
		m_pImageInstance->SetImagePointer(pImage);
	}


	return true;
}

void CSnowEnvironment::Destroy()
{
	SAFE_RELEASE(m_lpSnowTexture);
	SAFE_RELEASE(m_lpSnowRenderTargetSurface);
	SAFE_RELEASE(m_lpSnowDepthSurface);
	SAFE_RELEASE(m_lpAccumTexture);
	SAFE_RELEASE(m_lpAccumRenderTargetSurface);
	SAFE_RELEASE(m_lpAccumDepthSurface);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);

	stl_wipe(m_kVct_pkParticleSnow);
	CSnowParticle::DestroyPool();

	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = NULL;
	}

	__Initialize();
}

void CSnowEnvironment::__Initialize()
{

	m_bSnowEnable = FALSE;
	m_lpSnowTexture = NULL;
	m_lpSnowRenderTargetSurface = NULL;
	m_lpSnowDepthSurface = NULL;
	m_lpAccumTexture = NULL;
	m_lpAccumRenderTargetSurface = NULL;
	m_lpAccumDepthSurface = NULL;
	m_pVB = NULL;
	m_pIB = NULL;
	m_pImageInstance = NULL;

	m_kVct_pkParticleSnow.reserve(m_dwParticleMaxNum);
}

CSnowEnvironment::CSnowEnvironment()
{
	m_bBlurEnable = FALSE;
	m_dwParticleMaxNum = 3000;
	m_wBlurTextureSize = 512;

	__Initialize();
}
CSnowEnvironment::~CSnowEnvironment()
{
	Destroy();
}