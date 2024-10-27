#include "StdAfx.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "AreaTerrain.h"
#include "TerrainQuadtree.h"
#include "FlyingObjectManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/StateManager.h"
#include "../UserInterface/PythonBackground.h"

#define MAX_RENDER_SPALT 150

CArea::TCRCWithNumberVector m_dwRenderedCRCWithNumberVector;

CMapOutdoor::TTerrainNumVector CMapOutdoor::FSortPatchDrawStructWithTerrainNum::m_TerrainNumVector;

void CMapOutdoor::RenderTerrain()
{
	

	if (!IsVisiblePart(PART_TERRAIN))
		return;

	if (!m_bSettingTerrainVisible)
		return;

	if (!m_pTerrainPatchProxyList)
		return;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	auto val = ms_matView * ms_matProj;
	BuildViewFrustum(val);

	D3DXVECTOR3 v3Eye = pCamera->GetEye();
	m_fXforDistanceCaculation = -v3Eye.x;
	m_fYforDistanceCaculation = -v3Eye.y;

	//////////////////////////////////////////////////////////////////////////
	// Push
	m_PatchVector.clear();

	__RenderTerrain_RecurseRenderQuadTree(m_pRootNode);

	std::sort(m_PatchVector.begin(),m_PatchVector.end());

	if (CTerrainPatch::SOFTWARE_TRANSFORM_PATCH_ENABLE)
		__RenderTerrain_RenderSoftwareTransformPatch();
	else
		__RenderTerrain_RenderHardwareTransformPatch();
}

void CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree(CTerrainQuadtreeNode* Node, bool bCullCheckNeed)
{

		bCullCheckNeed = false;
		if (bCullCheckNeed)
		{
			switch (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius))
			{
			case VIEW_ALL:
				// all child nodes need not cull check
				bCullCheckNeed = false;
				break;
			case VIEW_PART:
				break;
			case VIEW_NONE:
				// no need to render
				return;
			}
			// if no need cull check more
			// -> bCullCheckNeed = false;
		}

		if (Node->Size == 1)
		{
			D3DXVECTOR3 v3Center = Node->center;
			__RenderTerrain_AppendPatch(v3Center, 0.0f, Node->PatchNum);
		}
		else
		{
			if (Node->NW_Node != NULL)
				__RenderTerrain_RecurseRenderQuadTree(Node->NW_Node, bCullCheckNeed);
			if (Node->NE_Node != NULL)
				__RenderTerrain_RecurseRenderQuadTree(Node->NE_Node, bCullCheckNeed);
			if (Node->SW_Node != NULL)
				__RenderTerrain_RecurseRenderQuadTree(Node->SW_Node, bCullCheckNeed);
			if (Node->SE_Node != NULL)
				__RenderTerrain_RecurseRenderQuadTree(Node->SE_Node, bCullCheckNeed);
		}

}

int	CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(const D3DXVECTOR3& c_v3Center, const float& c_fRadius)
{

		return VIEW_ALL;

}

void CMapOutdoor::__RenderTerrain_AppendPatch(const D3DXVECTOR3& c_rv3Center, float fDistance, long lPatchNum)
{
	if (Frustum::Instance().treeShadow < 2)
	{
		assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::__RenderTerrain_AppendPatch");
		if (!m_pTerrainPatchProxyList[lPatchNum].isUsed())
			return;
	}

	m_pTerrainPatchProxyList[lPatchNum].SetCenterPosition(c_rv3Center);
	m_PatchVector.push_back(std::make_pair(fDistance, lPatchNum));
}

void CMapOutdoor::ApplyLight(DWORD dwVersion, const D3DLIGHT8& c_rkLight)
{
	m_kSTPD.m_dwLightVersion=dwVersion;
	STATEMANAGER.SetLight(0, &c_rkLight);
}

void CMapOutdoor::InitializeVisibleParts()
{
	m_dwVisiblePartFlags=0xffffffff;
}

