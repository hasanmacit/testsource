// PythonBackground.h: interface for the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
#define AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../gamelib/MapManager.h"
#include "../gamelib/TerrainDecal.h"
#include "../gamelib/SnowEnvironment.h"

class CInstanceBase;

class CPythonBackground : public CMapManager, public CSingleton<CPythonBackground>
{
public:
	enum
	{
		SHADOW_NONE,
		SHADOW_GROUND,
		SHADOW_GROUND_AND_SOLO,
		SHADOW_ALL,
		SHADOW_ALL_HIGH,
		SHADOW_ALL_MAX,
	};

	enum
	{
		DISTANCE0,
		DISTANCE1,
		DISTANCE2,
		DISTANCE3,
		DISTANCE4,
		NUM_DISTANCE_SET
	};

	enum
	{
		DAY_MODE_LIGHT,
		DAY_MODE_DARK,
	};

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	enum EShadowQuality
	{
		SHADOW_QUALITY_BAD,
		SHADOW_QUALITY_AVERAGE,
		SHADOW_QUALITY_GOOD,
		SHADOW_QUALITY_ULTRA,
	};

	enum ESpecularQuality
	{
		SPECULAR_on,
		SPECULAR_off,
	};

	enum EBloodQuality
	{
		BLOOD_on,
		BLOOD_off,
	};

	enum EAmbianceEffectsQuality
	{
		AEFFECTS_on,
		AEFFECTS_off,
	};

	enum EVisibleLobbyQuality
	{
		VISIBLE_LOBBY_on,
		VISIBLE_LOBBY_off,
	};

	enum EAmbianceShadowType
	{
		SHADOWTYPE_on,
		SHADOWTYPE_soft,
		SHADOWTYPE_close,
		SHADOWTYPE_off,
	};

	enum EShadowTarget
	{
		SHADOW_TARGET_NONE,
		SHADOW_TARGET_GROUND,
		SHADOW_TARGET_GROUND_AND_SOLO,
		SHADOW_TARGET_ALL,
	};

	enum EShadowDistanceBehind
	{
		SHADOW_BEHIND_0,
		SHADOW_BEHIND_1,
		SHADOW_BEHIND_2,
		SHADOW_BEHIND_3,
	};

	enum ETerrainShadow
	{
		TERRAIN_SHADOW_on,
		TERRAIN_SHADOW_off,
		TERRAIN_SHADOW_low,
	};

	enum ECollisionLevel
	{
		COLLISION0,
		COLLISION1,
		COLLISION2,
	};

	enum ETreeShadow
	{
		TREE_SHADOW_on,
		TREE_SHADOW_off,
		TREE_SHADOW_high,
		TREE_SHADOW_all,
	};

	enum EPerformanceMode
	{
		PERFORMANCE_MODE_on,
		PERFORMANCE_MODE_medium,
		PERFORMANCE_MODE_off,
	};

	enum EDynamicLight
	{
		DYNAMIC_LIGHT_on,
		DYNAMIC_LIGHT_off,
		DYNAMIC_LIGHT_low,
	};

	enum ERainLevel
	{
		RAIN_0,
		RAIN_1,
		RAIN_2,
		RAIN_3,
		RAIN_4,
	};

#endif

	typedef struct SVIEWDISTANCESET
	{
		float m_fFogStart;
		float m_fFogEnd;
		float m_fFarClip;
		D3DXVECTOR3 m_v3SkyBoxScale;
	} TVIEWDISTANCESET;

public:
	CPythonBackground();
	virtual ~CPythonBackground();

	void Initialize();

	void Destroy();
	void Create();

	float ShopDistance;
	float NpcDistance;
	float MountDistance;
	float PetDistance;
	float MobDistance;
	float StoneDistance;
	int SelectRain;

	LPD3DXBUFFER pCode = NULL;
	LPD3DXBUFFER pErrorMsgs = NULL;
	HRESULT hr;
	IDirect3DPixelShader9* g_lpPixelShader = NULL;

	LPD3DXBUFFER pCode2 = NULL;
	LPD3DXBUFFER pErrorMsgs2 = NULL;
	HRESULT hr2;
	IDirect3DPixelShader9* g_lpPixelShader2 = NULL;

	LPD3DXBUFFER pCode3 = NULL;
	LPD3DXBUFFER pErrorMsgs3 = NULL;
	HRESULT hr3;
	IDirect3DPixelShader9* g_lpPixelShader3 = NULL;

	void GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
	void LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);

	void EnableTerrainOnlyForHeight();
	bool SetSplatLimit(int iSplatNum);
	bool SetVisiblePart(int ePart, bool isVisible);
	bool SetShadowLevel(int eLevel);
#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	bool SetShadowQualityLevel(int eLevel);
	bool SetShadowTargetLevel(int eLevel);
	bool SetShadowDistanceBehind(int eLevel);
	bool SetTerrainShadow(int eLevel);
	bool SetTreeShadow(int eLevel);
	bool SetCollisionLevel(int eLevel);
	bool SetPerformanceMode(int eLevel);
	bool SetRainLevel(int eLevel);
	bool SetDynamicLightLevel(int eLevel);
	bool SetSpecularLevel(int eLevel);
	bool SetBloodLevel(int eLevel);
	bool SetAmbianceEffectsLevel(int eLevel);
	bool SetShadowTypeLevel(int eLevel);
	bool SetVisibleLobbyLevel(int eLevel);
