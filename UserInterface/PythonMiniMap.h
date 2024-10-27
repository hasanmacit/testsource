#pragma once

#include "PythonBackground.h"
#if defined(__BL_PARTY_POSITION__)
#include "Packet.h"
#include <unordered_map>
#endif

class CPythonMiniMap : public CScreen, public CSingleton<CPythonMiniMap>
{
	public:
		enum
		{
			EMPIRE_NUM = 4,

			MINI_WAYPOINT_IMAGE_COUNT = 12,
			WAYPOINT_IMAGE_COUNT = 15,
			TARGET_MARK_IMAGE_COUNT = 2,
		};
		enum
		{
			TYPE_OPC,
			TYPE_OPCPVP,
			TYPE_OPCPVPSELF,
			TYPE_NPC,
			TYPE_MONSTER,
			TYPE_WARP,
			TYPE_WAYPOINT,
			TYPE_PARTY,
			TYPE_EMPIRE,
			TYPE_EMPIRE_END = TYPE_EMPIRE + EMPIRE_NUM,
			TYPE_TARGET,
			TYPE_COUNT,
		};

	public:
		CPythonMiniMap();
		virtual ~CPythonMiniMap();

		void Destroy();
		bool Create();

		bool IsAtlas();
		bool CanShow();
		bool CanShowAtlas();

		void SetMiniMapSize(float fWidth, float fHeight);
		void SetScale(float fScale);
		void ScaleUp();
		void ScaleDown();

		void SetCenterPosition(float fCenterX, float fCenterY);

		void Update(float fCenterX, float fCenterY);
		void Render(float fScreenX, float fScreenY);

		void Show();
		void Hide();

		bool GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor);

		// Atlas
		bool LoadAtlas();
		void UpdateAtlas();
		void RenderAtlas(float fScreenX, float fScreenY);
		void ShowAtlas();
		void HideAtlas();

#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		bool GetAtlasInfo(float fScreenX, float fScreenY, std::string& rReturnString, float* pReturnPosX, float* pReturnPosY, DWORD* pdwTextColor, DWORD* pdwGuildID, DWORD* time, bool* isBoss);
#else
		bool GetAtlasInfo(float fScreenX, float fScreenY, std::string& rReturnString, float* pReturnPosX, float* pReturnPosY, DWORD* pdwTextColor, DWORD* pdwGuildID);
#endif

		bool GetAtlasSize(float * pfSizeX, float * pfSizeY);

		void AddObserver(DWORD dwVID, float fSrcX, float fSrcY);
		void MoveObserver(DWORD dwVID, float fDstX, float fDstY);
		void RemoveObserver(DWORD dwVID);

		// WayPoint
		void AddWayPoint(BYTE byType, DWORD dwID, float fX, float fY, std::string strText, DWORD dwChrVID=0);
		void RemoveWayPoint(DWORD dwID);

		// SignalPoint
		void AddSignalPoint(float fX, float fY);
		void ClearAllSignalPoint();

		void RegisterAtlasWindow(PyObject* poHandler);
		void UnregisterAtlasWindow();
		void OpenAtlasWindow();
		void SetAtlasCenterPosition(int x, int y);

		// NPC List
		void ClearAtlasMarkInfo();
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		void RegisterAtlasMark(BYTE byType, const char* c_szName, long lx, long ly, DWORD mobVnum, DWORD time);
#else
		void RegisterAtlasMark(BYTE byType, const char* c_szName, long lx, long ly);
#endif

		// Guild
		void ClearGuildArea();
		void RegisterGuildArea(DWORD dwID, DWORD dwGuildID, long x, long y, long width, long height);
		DWORD GetGuildAreaID(DWORD x, DWORD y);

		// Target
		void CreateTarget(int iID, const char * c_szName);
		void CreateTarget(int iID, const char * c_szName, DWORD dwVID);
		void UpdateTarget(int iID, int ix, int iy);
		void DeleteTarget(int iID);
#if defined(__BL_PARTY_POSITION__)
		void RemovePartyPositionInfo(DWORD dwPID);
		void AddPartyPositionInfo(const TPartyPosition& Info);
#endif


	protected:
		void __Initialize();
		void __SetPosition();
		void __LoadAtlasMarkInfo();

		void __RenderWayPointMark(int ixCenter, int iyCenter);
		void __RenderMiniWayPointMark(int ixCenter, int iyCenter);
		void __RenderTargetMark(int ixCenter, int iyCenter);

		void __GlobalPositionToAtlasPosition(long lx, long ly, float * pfx, float * pfy);

	protected:
		// Atlas
		typedef struct SAtlasMarkInfo
		{
			BYTE m_byType;
			DWORD m_dwID; // For WayPoint
			float m_fX;
			float m_fY;
			float m_fScreenX;
			float m_fScreenY;
			float m_fMiniMapX;
			float m_fMiniMapY;
			DWORD m_dwChrVID;
			std::string m_strText;
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
			DWORD time;
#endif
		} TAtlasMarkInfo;

		// GuildArea
		typedef struct SGuildAreaInfo
		{
			DWORD dwGuildID;
			long lx, ly;
			long lwidth, lheight;

			float fsxRender, fsyRender;
			float fexRender, feyRender;
		} TGuildAreaInfo;

		struct SObserver
		{
			float fCurX;
			float fCurY;
			float fSrcX;
			float fSrcY;
			float fDstX;
			float fDstY;

			DWORD dwSrcTime;
			DWORD dwDstTime;
		};

		typedef struct SMarkPosition
		{
			float	m_fX;
			float	m_fY;
			UINT	m_eNameColor;
		} TMarkPosition;