void CMapOutdoor::SetVisiblePart(int ePart, bool isVisible)
{
	DWORD dwMask=(1<<ePart);
	if (isVisible)
	{
		m_dwVisiblePartFlags|=dwMask;
	}
	else
	{
		DWORD dwReverseMask=~dwMask;
		m_dwVisiblePartFlags&=dwReverseMask;
	}
}

bool CMapOutdoor::IsVisiblePart(int ePart)
{
	DWORD dwMask=(1<<ePart);
	if (dwMask & m_dwVisiblePartFlags)
		return true;

	return false;
}

void CMapOutdoor::SetSplatLimit(int iSplatNum)
{
	m_iSplatLimit = iSplatNum;
}

std::vector<int> & CMapOutdoor::GetRenderedSplatNum(int * piPatch, int * piSplat, float * pfSplatRatio)
{
	*piPatch = m_iRenderedPatchNum;
	*piSplat = m_iRenderedSplatNum;
	*pfSplatRatio = m_iRenderedSplatNumSqSum/float(m_iRenderedPatchNum);

	return m_RenderedTextureNumVector;
}

CArea::TCRCWithNumberVector & CMapOutdoor::GetRenderedGraphicThingInstanceNum(DWORD * pdwGraphicThingInstanceNum, DWORD * pdwCRCNum)
{
	*pdwGraphicThingInstanceNum = m_dwRenderedGraphicThingInstanceNum;
	*pdwCRCNum = m_dwRenderedCRCNum;

	return m_dwRenderedCRCWithNumberVector;
}

void CMapOutdoor::RenderBeforeLensFlare()
{
	m_LensFlare.DrawBeforeFlare();

	if (!mc_pEnvironmentData)
	{
		TraceError("CMapOutdoor::RenderBeforeLensFlare mc_pEnvironmentData is NULL");
		return;
	}

	m_LensFlare.Compute(mc_pEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction);
}

void CMapOutdoor::RenderAfterLensFlare()
{
	m_LensFlare.AdjustBrightness();
	m_LensFlare.DrawFlare();
}

void CMapOutdoor::RenderCollision()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (GetAreaPointer(i, &pArea))
			pArea->RenderCollision();
	}
}

void CMapOutdoor::RenderScreenFiltering()
{
	m_ScreenFilter.Render();
}

void CMapOutdoor::RenderSky()
{
	if (IsVisiblePart(PART_SKY))
		m_SkyBox.Render();
}

void CMapOutdoor::RenderCloud()
{
	if (IsVisiblePart(PART_CLOUD))
		m_SkyBox.RenderCloud();
}

void CMapOutdoor::RenderTree(unsigned long flags)
{
	if (IsVisiblePart(PART_TREE))
		CSpeedTreeForestDirectX8::Instance().Render(Forest_RenderAll | flags);
}

int z = 0.0f;
void CMapOutdoor::SetInverseViewAndDynamicShaodwMatrices()
{
	CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCamera)
		return;

	m_matViewInverse = pCamera->GetInverseViewMatrix();

	if (Frustum::Instance().isIndoor == true)
	{
		const TEnvironmentData* envData = 0;
		CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);

		D3DXVECTOR3 v3SunPosition = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
		D3DXVECTOR3 v3Target = pCamera->GetTarget();
		v3SunPosition.z = -0.65f;

		D3DXVECTOR3 v3SunPosition2 = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
		v3SunPosition2 = (v3Target)-v3SunPosition2 * (-200000.0f);
		v3SunPosition2.z = -0.65f;

		D3DXVECTOR3 v3LightEye(v3Target.x - 1.732f * 1250.0f,
			v3Target.y - 1250.0f,
			v3Target.z + 2.0f * 1.732f * 1250.0f);

		const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		D3DXVECTOR3 c_v3SunPosition;
		if (Frustum::Instance().isIndoor == true)
		{
			c_v3SunPosition = (v3Target)-current_v3SunPosition * (200000.0f);
		}
		else
		{
			c_v3SunPosition = (v3Target)-current_v3SunPosition * (-200000.0f);
		}

		const auto c_v3SunPosition_add = (v3Target)-current_v3SunPosition * (200000.0f);
		D3DXMatrixLookAtRH(&m_matLightView, &c_v3SunPosition_add, &v3Target, &vv);
		D3DXMatrixLookAtRH(&m_matLightView2, &c_v3SunPosition, &v3Target, &vv);
		m_matDynamicShadow = m_matViewInverse * m_matLightView * m_matDynamicShadowScale;
		m_matDynamicShadow2 = m_matViewInverse * m_matLightView * m_matDynamicShadowScale2;
		m_matDynamicShadow3 = m_matViewInverse * m_matLightView2 * m_matDynamicShadowScale3;
		m_matDynamicShadow5 = m_matViewInverse * m_matLightView2 * m_matDynamicShadowScale5;
	}
}

