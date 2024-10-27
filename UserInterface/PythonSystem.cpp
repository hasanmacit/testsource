#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"
#include "../EterLib/GrpBase.h"

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME		true

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
//	if (m_poInterfaceHandler)
//		Py_DECREF(m_poInterfaceHandler);

	m_poInterfaceHandler = poHandler;

//	if (m_poInterfaceHandler)
//		Py_INCREF(m_poInterfaceHandler);
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = NULL;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible	= iVisible;
	m_WindowStatus[iIndex].isMinimized	= iMinimized;
	m_WindowStatus[iIndex].ixPosition	= ix;
	m_WindowStatus[iIndex].iyPosition	= iy;
	m_WindowStatus[iIndex].iHeight		= iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D8 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

#ifdef DIRECTX9
	lpD3D->GetAdapterIdentifier(0, D3DENUM_WHQL_LEVEL, &d3dAdapterIdentifier);
#else
	lpD3D->GetAdapterIdentifier(0, D3DENUM_NO_WHQL_LEVEL, &d3dAdapterIdentifier);

#endif
	lpD3D->GetAdapterDisplayMode(0, &d3ddmDesktop);

#ifdef DIRECTX9
	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0, d3ddmDesktop.Format);
#else
	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0);
#endif

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
#ifdef DIRECTX9
		lpD3D->EnumAdapterModes(0, d3ddmDesktop.Format, iMode, &DisplayMode);
#else
		lpD3D->EnumAdapterModes(0, iMode, &DisplayMode);
#endif
		DWORD bpp = 0;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

