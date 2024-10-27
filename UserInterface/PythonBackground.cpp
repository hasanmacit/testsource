// PythonBackground.cpp: implementation of the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../eterlib/CullingManager.h"
#include "../eterlib/Camera.h"
#include "../eterPack/EterPackManager.h"
#include "../gamelib/MapOutDoor.h"
#include "../gamelib/PropertyLoader.h"

#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "PythonMiniMap.h"
#include "PythonSystem.h"

#include <fstream>
#include <sstream>
#include <string>
#include "../SpeedTreeLib/VertexShaders.h"
#include <random>

std::string g_strEffectName = "d:/ymir work/effect/etc/direction/direction_land.mse";

std::chrono::steady_clock::time_point lastExecutionTime = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point lastExecutionTimeDog;
std::chrono::steady_clock::time_point lastExecutionTimeWolf;
std::chrono::steady_clock::time_point lastExecutionTimeBirds;
std::chrono::steady_clock::time_point lastExecutionTimeBar;

DWORD CPythonBackground::GetRenderShadowTime()
{
	return m_dwRenderShadowTime;
}

bool CPythonBackground::SetVisiblePart(int eMapOutDoorPart, bool isVisible)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SetVisiblePart(eMapOutDoorPart, isVisible);
	return true;
}

void CPythonBackground::EnableTerrainOnlyForHeight()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.EnableTerrainOnlyForHeight(TRUE);
}

bool CPythonBackground::SetSplatLimit(int iSplatNum)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SetSplatLimit(iSplatNum);
	return true;
}

void CPythonBackground::CreateCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.CreateCharacterShadowTexture();
}

void CPythonBackground::ReleaseCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.ReleaseCharacterShadowTexture();
}

std::mt19937 rng;

float CPythonBackground::ambianceSoundPositionX(float x) {
	std::uniform_real_distribution<float> dist(-10.0, 50.0);  // Adjust range as needed
	float rand_num = dist(rng);
	return (rand() % 2 == 0) ? x + rand_num : x - rand_num;
}

float CPythonBackground::ambianceSoundPositionY(float y) {
	std::uniform_real_distribution<float> dist(-10.0, 50.0);  // Adjust range as needed
	float rand_num = dist(rng);
	return (rand() % 2 == 0) ? y + rand_num : y - rand_num;
}

float CPythonBackground::ambianceSoundPositionZ(float z) {
	std::uniform_real_distribution<float> dist(-10.0, 50.0);  // Adjust range as needed
	float rand_num = dist(rng);
	return (rand() % 2 == 0) ? z + rand_num : z - rand_num;
}

void CPythonBackground::RenderAmbianceAudio()
{	
	//End AmbianceSound Area 
}

void CPythonBackground::RefreshShadowLevel()
{

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	SetShadowQualityLevel(CPythonSystem::Instance().GetShadowQualityLevel());
	SetShadowTargetLevel(CPythonSystem::Instance().GetShadowTargetLevel());
	SetShadowDistanceBehind(CPythonSystem::Instance().GetShadowDistanceBehind());
	SetTerrainShadow(CPythonSystem::Instance().GetTerrainShadow());
	SetCollisionLevel(CPythonSystem::Instance().GetCollisionLevel());
	SetTreeShadow(CPythonSystem::Instance().GetTreeShadow());
	SetPerformanceMode(CPythonSystem::Instance().GetPerformanceMode());
	SetRainLevel(3);
	SetRainLevel(CPythonSystem::Instance().GetRainLevel());
	SetDynamicLightLevel(CPythonSystem::Instance().GetDynamicLightLevel());
	SetSpecularLevel(CPythonSystem::Instance().GetSpecularLevel());
	SetBloodLevel(CPythonSystem::Instance().GetBloodLevel());
	SetAmbianceEffectsLevel(CPythonSystem::Instance().GetAmbianceEffectsLevel());
	SetShadowTypeLevel(CPythonSystem::Instance().GetShadowTypeLevel());
	CPythonSystem::Instance().GetShadowIntensity();
	CPythonSystem::Instance().GetSoundVolume();
	CPythonSystem::Instance().GetMusicVolume();
	CPythonSystem::Instance().GetShadowIntensity();
	CPythonSystem::Instance().GetLEAVES();
#else
	SetShadowLevel(CPythonSystem::Instance().GetShadowLevel());
#endif
}

bool CPythonBackground::SetShadowLevel(int eLevel)
{



	if (!m_pkMap)
		return false;

	if (m_eShadowLevel == eLevel)
		return true;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowLevel = eLevel;

	switch (m_eShadowLevel)
	{
	case SHADOW_NONE:
		rkMap.SetDrawShadow(false);
		rkMap.SetShadowTextureSize(512);
		break;

	case SHADOW_GROUND:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(false);
		rkMap.SetShadowTextureSize(512);
		break;

	case SHADOW_GROUND_AND_SOLO:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(true);
		rkMap.SetShadowTextureSize(512);
		break;

	case SHADOW_ALL:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(true);
		rkMap.SetShadowTextureSize(512);
		break;

	case SHADOW_ALL_HIGH:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(true);
		rkMap.SetShadowTextureSize(1024);
		break;

	case SHADOW_ALL_MAX:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(true);
		rkMap.SetShadowTextureSize(2048);
		break;
	}

	return true;
}
// PS_RedColor



// Function to read shader code from file
std::string readShaderFromFile(const char* filename) {
	std::ifstream shaderFile(filename);
	if (!shaderFile.is_open()) {
		return "";
	}

	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	shaderFile.close();

	return shaderStream.str();
}