#endif
	void RefreshShadowLevel();
	void SelectViewDistanceNum(int eNum);
	void SetViewDistanceSet(int eNum, float fFarClip);
	float GetFarClip();

	DWORD GetRenderShadowTime();
	void GetDistanceSetInfo(int * peNum, float * pfStart, float * pfEnd, float * pfFarClip);

	bool GetPickingPoint(D3DXVECTOR3 * v3IntersectPt);
	bool GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
	bool GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
	BOOL GetLightDirection(D3DXVECTOR3 & rv3LightDirection);

	void Update(float fCenterX, float fCenterY, float fCenterZ);

	void CreateCharacterShadowTexture();
	void ReleaseCharacterShadowTexture();
	float ambianceSoundPositionX(float x);
	float ambianceSoundPositionY(float y);
	float ambianceSoundPositionZ(float z);
	void RenderAmbianceAudioFirst();
	void RenderAmbianceAudio();
	void Render();
	void RenderSnow();
	void RenderPCBlocker();
	void RenderCollision();
	void RenderCharacterShadowToTexture();
	void RenderCharacterShadowToTexture2();
	void RenderCharacterShadowToTexture3();
	void RenderCharacterShadowToTexture4();
	void RenderCharacterShadowToTexture5();
	void RenderShadow();
	void RenderSky();
	void RenderCloud();
	void RenderWater();
	void RenderEffect();
	void RenderBeforeLensFlare();
	void RenderAfterLensFlare();

	bool CheckAdvancing(CInstanceBase * pInstance);

	void SetCharacterDirLight();
	void SetBackgroundDirLight();

	void ChangeToDay();
	void ChangeToNight();
	void EnableSnowEnvironment();
	void DisableSnowEnvironment();
	void SetXMaxTree(int iGrade);

	void ClearGuildArea();
	void RegisterGuildArea(int isx, int isy, int iex, int iey);

	void CreateTargetEffect(DWORD dwID, DWORD dwChrVID);
	void CreateTargetEffect(DWORD dwID, long lx, long ly);
	void DeleteTargetEffect(DWORD dwID);

	void CreateSpecialEffect(DWORD dwID, float fx, float fy, float fz, const char * c_szFileName);
	void DeleteSpecialEffect(DWORD dwID);

	void Warp(DWORD dwX, DWORD dwY);

	void VisibleGuildArea();
	void DisableGuildArea();

	void RegisterDungeonMapName(const char * c_szMapName);
	TMapInfo* GlobalPositionToMapInfo(DWORD dwGlobalX, DWORD dwGlobalY);
	const char* GetWarpMapName();

	float sizeX = 12000.0f;
	float sizeY = 12000.0f;

	float sizeX2;
	float sizeY2;

	float shadowIntensity = 80.0f;

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
	bool IsBoomMap();
#endif

protected:
	void __CreateProperty();
	bool __IsSame(std::set<int> & rleft, std::set<int> & rright);

protected:
	std::string m_strMapName;

private:
	CSnowEnvironment m_SnowEnvironment;

	int m_iDayMode;
	int m_iXMasTreeGrade;

	int m_eShadowLevel;
	int m_eViewDistanceNum;

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
	int m_eShadowQualityLevel;
	int m_eShadowTargetLevel;
	int m_eShadowBehindDistance;
	int m_eTerrainShadow;
	int m_eTreeShadow;
	int m_ePerformanceMode;
	int m_eRainLevel;
	int m_eCollisionLevel;
	int m_eDynamicLightLevel;
	int m_eSpecularLevel;
	int m_eBloodLevel;
	int m_eAmbianceEffectsLevel;
	int m_eSelectShadowType;
	int m_eVisibleLobbyType;
#endif

	BOOL m_bVisibleGuildArea;

	DWORD m_dwRenderShadowTime;

	DWORD m_dwBaseX;
	DWORD m_dwBaseY;

	TVIEWDISTANCESET m_ViewDistanceSet[NUM_DISTANCE_SET];

	std::set<int> m_kSet_iShowingPortalID;
	std::set<std::string> m_kSet_strDungeonMapName;
	std::map<DWORD, DWORD> m_kMap_dwTargetID_dwChrID;

	struct SReserveTargetEffect
	{
		int ilx;
		int ily;
	};
	std::map<DWORD, SReserveTargetEffect> m_kMap_dwID_kReserveTargetEffect;

	struct FFindWarpMapName
	{
		DWORD m_dwX, m_dwY;
		FFindWarpMapName(DWORD dwX, DWORD dwY)
		{
			m_dwX = dwX;
			m_dwY = dwY;
		}
		bool operator() (TMapInfo & rMapInfo)
		{
			if (m_dwX < rMapInfo.m_dwBaseX || m_dwX >= rMapInfo.m_dwEndX || m_dwY < rMapInfo.m_dwBaseY || m_dwY >= rMapInfo.m_dwEndY)
				return false;
			return true;
		}
	};
#ifdef JTX_ENABLE_GRAPHICS_OPTIONS
	public:
		int Collision;
		IDirect3DPixelShader9* pShader = NULL;
		IDirect3DPixelShader9* pShader2 = NULL;
		IDirect3DPixelShader9* pBlurShadowBuilding = NULL;
#endif
};

#endif // !defined(AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