DWORD CPythonSystem::GetWidth()
{
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

int CPythonSystem::GetEffectsVolume()
{
	return m_Config.effects_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

void CPythonSystem::SetEffectsVolumef(float fVolume)
{
	m_Config.effects_volume = int(5 * fVolume);
}

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
int CPythonSystem::GetShadowTypeLevel()
{
	return MIN(MAX(m_Config.iShadowTypeLevel,
		CPythonBackground::SHADOWTYPE_on), /* min */
		CPythonBackground::SHADOWTYPE_off /* max */
	);
}

void CPythonSystem::SetShadowTypeLevel(unsigned int level)
{
	m_Config.iShadowTypeLevel = MIN(MAX(level,
		CPythonBackground::SHADOWTYPE_on), /* min */
		CPythonBackground::SHADOWTYPE_off /* max */
	);
}

int CPythonSystem::GetVisibleLobbyLevel()
{
	return MIN(MAX(m_Config.iVisibleLobbyLevel,
		CPythonBackground::VISIBLE_LOBBY_on), /* min */
		CPythonBackground::VISIBLE_LOBBY_off /* max */
	);
}

void CPythonSystem::SetVisibleLobbyLevel(unsigned int level)
{
	m_Config.iVisibleLobbyLevel = MIN(MAX(level,
		CPythonBackground::VISIBLE_LOBBY_on), /* min */
		CPythonBackground::VISIBLE_LOBBY_off /* max */
	);
}

int CPythonSystem::GetRainLevel()
{
	return MIN(MAX(m_Config.iRainLevel,
		CPythonBackground::RAIN_0), /* min */
		CPythonBackground::RAIN_3 /* max */
	);
}

void CPythonSystem::SetRainLevel(unsigned int level)
{
	m_Config.iRainLevel = MIN(MAX(level,
		CPythonBackground::RAIN_0), /* min */
		CPythonBackground::RAIN_3 /* max */
	);
}

//specular
int CPythonSystem::GetSpecularLevel()
{
	return MIN(MAX(m_Config.iSpecularLevel,
		CPythonBackground::SPECULAR_on), /* min */
		CPythonBackground::SPECULAR_off /* max */
	);
}

void CPythonSystem::SetSpecularLevel(unsigned int level)
{
	m_Config.iSpecularLevel = MIN(MAX(level,
		CPythonBackground::SPECULAR_on), /* min */
		CPythonBackground::SPECULAR_off /* max */
	);
}

//blood
int CPythonSystem::GetBloodLevel()
{
	return MIN(MAX(m_Config.iBloodLevel,
		CPythonBackground::BLOOD_on), /* min */
		CPythonBackground::BLOOD_off /* max */
	);
}

void CPythonSystem::SetBloodLevel(unsigned int level)
{
	m_Config.iBloodLevel = MIN(MAX(level,
		CPythonBackground::BLOOD_on), /* min */
		CPythonBackground::BLOOD_off /* max */
	);
}

//Ambiance eff
int CPythonSystem::GetAmbianceEffectsLevel()
{
	return MIN(MAX(m_Config.iAmbianceEffectsLevel,
		CPythonBackground::AEFFECTS_on), /* min */
		CPythonBackground::AEFFECTS_off /* max */
	);
}

void CPythonSystem::SetAmbianceEffectsLevel(unsigned int level)
{
	m_Config.iAmbianceEffectsLevel = MIN(MAX(level,
		CPythonBackground::AEFFECTS_on), /* min */
		CPythonBackground::AEFFECTS_off /* max */
	);
}

int CPythonSystem::GetDynamicLightLevel()
{
	return MIN(MAX(m_Config.iDynamicLightLevel,
		CPythonBackground::DYNAMIC_LIGHT_on), /* min */
		CPythonBackground::DYNAMIC_LIGHT_low /* max */
	);
}

void CPythonSystem::SetDynamicLightLevel(unsigned int level)
{
	m_Config.iDynamicLightLevel = MIN(MAX(level,
		CPythonBackground::DYNAMIC_LIGHT_on), /* min */
		CPythonBackground::DYNAMIC_LIGHT_low /* max */
	);
}

int CPythonSystem::GetShadowQualityLevel()
{
	return MIN(MAX(m_Config.iShadowQualityLevel,
		CPythonBackground::SHADOW_QUALITY_BAD), /* min */
		CPythonBackground::SHADOW_QUALITY_ULTRA /* max */
	);
}

void CPythonSystem::SetShadowQualityLevel(unsigned int level)
{
	m_Config.iShadowQualityLevel = MIN(MAX(level,
		CPythonBackground::SHADOW_QUALITY_BAD), /* min */
		CPythonBackground::SHADOW_QUALITY_ULTRA /* max */
	);
}

int CPythonSystem::GetTerrainShadow()
{
	return MIN(MAX(m_Config.iTerrainShadow,
		CPythonBackground::TERRAIN_SHADOW_on), /* min */
		CPythonBackground::TERRAIN_SHADOW_low /* max */
	);
}

void CPythonSystem::SetTerrainShadow(unsigned int level)
{
	m_Config.iTerrainShadow = MIN(MAX(level,
		CPythonBackground::TERRAIN_SHADOW_on), /* min */
		CPythonBackground::TERRAIN_SHADOW_low /* max */
	);
}

int CPythonSystem::GetTreeShadow()
{
	return MIN(MAX(m_Config.iTreeShadow,
		CPythonBackground::TREE_SHADOW_on), /* min */
		CPythonBackground::TREE_SHADOW_all /* max */
	);
}

void CPythonSystem::SetTreeShadow(unsigned int level)
{
	m_Config.iTreeShadow = MIN(MAX(level,
		CPythonBackground::TREE_SHADOW_on), /* min */
		CPythonBackground::TREE_SHADOW_all /* max */
	);
}

int CPythonSystem::GetCollisionLevel()
{
	return MIN(MAX(m_Config.iCollisionLevel,
		CPythonBackground::COLLISION0), /* min */
		CPythonBackground::COLLISION2 /* max */
	);
}

void CPythonSystem::SetCollisionLevel(unsigned int level)
{
	m_Config.iCollisionLevel = MIN(MAX(level,
		CPythonBackground::COLLISION0), /* min */
		CPythonBackground::COLLISION2 /* max */
	);
}

int CPythonSystem::GetPerformanceMode()
{
	return MIN(MAX(m_Config.iPerformanceMode,
		CPythonBackground::PERFORMANCE_MODE_on), /* min */
		CPythonBackground::PERFORMANCE_MODE_off /* max */
	);
}

void CPythonSystem::SetPerformanceMode(unsigned int level)
{
	m_Config.iPerformanceMode = MIN(MAX(level,
		CPythonBackground::PERFORMANCE_MODE_on), /* min */
		CPythonBackground::PERFORMANCE_MODE_off /* max */
	);
}

int CPythonSystem::GetShadowDistanceBehind()
{
	return MIN(MAX(m_Config.iShadowDistanceBehind,
		CPythonBackground::SHADOW_BEHIND_0), /* min */
		CPythonBackground::SHADOW_BEHIND_3 /* max */
	);
}

void CPythonSystem::SetShadowDistanceBehind(unsigned int level)
{
	m_Config.iShadowDistanceBehind = MIN(MAX(level,
		CPythonBackground::SHADOW_BEHIND_0), /* min */
		CPythonBackground::SHADOW_BEHIND_3 /* max */
	);
}

int CPythonSystem::GetShadowTargetLevel()
{
	return MIN(MAX(m_Config.iShadowTargetLevel,
		CPythonBackground::SHADOW_TARGET_NONE), /* min */
		CPythonBackground::SHADOW_TARGET_ALL /* max */
	);
}

void CPythonSystem::SetShadowTargetLevel(unsigned int level)
{
	m_Config.iShadowTargetLevel = MIN(MAX(level,
		CPythonBackground::SHADOW_TARGET_NONE), /* min */
		CPythonBackground::SHADOW_TARGET_ALL /* max */
	);
}
#endif

int CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	//CPythonBackground::instance().RefreshShadowLevel();
}

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
	m_Config.bpp				= 32;

#if defined( LOCALE_SERVICE_WE_JAPAN )
	m_Config.bWindowed			= true;
#else
	m_Config.bWindowed			= false;
#endif

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	m_Config.shopnames_range = 0.5f;
#endif
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume		= 5;
	m_Config.effects_volume		= 5;

	m_Config.bDecompressDDS		= 0;
	m_Config.bSoftwareTiling	= 0;
	m_Config.iShadowLevel		= 3;
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	m_Config.iShadowQualityLevel = CPythonBackground::SHADOW_QUALITY_BAD;
	m_Config.iVisibleLobbyLevel = CPythonBackground::VISIBLE_LOBBY_on;
	m_Config.iShadowDistanceBehind = CPythonBackground::SHADOW_BEHIND_0;
	m_Config.iShadowTargetLevel = CPythonBackground::SHADOW_TARGET_ALL;
	m_Config.iTerrainShadow = CPythonBackground::TERRAIN_SHADOW_off;
	m_Config.iTreeShadow = CPythonBackground::TREE_SHADOW_off;
	m_Config.iPerformanceMode = CPythonBackground::PERFORMANCE_MODE_off;
	m_Config.iRainLevel = CPythonBackground::RAIN_0;
	m_Config.iCollisionLevel = CPythonBackground::COLLISION0;
	m_Config.iDynamicLightLevel = CPythonBackground::DYNAMIC_LIGHT_off;
	m_Config.iSpecularLevel = CPythonBackground::SPECULAR_off;
	m_Config.iBloodLevel = CPythonBackground::BLOOD_off;
	m_Config.iAmbianceEffectsLevel = CPythonBackground::AEFFECTS_off;
	m_Config.iShadowTypeLevel = CPythonBackground::SHADOWTYPE_off;
#endif
	m_Config.bViewChat			= true;
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	m_Config.bShowMobAIFlag		= true;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	m_Config.bShowMobLevel		= true;
#endif

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
	m_Config.bShowNightMode = true;
	m_Config.bShowSnowMode = true;
	m_Config.bShowSnowTextureMode = true;
#endif

#if defined(ENABLE_FOV_OPTION)
	m_Config.fFOV = c_fDefaultCameraPerspective;
	m_Config.fSHOP = c_fDefaultShopDistance;
#endif
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}

