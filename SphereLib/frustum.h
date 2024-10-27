/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */
#pragma once

 /***********************************************************************/
 /** FRUSTUM.H   : Represents a clipping frustum.                       */
 /**               You should replace this with your own more robust    */
 /**               view frustum clipper.                                */
 /**                                                                    */
 /**               Written by John W. Ratcliff jratcliff@att.net        */
 /***********************************************************************/

#include "vector.h"
#include "../EterBase/Singleton.h"

enum ViewState
{
	VS_INSIDE,   // completely inside the frustum.
	VS_PARTIAL,  // partially inside and partially outside the frustum.
	VS_OUTSIDE   // completely outside the frustum
};

class Frustum : public CSingleton<Frustum>
{
public:
	void BuildViewFrustum(D3DXMATRIX& mat);
	void BuildViewFrustum2(D3DXMATRIX& mat, float fNear, float fFar, float fFov, float fAspect, const D3DXVECTOR3& vCamera, const D3DXVECTOR3& vLook);
	ViewState ViewVolumeTest(const Vector3d& c_v3Center, const float c_fRadius) const;
	ViewState ViewVolumeTestEffects(const Vector3d& c_v3Center, const float c_fRadius) const;
	float increaseShadowRangeFrustum = 1.0f;
	float treeLeaves = 1.0f;
	int terrainShadow;
	int  treeShadow;
	bool OnlyTreeShadow;
	int performanceMode;
	int TreeLight;
	bool isIndoor = false;
	bool renderGrassShadow = false;
	bool refreshEffect = true;
	int dynamicLight;
	int realtimeSpecular;
	int bloodLevel;
	int ambianceEffectsLevel;
	bool updateSpecular = false;
	int refreshshadowLevel = 1;
	//bool nightEnviromnent = false;
	bool isWarp = false;
	int	 chunkLoadCounter = 0;
	bool isLobbyMap = false;
	std::string				currentMapNameSave;
	bool checkWikiAlreadyLoaded = false;
	D3DXMATRIX matLightView;
	D3DXMATRIX matLightProj;
	bool isSnowing = false;
	D3DXVECTOR3 characterPosition;
	int selectRainFrustun;
	bool isThunderTime = false;
	bool isThunderTimeTerrain = false;
	bool isCerApus4Environment = false;
	bool isCerApus4EnvironmentAzrael = false;
	float specularRainPower = 15.0f;
	float specularTreePower = 3.5f;
	float specularTerrainPower = 3.5f;
	float shinessTerrain = 0.0f;
	float getLeaves;
	bool renderOtherWater = false;
	int shadowType;
	bool updatePostProcessing = false;
	bool isWikiOpened = false;
	bool isChangingSky = false;
	D3DXVECTOR3 previousSunPosition;
	float auraEnabled = 0.0f;
	D3DXVECTOR3 weaponPosition;
	float pointLightIntensity = 1.0f;
	int alwaysHiddenLobby;
	int rainEffect = 0;

	LPDIRECT3DTEXTURE9 shadowTexture = nullptr;

	int funcCallCounter;

private:
	bool m_bUsingSphere;
	D3DXVECTOR3 m_v3Center;
	float m_fRadius;
	D3DXPLANE m_plane[6];
};

//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
