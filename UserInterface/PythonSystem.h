#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS,
			WINDOW_INVENTORY,
			WINDOW_ABILITY,
			WINDOW_SOCIETY,
			WINDOW_JOURNAL,
			WINDOW_COMMAND,

			WINDOW_QUICK,
			WINDOW_GAUGE,
			WINDOW_MINIMAP,
			WINDOW_CHAT,

			WINDOW_MAX_NUM,
		};

		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SWindowStatus
		{
			int		isVisible;
			int		isMinimized;

			int		ixPosition;
			int		iyPosition;
			int		iHeight;
		} TWindowStatus;

		typedef struct SConfig
		{
			DWORD			width;
			DWORD			height;
			DWORD			bpp;
			DWORD			frequency;

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
			int iShadowQualityLevel;
			int iShadowTargetLevel;
			int iShadowDistanceBehind;
			int iTerrainShadow;
			int iTreeShadow;
			int iCollisionLevel;
			int iPerformanceMode;
			int iRainLevel;
			int iDynamicLightLevel;
			int iSpecularLevel;
			int iBloodLevel;
			int iAmbianceEffectsLevel;
			int iShadowTypeLevel;
			int iVisibleLobbyLevel;
#endif

			bool			is_software_cursor;
			bool			is_object_culling;
			int				iDistance;
			int				iShadowLevel;

			FLOAT			music_volume;
			BYTE			voice_volume;
			FLOAT			effects_volume;

			int				gamma;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			float			shopnames_range;
#endif
			int				isSaveID;
			char			SaveID[20];

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			BYTE			bSoftwareTiling;
			bool			bViewChat;
			bool			bAlwaysShowName;
			bool			bShowDamage;
			bool			bShowSalesText;
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
			bool bShowNightMode;
			bool bShowSnowMode;
			bool bShowSnowTextureMode;
#endif
#if defined(ENABLE_FOV_OPTION)
			float fFOV;
#endif
#ifdef JTX_DISTANCE_SETTINGS
			int fShadowIntensity;
			float fLeaves;
			float fSHOP;
			float fSHOPTEXT;
			float fEFFECT;
			float fMOB;
			float fSTONE;
			float fMOUNT;
			float fPET;
			float fNPC;
			float fTEXT;
			float fStoneScale;
			float fBossScale;
			float fMountScale;
#endif

			int pFlag;
			int pInfo[5];
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
			bool			bShowMobAIFlag;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
			bool			bShowMobLevel;
#endif
		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int iIndex);

		DWORD							GetWidth();
		DWORD							GetHeight();
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		void							SetShopNamesRange(float fRange) { m_Config.shopnames_range = fRange; }
		float							GetShopNamesRange() { return m_Config.shopnames_range; }
#endif
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
		void SetNightModeOption(int iFlag);
		bool GetNightModeOption();
		void SetSnowModeOption(int iFlag);
		bool GetSnowModeOption();
		void SetSnowTextureModeOption(int iFlag);
		bool GetSnowTextureModeOption();
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		bool							IsShowMobAIFlag();
		void							SetShowMobAIFlagFlag(int iFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		bool							IsShowMobLevel();
		void							SetShowMobLevelFlag(int iFlag);
#endif

		// Window
		void							SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight);

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		int								GetSoundVolume();
		int								GetEffectsVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);
		void							SetEffectsVolumef(float fVolume);

		int								GetDistance();
		int								GetShadowLevel();
		void							SetShadowLevel(unsigned int level);

#if defined(JTX_ENABLE_GRAPHICS_OPTIONS)
		int GetShadowQualityLevel();
		void SetShadowQualityLevel(unsigned int level);

		int GetVisibleLobbyLevel();
		void SetVisibleLobbyLevel(unsigned int level);

		int GetShadowTargetLevel();
		void SetShadowTargetLevel(unsigned int level);

		int GetShadowDistanceBehind();
		void SetShadowDistanceBehind(unsigned int level);

		int GetTerrainShadow();
		void SetTerrainShadow(unsigned int level);

		int GetTreeShadow();
		void SetTreeShadow(unsigned int level);

		int GetPerformanceMode();
		void SetPerformanceMode(unsigned int level);

		int GetRainLevel();
		void SetRainLevel(unsigned int level);

		int GetCollisionLevel();
		void SetCollisionLevel(unsigned int level);

		int GetDynamicLightLevel();
		void SetDynamicLightLevel(unsigned int level);

		int GetSpecularLevel();
		void SetSpecularLevel(unsigned int level);

		int GetBloodLevel();
		void SetBloodLevel(unsigned int level);

		int GetAmbianceEffectsLevel();
		void SetAmbianceEffectsLevel(unsigned int level);

		int GetShadowTypeLevel();
		void SetShadowTypeLevel(unsigned int level);
#endif
#if defined(ENABLE_FOV_OPTION)
		// FoV
		float GetFOV();
		void SetFOV(const float c_fValue);
#endif
#ifdef JTX_DISTANCE_SETTINGS
		// SHOP
		float GetLEAVES();
		void SetLEAVES(const float c_fValue);
		// Shadow Intensity
		float GetShadowIntensity();
		void SetShadowIntensity(const float c_fValue);
		// SHOP
		float GetSHOP();
		void SetSHOP(const float c_fValue);
		// SHOP
		float GetSHOPTEXT();
		void SetSHOPTEXT(const float c_fValue);
		// MOB
		float GetMOB();
		void SetMOB(const float c_fValue);
		// STONE
		float GetSTONE();
		void SetSTONE(const float c_fValue);
		// PET
		float GetPET();
		void SetPET(const float c_fValue);
		// MOUNT
		float GetMOUNT();
		void SetMOUNT(const float c_fValue);
		// NPC
		float GetNPC();
		void SetNPC(const float c_fValue);
		// TEXT
		float GetTEXT();
		void SetTEXT(const float c_fValue);
		// Effect
		float GetEFFECT();
		void SetEFFECT(const float c_fValue);

		// Stone Scale
		float GetStoneScale();
		void SetStoneScale(const float c_fValue);

		// Boss Scale
		float GetBossScale();
		void SetBossScale(const float c_fValue);

		// Mount Scale
		float GetMountScale();
		void SetMountScale(const float c_fValue);
#endif
		int GetPlayerFlag();
		int GetPlayerInfo(int type);
		void SetPlayerFlag(int flag);
		void SetPlayerInfo(int type, int value);

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
