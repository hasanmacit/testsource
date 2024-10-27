#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/ResourceManager.h"
#include "../EterLib/Camera.h"
#include "../UserInterface/PythonBackground.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "FlyingObjectManager.h"
#include "MapManager.h"
#include "MapOutdoor.h"
#include <unordered_set>

void CMapOutdoor::LoadWaterTexture()
{
	UnloadWaterTexture();
	char buf[256];
	for (int i = 0; i < 30; ++i)
	{
		if (std::strcmp(currentMapName.c_str(), "metin2_map_n_flame_dungeon_01") == 0 ||
			std::strcmp(currentMapName.c_str(), "metin2_map_n_flame_01") == 0)
		{
			Frustum::Instance().renderOtherWater = true;
			sprintf(buf, "d:/ymir Work/special/lava/%02d.dds", i + 1);
		}
		else if (std::strcmp(currentMapName.c_str(), "map_n_snowm_01") == 0)
		{
			Frustum::Instance().renderOtherWater = true;
			sprintf(buf, "d:/ymir Work/special/ice/%02d.dds", i + 1);
		}
		else
		{
			Frustum::Instance().renderOtherWater = false;
			sprintf(buf, "d:/ymir Work/special/water/%02d.dds", i + 1);
		}

		m_WaterInstances[i].SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(buf));
	}

	if (std::strcmp(currentMapName.c_str(), "metin2_map_n_desert_01") == 0 ||
		std::strcmp(currentMapName.c_str(), "map_n_snowm_01") == 0 ||
		std::strcmp(currentMapName.c_str(), "metin2_map_n_flame_01") == 0)
	{
		Frustum::Instance().renderGrassShadow = true;
	}
	else
	{
		Frustum::Instance().renderGrassShadow = false;
	}

	if (std::strcmp(currentMapName.c_str(), "metin2_map_lobby") == 0)
	{
		Frustum::Instance().isLobbyMap = true;
	}
	else
	{
		Frustum::Instance().isLobbyMap = false;
	}
}

void CMapOutdoor::UnloadWaterTexture()
{
	for (int i = 0; i < 30; ++i)
		m_WaterInstances[i].Destroy();
}

float time2 = 0.0f;
int updateCharacterPosition = 0;
D3DXVECTOR3 rippleOrigin = { 0.0f,0.0f,0.0f };
void CMapOutdoor::RenderWater()
{
	if (m_PatchVector.empty())
		return;

	if (!IsVisiblePart(PART_WATER))
		return;

	//ms_lpd3dDevice->SetPixelShader(CPythonBackground::Instance().pBlurShadowBuilding);
	//////////////////////////////////////////////////////////////////////////
	// RenderState
	D3DXMATRIX matTexTransformWater;
	//MapTextureReflection();

	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);



	STATEMANAGER.SetTexture(0, m_WaterInstances[((ELTimer_GetMSec() / 70) % 30)].GetTexturePointer()->GetD3DTexture());


	if (Frustum::Instance().shadowType == 2)
	{
		STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
	}
	else
	{
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetTexture(2, NULL);
	}
	//STATEMANAGER.SetTexture(2, m_lpCharacterShadowMapTexture);




	D3DXMatrixScaling(&matTexTransformWater, m_fWaterTexCoordBase*2, -m_fWaterTexCoordBase*2, 0.0f);
	D3DXMatrixMultiply(&matTexTransformWater, &m_matViewInverse, &matTexTransformWater);

	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &matTexTransformWater);

	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);


	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	// Stage 0
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	// Stage 1

	// RenderState
	//////////////////////////////////////////////////////////////////////////

	static float s_fWaterHeightCurrent = 0;
	static float s_fWaterHeightBegin = 0;
	static float s_fWaterHeightEnd = 0;
	static DWORD s_dwLastHeightChangeTime = CTimer::Instance().GetCurrentMillisecond();
	static DWORD s_dwBlendtime = 300;

	if ((CTimer::Instance().GetCurrentMillisecond() - s_dwLastHeightChangeTime) > s_dwBlendtime)
	{
		s_dwBlendtime = random_range(1000, 6000);

		if (s_fWaterHeightEnd == 0)
			s_fWaterHeightEnd = -random_range(0, 30);
		else
			s_fWaterHeightEnd = 0;

		s_fWaterHeightBegin = s_fWaterHeightCurrent;
		s_dwLastHeightChangeTime = CTimer::Instance().GetCurrentMillisecond();
	}

	s_fWaterHeightCurrent = s_fWaterHeightBegin + (s_fWaterHeightEnd - s_fWaterHeightBegin) * (float)((CTimer::Instance().GetCurrentMillisecond() - s_dwLastHeightChangeTime) / (float)s_dwBlendtime);
	m_matWorldForCommonUse._43 = s_fWaterHeightCurrent;

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

	float fFogDistance = __GetFogDistance() * 10;

	std::vector<std::pair<float, long> >::iterator i;

	for (i = m_PatchVector.begin(); i != m_PatchVector.end(); ++i)
	{
		DrawWater(i->second);
	}

	time2 += 0.1f;
	updateCharacterPosition = updateCharacterPosition + 1;

	if (updateCharacterPosition == 1000)
		updateCharacterPosition = 0;

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	m_matWorldForCommonUse._43 = 0.0f;

	//////////////////////////////////////////////////////////////////////////
	// RenderState