//JTX OPTIONS
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
ID3DXBuffer* pShaderBuffer;
ID3DXBuffer* pShaderBuffer2;
ID3DXBuffer* pShaderBufferShadowBuilding;
ID3DXBuffer* pErrorBuffer;
ID3DXBuffer* pErrorBuffer2;
const char* PS_RedColor;
const char* PS_RedColor2;
const char* PS_RedColorShadowBuilding;

int numberofloads = 0;

bool CPythonBackground::SetShadowQualityLevel(int eLevel)
{
	if (numberofloads == 0)
	{
		// Embed the HLSL shader code as a local C++ multi-line string.
		const char* PS_RedColor =
			"sampler2D sampler0 : register(s0);"
			"sampler2D sampler1 : register(s1);"
			"sampler2D sampler2 : register(s2);"
			"float4 PS_RedColor(float2 texCoord0 : TEXCOORD0, float2 texCoord1 : TEXCOORD1, float2 texCoord2 : TEXCOORD2) : COLOR"
			"{"
			"   float2 center = float2(0.5, 0.5);"
			"   float2 size = float2(0.159689, 0.159689);"
			"   float2 pixelSize = 10.0 / float2(3840.0, 2160.0);"
			"   float4 tex0 = tex2D(sampler0, texCoord0);"
			"   float4 tex1 = float4(0.0, 0.0, 0.0, 0.0);"
			"   for(int dx = -2; dx <= 2; dx++) {"
			"       for(int dy = -2; dy <= 2; dy++) {"
			"           tex1 += tex2D(sampler1, texCoord1 + float2(dx, dy) * pixelSize);"
			"       }"
			"   }"
			"   tex1 /= 31.0;"
			"   float4 tex2 = tex2D(sampler2, texCoord2);"
			"   float4 result = tex0 * tex1 * 6.0;"
			"   result.a = tex0.a * 3;"
			"   return result;"
			"}";

		// Now use this string for your shader compilation
		D3DXCompileShader(PS_RedColor, strlen(PS_RedColor), NULL, NULL, "PS_RedColor", "ps_2_0", 0, &pShaderBuffer, NULL, NULL);
		ms_lpd3dDevice->CreatePixelShader((DWORD*)pShaderBuffer->GetBufferPointer(), &pShader);
		pShaderBuffer->Release();

		numberofloads = 1;
	}




	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	//rkMap.CreateCharacterShadowTexture();

	m_eShadowQualityLevel = eLevel;

	switch (eLevel)
	{
	case SHADOW_QUALITY_BAD:
		rkMap.SetShadowTextureSize(1024);
		rkMap.CreateCharacterShadowTexture();
		break;
	case SHADOW_QUALITY_AVERAGE:
		rkMap.SetShadowTextureSize(1500);
		rkMap.CreateCharacterShadowTexture();
		break;
	case SHADOW_QUALITY_GOOD:
		rkMap.SetShadowTextureSize(1750);
		rkMap.CreateCharacterShadowTexture();
		//Frustum::Instance().refreshEffect = true;
		break;
	case SHADOW_QUALITY_ULTRA:
		rkMap.SetShadowTextureSize(2048);
		rkMap.CreateCharacterShadowTexture();
		//Frustum::Instance().refreshEffect = true;
		break;
	}

	return true;
}

bool CPythonBackground::SetShadowDistanceBehind(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowBehindDistance = eLevel;

	switch (eLevel)
	{
	case SHADOW_BEHIND_0:
		Frustum::Instance().increaseShadowRangeFrustum = 1.0f;
		break;
	case SHADOW_BEHIND_1:
		Frustum::Instance().increaseShadowRangeFrustum = 2.0f;
		break;
	case SHADOW_BEHIND_2:
		Frustum::Instance().increaseShadowRangeFrustum = 5.0f;
		break;
	case SHADOW_BEHIND_3:
		Frustum::Instance().increaseShadowRangeFrustum = 15.0f;
		break;
	}

	return true;
}

bool CPythonBackground::SetTerrainShadow(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eTerrainShadow = eLevel;

	switch (eLevel)
	{
	case TERRAIN_SHADOW_on:
		Frustum::Instance().terrainShadow = 2;
		break;
	case TERRAIN_SHADOW_off:
		Frustum::Instance().terrainShadow = 0;
		break;
	case TERRAIN_SHADOW_low:
		Frustum::Instance().terrainShadow = 1;
		break;
	}

	return true;
}

//COLLISION JTX
bool CPythonBackground::SetCollisionLevel(int eLevel)
{

	m_eCollisionLevel = eLevel;

	switch (m_eCollisionLevel)
	{
	case COLLISION0:
		Collision = 0;
		break;

	case COLLISION1:
		Collision = 1;
		break;

	case COLLISION2:
		Collision = 2;
		break;
	}

	return true;
}

//DYNAMIC LIGHT JTX
bool CPythonBackground::SetDynamicLightLevel(int eLevel)
{

	m_eDynamicLightLevel = eLevel;

	switch (m_eDynamicLightLevel)
	{
	case DYNAMIC_LIGHT_off:
		Frustum::Instance().dynamicLight = 0;
		break;

	case DYNAMIC_LIGHT_on:
		Frustum::Instance().dynamicLight = 2;
		Frustum::Instance().refreshEffect = true;
		Frustum::Instance().updatePostProcessing = true;
		break;

	case DYNAMIC_LIGHT_low:
		Frustum::Instance().dynamicLight = 1;
		Frustum::Instance().refreshEffect = true;
		Frustum::Instance().updatePostProcessing = true;
		break;
	}

	return true;
}