void CMapOutdoor::OnRender()
{
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=ELTimer_GetMSec();
	SetInverseViewAndDynamicShaodwMatrices();

	SetBlendOperation();
	DWORD t2=ELTimer_GetMSec();
	RenderArea();
	DWORD t3=ELTimer_GetMSec();
	if (!m_bEnableTerrainOnlyForHeight)
		RenderTerrain();
	DWORD t4=ELTimer_GetMSec();
	RenderTree();
	DWORD t5=ELTimer_GetMSec();
	DWORD tEnd=ELTimer_GetMSec();

	if (tEnd-t1<7)
		return;

	static FILE* fp=fopen("perf_map_render.txt", "w");
 	fprintf(fp, "MAP.Total %d (Time %d)\n", tEnd-t1, ELTimer_GetMSec());
	fprintf(fp, "MAP.ENV %d\n", t2-t1);
	fprintf(fp, "MAP.OBJ %d\n", t3-t2);
	fprintf(fp, "MAP.TRN %d\n", t4-t3);
	fprintf(fp, "MAP.TRE %d\n", t5-t4);

#else
	SetInverseViewAndDynamicShaodwMatrices();

	SetBlendOperation();
	RenderArea();
	RenderTree(Forest_RenderAll);
	if (!m_bEnableTerrainOnlyForHeight)
		RenderTerrain();
	RenderBlendArea();
#endif
}

void CMapOutdoor::OnRenderShadow()
{
	SetInverseViewAndDynamicShaodwMatrices();
	if (Frustum::Instance().treeShadow >= 2)
	{

		// Should terrain cast shadows (self shadows)
		__RenderTerrain_RenderHardwareTransformPatch2();
		
		RenderArea2();
	}
}

struct FAreaRenderShadow
{
	void operator () (CGraphicObjectInstance * pInstance)
	{
		pInstance->RenderShadow();
		pInstance->Hide();
	}
};

struct FPCBlockerHide
{
	void operator () (CGraphicObjectInstance * pInstance)
	{
#ifndef JETTYX_TRANSPARENCY_BUILDING_FIX
		pInstance->Hide();
#endif
	}
};

struct FRenderPCBlocker
{
	void operator () (CGraphicObjectInstance * pInstance)
	{
		pInstance->Show();
		CGraphicThingInstance* pThingInstance = dynamic_cast <CGraphicThingInstance*> (pInstance);
		if (pThingInstance != NULL)
		{
			if (pThingInstance->HaveBlendThing())
			{
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
				pThingInstance->BlendRender();
				return;
			}
		}
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

#ifndef JETTYX_TRANSPARENCY_BUILDING_FIX
		pInstance->RenderPCBlocker();
#endif
	}
};

void CMapOutdoor::RenderEffect()
{
	if (Frustum::Instance().ambianceEffectsLevel == 0)
	{
		if (!IsVisiblePart(PART_OBJECT))
			return;
		for (int i = 0; i < AROUND_AREA_NUM; ++i)
		{
			CArea* pArea;
			if (GetAreaPointer(i, &pArea))
			{
				pArea->RenderEffect();
			}
		}
	}
}

