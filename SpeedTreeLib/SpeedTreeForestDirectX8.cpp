/*
* CSpeedTreeForestDirectX8 Class
*
* (c) 2003 IDV, Inc.
*
* This class is provided to illustrate one way to incorporate
* SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
* calls that must be made on a per tree basis are done by this class.
* Calls that apply to all trees (i.e. static SpeedTreeRT functions)
* are made in the functions in main.cpp.
*
*
* *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
*
* This software is supplied under the terms of a license agreement or
* nondisclosure agreement with Interactive Data Visualization and may
* not be copied or disclosed except in accordance with the terms of
* that agreement.
*
* Copyright (c) 2001-2003 IDV, Inc.
* All Rights Reserved.
*
* IDV, Inc.
* 1233 Washington St. Suite 610
* Columbia, SC 29201
* Voice: (803) 799-1699
* Fax: (803) 931-0320
* Web: http://www.idvinc.com
*/

#include "StdAfx.h"

#include <stdio.h>
#ifdef DIRECTX9
#include <MicrosoftDirectX/d3d9.h>
#include <MicrosoftDirectX/d3dx9.h>
#include <MicrosoftDirectX/d3d9types.h>
#else
#include <d3d8.h>
#include <d3dx8.h>
#include <d3d8types.h>
#endif

#include "../EterBase/Timer.h"
#include "../Eterlib/StateManager.h"
#include "../Eterlib/Camera.h"

#include "SpeedTreeForestDirectX8.h"
#include "SpeedTreeConfig.h"
#include "VertexShaders.h"
#include <unordered_set>

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8() : m_pDx(NULL), m_dwBranchVertexShader(0), m_dwLeafVertexShader(0) {}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8() = default;

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::InitVertexShaders

bool CSpeedTreeForestDirectX8::InitVertexShaders(void)
{
	NANOBEGIN
		// load the vertex shaders
		if (!m_dwBranchVertexShader)
			m_dwBranchVertexShader = LoadBranchShader(m_pDx);

	if (!m_dwLeafVertexShader)
		m_dwLeafVertexShader = LoadLeafShader(m_pDx);

	if (m_dwBranchVertexShader && m_dwLeafVertexShader)
	{
		CSpeedTreeWrapper::SetVertexShaders(m_dwBranchVertexShader, m_dwLeafVertexShader);
		return true;
	}

	NANOEND
		return false;
}

bool CSpeedTreeForestDirectX8::SetRenderingDevice(LPDIRECT3DDEVICE8 lpDevice)
{
	m_pDx = lpDevice;

	if (!InitVertexShaders())
		return false;

	const float c_afLightPosition[4] = { -0.707f, -0.300f, 0.707f, 0.0f };
	const float	c_afLightAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const float	c_afLightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float	c_afLightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float afLight1[] =
	{
		c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2], // pos
		c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2], // diffuse
		c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2], // ambient
		c_afLightSpecular[0], c_afLightSpecular[1], c_afLightSpecular[2], // specular
		c_afLightPosition[3], // directional flag
		1.0f, 0.0f, 0.0f // attenuation (constant, linear, quadratic)
	};

	CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);

	CSpeedTreeRT::SetLightAttributes(0, afLight1);
	CSpeedTreeRT::SetLightState(0, true);
	return true;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::UploadWindMatrix

void CSpeedTreeForestDirectX8::UploadWindMatrix(UINT uiLocation, const float* pMatrix) const
{
	STATEMANAGER.SetVertexShaderConstant(uiLocation, pMatrix, 4);
}

void CSpeedTreeForestDirectX8::UpdateCompundMatrix(const D3DXVECTOR3 & c_rEyeVec, const D3DXMATRIX & c_rmatView, const D3DXMATRIX & c_rmatProj)
{
	// setup composite matrix for shader
	D3DXMATRIX matBlend;
	D3DXMatrixIdentity(&matBlend);

	D3DXMATRIX matBlendShader;
	D3DXMatrixMultiply(&matBlendShader, &c_rmatView, &c_rmatProj);

	float afDirection[3];
	afDirection[0] = matBlendShader.m[0][2];
	afDirection[1] = matBlendShader.m[1][2];
	afDirection[2] = matBlendShader.m[2][2];
	CSpeedTreeRT::SetCamera(c_rEyeVec, afDirection);

	D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, &matBlendShader, 4);
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::Render