//SPECULAR JTX
bool CPythonBackground::SetSpecularLevel(int eLevel)
{

	m_eSpecularLevel = eLevel;

	switch (m_eSpecularLevel)
	{
	case SPECULAR_off:
		Frustum::Instance().realtimeSpecular = 0;
		break;

	case SPECULAR_on:
		Frustum::Instance().realtimeSpecular = 1;
		break;
	}

	return true;
}

//BLOOD JTX
bool CPythonBackground::SetBloodLevel(int eLevel)
{

	m_eBloodLevel = eLevel;

	switch (m_eBloodLevel)
	{
	case BLOOD_off:
		Frustum::Instance().bloodLevel = 0;
		break;

	case BLOOD_on:
		Frustum::Instance().bloodLevel = 1;
		break;
	}

	return true;
}

//AMBIANCE EFFECTS
bool CPythonBackground::SetShadowTypeLevel(int eLevel)
{

	m_eSelectShadowType = eLevel;
	CMapOutdoor& rkMap = GetMapOutdoorRef();

	switch (m_eSelectShadowType)
	{
	case SHADOWTYPE_on:
		Frustum::Instance().shadowType = 0;
		rkMap.recreate = true;
		break;

	case SHADOWTYPE_soft:
		Frustum::Instance().shadowType = 1;
		rkMap.recreate = true;
		break;

	case SHADOWTYPE_off:
		Frustum::Instance().shadowType = 2;
		rkMap.recreate = true;
		break;

	case SHADOWTYPE_close:
		Frustum::Instance().shadowType = 3;
		rkMap.recreate = true;
		break;
	}

	return true;
}

//AMBIANCE EFFECTS
bool CPythonBackground::SetAmbianceEffectsLevel(int eLevel)
{

	m_eAmbianceEffectsLevel = eLevel;

	switch (m_eAmbianceEffectsLevel)
	{
	case AEFFECTS_off:
		Frustum::Instance().ambianceEffectsLevel = 0;
		break;

	case AEFFECTS_on:
		Frustum::Instance().ambianceEffectsLevel = 1;
		break;
	}

	return true;
}

//VISIBLE LOBBY
bool CPythonBackground::SetVisibleLobbyLevel(int eLevel)
{

	m_eVisibleLobbyType = eLevel;

	switch (m_eVisibleLobbyType)
	{
	case VISIBLE_LOBBY_off:
		Frustum::Instance().alwaysHiddenLobby = 0;
		break;

	case VISIBLE_LOBBY_on:
		Frustum::Instance().alwaysHiddenLobby = 1;
		break;
	}

	return true;
}
bool CPythonBackground::SetPerformanceMode(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_ePerformanceMode = eLevel;

	switch (eLevel)
	{
	case PERFORMANCE_MODE_on:
		Frustum::Instance().performanceMode = 2;
		if (!Frustum::Instance().instance().isWarp)
		{
			rkMap.ClearObjects();
		}
		rkMap.refreshObjects = true;

		break;
	case PERFORMANCE_MODE_medium:
		Frustum::Instance().performanceMode = 1;
		if (!Frustum::Instance().instance().isWarp)
		{
			rkMap.ClearObjects();
			rkMap.refreshObjects = true;
		}

		break;
	case PERFORMANCE_MODE_off:
		Frustum::Instance().performanceMode = 0;
		if (!Frustum::Instance().instance().isWarp)
		{
			rkMap.ClearObjects();
			rkMap.refreshObjects = true;
		}
		break;
	}

	return true;
}

bool CPythonBackground::SetTreeShadow(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eTreeShadow = eLevel;

	switch (eLevel)
	{
	case TREE_SHADOW_on:
		Frustum::Instance().treeShadow = 0;
		break;

	case TREE_SHADOW_off:
		Frustum::Instance().treeShadow = 1;
		break;

	case TREE_SHADOW_high:
		Frustum::Instance().treeShadow = 2;
		break;

	case TREE_SHADOW_all:
		Frustum::Instance().treeShadow = 3;
		break;

	}

	return true;
}

bool CPythonBackground::SetShadowTargetLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowTargetLevel = eLevel;

	switch (eLevel)
	{
	case SHADOW_TARGET_NONE:
		rkMap.SetDrawShadow(false);
		break;
	case SHADOW_TARGET_GROUND:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(false);
		break;
	case SHADOW_TARGET_GROUND_AND_SOLO:
	case SHADOW_TARGET_ALL:
		rkMap.SetDrawShadow(true);
		rkMap.SetDrawCharacterShadow(true);
		break;
	}

	return true;
}