struct CMapOutdoor_LessThingInstancePtrRenderOrder
{
	bool operator() (CGraphicThingInstance* pkLeft, CGraphicThingInstance* pkRight)
	{
		CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
		const D3DXVECTOR3 & c_rv3CameraPos = pCurrentCamera->GetEye();
		const D3DXVECTOR3 & c_v3LeftPos  = pkLeft->GetPosition();
		const D3DXVECTOR3 & c_v3RightPos = pkRight->GetPosition();

		auto val1 = D3DXVECTOR3(c_rv3CameraPos - c_v3LeftPos);
		auto val2 = D3DXVECTOR3(c_rv3CameraPos - c_v3RightPos);
		return D3DXVec3LengthSq(&val1) < D3DXVec3LengthSq(&val2 );
	}
};

struct CMapOutdoor_FOpaqueThingInstanceRender
{
	inline void operator () (CGraphicThingInstance * pkThingInst)
	{
		pkThingInst->Render();
	}
};

struct CMapOutdoor_FOpaqueThingInstanceRenderShadow
{
	inline void operator () (CGraphicThingInstance* pkThingInst)
	{
		pkThingInst->RenderToShadowMap();
	}
};

struct CMapOutdoor_FBlendThingInstanceRender
{
	inline void operator () (CGraphicThingInstance * pkThingInst)
	{
		pkThingInst->BlendRender();
	}
};