void CPythonSystem::SetShowMobAIFlagFlag(int iFlag)
{
	m_Config.bShowMobAIFlag = iFlag == 1 ? true : false;
}
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobLevelFlag(int iFlag)
{
	m_Config.bShowMobLevel = iFlag == 1 ? true : false;
}
#endif

#if defined(ENABLE_FOV_OPTION)
float CPythonSystem::GetFOV()
{
	return m_Config.fFOV;
}

void CPythonSystem::SetFOV(float fFlag)
{
	m_Config.fFOV = fMINMAX(c_fMinCameraPerspective, fFlag, c_fMaxCameraPerspective);
}
#endif
#if defined(JTX_DISTANCE_SETTINGS)
//Leaves Level
float CPythonSystem::GetLEAVES()
{
	if (m_Config.fLeaves < 1.0f)
	{
		m_Config.fLeaves = 1.0f;
	}

	Frustum::Instance().getLeaves = m_Config.fLeaves;

	return m_Config.fLeaves;
}

void CPythonSystem::SetLEAVES(float fFlag)
{
	m_Config.fLeaves = fMINMAX(c_fMinLeaves, fFlag, c_fMaxLeaves);
}

//SHADOW INTENSITY
float CPythonSystem::GetShadowIntensity()
{
	if (m_Config.fShadowIntensity < 10.0f)
	{
		m_Config.fShadowIntensity = 49.0f;
	}


	CGraphicBase::ms_fShadowIntensity = m_Config.fShadowIntensity;

	return m_Config.fShadowIntensity;
}