//RAIN
bool CPythonBackground::SetRainLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	if (m_eRainLevel == eLevel)
		return true;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eRainLevel = eLevel;

	switch (m_eRainLevel)
	{
	case RAIN_0:
		DisableSnowEnvironment();
		//Frustum::Instance().windSpeed = 0.6f;
		rkMap.VitesseVent = 0.6f;
		SelectRain = 0;
		Frustum::Instance().selectRainFrustun = 0;
		Frustum::Instance().TreeLight = 0;
		Frustum::Instance().refreshEffect = true;
		if (Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_01" || Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_dungeon_01")
		{
			Frustum::Instance().ambianceEffectsLevel = 0;
		}
		break;

	case RAIN_1:
		EnableSnowEnvironment();
		//Frustum::Instance().windSpeed = 0.6f;
		rkMap.VitesseVent = 0.8f;
		SelectRain = 8;
		Frustum::Instance().selectRainFrustun = 8;
		Frustum::Instance().TreeLight = 0;
		Frustum::Instance().refreshEffect = true;
		if (Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_01" || Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_dungeon_01")
		{
			Frustum::Instance().ambianceEffectsLevel = 1;
		}
		break;

	case RAIN_2:
		EnableSnowEnvironment();
		rkMap.VitesseVent = 1.0f;
		SelectRain = 16;
		Frustum::Instance().selectRainFrustun = 16;
		Frustum::Instance().TreeLight = 1;
		Frustum::Instance().refreshEffect = true;
		if (Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_01" || Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_dungeon_01")
		{
			Frustum::Instance().ambianceEffectsLevel = 1;
		}
		break;

	case RAIN_3:
		EnableSnowEnvironment();
		rkMap.VitesseVent = 1.2f;
		SelectRain = 25;
		Frustum::Instance().selectRainFrustun = 25;
		Frustum::Instance().TreeLight = 1;
		Frustum::Instance().refreshEffect = true;
		if (Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_01" || Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_dungeon_01")
		{
			Frustum::Instance().ambianceEffectsLevel = 1;
		}
		break;

	case RAIN_4:
		DisableSnowEnvironment();
		//Frustum::Instance().windSpeed = 0.6f;
		rkMap.VitesseVent = 0.4f;
		SelectRain = 0;
		Frustum::Instance().selectRainFrustun = 0;
		Frustum::Instance().TreeLight = 0;
		Frustum::Instance().refreshEffect = true;
		if (Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_01" || Frustum::Instance().currentMapNameSave == "metin2_map_n_flame_dungeon_01")
		{
			Frustum::Instance().ambianceEffectsLevel = 1;
		}
		break;
	}


	return true;
}
#endif

void CPythonBackground::SelectViewDistanceNum(int eNum)
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (!mc_pcurEnvironmentData)
	{
		TraceError("CPythonBackground::SelectViewDistanceNum(int eNum=%d) mc_pcurEnvironmentData is NULL", eNum);
		return;
	}

	m_eViewDistanceNum = eNum;

	TEnvironmentData* env = ((TEnvironmentData*)mc_pcurEnvironmentData);

	if (env->bReserve)
	{
		env->m_fFogNearDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
		env->m_fFogFarDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
		env->v3SkyBoxScale = m_ViewDistanceSet[m_eViewDistanceNum].m_v3SkyBoxScale;
		rkMap.SetEnvironmentSkyBox();
	}
}

void CPythonBackground::SetViewDistanceSet(int eNum, float fFarClip)
{
	if (!m_pkMap)
		return;

	m_ViewDistanceSet[eNum].m_fFogStart = fFarClip * 0.5f;//0.3333333f;
	m_ViewDistanceSet[eNum].m_fFogEnd = fFarClip * 0.7f;//0.6666667f;

	float fSkyBoxScale = fFarClip * 0.6f;//0.5773502f;
	m_ViewDistanceSet[eNum].m_v3SkyBoxScale = D3DXVECTOR3(fSkyBoxScale, fSkyBoxScale, fSkyBoxScale);
	m_ViewDistanceSet[eNum].m_fFarClip = fFarClip;

	if (eNum == m_eViewDistanceNum)
		SelectViewDistanceNum(eNum);
}