static std::vector<CGraphicThingInstance*> s_kVct_pkOpaqueThingInstSort;
float time4 = 0.0f;
void CMapOutdoor::RenderArea(bool bRenderAmbience)
{
	if (!IsVisiblePart(PART_OBJECT))
		return;

	m_dwRenderedCRCNum = 0;
	m_dwRenderedGraphicThingInstanceNum = 0;
	m_dwRenderedCRCWithNumberVector.clear();

	if (std::strcmp(currentMapName.c_str(), "metin2_map_defensewave") != 0)
	{
		for (int j = 0; j < AROUND_AREA_NUM; ++j)
		{
			CArea* pArea;
			if (GetAreaPointer(j, &pArea))
			{
				pArea->RenderDungeon();
			}
		}
	}

	time4 += 0.1f;

	///* Save shadowmap for debug
	//if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	 // HRESULT hr = D3DXSaveTextureToFile("depth.png", D3DXIFF_PNG, m_lpCharacterShadowMapTexture, nullptr); //*/
	if (Frustum::Instance().shadowType != 3)
	{
		if (Frustum::Instance().isIndoor == false)
		{
			if (Frustum::Instance().shadowType == 1)
			{
				STATEMANAGER.SetVertexShader(m_lpShadowMapVertexShaderObjects2);
				STATEMANAGER.SetPixelShader(m_lpShadowMapPixelShaderObjects2);

			}

			if (Frustum::Instance().shadowType == 0)
			{
				STATEMANAGER.SetVertexShader(m_lpShadowMapVertexShaderObjects3);
				STATEMANAGER.SetPixelShader(m_lpShadowMapPixelShaderObjects3);
			}

			if (Frustum::Instance().shadowType == 2)
			{
				STATEMANAGER.SetVertexShader(m_lpShadowMapVertexShaderObjects);
				STATEMANAGER.SetPixelShader(m_lpShadowMapPixelShaderObjects);
			}

			// Vertex shader constants ----
			// Transformation matrix
			D3DXMATRIX worldMat;
			//STATEMANAGER.GetTransform(D3DTS_WORLD, &worldMat);
			ms_lpd3dDevice->GetTransform(D3DTS_WORLD, &worldMat);
			D3DXMatrixTranspose(&worldMat, &worldMat);
			//D3DXMatrixIdentity(&worldMat);
			ms_lpd3dDevice->SetVertexShaderConstantF(0, (const float*)(&worldMat), 4);
			// View-Projection
			D3DXMATRIX viewProjMat, viewMat, projMat;
			ms_lpd3dDevice->GetTransform(D3DTS_VIEW, &viewMat);
			ms_lpd3dDevice->GetTransform(D3DTS_PROJECTION, &projMat);
			// Projection matrix is not w-friendly, so fog doesn't work properly
			//ms_lpd3dDevice->SetRenderState(D3DRS_FOGENABLE, 1); // replaced with fog in the shader

			D3DXMatrixMultiplyTranspose(&viewProjMat, &viewMat, &projMat);
			ms_lpd3dDevice->SetVertexShaderConstantF(4, (const float*)(&viewProjMat), 4);
			// Light space
			ms_lpd3dDevice->SetVertexShaderConstantF(8, (const float*)(&m_MatLightViewProj), 4);
		
			ms_lpd3dDevice->SetVertexShaderConstantF(16, (const float*)(&m_MatLightViewProj2), 4);
			

			// Light ViewProj matrix (TODO)


			// Pixel shader constants ----
			//D3DXVECTOR4 diffuseMatColor = { 1.0f, 1.0f, 1.0f, 1.0f };
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

			STATEMANAGER.SetPixelShaderConstant(7, &material.Diffuse, 1);

			float shiness;
			D3DXVECTOR3 specularLight;
			float reducePointLightIntensity = 2.0f;

			if (Frustum::Instance().selectRainFrustun == 0)
			{
				shiness = 10.0f;
				specularLight = { 2.5f,2.5f,2.5f };

				if (Frustum::Instance().isCerApus4Environment == true || Frustum::Instance().isCerApus4EnvironmentAzrael == true)
				{
					shiness = 12.0f;
					specularLight = { 2.5f,2.5f,2.5f };
				}

				if (isPinkEnvironment == true)
				{
					shiness = 7.0f;
					specularLight = { 15.0f,15.0f,15.0f };
				}

				if (isNoapteEnvironment == true || isEnvironment6 == true || isEnvironment7 == true)
				{
					shiness = 7.0f;
					specularLight = { 0.75f, 0.75f, 0.75f };
				}

				if (isEnvironment9 == true || isEnvironment10 == true || isEnvironment11 == true)
				{
					shiness = 7.0f;
					specularLight = { 2.0f,2.0f,2.0f };
				}

				if (isEnvironment2 == true)
				{
					shiness = 7.0f;
					specularLight = { 2.0f,2.0f,2.0f };
				}

				if (isEnvironment3 == true)
				{
					shiness = 7.0f;
					specularLight = { 2.0f,2.0f,2.0f };
				}

				if (isEnvironment4 == true)
				{
					shiness = 7.0f;
					specularLight = { 0.5f,0.5f,0.5f };
				}

				if (isEnvironment5 == true)
				{
					shiness = 87.0f;
					specularLight = { 1.75f, 1.75f, 1.75f };
				}
			}
			else
			{
				if (Frustum::Instance().isThunderTime == true)
				{
					shiness = 17.0f;
					specularLight = { Frustum::Instance().specularRainPower,Frustum::Instance().specularRainPower,Frustum::Instance().specularRainPower };
				}
				else
				{
					shiness = 69.0f;
					specularLight = { 9.7f, 9.7f, 9.7f };
				}
			}

			if (isNoapteEnvironment || isEnvironment5 || isEnvironment11)
			{
				reducePointLightIntensity = Frustum::Instance().pointLightIntensity = 1.0f;
			}

			STATEMANAGER.SetPixelShaderConstant(15, &reducePointLightIntensity, 1);

			STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
			STATEMANAGER.SetPixelShaderConstant(9, &specularLight, 1);



			float shadowOpacity = GetShadowIntensity() / 1.1f;
			if (shadowOpacity < 27.0f)
				shadowOpacity = 27.0f;

			if (Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25 || Frustum::Instance().selectRainFrustun == 8)
			{
				shadowOpacity = 60.0f;
			}

			STATEMANAGER.SetPixelShaderConstant(10, (int*)&shadowOpacity, 1);

			float specularLightEnabled = (float)Frustum::Instance().realtimeSpecular;
			STATEMANAGER.SetPixelShaderConstant(11, &specularLightEnabled, 1);
			STATEMANAGER.SetPixelShaderConstant(12, &time4, 1);

			float enableLights;

			if (Frustum::Instance().ambianceEffectsLevel == 0)
				enableLights = 0.0f;
			if (Frustum::Instance().ambianceEffectsLevel == 1)
				enableLights = 1.0f;

			STATEMANAGER.SetPixelShaderConstant(13, &enableLights, 1);

			// Shadow Receiver
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


			//STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
			//STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
			//STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);
			if (m_lpCharacterShadowMapTexture)
				STATEMANAGER.SetTexture(2, m_lpCharacterShadowMapTexture);
			if (m_lpCharacterShadowMapTexture4)
				STATEMANAGER.SetTexture(3, m_lpCharacterShadowMapTexture4);


		}
	}

	if (std::strcmp(currentMapName.c_str(), "metin2_map_defensewave") == 0)
	{
		for (int j = 0; j < AROUND_AREA_NUM; ++j)
		{
			CArea* pArea;
				if (GetAreaPointer(j, &pArea))
				{
					pArea->RenderDungeon();
				}
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

	s_kVct_pkOpaqueThingInstSort.clear();

	// NOTE - 20041201.levites.���� �׸��� �߰�


	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea* pArea;
		if (GetAreaPointer(i, &pArea))
		{
			pArea->CollectRenderingObject(s_kVct_pkOpaqueThingInstSort);
		}

	}

	std::for_each(s_kVct_pkOpaqueThingInstSort.begin(), s_kVct_pkOpaqueThingInstSort.end(), CMapOutdoor_FOpaqueThingInstanceRender());


	//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);

	if (Frustum::Instance().shadowType != 3)
	{
		//STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);	

		if (Frustum::Instance().isIndoor == false)
		{
			ms_lpd3dDevice->SetVertexShader(nullptr);
			ms_lpd3dDevice->SetPixelShader(nullptr);
		}
	}
}

void CMapOutdoor::RenderArea2()
{
	
	std::for_each(s_kVct_pkOpaqueThingInstSort.begin(), s_kVct_pkOpaqueThingInstSort.end(), CMapOutdoor_FOpaqueThingInstanceRenderShadow());
	if (std::strcmp(currentMapName.c_str(), "metin2_map_defensewave") == 0)
	{
		RenderDungeon();
	}
}

void CMapOutdoor::RenderBlendArea()
{
	if (!IsVisiblePart(PART_OBJECT))
		return;

	static std::vector<CGraphicThingInstance*> s_kVct_pkBlendThingInstSort;
	s_kVct_pkBlendThingInstSort.clear();

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea* pArea;
		if (GetAreaPointer(i, &pArea))
		{
			pArea->CollectBlendRenderingObject(s_kVct_pkBlendThingInstSort);
		}
	}

	if (s_kVct_pkBlendThingInstSort.size() != 0)
	{


		//STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		//STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		//STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		//STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
		//STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

		//// Transform
		//STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);
		//STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);

		//STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		//STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		//STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		//STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		//STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		//STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		//STATEMANAGER.SaveTextureStageState(1, D3DTSS_BORDERCOLOR, 0xFFFFFFFF);

		////std::for_each(m_ShadowReceiverVector.begin(), m_ShadowReceiverVector.end(), FAreaRenderShadow());

		//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
		//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
		//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSU);
		//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSV);
		//STATEMANAGER.RestoreTextureStageState(1, D3DTSS_BORDERCOLOR);

		//STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);


		std::sort(s_kVct_pkBlendThingInstSort.begin(), s_kVct_pkBlendThingInstSort.end(), CMapOutdoor_LessThingInstancePtrRenderOrder());

		STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, TRUE);
		STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		std::for_each(s_kVct_pkBlendThingInstSort.begin(), s_kVct_pkBlendThingInstSort.end(), CMapOutdoor_FBlendThingInstanceRender());

		STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
		STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
		STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
		STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	}
}
void CMapOutdoor::RenderDungeon()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (!GetAreaPointer(i, &pArea))
			continue;
		pArea->RenderDungeon();
	}
}