void CPythonSystem::SetShadowIntensity(float fFlag)
{
	m_Config.fShadowIntensity = fMINMAX(c_fMinShadowIntensity, fFlag, c_fMaxShadowIntensity);
}

//SHOP
float CPythonSystem::GetSHOP()
{
	return m_Config.fSHOP;
}

void CPythonSystem::SetSHOP(float fFlag)
{
	m_Config.fSHOP = fMINMAX(c_fMinShopDistance, fFlag, c_fMaxShopDistance);
}

//SHOPTEXT
float CPythonSystem::GetSHOPTEXT()
{
	return m_Config.fSHOP < m_Config.fSHOPTEXT ? m_Config.fSHOP : m_Config.fSHOPTEXT;
}

void CPythonSystem::SetSHOPTEXT(float fFlag)
{
	m_Config.fSHOPTEXT = fMINMAX(c_fMinShopTextDistance, fFlag, c_fMaxShopTextDistance);
}

//MOB
float CPythonSystem::GetMOB()
{
	if (m_Config.fMOB < 2500.0f)
	{
		m_Config.fMOB = 2500.0f;
	}

	return m_Config.fMOB;
}

void CPythonSystem::SetMOB(float fFlag)
{
	m_Config.fMOB = fMINMAX(c_fMinMobDistance, fFlag, c_fMaxMobDistance);
}

//STONE
float CPythonSystem::GetSTONE()
{
	if (m_Config.fSTONE < 2500.0f)
	{
		m_Config.fSTONE = 2500.0f;
	}

	return m_Config.fSTONE;
}

void CPythonSystem::SetSTONE(float fFlag)
{
	m_Config.fSTONE = fMINMAX(c_fMinStoneDistance, fFlag, c_fMaxStoneDistance);
}

//PET
float CPythonSystem::GetPET()
{
	if (m_Config.fPET < 500.0f)
	{
		m_Config.fPET = 500.0f;
	}

	return m_Config.fPET;
}

void CPythonSystem::SetPET(float fFlag)
{
	m_Config.fPET = fMINMAX(c_fMinPetDistance, fFlag, c_fMaxPetDistance);
}

//MOUNT
float CPythonSystem::GetMOUNT()
{
	if (m_Config.fMOUNT < 500.0f)
	{
		m_Config.fMOUNT = 500.0f;
	}

	return m_Config.fMOUNT;
}

void CPythonSystem::SetMOUNT(float fFlag)
{
	m_Config.fMOUNT = fMINMAX(c_fMinMountDistance, fFlag, c_fMaxMountDistance);
}