void CSpeedTreeForestDirectX8::Render(unsigned long ulRenderBitVector)
{
	UpdateSystem(CTimer::Instance().GetCurrentSecond());

	if (m_pMainTreeMap.empty())
		return;

	if (!(ulRenderBitVector & Forest_RenderToShadow) && !(ulRenderBitVector & Forest_RenderToMiniMap))
		UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	DWORD dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwColorVertexState = STATEMANAGER.GetRenderState(D3DRS_COLORVERTEX);
	DWORD dwFogVertexMode = STATEMANAGER.GetRenderState(D3DRS_FOGVERTEXMODE);

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
#else
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, TRUE);
#endif

	TTreeMap::iterator itor;
	UINT uiCount;

	itor = m_pMainTreeMap.begin();

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Light, m_afLighting, 3);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, m_afFog, 1);

	if (ulRenderBitVector & Forest_RenderToShadow)
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// set up fog if it is enabled
	if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
	{
#ifdef WRAPPER_USE_GPU_WIND
		STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); // GPU needs to work on all cards
#endif
	}

	// choose fixed function pipeline or custom shader for fronds and branches


	// render branches


	// set render states
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// render fronds

	if (ulRenderBitVector & Forest_RenderFronds)
	{
		itor = m_pMainTreeMap.begin();

		// Get the position of the character
		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();

				D3DXVECTOR3 treePosition = pInstance->GetPosition();
				D3DXVECTOR3 characterPosition = Frustum::Instance().characterPosition;
				D3DXVECTOR3 diff = treePosition - characterPosition;
				float distance = D3DXVec3Length(&diff);

				if (distance > 25000.0f)
					continue;

				pInstance->Advance();
			}

			STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
			STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);

			pMainTree->SetupBranchForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				D3DXVECTOR3 treePosition = pInstance->GetPosition();
				D3DXVECTOR3 characterPosition = Frustum::Instance().characterPosition;
				D3DXVECTOR3 diff = treePosition - characterPosition;
				float distance = D3DXVec3Length(&diff);

				if (distance > 25000.0f)
					continue;

				if (!pInstance->isShow())
					continue;


				pInstance->RenderBranches();
			}

			pMainTree->SetupFrondForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();

				// add a distance check here
				D3DXVECTOR3 treePosition = pInstance->GetPosition();
				D3DXVECTOR3 characterPosition = Frustum::Instance().characterPosition;
				D3DXVECTOR3 diff = treePosition - characterPosition;
				float distance = D3DXVec3Length(&diff);

				if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") == 0
					|| std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_n_desert_01") == 0
					|| std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_12zi_stage") == 0
					|| std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_12zi_stage_2") == 0
					|| std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_n_flame_01") == 0)
				{
					if (distance > 35000.0f)
						continue;
				}
				else
				{
					if (distance > 8000.0f)
						continue;
				}

				if (!pInstance->isShow())
					continue;


				pInstance->RenderFronds();
			}

			STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
			STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);

			pMainTree->SetupLeafForTreeType();
			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				D3DXVECTOR3 treePosition = pInstance->GetPosition();
				D3DXVECTOR3 characterPosition = Frustum::Instance().characterPosition;
				D3DXVECTOR3 diff = treePosition - characterPosition;
				float distance = D3DXVec3Length(&diff);

				if (distance > 25000.0f)
					continue;

				if (!pInstance->isShow())
					continue;


				pInstance->RenderLeaves();
			}

			STATEMANAGER.SetVertexShader(NULL);
		}

	}

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, dwColorVertexState);
	STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, dwFogVertexMode);

	// ����������� ���� TextureStage 1�� COLOROP�� ALPHAOP�� ����� ���� ������ �� �����
	// ����� ���´�. (�ȱ׷��� �˰� ���� ���ɼ���..)
	if (!(ulRenderBitVector & Forest_RenderToShadow))
	{
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CSpeedTreeForestDirectX8::UpdateCompundMatrix2(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj)
{
	// setup composite matrix for shader
	D3DXMATRIX matBlend;
	D3DXMatrixIdentity(&matBlend);

	D3DXMATRIX matBlendShader;
	D3DXMatrixMultiply(&matBlendShader, &c_rmatView, &c_rmatProj);

	float afDirection[3];
	//afDirection[0] = matBlendShader.m[0][2];
	//afDirection[1] = matBlendShader.m[1][2];
	//afDirection[2] = matBlendShader.m[2][2];
	//CSpeedTreeRT::SetCamera(c_rEyeVec, afDirection);

	D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, &matBlendShader, 4);
}