void CMapOutdoor::RenderPCBlocker()
{
#ifndef JETTYX_TRANSPARENCY_BUILDING_FIX
#ifndef WORLD_EDITOR
	// PCBlocker
	if (m_PCBlockerVector.size() != 0)
	{
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

		STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matBuildingTransparent);
		STATEMANAGER.SetTexture(1, m_BuildingTransparentImageInstance.GetTexturePointer()->GetD3DTexture());

		std::for_each(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), FRenderPCBlocker());

		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

		STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
		STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSU);
		STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSV);
		STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	}
#endif
#else

#endif
}

void CMapOutdoor::SelectIndexBuffer(BYTE byLODLevel, WORD * pwPrimitiveCount, D3DPRIMITIVETYPE * pePrimitiveType)
{
#ifdef WORLD_EDITOR
	*pwPrimitiveCount = m_wNumIndices - 2;
	*pePrimitiveType = D3DPT_TRIANGLESTRIP;
	STATEMANAGER.SetIndices(m_IndexBuffer.GetD3DIndexBuffer(), 0);
#else
	if (0 == byLODLevel)
	{
		*pwPrimitiveCount = m_wNumIndices[byLODLevel] - 2;
		*pePrimitiveType = D3DPT_TRIANGLESTRIP;
	}
	else
	{
		*pwPrimitiveCount =  m_wNumIndices[byLODLevel]/3;
		*pePrimitiveType = D3DPT_TRIANGLELIST;
	}
	STATEMANAGER.SetIndices(m_IndexBuffer[byLODLevel].GetD3DIndexBuffer(), 0);
#endif
}