float CPythonBackground::GetFarClip()
{
	if (!m_pkMap)
		return 50000.0f;

	if (m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip == 0.0f)
	{
		TraceError("CPythonBackground::GetFarClip m_eViewDistanceNum=%d", m_eViewDistanceNum);
		m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip = 25600.0f;
	}

	return m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

void CPythonBackground::GetDistanceSetInfo(int* peNum, float* pfStart, float* pfEnd, float* pfFarClip)
{
	if (!m_pkMap)
	{
		*peNum = 4;
		*pfStart = 10000.0f;
		*pfEnd = 15000.0f;
		*pfFarClip = 50000.0f;
		return;
	}
	*peNum = m_eViewDistanceNum;
	*pfStart = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
	*pfEnd = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
	*pfFarClip = m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPythonBackground::CPythonBackground()
{


	m_dwRenderShadowTime = 0;
	m_eViewDistanceNum = 0;
	m_eViewDistanceNum = 0;
	m_eViewDistanceNum = 0;
	m_eShadowLevel = SHADOW_NONE;
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	m_eShadowQualityLevel = SHADOW_QUALITY_BAD;
	m_eShadowTargetLevel = SHADOW_TARGET_NONE;
	m_eShadowBehindDistance = SHADOW_BEHIND_0;
	m_eTerrainShadow = TERRAIN_SHADOW_off;
	m_eTreeShadow = TREE_SHADOW_off;
	m_ePerformanceMode = PERFORMANCE_MODE_off;
	m_eRainLevel = RAIN_0;
	m_eCollisionLevel = COLLISION0;
	m_eDynamicLightLevel = DYNAMIC_LIGHT_off;
	m_eSpecularLevel = SPECULAR_off;
	m_eBloodLevel = BLOOD_off;
	m_eAmbianceEffectsLevel = AEFFECTS_off;
	m_eSelectShadowType = SHADOWTYPE_off;
	m_eVisibleLobbyType = VISIBLE_LOBBY_on;
#endif
	m_dwBaseX = 0;
	m_dwBaseY = 0;
	m_strMapName = "";
	m_iDayMode = DAY_MODE_LIGHT;
	m_iXMasTreeGrade = 0;
	m_bVisibleGuildArea = FALSE;

	SetViewDistanceSet(4, 25600.0f);
	SetViewDistanceSet(3, 25600.0f);
	SetViewDistanceSet(2, 25600.0f);
	SetViewDistanceSet(1, 25600.0f);
	SetViewDistanceSet(0, 25600.0f);
	Initialize();
}

CPythonBackground::~CPythonBackground()
{
	Tracen("CPythonBackground Clear");
}

#define ENABLE_ATLASINFO_FROM_ROOT
void CPythonBackground::Initialize()
{
#ifdef ENABLE_ATLASINFO_FROM_ROOT
	std::string stAtlasInfoFileName("AtlasInfo.txt");
#else
	std::string stAtlasInfoFileName(LocaleService_GetLocalePath());
	stAtlasInfoFileName += "/AtlasInfo.txt";
#endif
	SetAtlasInfoFileName(stAtlasInfoFileName.c_str());
	CMapManager::Initialize();
}

void CPythonBackground::__CreateProperty()
{
	if (CEterPackManager::SEARCH_FILE_FIRST == CEterPackManager::Instance().GetSearchMode() &&
		_access("property", 0) == 0)
	{
		m_PropertyManager.Initialize(NULL);

		CPropertyLoader PropertyLoader;
		PropertyLoader.SetPropertyManager(&m_PropertyManager);
		PropertyLoader.Create("*.*", "Property");
	}
	else
	{
		m_PropertyManager.Initialize("pack\\property");
	}
}

//////////////////////////////////////////////////////////////////////
// Normal Functions
//////////////////////////////////////////////////////////////////////

bool CPythonBackground::GetPickingPoint(D3DXVECTOR3* v3IntersectPt)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPoint(v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRay(const CRay& rRay, D3DXVECTOR3* v3IntersectPt)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRay(rRay, v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRayOnlyTerrain(const CRay& rRay, D3DXVECTOR3* v3IntersectPt)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRayOnlyTerrain(rRay, v3IntersectPt);
}

BOOL CPythonBackground::GetLightDirection(D3DXVECTOR3& rv3LightDirection)
{
	if (!mc_pcurEnvironmentData)
		return FALSE;

	rv3LightDirection.x = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
	rv3LightDirection.y = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
	rv3LightDirection.z = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CPythonBackground::Destroy()
{
	CMapManager::Destroy();
	m_SnowEnvironment.Destroy();
	m_bVisibleGuildArea = FALSE;
}

void CPythonBackground::Create()
{
	static int s_isCreateProperty = false;

	if (!s_isCreateProperty)
	{
		s_isCreateProperty = true;
		__CreateProperty();
	}

	CMapManager::Create();

	m_SnowEnvironment.Create();
}

struct FGetPortalID
{
	float m_fRequestX, m_fRequestY;
	std::set<int> m_kSet_iPortalID;
	FGetPortalID(float fRequestX, float fRequestY)
	{
		m_fRequestX = fRequestX;
		m_fRequestY = fRequestY;
	}
	void operator () (CGraphicObjectInstance* pObject)
	{
		for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			int iID = pObject->GetPortal(i);
			if (0 == iID)
				break;

			m_kSet_iPortalID.insert(iID);
		}
	}
};

void CPythonBackground::Update(float fCenterX, float fCenterY, float fCenterZ)
{
	if (!IsMapReady())
		return;
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1 = ELTimer_GetMSec();
#endif
	UpdateMap(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2 = ELTimer_GetMSec();
#endif
	UpdateAroundAmbience(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3 = ELTimer_GetMSec();
#endif
	m_SnowEnvironment.Update(D3DXVECTOR3(fCenterX, -fCenterY, fCenterZ));

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp = fopen("perf_bg_update.txt", "w");
		if (t3 - t1 > 5)
		{
			fprintf(fp, "BG.Total %d (Time %f)\n", t3 - t1, ELTimer_GetMSec() / 1000.0f);
			fprintf(fp, "BG.UpdateMap %d\n", t2 - t1);
			fprintf(fp, "BG.UpdateAmb %d\n", t3 - t2);
			fflush(fp);
		}
	}
#endif

	// Portal Process
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	if (rkMap.IsEnablePortal())
	{
		CCullingManager& rkCullingMgr = CCullingManager::Instance();
		FGetPortalID kGetPortalID(fCenterX, -fCenterY);

		Vector3d aVector3d;
		aVector3d.Set(fCenterX, -fCenterY, fCenterZ);

		Vector3d toTop;
		toTop.Set(0, 0, 25000.0f);

		rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

		std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
		if (!__IsSame(kGetPortalID.m_kSet_iPortalID, m_kSet_iShowingPortalID))
		{
			ClearPortal();
			std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
			for (; itor != kGetPortalID.m_kSet_iPortalID.end(); ++itor)
			{
				AddShowingPortalID(*itor);
			}
			RefreshPortal();

			m_kSet_iShowingPortalID = kGetPortalID.m_kSet_iPortalID;
		}
	}

	// Target Effect Process
	{
		std::map<DWORD, DWORD>::iterator itor = m_kMap_dwTargetID_dwChrID.begin();
		for (; itor != m_kMap_dwTargetID_dwChrID.end(); ++itor)
		{
			DWORD dwTargetID = itor->first;
			DWORD dwChrID = itor->second;

			CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

			if (!pInstance)
				continue;

			TPixelPosition kPixelPosition;
			pInstance->NEW_GetPixelPosition(&kPixelPosition);

			CreateSpecialEffect(dwTargetID,
				+kPixelPosition.x,
				-kPixelPosition.y,
				+kPixelPosition.z,
				g_strEffectName.c_str());
		}
	}

	// Reserve Target Effect
	{
		std::map<DWORD, SReserveTargetEffect>::iterator itor = m_kMap_dwID_kReserveTargetEffect.begin();
		for (; itor != m_kMap_dwID_kReserveTargetEffect.end();)
		{
			DWORD dwID = itor->first;
			SReserveTargetEffect& rReserveTargetEffect = itor->second;

			float ilx = float(rReserveTargetEffect.ilx);
			float ily = float(rReserveTargetEffect.ily);

			float fHeight = rkMap.GetHeight(ilx, ily);
			if (0.0f == fHeight)
			{
				++itor;
				continue;
			}

			CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());

			itor = m_kMap_dwID_kReserveTargetEffect.erase(itor);
		}
	}
}

bool CPythonBackground::__IsSame(std::set<int>& rleft, std::set<int>& rright)
{
	std::set<int>::iterator itor_l;
	std::set<int>::iterator itor_r;

	for (itor_l = rleft.begin(); itor_l != rleft.end(); ++itor_l)
	{
		if (rright.end() == rright.find(*itor_l))
			return false;
	}

	for (itor_r = rright.begin(); itor_r != rright.end(); ++itor_r)
	{
		if (rleft.end() == rleft.find(*itor_r))
			return false;
	}

	return true;
}

void CPythonBackground::Render()
{
	if (!IsMapReady())
		return;

	m_SnowEnvironment.Deform();

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.Render();
	if (m_bVisibleGuildArea)
		rkMap.RenderMarkedArea();
}

void CPythonBackground::RenderSnow()
{
	m_SnowEnvironment.Render();
}

void CPythonBackground::RenderPCBlocker()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderPCBlocker();
}

void CPythonBackground::RenderCollision()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderCollision();
}

void CPythonBackground::RenderShadow()
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderShadow();
	if (m_bVisibleGuildArea)
		rkMap.RenderMarkedArea();
}