//NPC
float CPythonSystem::GetNPC()
{
	if (m_Config.fNPC < 2500.0f)
	{
		m_Config.fNPC = 2500.0f;
	}

	return m_Config.fNPC;
}

void CPythonSystem::SetNPC(float fFlag)
{
	m_Config.fNPC = fMINMAX(c_fMinNpcDistance, fFlag, c_fMaxNpcDistance);
}

//TEXT
float CPythonSystem::GetTEXT()
{
	return m_Config.fTEXT;
}

void CPythonSystem::SetTEXT(float fFlag)
{
	m_Config.fTEXT = fMINMAX(c_fMinTextDistance, fFlag, c_fMaxTextDistance);
}

//EFFECT
float CPythonSystem::GetEFFECT()
{
	if (m_Config.fEFFECT < 0.0f)
	{
		m_Config.fEFFECT = 0.0f;
	}

	if (m_Config.fEFFECT >= 10000.0f)
	{
		m_Config.fEFFECT = 10000.0f;
	}

	return m_Config.fEFFECT;
}

void CPythonSystem::SetEFFECT(float fFlag)
{
	m_Config.fEFFECT = fMINMAX(c_fMinEffectDistance, fFlag, c_fMaxEffectDistance);
}

//Stone Scale
float CPythonSystem::GetStoneScale()
{
	if (m_Config.fStoneScale < 100.0f)
	{
		m_Config.fStoneScale = 100.0f;
	}

	if (m_Config.fStoneScale >= 200.0f)
	{
		m_Config.fStoneScale = 200.0f;
	}

	return m_Config.fStoneScale;
}

void CPythonSystem::SetStoneScale(float fFlag)
{
	m_Config.fStoneScale = fMINMAX(c_fMinStoneScale, fFlag, c_fMaxStoneScale);
}

//Boss Scale
float CPythonSystem::GetBossScale()
{
	if (m_Config.fBossScale < 100.0f)
	{
		m_Config.fBossScale = 100.0f;
	}

	if (m_Config.fBossScale >= 150.0f)
	{
		m_Config.fBossScale = 150.0f;
	}

	return m_Config.fBossScale;
}

void CPythonSystem::SetBossScale(float fFlag)
{
	m_Config.fBossScale = fMINMAX(c_fMinBossScale, fFlag, c_fMaxBossScale);
}

//Mount Scale
float CPythonSystem::GetMountScale()
{
	if (m_Config.fMountScale < 50.0f)
	{
		m_Config.fMountScale = 50.0f;
	}

	if (m_Config.fMountScale >= 100.0f)
	{
		m_Config.fMountScale =	100.0f;
	}

	return m_Config.fMountScale;
}

void CPythonSystem::SetMountScale(float fFlag)
{
	m_Config.fMountScale = fMINMAX(c_fMinMountScale, fFlag, c_fMaxMountScale);
}
#endif

int CPythonSystem::GetPlayerFlag()
{
	return m_Config.pFlag;
}

int CPythonSystem::GetPlayerInfo(int type)
{
	return m_Config.pInfo[type];
}

void CPythonSystem::SetPlayerFlag(int pFlag)
{
	m_Config.pFlag = pFlag;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.RefreshAllPCTextTail();
}

void CPythonSystem::SetPlayerInfo(int type, int value)
{
	m_Config.pInfo[type] = value;
}

bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
void CPythonSystem::SetNightModeOption(int iOpt)
{
	m_Config.bShowNightMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetNightModeOption()
{
	return m_Config.bShowNightMode;
}

void CPythonSystem::SetSnowModeOption(int iOpt)
{
	m_Config.bShowSnowMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowModeOption()
{
	return m_Config.bShowSnowMode;
}

void CPythonSystem::SetSnowTextureModeOption(int iOpt)
{
	m_Config.bShowSnowTextureMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowTextureModeOption()
{
	return m_Config.bShowSnowTextureMode;
}
#endif

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	FILE * fp = NULL;

	if (NULL == (fp = fopen("config/zenaris.cfg", "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width = atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height = atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp = atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { // Old compatiability
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} 
		else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "EFFECTS_VOLUME")) {
			if (strchr(value, '.') == 0) { // Old compatiability
				m_Config.effects_volume = pow(10.0f, (-1.0f + (((float)atoi(value)) / 5.0f)));
				if (atoi(value) == 0)
					m_Config.effects_volume = 0.0f;
			}
			else
				m_Config.effects_volume = atof(value);
		}
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		else if (!stricmp(command, "shop_range"))
		{
			if (strchr(value, '.') == 0)
			{
				m_Config.shopnames_range = pow(10.0f, (-1.0f + (((float)atoi(value)) / 5.0f)));
				if (atoi(value) == 0)
					m_Config.shopnames_range = 0.5f;
			}
			else
				m_Config.shopnames_range = atof(value);
		}
#endif
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
		else if (!stricmp(command, "SHADOW_TYPE"))
			m_Config.iShadowTypeLevel = atoi(value);
		else if (!stricmp(command, "VISIBLE_LOBBY"))
			m_Config.iVisibleLobbyLevel = atoi(value);
		else if (!stricmp(command, "SHADOW_TARGET_LEVEL"))
			m_Config.iShadowTargetLevel = atoi(value);
		else if (!stricmp(command, "DYNAMIC_LIGHT"))
			m_Config.iDynamicLightLevel = atoi(value);
		else if (!stricmp(command, "SHADOW_DISTANCE_BEHIND"))
			m_Config.iShadowDistanceBehind = atoi(value);
		else if (!stricmp(command, "SHADOW_QUALITY_LEVEL"))
			m_Config.iShadowQualityLevel = atoi(value);
		else if (!stricmp(command, "TERRAIN_SHADOW"))
			m_Config.iTerrainShadow = atoi(value);
		else if (!stricmp(command, "SHADOW_DETAILS"))
			m_Config.iTreeShadow = atoi(value);
		else if (!stricmp(command, "PERFORMANCE_MODE"))
			m_Config.iPerformanceMode = atoi(value);
		else if (!stricmp(command, "RAIN_LEVEL"))
			m_Config.iRainLevel = atoi(value);
		else if (!stricmp(command, "COLLISION_LEVEL"))
			m_Config.iCollisionLevel = atoi(value);
		else if (!stricmp(command, "SPECULAR"))
			m_Config.iSpecularLevel = atoi(value);
		else if (!stricmp(command, "BLOOD"))
			m_Config.iBloodLevel = atoi(value);
		else if (!stricmp(command, "AMBIANCE_EFFECTS"))
			m_Config.iAmbianceEffectsLevel = atoi(value);
#else
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
#endif
#if defined(ENABLE_FOV_OPTION)
		else if (!stricmp(command, "FOV"))
		{
			m_Config.fFOV = atof(value);
		}
#endif
#if defined(JTX_DISTANCE_SETTINGS)
		else if (!stricmp(command, "SHADOW"))
		{
			m_Config.fShadowIntensity = atoi(value);
		}
		else if (!stricmp(command, "LEAVES"))
		{
			m_Config.fLeaves = atof(value);
		}
		else if (!stricmp(command, "SHOP"))
		{
			m_Config.fSHOP = atof(value);
		}
		else if (!stricmp(command, "SHOPTEXT"))
		{
			m_Config.fSHOPTEXT = atof(value);
		}
		else if (!stricmp(command, "MOB"))
		{
			m_Config.fMOB = atof(value);
		}
		else if (!stricmp(command, "STONE"))
		{
			m_Config.fSTONE = atof(value);
		}
		else if (!stricmp(command, "PET"))
		{
			m_Config.fPET = atof(value);
		}
		else if (!stricmp(command, "MOUNT"))
		{
			m_Config.fMOUNT = atof(value);
		}
		else if (!stricmp(command, "NPC"))
		{
			m_Config.fNPC = atof(value);
		}
		else if (!stricmp(command, "EFFECT"))
		{
			m_Config.fEFFECT = atof(value);
		}
		else if (!stricmp(command, "TEXT"))
		{
			m_Config.fTEXT = atof(value);
		}
		else if (!stricmp(command, "STONE_SCALE"))
		{
			m_Config.fStoneScale = atof(value);
		}
		else if (!stricmp(command, "BOSS_SCALE"))
		{
			m_Config.fBossScale = atof(value);
		}
		else if (!stricmp(command, "MOUNT_SCALE"))
		{
			m_Config.fMountScale = atof(value);
		}
#endif
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
		else if (!stricmp(command, "NIGHT_MODE_ON"))
		m_Config.bShowNightMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
		m_Config.bShowSnowMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
		m_Config.bShowSnowTextureMode = atoi(value) == 1 ? true : false;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
#endif
	}

	if (m_Config.bWindowed)
	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
		{
			m_Config.width = screen_width;
		}
		if (m_Config.height >= screen_height)
		{
			int config_height = m_Config.height;
			int difference = (config_height - screen_height) + 7;
			m_Config.height = config_height - difference;
		}
	}

	m_OldConfig = m_Config;

	fclose(fp);

//	Tracef("LoadConfig: Resolution: %dx%d %dBPP %dHZ Software Cursor: %d, Music/Voice Volume: %d/%d Gamma: %d\n",
//		m_Config.width,
//		m_Config.height,
//		m_Config.bpp,
//		m_Config.frequency,
//		m_Config.is_software_cursor,
//		m_Config.music_volume,
//		m_Config.voice_volume,
//		m_Config.gamma);

	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	if (NULL == (fp = fopen("config/zenaris.cfg", "wt")))
		return false;

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"EFFECTS_VOLUME				%.3f\n"
				"GAMMA						%d\n"
				"shop_range					%.3f\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
				"DECOMPRESSED_TEXTURE		%d\n",
				m_Config.width,
				m_Config.height,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.effects_volume,
				m_Config.gamma,
				//m_Config.shopnames_range,
				m_Config.isSaveID,
				m_Config.SaveID,
				m_Config.bDecompressDDS);

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
	fprintf(fp, "NIGHT_MODE_ON		 	%d\n", m_Config.bShowNightMode);
	fprintf(fp, "SNOW_MODE_ON			%d\n", m_Config.bShowSnowMode);
	fprintf(fp, "SNOW_TEXTURE_MODE		%d\n", m_Config.bShowSnowTextureMode);