#if defined(__BL_PARTY_POSITION__)
		struct SPartyPlayerPosition
		{
			~SPartyPlayerPosition()
			{
				grMarkImage.Destroy();
			}
			std::string sName;
			long	lX, lY;
			float	fScreenX, fScreenY;
			float	fRot;
			CGraphicExpandedImageInstance grMarkImage;
		};
#endif


		typedef std::vector<TMarkPosition>				TInstanceMarkPositionVector;
		typedef TInstanceMarkPositionVector::iterator	TInstancePositionVectorIterator;

	protected:
		bool __GetWayPoint(DWORD dwID, TAtlasMarkInfo ** ppkInfo);
		void __UpdateWayPoint(TAtlasMarkInfo * pkInfo, int ix, int iy);

	protected:
		float							m_fWidth;
		float							m_fHeight;

		float							m_fScale;

		float							m_fCenterX;
		float							m_fCenterY;

		float							m_fCenterCellX;
		float							m_fCenterCellY;

		float							m_fScreenX;
		float							m_fScreenY;

		float							m_fMiniMapRadius;

		LPDIRECT3DTEXTURE8				m_lpMiniMapTexture[AROUND_AREA_NUM];

		CGraphicImageInstance			m_MiniMapFilterGraphicImageInstance;
		CGraphicExpandedImageInstance	m_MiniMapCameraraphicImageInstance;

		CGraphicExpandedImageInstance	m_PlayerMark;
		CGraphicImageInstance			m_WhiteMark;
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		CGraphicImageInstance			m_BossMark;
		CGraphicImageInstance			m_DungeonMark;
#endif

		TInstanceMarkPositionVector		m_PartyPCPositionVector;
		TInstanceMarkPositionVector		m_OtherPCPositionVector;
		TInstanceMarkPositionVector		m_NPCPositionVector;
		TInstanceMarkPositionVector		m_MetinPositionVector;
		TInstanceMarkPositionVector		m_MonsterPositionVector;
		TInstanceMarkPositionVector		m_WarpPositionVector;
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		TInstanceMarkPositionVector		m_BossPositionVector;
		TInstanceMarkPositionVector		m_DungeonPositionVector;
#endif
		std::map<DWORD, SObserver>		m_kMap_dwVID_kObserver;

		bool							m_bAtlas;
		bool							m_bShow;

		CGraphicVertexBuffer			m_VertexBuffer;
		CGraphicIndexBuffer				m_IndexBuffer;

		D3DXMATRIX						m_matIdentity;
		D3DXMATRIX						m_matWorld;
		D3DXMATRIX						m_matMiniMapCover;

		bool							m_bShowAtlas;
		CGraphicImageInstance			m_AtlasImageInstance;
		D3DXMATRIX						m_matWorldAtlas;
		CGraphicExpandedImageInstance	m_AtlasPlayerMark;
#if defined(__BL_PARTY_POSITION__)
		std::unordered_map<DWORD, std::shared_ptr<SPartyPlayerPosition>> mAtlasPartyPlayerMark;
		DWORD m_dwLastPartyMarkUpdate;
#endif

		float							m_fAtlasScreenX;
		float							m_fAtlasScreenY;

		DWORD							m_dwAtlasBaseX;
		DWORD							m_dwAtlasBaseY;

		float							m_fAtlasMaxX;
		float							m_fAtlasMaxY;

		float							m_fAtlasImageSizeX;
		float							m_fAtlasImageSizeY;

		typedef std::vector<TAtlasMarkInfo>		TAtlasMarkInfoVector;
		typedef TAtlasMarkInfoVector::iterator	TAtlasMarkInfoVectorIterator;
		typedef std::vector<TGuildAreaInfo>		TGuildAreaInfoVector;
		typedef TGuildAreaInfoVector::iterator	TGuildAreaInfoVectorIterator;
		TAtlasMarkInfoVectorIterator			m_AtlasMarkInfoVectorIterator;
		TAtlasMarkInfoVector					m_AtlasNPCInfoVector;
		TAtlasMarkInfoVector					m_AtlasWarpInfoVector;
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		TAtlasMarkInfoVector					m_AtlasDungeonInfoVector;
		TAtlasMarkInfoVector					m_AtlasBossInfoVector;
#endif
		// WayPoint
		CGraphicExpandedImageInstance			m_MiniWayPointGraphicImageInstances[MINI_WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_WayPointGraphicImageInstances[WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_TargetMarkGraphicImageInstances[TARGET_MARK_IMAGE_COUNT];
		CGraphicImageInstance					m_GuildAreaFlagImageInstance;
		TAtlasMarkInfoVector					m_AtlasWayPointInfoVector;
		TGuildAreaInfoVector					m_GuildAreaInfoVector;

		// SignalPoint
		struct TSignalPoint
		{
			D3DXVECTOR2 v2Pos;
			unsigned int id;
		};
		std::vector<TSignalPoint>				m_SignalPointVector;

		PyObject*							m_poHandler;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