void CPythonBackground::RenderCharacterShadowToTexture()
{


	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();


	if (Frustum::Instance().isIndoor == false)
	{
		rkMap.SkyUtil();
		if (Frustum::Instance().shadowType != 3)
		{
			if (Frustum::Instance().treeShadow == 3)
			{
				bool canRender = rkMap.BeginRenderCharacterShadowToTexture();
				if (canRender)
				{
					CSpeedTreeForestDirectX8::Instance().Render3(Forest_RenderAll); //tree to shadowmap
				}
				rkMap.EndRenderCharacterShadowToTexture();
			}
		}
	}


}

void CPythonBackground::RenderCharacterShadowToTexture2()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		if (Frustum::Instance().isIndoor == true)
		{
			bool canRender = rkMap.BeginRenderCharacterShadowToTexture2();
			if (canRender)
			{

				rkMap.RenderDungeon();
			}
			rkMap.EndRenderCharacterShadowToTexture2();
		}
	}

}

void CPythonBackground::RenderCharacterShadowToTexture3()
{


	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	if (Frustum::Instance().isIndoor)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		bool canRender = rkMap.BeginRenderCharacterShadowToTexture3();
		if (canRender)
		{
			CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

			if (Frustum::Instance().treeShadow == 0)
			{
				rkChrMgr.RenderShadowMainInstance();
			}

			if (Frustum::Instance().treeShadow >= 1)
			{
				rkChrMgr.RenderShadowAllInstances();
				RenderShadow(); //buildings
			}

			if (Frustum::Instance().treeShadow == 3)
				CSpeedTreeForestDirectX8::Instance().Render2(Forest_RenderAll); //tree to shadowmap

			//rkMap.RenderTree(Forest_RenderToShadow);



		}
		rkMap.EndRenderCharacterShadowToTexture3();
	}

}

void CPythonBackground::RenderCharacterShadowToTexture4()
{


	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	if (Frustum::Instance().isIndoor)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		if (Frustum::Instance().shadowType == 2)
		{
			bool canRender = rkMap.BeginRenderCharacterShadowToTexture4();
			if (canRender)
			{

				CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

				if (Frustum::Instance().treeShadow == 0)
				{
					rkChrMgr.RenderShadowMainInstance();
				}

				if (Frustum::Instance().treeShadow >= 1)
				{
					rkChrMgr.RenderShadowAllInstances();
					RenderShadow(); //buildings
				}

				if (Frustum::Instance().treeShadow == 3)
					CSpeedTreeForestDirectX8::Instance().Render2(Forest_RenderAll); //tree to shadowmap

				//rkMap.RenderTree(Forest_RenderToShadow);
			}
			rkMap.EndRenderCharacterShadowToTexture4();
		}
		else
		{
			if (Frustum::Instance().treeShadow == 3)
			{
				bool canRender = rkMap.BeginRenderCharacterShadowToTexture4();
				if (canRender)
				{
					CSpeedTreeForestDirectX8::Instance().Render3(Forest_RenderAll); //tree to shadowmap
				}
				rkMap.EndRenderCharacterShadowToTexture4();
			}
		}
	}


}

inline float Interpolate(float fStart, float fEnd, float fPercent)
{
	return fStart + (fEnd - fStart) * fPercent;
}
struct CollisionChecker
{
	bool isBlocked;
	CInstanceBase* pInstance;
	CollisionChecker(CInstanceBase* pInstance) : pInstance(pInstance), isBlocked(false) {}
	void operator () (CGraphicObjectInstance* pOpponent)
	{
		if (isBlocked)
			return;

		if (!pOpponent)
			return;

		if (pInstance->IsBlockObject(*pOpponent))
			isBlocked = true;
	}
};