void CSpeedTreeForestDirectX8::Render2(unsigned long ulRenderBitVector)
{
	if (m_pMainTreeMap.empty())
		return;

	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	const auto& frustumInstance = Frustum::Instance();
	const D3DXVECTOR3 characterPosition(frustumInstance.characterPosition);
	const std::string& currentMapName = frustumInstance.currentMapNameSave;
	float compareDistance = (frustumInstance.terrainShadow == 1) ? 5000.0f :
		(frustumInstance.terrainShadow == 2) ? 25000.0f : 0.0f;

	static const std::unordered_set<std::string> specialMaps = {
		"map_n_snowm_01", "metin2_map_n_desert_01", "metin2_12zi_stage",
		"metin2_12zi_stage_2", "metin2_map_n_flame_01" };

	UINT count;

	for (const auto& [key, pMainTree] : m_pMainTreeMap)
	{
		auto ppInstances = pMainTree->GetInstances(count);

		// Render Branches
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);
		pMainTree->SetupBranchForTreeTypeShadow();

		for (const auto& weakInstance : ppInstances)
		{
			auto pInstance = weakInstance.lock();
			if (pInstance && pInstance->isShow())
				pInstance->RenderBranches();
		}

		// Render Fronds
		if (currentMapName != "metin2_map_dawnmist_dungeon_01")
		{
			pMainTree->SetupFrondForTreeType();

			for (const auto& weakInstance : ppInstances)
			{
				auto pInstance = weakInstance.lock();
				if (!pInstance || !pInstance->isShow())
					continue;

				const float* positionArray = pInstance->GetPosition();
				D3DXVECTOR3 treePosition(positionArray[0], positionArray[1], positionArray[2]);
				D3DXVECTOR3 diff = treePosition - characterPosition;
				float distance = D3DXVec3Length(&diff);


				if (specialMaps.find(currentMapName) != specialMaps.end() && distance > 35000.0f)
					continue;
				if (distance > compareDistance)
					continue;

				pInstance->RenderFronds();
			}
		}

		// Render Leaves
		if (!Forest_RenderFakeShadow)
		{
			STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
			STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);
			pMainTree->SetupLeafForTreeType();

			for (const auto& weakInstance : ppInstances)
			{
				auto pInstance = weakInstance.lock();
				if (pInstance && pInstance->isShow())
					pInstance->RenderLeaves();
			}
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}



void CSpeedTreeForestDirectX8::Render3(unsigned long ulRenderBitVector)
{
	UpdateSystem(CTimer::Instance().GetCurrentSecond());

	if (m_pMainTreeMap.empty())
		return;


	TTreeMap::iterator itor;
	UINT uiCount;


	itor = m_pMainTreeMap.begin();

	// Get the position of the character
	while (itor != m_pMainTreeMap.end())
	{

		SpeedTreeWrapperPtr pMainTree = (itor++)->second;
		std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);


		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);


		pMainTree->SetupLeafForTreeType();
		for (auto it : ppInstances)
		{
			if (it.expired())
				continue;

			auto pInstance = it.lock();
			if (!pInstance->isShow())
				continue;


			pInstance->RenderLeaves2();

		}
	}
}