#ifdef DIRECTX9
	STATEMANAGER.RestoreFVF();
#else
	STATEMANAGER.RestoreVertexShader();
#endif
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	//STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);
#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);

	//STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
	//STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
	//STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
	//STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
	//STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MIPFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSV);
#endif
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

	//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);

	STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	//ms_lpd3dDevice->SetPixelShader(nullptr);
}

void CMapOutdoor::DrawWater(long patchnum)
{
	assert(NULL != m_pTerrainPatchProxyList);
	if (!m_pTerrainPatchProxyList)
		return;

	CTerrainPatchProxy& rkTerrainPatchProxy = m_pTerrainPatchProxyList[patchnum];

	if (!rkTerrainPatchProxy.isUsed())
		return;

	if (!rkTerrainPatchProxy.isWaterExists())
		return;

	CGraphicVertexBuffer* pkVB = rkTerrainPatchProxy.GetWaterVertexBufferPointer();
	if (!pkVB)
		return;

	if (!pkVB->GetD3DVertexBuffer())
		return;

	UINT uPriCount = rkTerrainPatchProxy.GetWaterFaceCount();
	if (!uPriCount)
		return;

	if (Frustum::Instance().shadowType == 3 || !Frustum::Instance().realtimeSpecular)
	{

	}
	else
	{
		STATEMANAGER.SetVertexShader(m_lpShadowMapVertexShaderWater);
		STATEMANAGER.SetPixelShader(m_lpShadowMapPixelShaderWater);
		

		// Transformations

		D3DXMATRIX worldMat;
		STATEMANAGER.GetTransform(D3DTS_WORLD, &worldMat);
		D3DXMatrixTranspose(&worldMat, &worldMat);
		STATEMANAGER.SetVertexShaderConstant(20, &worldMat, 4);
		// View-Projection
		D3DXMATRIX viewProjMat, viewMat, projMat;
		STATEMANAGER.GetTransform(D3DTS_VIEW, &viewMat);
		STATEMANAGER.GetTransform(D3DTS_PROJECTION, &projMat);
		D3DXMatrixMultiplyTranspose(&viewProjMat, &viewMat, &projMat);
		STATEMANAGER.SetVertexShaderConstant(4, &viewProjMat, 4);
		// Light space
		STATEMANAGER.SetVertexShaderConstant(8, &m_MatLightViewProj, 4);
		STATEMANAGER.SetVertexShaderConstant(30, &time2, 1);
		float waveSpeed = 0.75f;
		float WaveHeight = 0.2f;
		float thresholdDistance = 10.5f;
		D3DXVECTOR2 waveFrequency = { 0.5f,0.5f };
		STATEMANAGER.SetVertexShaderConstant(31, &waveSpeed, 1);
		STATEMANAGER.SetVertexShaderConstant(32, &WaveHeight, 1);
		STATEMANAGER.SetVertexShaderConstant(33, &waveFrequency, 1);


		if (updateCharacterPosition == 0)
		{
			rippleOrigin = Frustum::Instance().characterPosition;
		}
		STATEMANAGER.SetVertexShaderConstant(34, &rippleOrigin, 1);

		if (Frustum::Instance().shadowType == 2)
		{
			STATEMANAGER.SetVertexShaderConstant(16, &m_MatLightViewProj2, 4);
		}
		else
		{
			STATEMANAGER.SetVertexShaderConstant(16, &m_MatLightViewProj, 4);
		}

		D3DMATERIAL9 material;
		STATEMANAGER.GetMaterial(&material);
		D3DLIGHT8 light;
		STATEMANAGER.GetLight(0, &light);
		STATEMANAGER.SetPixelShaderConstant(0, &material.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(1, &current_v3SunPosition, 1);
		STATEMANAGER.SetPixelShaderConstant(2, &light.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(3, &light.Diffuse, 1);
		// Fog settings
		D3DVECTOR cameraPos = CCameraManager::instance().GetCurrentCamera()->GetEye();
		STATEMANAGER.SetPixelShaderConstant(4, &cameraPos, 1);
		D3DCOLOR fogColor;
		STATEMANAGER.GetRenderState(D3DRS_FOGCOLOR, &fogColor);
		D3DXCOLOR fogColorX(fogColor);
		STATEMANAGER.SetPixelShaderConstant(5, (float*)fogColorX, 1);
		D3DVECTOR fogProps;
		STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fogProps.x);
		STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fogProps.y);
		STATEMANAGER.SetPixelShaderConstant(6, &fogProps, 1);

		STATEMANAGER.SetPixelShaderConstant(7, &light.Diffuse, 1);

		float shadowOpacity = GetShadowIntensity() / 1.1;
		if (shadowOpacity < 42.0f)
			shadowOpacity = 42.0f;
		STATEMANAGER.SetPixelShaderConstant(10, (int*)&shadowOpacity, 1);

		float specularLightEnabled = (float)Frustum::Instance().realtimeSpecular;
		STATEMANAGER.SetPixelShaderConstant(11, &specularLightEnabled, 1);

		float enableLights;

		if (Frustum::Instance().ambianceEffectsLevel == 0)
			enableLights = 0.0f;
		if (Frustum::Instance().ambianceEffectsLevel == 1)
			enableLights = 1.0f;

		STATEMANAGER.SetPixelShaderConstant(13, &enableLights, 1);

		float shiness;
		D3DXVECTOR3 specularLight;
		D3DXVECTOR3 tint;

		// Conditionally check the Frustum instance for different environment types
		if (Frustum::Instance().isCerApus4Environment || Frustum::Instance().isCerApus4EnvironmentAzrael)
		{
			shiness = 64.0f;
			specularLight = { 11.5f, 11.5f, 11.5f };

			// Specific maps to override the shiness and specularLight values
			std::unordered_set<std::string> specialMaps = {
				"metin2_map_defensewave",
				"metin2_map_n_desert_01",
				"metin2_map_lobby"
			};

			if (specialMaps.find(currentMapName) != specialMaps.end())
			{
				shiness = 64.0f;
				specularLight = { 17.5f, 17.5f, 17.5f };
			}

			tint = { 1.0f,0.5f,0.0f };

			if (Frustum::Instance().isCerApus4EnvironmentAzrael)
			{
				shiness = 69.0f;
				specularLight = { 12.75f, 12.75f, 12.75f };
			}

			STATEMANAGER.SetPixelShaderConstant(12, &tint, 1);
	}
		else
		{
			shiness = 100.0f;
			specularLight = { 0.08f, 0.08f, 0.08f };
			STATEMANAGER.SetPixelShaderConstant(12, &light.Ambient, 1);
		}

		// Set common shader constants
		STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
		STATEMANAGER.SetPixelShaderConstant(9, &specularLight, 1);

}

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), sizeof(SWaterVertex));
	STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, uPriCount);

	ms_faceCount += uPriCount;

	if (Frustum::Instance().shadowType == 3 || !Frustum::Instance().realtimeSpecular)
	{

	}
	else
	{
		STATEMANAGER.SetVertexShader(nullptr);
		STATEMANAGER.SetPixelShader(nullptr);
	}

}
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