struct CollisionAdjustChecker
{
	bool isBlocked;
	CInstanceBase* pInstance;
	CollisionAdjustChecker(CInstanceBase* pInstance) : pInstance(pInstance), isBlocked(false) {}
	void operator () (CGraphicObjectInstance* pOpponent)
	{
		if (!pOpponent)
			return;

		if (pInstance->AvoidObject(*pOpponent))
			isBlocked = true;
	}
};

bool CPythonBackground::CheckAdvancing(CInstanceBase* pInstance)
{
	if (!IsMapReady())
		return true;

	Vector3d center;
	float radius;
	pInstance->GetGraphicThingInstanceRef().GetBoundingSphere(center, radius);

	CCullingManager& rkCullingMgr = CCullingManager::Instance();

	CollisionAdjustChecker kCollisionAdjustChecker(pInstance);
	rkCullingMgr.ForInRange(center, radius, &kCollisionAdjustChecker);
	if (kCollisionAdjustChecker.isBlocked)
	{
		CollisionChecker kCollisionChecker(pInstance);
		rkCullingMgr.ForInRange(center, radius, &kCollisionChecker);
		if (kCollisionChecker.isBlocked)
		{
			pInstance->BlockMovement();
			return true;
		}
		else
		{
			pInstance->NEW_MoveToDestPixelPositionDirection(pInstance->NEW_GetDstPixelPositionRef());
		}
		return false;
	}
	return false;
}

void CPythonBackground::RenderSky()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderSky();
}

void CPythonBackground::RenderCloud()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderCloud();
}

void CPythonBackground::RenderWater()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderWater();
}

void CPythonBackground::RenderEffect()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderEffect();
}

void CPythonBackground::RenderBeforeLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderBeforeLensFlare();
}

void CPythonBackground::RenderAfterLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderAfterLensFlare();
}

void CPythonBackground::ClearGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.ClearGuildArea();
}

void CPythonBackground::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RegisterGuildArea(isx, isy, iex, iey);
}

void CPythonBackground::SetCharacterDirLight()
{
	if (!IsMapReady())
		return;

	if (!mc_pcurEnvironmentData)
		return;

	if (Frustum::Instance().shadowType == 3)
	{
		D3DLIGHT9 Light;
		Light.Type = D3DLIGHT_DIRECTIONAL;
		Light.Position = D3DXVECTOR3(50.0f, 150.0f, 350.0f);
		Light.Direction = D3DXVECTOR3(-0.15f, -0.3f, -0.9f);
		Light.Diffuse.r = 10.0f;
		Light.Diffuse.g = 10.0f;
		Light.Diffuse.b = 10.0f;
		Light.Diffuse.a = 10.0f;
		Light.Ambient.r = 10.0f;
		Light.Ambient.g = 10.0f;
		Light.Ambient.b = 10.0f;
		Light.Ambient.a = 10.0f;

		STATEMANAGER.SetLight(0, &Light);
	}
}

void CPythonBackground::SetBackgroundDirLight()
{
	if (!IsMapReady())
		return;
	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.SetLight(0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND]);
}

void CPythonBackground::GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY)
{
	rGlobalX -= m_dwBaseX;
	rGlobalY -= m_dwBaseY;
}

void CPythonBackground::LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	rLocalX += m_dwBaseX;
	rLocalY += m_dwBaseY;
}

void CPythonBackground::RegisterDungeonMapName(const char* c_szMapName)
{
	m_kSet_strDungeonMapName.insert(c_szMapName);
}

CPythonBackground::TMapInfo* CPythonBackground::GlobalPositionToMapInfo(DWORD dwGlobalX, DWORD dwGlobalY)
{
	TMapInfoVector::iterator f = std::find_if(m_kVct_kMapInfo.begin(), m_kVct_kMapInfo.end(), FFindWarpMapName(dwGlobalX, dwGlobalY));
	if (f == m_kVct_kMapInfo.end())
		return NULL;

	return &(*f);
}

bool map1isloaded = false;
void CPythonBackground::Warp(DWORD dwX, DWORD dwY)
{
	TMapInfo* pkMapInfo = GlobalPositionToMapInfo(dwX, dwY);
	if (!pkMapInfo)
	{
		TraceError("NOT_FOUND_GLOBAL_POSITION(%d, %d)", dwX, dwY);
		return;
	}

	CInstanceBase pk_inst;
	RefreshShadowLevel();
	Frustum::Instance().refreshshadowLevel = 1;
	TMapInfo& rMapInfo = *pkMapInfo;
	assert((dwX >= rMapInfo.m_dwBaseX) && (dwY >= rMapInfo.m_dwBaseY));

	if (!LoadMap(rMapInfo.m_strName, float(dwX - rMapInfo.m_dwBaseX), float(dwY - rMapInfo.m_dwBaseY), 0))
	{
		// LOAD_MAP_ERROR_HANDLING
		PostQuitMessage(0);
		// END_OF_LOAD_MAP_ERROR_HANDLING
		return;
	}

	//create Shaders
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.CreateShadowmapShaders();

	CPythonMiniMap::Instance().LoadAtlas();

	m_dwBaseX = rMapInfo.m_dwBaseX;
	m_dwBaseY = rMapInfo.m_dwBaseY;

	m_strMapName = rMapInfo.m_strName;
	//TraceError("Error aici Vasile.");

	//TraceError("Current Map: %s", m_strMapName);

	SetXMaxTree(m_iXMasTreeGrade);

	if (m_kSet_strDungeonMapName.end() != m_kSet_strDungeonMapName.find(m_strMapName))
	{
		EnableTerrainOnlyForHeight();

		CMapOutdoor& rkMap = GetMapOutdoorRef();
		rkMap.EnablePortal(TRUE);
	}

	m_kSet_iShowingPortalID.clear();
	m_kMap_dwTargetID_dwChrID.clear();
	m_kMap_dwID_kReserveTargetEffect.clear();
}

