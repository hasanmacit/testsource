﻿/*
* CSpeedTreeWrapper Class
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

#pragma warning(disable:4786)

///////////////////////////////////////////////////////////////////////
// Include Files

#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>

#include "../EterBase/Debug.h"
#include "../EterBase/Timer.h"
#include "../EterBase/Filename.h"
#include "../EterLib/ResourceManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/GrpObjectInstance.h"

#include "SpeedTreeConfig.h"
#include "SpeedTreeForestDirectX8.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

using namespace std;

LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_dwBranchVertexShader = 0;
LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_dwLeafVertexShader = 0;
bool CSpeedTreeWrapper::ms_bSelfShadowOn = true;

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper() :
	m_pSpeedTree(new CSpeedTreeRT),
	m_bIsInstance(false),
	m_pInstanceOf(NULL),
	m_pGeometryCache(NULL),
	m_usNumLeafLods(0),
	m_pBranchIndexCounts(NULL),
	m_pBranchIndexBuffer(NULL),
	m_pBranchVertexBuffer(NULL),
	m_pFrondIndexCounts(NULL),
	m_pFrondIndexBuffer(NULL),
	m_pFrondVertexBuffer(NULL),
	m_pLeafVertexBuffer(NULL),
	m_pLeavesUpdatedByCpu(NULL),
	m_unBranchVertexCount(0),
	m_unFrondVertexCount(0),
	m_pTextureInfo(NULL)
{
	// set initial position
	m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;

	m_pSpeedTree->SetWindStrength(1.0f);
	m_pSpeedTree->SetLocalMatrices(0, 4);
}

void CSpeedTreeWrapper::SetVertexShaders(LPDIRECT3DVERTEXSHADER9 dwBranchVertexShader, LPDIRECT3DVERTEXSHADER9 dwLeafVertexShader)
{
	ms_dwBranchVertexShader = dwBranchVertexShader;
	ms_dwLeafVertexShader = dwLeafVertexShader;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
	
}

void CSpeedTreeWrapper::OnRender()
{
	if (ms_dwBranchVertexShader == 0)
	{
		ms_dwBranchVertexShader = LoadBranchShader(ms_lpd3dDevice);
		//LogBox("Vertex Shader not assigned. You must call CSpeedTreeWrapper::SetVertexShader for this");
	}

	if (ms_dwLeafVertexShader == 0)
	{
		ms_dwLeafVertexShader = LoadLeafShader(ms_lpd3dDevice);
		//LogBox("Vertex Shader not assigned. You must call CSpeedTreeWrapper::SetVertexShader for this");
	}

	CSpeedTreeForestDirectX8::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);

	// �ϳ��� ������ �� ���� LOD ������� ����
	m_pSpeedTree->SetLodLevel(1.0f);
	//Advance();

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);

	// choose fixed function pipeline or custom shader for fronds and branches
	STATEMANAGER.SetVertexShader(ms_dwBranchVertexShader);

	SetupBranchForTreeType();
	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	SetupFrondForTreeType();
	RenderFronds();

	STATEMANAGER.SetVertexShader(ms_dwLeafVertexShader);

	SetupLeafForTreeType();
	RenderLeaves();
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper()
{
	//TraceError("Tree destruction !");

	// if this is not an instance, clean up
	if (!m_bIsInstance)
	{
		if (m_unBranchVertexCount > 0)
		{
			SAFE_RELEASE(m_pBranchVertexBuffer);
			SAFE_RELEASE(m_pBranchIndexBuffer);
			SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
		}

		if (m_unFrondVertexCount > 0)
		{
			SAFE_RELEASE(m_pFrondVertexBuffer);
			SAFE_RELEASE(m_pFrondIndexBuffer);
			SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
		}

		for (short i = 0; i < m_usNumLeafLods; ++i)
		{
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

			if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
				SAFE_RELEASE(m_pLeafVertexBuffer[i]);
		}

		SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
		SAFE_DELETE_ARRAY(m_pLeafVertexBuffer);

		SAFE_DELETE(m_pTextureInfo);

		SAFE_DELETE(m_pGeometryCache);
	}

	// always delete the speedtree
	delete m_pSpeedTree;
	m_pSpeedTree = nullptr;


	Clear();
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, const BYTE* c_pbBlock, unsigned int uiBlockSize, UINT nSeed, float fSize, float fSizeVariance)
{
	bool bSuccess = false;

	// directx, so allow for flipping of the texture coordinate
#ifdef WRAPPER_FLIP_T_TEXCOORD
	m_pSpeedTree->SetTextureFlip(true);
#endif

	// load the tree file
	if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
	{
		if (!m_pSpeedTree->LoadTree(pszSptFile))
		{
			TraceError("SpeedTreeRT Error: %s", CSpeedTreeRT::GetCurrentError());
			return false;
		}
	}

	// override the lighting method stored in the spt file
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
#else
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
#endif

	// set the wind method
#ifdef WRAPPER_USE_GPU_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
#endif
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_CPU);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_CPU);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_CPU);
#endif
#ifdef WRAPPER_USE_NO_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
#endif

	m_pSpeedTree->SetNumLeafRockingGroups(1);

	// override the size, if necessary
	if (fSize >= 0.0f && fSizeVariance >= 0.0f)
		m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

	// generate tree geometry
	if (m_pSpeedTree->Compute(NULL, nSeed))
	{
		// get the dimensions
		m_pSpeedTree->GetBoundingBox(m_afBoundingBox);

		// make the leaves rock in the wind
		m_pSpeedTree->SetLeafRockingState(true);

		// billboard setup
#ifdef WRAPPER_NO_BILLBOARD_MODE
		CSpeedTreeRT::SetDropToBillboard(false);
#else
		CSpeedTreeRT::SetDropToBillboard(true);
#endif

		// query & set materials
		m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial());
		m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial());
		m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial());

		// adjust lod distances
		float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
		//m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);

		// query textures
		m_pTextureInfo = new CSpeedTreeRT::STextures;
		m_pSpeedTree->GetTextures(*m_pTextureInfo);

		// load branch textures
		auto vs1 = std::string(pszSptFile);
		auto vs2 = std::string(m_pTextureInfo->m_pBranchTextureFilename);
		LoadTexture((CFileNameHelper::GetPath(vs1) + CFileNameHelper::NoExtension(vs2) + ".dds").c_str(), m_BranchImageInstance);

#ifdef WRAPPER_RENDER_SELF_SHADOWS

#endif
		if (m_pTextureInfo->m_pCompositeFilename)
		{
			auto vss = std::string(m_pTextureInfo->m_pCompositeFilename);
			LoadTexture((CFileNameHelper::GetPath(vs1) + CFileNameHelper::NoExtension(vss) + ".dds").c_str(), m_CompositeImageInstance);
		}

		// setup the index and vertex buffers
		SetupBuffers();

		// everything appeared to go well
		bSuccess = true;
	}
	else // tree failed to compute
		fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError());

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
	// read all the geometry for highest LOD into the geometry cache (just a precaution, it's updated later)
	m_pSpeedTree->SetLodLevel(1.0f);

	if (m_pGeometryCache == NULL)
		m_pGeometryCache = new CSpeedTreeRT::SGeometry;

	m_pSpeedTree->GetGeometry(*m_pGeometryCache);

	// setup the buffers for each part
	SetupBranchBuffers();
	SetupFrondBuffers();
	SetupLeafBuffers();
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
	// reference to branch structure
	CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
	m_unBranchVertexCount = pBranches->m_usVertexCount; // we asked for a contiguous strip

	// check if this tree has branches
	if (m_unBranchVertexCount > 1)
	{
		// create the vertex buffer for storing branch vertices
		SFVFBranchVertex* pVertexBuffer = NULL;

#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pBranchVertexBuffer,nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pBranchVertexBuffer->Lock(0, 0, DXLOCKTYPE(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pBranchVertexBuffer);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		{
			for (UINT i = 0; i < m_unBranchVertexCount; ++i)
			{
				// position
				memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

				// normal or color
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
				memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));
#else
				pVertexBuffer->m_dwDiffuseColor = pBranches->m_pColors[i];
#endif

				// texcoords for layer 0
				pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
				pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

				// texcoords for layer 1 (if enabled)
#ifdef WRAPPER_RENDER_SELF_SHADOWS

#endif

				// extra data for gpu wind
#ifdef WRAPPER_USE_GPU_WIND
				pVertexBuffer->m_fWindIndex = 4.0f * pBranches->m_pWindMatrixIndices[i];
				pVertexBuffer->m_fWindWeight = pBranches->m_pWindWeights[i];
#endif

				++pVertexBuffer;
			}
			m_pBranchVertexBuffer->Unlock();
		}

		// create and fill the index counts for each LOD
		UINT unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();
		m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
		for (UINT i = 0; i < unNumLodLevels; ++i)
		{
			// force update for particular LOD
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, i);

			// check if this LOD has branches
			if (pBranches->m_usNumStrips > 0)
				m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
			else
				m_pBranchIndexCounts[i] = 0;
		}
		// set back to highest LOD
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);

		// the first LOD level contains the most indices of all the levels, so
		// we use its size to allocate the index buffer
		ms_lpd3dDevice->CreateIndexBuffer(m_pBranchIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pBranchIndexBuffer, nullptr);

		// fill the index buffer
		unsigned short* pIndexBuffer = NULL;
		m_pBranchIndexBuffer->Lock(0, 0, DXLOCKTYPE(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
		m_pBranchIndexBuffer->Unlock();
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
	// reference to frond structure
	CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
	m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

	// check if tree has fronds
	if (m_unFrondVertexCount > 1)
	{
		// create the vertex buffer for storing frond vertices
		SFVFBranchVertex* pVertexBuffer = NULL;
#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pFrondVertexBuffer, nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pFrondVertexBuffer->Lock(0, 0, DXLOCKTYPE(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pFrondVertexBuffer);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		for (UINT i = 0; i < m_unFrondVertexCount; ++i)
		{
			// position
			memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

			// normal or color
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
			memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));
#else
			pVertexBuffer->m_dwDiffuseColor = pFronds->m_pColors[i];
#endif

			// texcoords for layer 0
			pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
			pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

			// texcoords for layer 1 (if enabled)
#ifdef WRAPPER_RENDER_SELF_SHADOWS

#endif

			// extra data for gpu wind
#ifdef WRAPPER_USE_GPU_WIND
			pVertexBuffer->m_fWindIndex = 4.0f * pFronds->m_pWindMatrixIndices[i];
			pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];
#endif

			++pVertexBuffer;
		}
		m_pFrondVertexBuffer->Unlock();

		// create and fill the index counts for each LOD
		UINT unNumLodLevels = m_pSpeedTree->GetNumFrondLodLevels();
		m_pFrondIndexCounts = new unsigned short[unNumLodLevels];
		for (WORD j = 0; j < unNumLodLevels; ++j)
		{
			// force update for this LOD
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, j);

			// check if this LOD has fronds
			if (pFronds->m_usNumStrips > 0)
				m_pFrondIndexCounts[j] = pFronds->m_pStripLengths[0];
			else
				m_pFrondIndexCounts[j] = 0;
		}
		// go back to highest LOD
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);

		// the first LOD level contains the most indices of all the levels, so
		// we use its size to allocate the index buffer
		ms_lpd3dDevice->CreateIndexBuffer(m_pFrondIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pFrondIndexBuffer, nullptr);

		// fill the index buffer
		unsigned short* pIndexBuffer = NULL;
		m_pFrondIndexBuffer->Lock(0, 0, DXLOCKTYPE(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pFronds->m_pStrips[0], pFronds->m_pStripLengths[0] * sizeof(unsigned short));
		m_pFrondIndexBuffer->Unlock();
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
	// set up constants
	const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
	//const int nNumLeafMaps = m_pTextureInfo->m_uiLeafTextureCount;

	// set up the leaf counts for each LOD
	m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();

	// create array of vertex buffers (one for each LOD)
	m_pLeafVertexBuffer = new LPDIRECT3DVERTEXBUFFER8[m_usNumLeafLods];

	// create array of bools for CPU updating (so we don't update for each instance)
	m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];

	// cycle through LODs
	for (UINT unLod = 0; unLod < m_usNumLeafLods; ++unLod)
	{
		m_pLeavesUpdatedByCpu[unLod] = false;
		m_pLeafVertexBuffer[unLod] = NULL;

		// if this LOD has no leaves, skip it
		unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;

		if (usLeafCount < 1)
			continue;

		SFVFLeafVertex* pVertexBuffer = NULL;
		// create the vertex buffer for storing leaf vertices
#ifndef WRAPPER_USE_CPU_LEAF_PLACEMENT
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_MANAGED, &m_pLeafVertexBuffer[unLod], nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pLeafVertexBuffer[unLod]->Lock(0, 0, DXLOCKTYPE(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_SYSTEMMEM, &m_pLeafVertexBuffer[unLod], nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pLeafVertexBuffer[unLod]->Lock(0, 0, DXLOCKTYPE(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		SFVFLeafVertex* pVertex = pVertexBuffer;
		for (UINT unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
		{
			const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
			for (UINT unVert = 0; unVert < 6; ++unVert)  // 6 verts == 2 triangles
			{
				// position
				memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
				// normal
				memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
#else
				// color
				pVertex->m_dwDiffuseColor = pLeaf->m_pColors[unLeaf];
#endif

				// tex coord
				memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));

				// wind weights
#ifdef WRAPPER_USE_GPU_WIND
				pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
				pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
#endif

				// GPU placement data
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
				pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
				pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments()[unLod];
#endif

				++pVertex;
			}
		}
		m_pLeafVertexBuffer[unLod]->Unlock();
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
	// compute LOD level (based on distance from camera)
	m_pSpeedTree->ComputeLodLevel();
	m_pSpeedTree->SetLodLevel(1.0f);

	// compute wind
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->ComputeWindEffects(true, true, true);
#endif
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper::SpeedTreeWrapperPtr CSpeedTreeWrapper::MakeInstance()
{
	std::shared_ptr<CSpeedTreeWrapper> pInstance = std::make_shared<CSpeedTreeWrapper>();

	pInstance->m_pSpeedTree->~CSpeedTreeRT();

	// make an instance of this object's SpeedTree
	pInstance->m_bIsInstance = true;
	pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance();

	if (pInstance->m_pSpeedTree)
	{
		// use the same materials
		pInstance->m_cBranchMaterial = m_cBranchMaterial;
		pInstance->m_cLeafMaterial = m_cLeafMaterial;
		pInstance->m_cFrondMaterial = m_cFrondMaterial;
		pInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
		pInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());

		if (!m_ShadowImageInstance.IsEmpty())
			pInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());

		pInstance->m_pTextureInfo = m_pTextureInfo;

		// use the same geometry cache
		pInstance->m_pGeometryCache = m_pGeometryCache;

		// use the same buffers
		pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
		pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
		pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
		pInstance->m_unBranchVertexCount = m_unBranchVertexCount;

		pInstance->m_pFrondIndexBuffer = m_pFrondIndexBuffer;
		pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
		pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
		pInstance->m_unFrondVertexCount = m_unFrondVertexCount;

		pInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
		pInstance->m_usNumLeafLods = m_usNumLeafLods;
		pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;

		// new stuff
		memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
		memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
		pInstance->m_pInstanceOf = shared_from_this();
		m_vInstances.push_back(pInstance);
	}
	else
	{
		fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError());
		pInstance.reset();
	}

	return pInstance;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::GetInstances

std::list<CSpeedTreeWrapper::SpeedTreeWrapperWeakPtr> CSpeedTreeWrapper::GetInstances(UINT& nCount)
{
	std::list<SpeedTreeWrapperWeakPtr> kResult;

	nCount = m_vInstances.size();
	if (nCount)
	{
		for (auto it : m_vInstances)
		{
			kResult.push_back(it);
		}
	}

	return kResult;
}



void CSpeedTreeWrapper::DeleteInstance(SpeedTreeWrapperPtr pInstance)
{
	std::vector<SpeedTreeWrapperPtr>::iterator itor = m_vInstances.begin();

	while (itor != m_vInstances.end())
	{
		if (*itor == pInstance)
		{
			itor = m_vInstances.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupBranchForTreeType
void CSpeedTreeWrapper::SetupBranchForTreeTypeShadow(void)
{

	STATEMANAGER.SetTexture(0, Frustum::Instance().shadowTexture);
	
	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{
		// activate the branch vertex buffer
		STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
		// set the index buffer
		STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
	}
}


void CSpeedTreeWrapper::SetupBranchForTreeType(void)
{
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	// set lighting material
	STATEMANAGER.SetMaterial(m_cBranchMaterial.Get());
	SetShaderConstants(m_pSpeedTree->GetBranchMaterial());
#endif

	// update the branch geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
	{
		// update the vertex array
		SFVFBranchVertex* pVertexBuffer = NULL;
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		for (UINT i = 0; i < m_unBranchVertexCount; ++i)
		{
			memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
		}
		m_pBranchVertexBuffer->Unlock();
	}
#endif

	LPDIRECT3DTEXTURE8 lpd3dTexture;

	// set texture map
	if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(0, lpd3dTexture);

	// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
		STATEMANAGER.SetTexture(1, NULL);
#endif

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{
		// activate the branch vertex buffer
		STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
		// set the index buffer
		STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::RenderBranches

void CSpeedTreeWrapper::RenderBranches(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_fBranchAlphaTestValue)
	{
		PositionTree();

		// set alpha test value
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));

		// render if this LOD has branches
		if (m_pBranchIndexCounts &&
			m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
		{
			ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
		}
	}
	
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupFrondForTreeType

void CSpeedTreeWrapper::SetupFrondForTreeType(void)
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		// set lighting material
		STATEMANAGER.SetMaterial(m_cFrondMaterial.Get());
		SetShaderConstants(m_pSpeedTree->GetFrondMaterial());


		// update the frond geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
		if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
		{
			// update the vertex array
			SFVFBranchVertex* pVertexBuffer = NULL;
			m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unFrondVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pFrondVertexBuffer->Unlock();
		}
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
		STATEMANAGER.SetTexture(1, NULL);
#endif

		if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
		{
			// activate the frond vertex buffer
			STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));
			// set the index buffer
			STATEMANAGER.SetIndices(m_pFrondIndexBuffer, 0);
		}
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::RenderFronds

void CSpeedTreeWrapper::RenderFronds(void) const
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

		if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
		{
			PositionTree();

			// set alpha test value
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

			// render if this LOD has fronds
			if (m_pFrondIndexCounts &&
				m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
			{
				ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, (m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2) / Frustum::Instance().getLeaves);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetupLeafForTreeType

void CSpeedTreeWrapper::SetupLeafForTreeType(void)
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		// set lighting material
		STATEMANAGER.SetMaterial(m_cLeafMaterial.Get());
		SetShaderConstants(m_pSpeedTree->GetLeafMaterial());


		// pass leaf tables to shader
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		UploadLeafTables(c_nVertexShader_LeafTables);
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
		STATEMANAGER.SetTexture(1, NULL);
#endif
	}
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::UploadLeafTables

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
void CSpeedTreeWrapper::UploadLeafTables(UINT uiLocation) const
{
	// query leaf cluster table from RT
	UINT uiEntryCount = 0;
	const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);

	// upload for vertex shader use
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafTables, pTable, uiEntryCount / 4);
}
#endif

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::RenderLeaves

void CSpeedTreeWrapper::RenderLeaves(void) const
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		// update leaf geometry
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

		PositionTree();

		// render leaves for only one LOD level
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &m_pGeometryCache->m_sLeaves0;
		int unLod = pLeaf->m_nDiscreteLodLevel;

		if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
		{
			STATEMANAGER.SetStreamSource(0, m_pLeafVertexBuffer[unLod], sizeof(SFVFLeafVertex));
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

			UINT tenthLeafCount = pLeaf->m_usLeafCount / Frustum::Instance().getLeaves; // Calculate 1/10th of the leaves
			ms_faceCount += tenthLeafCount * 2; // Update to 1/10th of the leaves

			STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, tenthLeafCount * 2); // Draw 1/10th of the leaves
		}
	}
}

void CSpeedTreeWrapper::RenderLeaves2(void) const
{
	// update leaf geometry
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);


	PositionTree();

	// render LODs, if needed

}

void CSpeedTreeWrapper::RenderLeaves3(void) const
{
	// update leaf geometry
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

	PositionTree();

	// render leaves for only one LOD level
	const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &m_pGeometryCache->m_sLeaves0;
	int unLod = pLeaf->m_nDiscreteLodLevel;

	if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
	{
		STATEMANAGER.SetStreamSource(0, m_pLeafVertexBuffer[unLod], sizeof(SFVFLeafVertex));
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

		UINT tenthLeafCount = pLeaf->m_usLeafCount / 1.7; // Calculate 1/10th of the leaves
		ms_faceCount += tenthLeafCount * 2; // Update to 1/10th of the leaves

		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, tenthLeafCount * 2); // Draw 1/10th of the leaves
	}
}




///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::EndLeafForTreeType

void CSpeedTreeWrapper::EndLeafForTreeType(void)
{
	// reset copy flags for CPU wind
	//for (UINT i = 0; i < m_usNumLeafLods; ++i)
	//	m_pLeavesUpdatedByCpu[i] = false;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::RenderBillboards

void CSpeedTreeWrapper::RenderBillboards(void) const
{
	// render billboards in immediate mode (as close as DirectX comes to immediate mode)
#ifdef WRAPPER_BILLBOARD_MODE
	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	PositionTree();

	struct SBillboardVertex
	{
		float fX, fY, fZ;
		float fU, fV;
	};

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

	if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};

		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

	// if tree supports 360 degree billboards, render the second
	if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

#ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
	// render horizontal billboard (if enabled)
	if (m_pGeometryCache->m_sHorizontalBillboard.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sHorizontalBillboard.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

#endif
#endif
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
	if (!m_bIsInstance)
		m_pSpeedTree->DeleteTransientData();
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::PositionTree

void CSpeedTreeWrapper::PositionTree(void) const
{
	D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
	D3DXMATRIX matTranslation;
	D3DXMatrixIdentity(&matTranslation);
	D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);

	// store translation for client-side transformation
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);

	// store translation for use in vertex shader
	D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::LoadTexture

bool CSpeedTreeWrapper::LoadTexture(const char* pFilename, CGraphicImageInstance& rImage)
{
	CResource* pResource = CResourceManager::Instance().GetResourcePointer(pFilename);
	rImage.SetImagePointer(static_cast<CGraphicImage*>(pResource));

	if (rImage.IsEmpty())
		return false;

	//TraceError("SpeedTreeWrapper::LoadTexture: %s", pFilename);
	return true;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeWrapper::SetShaderConstants

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
	const float afUsefulConstants[] =
	{
		m_pSpeedTree->GetLeafLightingAdjustment(), 0.0f, 0.0f, 0.0f,
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

	const float afMaterial[] =
	{
		pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
			pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
	m_afPos[0] = x;
	m_afPos[1] = y;
	m_afPos[2] = z;
	m_pSpeedTree->SetTreePosition(x, y, z);
	CGraphicObjectInstance::SetPosition(x, y, z);
}

bool CSpeedTreeWrapper::GetBoundingSphere(D3DXVECTOR3& v3Center, float& fRadius)
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	v3Center.x = 0.0f;
	v3Center.y = 0.0f;
	v3Center.z = fZ * 0.5f;

	fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f; // 0.9f for reduce size

	D3DXVECTOR3 vec = m_pSpeedTree->GetTreePosition();

	v3Center += vec;

	return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	m_v3BBoxMin.x = -fX / 2.0f;
	m_v3BBoxMin.y = -fY / 2.0f;
	m_v3BBoxMin.z = 0.0f;
	m_v3BBoxMax.x = fX / 2.0f;
	m_v3BBoxMax.y = fY / 2.0f;
	m_v3BBoxMax.z = fZ;

	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX& c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}

// collision detection routines
UINT CSpeedTreeWrapper::GetCollisionObjectCount()
{
	assert(m_pSpeedTree);
	return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	assert(m_pSpeedTree);
	m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}

const float* CSpeedTreeWrapper::GetPosition()
{
	return m_afPos;
}

void CSpeedTreeWrapper::GetTreeSize(float& r_fSize, float& r_fVariance)
{
	m_pSpeedTree->GetTreeSize(r_fSize, r_fVariance);
}

// pscdVector may be null
void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector* /*pscdVector*/)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, m_afPos[0], m_afPos[1], m_afPos[2]);

	/////
	for (UINT i = 0; i < GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType ObjectType;
		CStaticCollisionData CollisionData;

		GetCollisionObject(i, ObjectType, (float*)&CollisionData.v3Position, CollisionData.fDimensions);

		if (ObjectType == CSpeedTreeRT::CO_BOX)
			continue;

		switch (ObjectType)
		{
		case CSpeedTreeRT::CO_SPHERE:
			CollisionData.dwType = COLLISION_TYPE_SPHERE;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

		case CSpeedTreeRT::CO_CYLINDER:
			CollisionData.dwType = COLLISION_TYPE_CYLINDER;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			CollisionData.fDimensions[1] = CollisionData.fDimensions[1] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

			//case CSpeedTreeRT::CO_BOX:
			//	break;
		}
		AddCollision(&CollisionData, &mat);
	}
}