#endif
	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	if (m_Config.bShowMobAIFlag == 0)
		fprintf(fp, "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	if (m_Config.bShowMobLevel == 0)
		fprintf(fp, "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
#endif

	fprintf(fp, "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
	fprintf(fp, "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	fprintf(fp, "SHADOW_TYPE				%d\n", m_Config.iShadowTypeLevel);
	fprintf(fp, "SHADOW_TARGET_LEVEL		%d\n", m_Config.iShadowTargetLevel);
	fprintf(fp, "SHADOW_DISTANCE_BEHIND		%d\n", m_Config.iShadowDistanceBehind);
	fprintf(fp, "SHADOW_QUALITY_LEVEL		%d\n", m_Config.iShadowQualityLevel);
	fprintf(fp, "TERRAIN_SHADOW				%d\n", m_Config.iTerrainShadow);
	fprintf(fp, "SHADOW_DETAILS				%d\n", m_Config.iTreeShadow);
	fprintf(fp, "PERFORMANCE_MODE			%d\n", m_Config.iPerformanceMode);
	fprintf(fp, "RAIN_LEVEL					%d\n", m_Config.iRainLevel);
	fprintf(fp, "COLLISION_LEVEL			%d\n", m_Config.iCollisionLevel);
	fprintf(fp, "DYNAMIC_LIGHT				%d\n", m_Config.iDynamicLightLevel);
	fprintf(fp, "SPECULAR					%d\n", m_Config.iSpecularLevel);
	fprintf(fp, "BLOOD						%d\n", m_Config.iBloodLevel);
	fprintf(fp, "AMBIANCE_EFFECTS			%d\n", m_Config.iAmbianceEffectsLevel);
	fprintf(fp, "VISIBLE_LOBBY				%d\n", m_Config.iVisibleLobbyLevel);
#else
	fprintf(fp, "SHADOW_LEVEL				%d\n", m_Config.iShadowLevel);
#endif
#if defined(ENABLE_FOV_OPTION)
	fprintf(fp, "FOV						%.2f\n", m_Config.fFOV);
#endif
#if defined(JTX_DISTANCE_SETTINGS)
	fprintf(fp, "SHOP						%.2f\n", m_Config.fSHOP);
	fprintf(fp, "SHOPTEXT					%.2f\n", m_Config.fSHOPTEXT);
	fprintf(fp, "MOB						%.2f\n", m_Config.fMOB);
	fprintf(fp, "PET						%.2f\n", m_Config.fPET);
	fprintf(fp, "MOUNT						%.2f\n", m_Config.fMOUNT);
	fprintf(fp, "STONE						%.2f\n", m_Config.fSTONE);
	fprintf(fp, "NPC						%.2f\n", m_Config.fNPC);
	fprintf(fp, "TEXT						%.2f\n", m_Config.fTEXT);
	fprintf(fp, "EFFECT						%.2f\n", m_Config.fEFFECT);
	fprintf(fp, "STONE_SCALE				%.2f\n", m_Config.fStoneScale);
	fprintf(fp, "BOSS_SCALE					%.2f\n", m_Config.fBossScale);
	fprintf(fp, "MOUNT_SCALE				%.2f\n", m_Config.fMountScale);
	fprintf(fp, "SHADOW						%d\n", m_Config.fShadowIntensity);
	fprintf(fp, "LEAVES						%.2f\n", m_Config.fLeaves);
	fprintf(fp, "PLAYER_FLAG				%d\n", m_Config.pFlag);
	fprintf(fp, "EVENT_INFO					%d\n", m_Config.pInfo[0]);
	fprintf(fp, "SAVE_PM				%d\n", m_Config.pInfo[1]);
	fprintf(fp, "AFFECT_SHOW				%d\n", m_Config.pInfo[2]);
	fprintf(fp, "QUEUE_INFO				%d\n", m_Config.pInfo[3]);
	fprintf(fp, "OFF_SHOP_INFO				%d\n", m_Config.pInfo[4]);
#endif
	fprintf(fp, "\n");

	fclose(fp);
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	FILE * File;
	File = fopen("interface.cfg", "rb");

	if (!File)
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
	return true;
}

void CPythonSystem::SaveInterfaceStatus()
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	FILE * File;

	File = fopen("interface.cfg", "wb");

	if (!File)
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
}

bool CPythonSystem::isInterfaceConfig()
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex)
{
	return m_WindowStatus[iIndex];
}

void CPythonSystem::ApplyConfig()
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
			case 0: val = 0.4f;	break;
			case 1: val = 0.7f; break;
			case 2: val = 1.0f; break;
			case 3: val = 1.2f; break;
			case 4: val = 1.4f; break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	// Shadow
	/*
	if (m_Config.is_shadow)
		CScreen::SetShadowFlag(true);
	else
		CScreen::SetShadowFlag(false);
	*/
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	/*
	float fMusicVolume;
	if (0 == m_Config.music_volume)
		fMusicVolume = 0.0f;
	else
		fMusicVolume= (float)pow(10.0f, (-1.0f + (float)m_Config.music_volume / 5.0f));
		*/
	rkSndMgr.SetMusicVolume(m_Config.music_volume);

	/*
	float fVoiceVolume;
	if (0 == m_Config.voice_volume)
		fVoiceVolume = 0.0f;
	else
		fVoiceVolume = (float)pow(10.0f, (-1.0f + (float)m_Config.voice_volume / 5.0f));
	*/
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);
	rkSndMgr.SetEffectsVolumeGrade(m_Config.effects_volume);
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(NULL);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = NULL;

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==NULL && "CPythonSystem MUST CLEAR!");
}
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