void CPythonBackground::VisibleGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.VisibleMarkedArea();

	m_bVisibleGuildArea = TRUE;
}

void CPythonBackground::DisableGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.DisableMarkedArea();

	m_bVisibleGuildArea = FALSE;
}

const char* CPythonBackground::GetWarpMapName()
{
	return m_strMapName.c_str();
}

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
bool CPythonBackground::IsBoomMap()
{
	std::vector<std::string> mapVec{
		"metin2_map_n_flame_01",
			"metin2_map_n_desert_01",
			"metin2_mapb_b1",
			"metin2_map_spiderdungeon_01",
			"metin2_map_spiderdungeon_02",
			"metin2_map_spiderdungeon_03",
			"metin2_map_spiderdungeon_03_dungeon",
			"metin2_map_deviltower1",
			"season1/metin2_map_sungzi_flame_hill_01",
			"season1/metin2_map_sungzi_flame_hill_02",
			"season1/metin2_map_sungzi_flame_hill_03",
			"season1/metin2_map_sungzi_desert_01",
			"season1/metin2_map_sungzi_desert_hill_01",
			"season1/metin2_map_sungzi_desert_hill_02",
			"season1/metin2_map_sungzi_desert_hill_03",
			"season2/metin2_map_empirewar03",
			"metin2_map_devilsCatacomb",
			"metin2_map_Mt_Thunder",
			"metin2_map_n_flame_dungeon_01",
			"metin2_map_dawnmist_dungeon_01",
			"metin2_map_Mt_Th_dungeon_01"
	};

	if (std::find(std::begin(mapVec), std::end(mapVec), m_strMapName.c_str()) != std::end(mapVec))
		return false;

	return true;
}
#endif

void CPythonBackground::ChangeToDay()
{
	m_iDayMode = DAY_MODE_LIGHT;
}

void CPythonBackground::ChangeToNight()
{
	m_iDayMode = DAY_MODE_DARK;
}

void CPythonBackground::EnableSnowEnvironment()
{
	m_SnowEnvironment.Enable();
}

void CPythonBackground::DisableSnowEnvironment()
{
	m_SnowEnvironment.Disable();
}

const D3DXVECTOR3 c_v3TreePos = D3DXVECTOR3(76500.0f, -60900.0f, 20215.0f);

void CPythonBackground::SetXMaxTree(int iGrade)
{
	if (!m_pkMap)
		return;

	assert(iGrade >= 0 && iGrade <= 3);
	m_iXMasTreeGrade = iGrade;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if ("map_n_snowm_01" != m_strMapName)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	if (0 == iGrade)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	//////////////////////////////////////////////////////////////////////

	iGrade -= 1;
	iGrade = max(iGrade, 0);
	iGrade = min(iGrade, 2);

	static std::string s_strTreeName[3] = {
		"d:/ymir work/tree/christmastree1.spt",
		"d:/ymir work/tree/christmastree2.spt",
		"d:/ymir work/tree/christmastree3.spt"
	};
	static std::string s_strEffectName[3] = {
		"d:/ymir work/effect/etc/christmas_tree/tree_1s.mse",
		"d:/ymir work/effect/etc/christmas_tree/tree_2s.mse",
		"d:/ymir work/effect/etc/christmas_tree/tree_3s.mse",
	};
	rkMap.XMasTree_Set(c_v3TreePos.x, c_v3TreePos.y, c_v3TreePos.z, s_strTreeName[iGrade].c_str(), s_strEffectName[iGrade].c_str());
}

void CPythonBackground::CreateTargetEffect(DWORD dwID, DWORD dwChrVID)
{
	m_kMap_dwTargetID_dwChrID.emplace(dwID, dwChrVID);
}

void CPythonBackground::CreateTargetEffect(DWORD dwID, long lx, long ly)
{
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	DWORD dwBaseX;
	DWORD dwBaseY;
	rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

	int ilx = +(lx - int(dwBaseX));
	int ily = -(ly - int(dwBaseY));

	float fHeight = rkMap.GetHeight(float(ilx), float(ily));

	if (0.0f == fHeight)
	{
		SReserveTargetEffect ReserveTargetEffect;
		ReserveTargetEffect.ilx = ilx;
		ReserveTargetEffect.ily = ily;
		m_kMap_dwID_kReserveTargetEffect.emplace(dwID, ReserveTargetEffect);
		return;
	}

	CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());
}

void CPythonBackground::DeleteTargetEffect(DWORD dwID)
{
	if (m_kMap_dwID_kReserveTargetEffect.end() != m_kMap_dwID_kReserveTargetEffect.find(dwID))
	{
		m_kMap_dwID_kReserveTargetEffect.erase(dwID);
	}
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
	{
		m_kMap_dwTargetID_dwChrID.erase(dwID);
	}

	DeleteSpecialEffect(dwID);
}

void CPythonBackground::CreateSpecialEffect(DWORD dwID, float fx, float fy, float fz, const char* c_szFileName)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialEffect(DWORD dwID)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffect_Delete(dwID);
}
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