void CMapOutdoor::SetPatchDrawVector()
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::__SetPatchDrawVector");

	m_PatchDrawStructVector.clear();

	std::vector<std::pair<float, long> >::iterator aDistancePatchVectorIterator;

	TPatchDrawStruct aPatchDrawStruct;

	aDistancePatchVectorIterator = m_PatchVector.begin();
	while(aDistancePatchVectorIterator != m_PatchVector.end())
	{
		std::pair<float, long> adistancePatchPair = *aDistancePatchVectorIterator;

		CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[adistancePatchPair.second];

		if (!pTerrainPatchProxy->isUsed())
		{
			++aDistancePatchVectorIterator;
			continue;
		}

		long lPatchNum = pTerrainPatchProxy->GetPatchNum();
		if (lPatchNum < 0)
		{
			++aDistancePatchVectorIterator;
			continue;
		}

		BYTE byTerrainNum = pTerrainPatchProxy->GetTerrainNum();
		if (0xFF == byTerrainNum)
		{
			++aDistancePatchVectorIterator;
			continue;
		}

		CTerrain * pTerrain;
		if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		{
			++aDistancePatchVectorIterator;
			continue;
		}

		aPatchDrawStruct.fDistance				= adistancePatchPair.first;
		aPatchDrawStruct.byTerrainNum			= byTerrainNum;
		aPatchDrawStruct.lPatchNum				= lPatchNum;
		aPatchDrawStruct.pTerrainPatchProxy		= pTerrainPatchProxy;

		m_PatchDrawStructVector.push_back(aPatchDrawStruct);

		++aDistancePatchVectorIterator;
	}

	std::stable_sort(m_PatchDrawStructVector.begin(), m_PatchDrawStructVector.end(), FSortPatchDrawStructWithTerrainNum());
}

float CMapOutdoor::__GetNoFogDistance()
{
	return (float)(CTerrainImpl::CELLSCALE * m_lViewRadius) * 0.5f;
}

float CMapOutdoor::__GetFogDistance()
{
	return (float)(CTerrainImpl::CELLSCALE * m_lViewRadius) * 0.75f;
}

struct FPatchNumMatch
{
	long m_lPatchNumToCheck;
	FPatchNumMatch(long lPatchNum)
	{
		m_lPatchNumToCheck = lPatchNum;
	}
	bool operator() (std::pair<long, BYTE> aPair)
	{
		return m_lPatchNumToCheck == aPair.first;
	}
};

void CMapOutdoor::NEW_DrawWireFrame(CTerrainPatchProxy * pTerrainPatchProxy, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	DWORD dwFillMode = STATEMANAGER.GetRenderState(D3DRS_FILLMODE);
	STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

	STATEMANAGER.SetRenderState(D3DRS_FILLMODE, dwFillMode);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
}

void CMapOutdoor::DrawWireFrame(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::DrawWireFrame");

	CTerrainPatchProxy * pTerrainPatchProxy= &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;
	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	DWORD dwFillMode = STATEMANAGER.GetRenderState(D3DRS_FILLMODE);
	STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

	STATEMANAGER.SetRenderState(D3DRS_FILLMODE, dwFillMode);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

 	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
}

// Attr
void CMapOutdoor::RenderMarkedArea()
{
	if (!m_pTerrainPatchProxyList)
		return;

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE eType;
	SelectIndexBuffer(0, &wPrimitiveCount, &eType);

	D3DXMATRIX matTexTransform, matTexTransformTemp;

	D3DXMatrixScaling(&matTexTransform, m_fTerrainTexCoordBase * 32.0f, -m_fTerrainTexCoordBase * 32.0f, 0.0f);
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &matTexTransform);
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &matTexTransform);
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &matTexTransform);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	static long lStartTime = timeGetTime();
	float fTime = float((timeGetTime() - lStartTime)%3000) / 3000.0f;
	float fAlpha = fabs(fTime - 0.5f) / 2.0f + 0.1f;
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f, 1.0f, 1.0f, fAlpha));
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

	STATEMANAGER.SetTexture(0, m_attrImageInstance.GetTexturePointer()->GetD3DTexture());

	RecurseRenderAttr(m_pRootNode);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);
#else
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_MAGFILTER);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_MIPFILTER);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSV);
#endif

	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}

void CMapOutdoor::RecurseRenderAttr(CTerrainQuadtreeNode *Node, bool bCullEnable)
{
	if (bCullEnable)
	{
		if (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius)==VIEW_NONE)
			return;
	}

	{
		if (Node->Size == 1)
		{
			DrawPatchAttr(Node->PatchNum);
		}
		else
		{
			if (Node->NW_Node != NULL)
				RecurseRenderAttr(Node->NW_Node, bCullEnable);
			if (Node->NE_Node != NULL)
				RecurseRenderAttr(Node->NE_Node, bCullEnable);
			if (Node->SW_Node != NULL)
				RecurseRenderAttr(Node->SW_Node, bCullEnable);
			if (Node->SE_Node != NULL)
				RecurseRenderAttr(Node->SE_Node, bCullEnable);
		}
 	}
}

void CMapOutdoor::DrawPatchAttr(long patchnum)
{
	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	// Deal with this material buffer
	CTerrain * pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	if (!pTerrain->IsMarked())
		return;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	m_matWorldForCommonUse._41 = -(float) (wCoordX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
	m_matWorldForCommonUse._42 = (float) (wCoordY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);

	D3DXMATRIX matTexTransform, matTexTransformTemp;
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matTexTransform, &matTexTransform, &m_matStaticShadow);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matTexTransform);

	TTerrainSplatPatch & rAttrSplatPatch = pTerrain->GetMarkedSplatPatch();
 	STATEMANAGER.SetTexture(1, rAttrSplatPatch.Splats[0].pd3dTexture);

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
#endif
	STATEMANAGER.SetStreamSource(0, pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr()->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

#ifdef WORLD_EDITOR
	STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices - 2);
#else
	STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices[0] - 2);
#endif
}
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
