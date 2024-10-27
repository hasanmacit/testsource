﻿#pragma once

#ifdef ENABLE_RENDER_TARGET
#include <memory>
#include "GrpRenderTargetTexture.h"

class CInstanceBase;
class CGraphicImageInstance;

class CRenderTarget
{
public:
	CRenderTarget(DWORD width, DWORD height);
	~CRenderTarget();

	void SetVisibility(bool isShow){ m_visible = isShow; }
	void SetRotation(bool bRot) { m_pmodelRotationActive = bRot; }
	bool IsReady() { return (m_visible && m_pModel); }
	std::unique_ptr<CInstanceBase>& GetModel() { return m_pModel; }

	void RotateEyeAroundTarget(float fPitchDegree, float fRollDegree);
	void Zoom(float fRatio);

	void SelectModel(int index, bool isFromScale= false);
	void ResetModel();

	void UpdateModel();
	void DeformModel();
	void RenderModel();
	void SetRenderingRect(RECT* rect) const;
	void SetRenderingBox(RECT* rect) const;

	void CreateTextures() const;
	void ReleaseTextures() const;
	void RenderTexture() const;

	bool CreateBackground(const char* imgPath, DWORD width, DWORD height);
	void RenderBackground() const;

	void SetModelV3Eye(float x, float y, float z);
	void SetModelV3Target(float x, float y, float z);

	void SetModelStatus(bool bStatus) { m_bShowModel = bStatus; }

	float GetScale() { return m_bScale; }
	void SetScale(float fScale);

	bool IsShow() { return m_visible; }

	bool GetMotionStatus() { return m_bMotion; }
	void SetMotionStatus(bool bFlag);

	bool GetEffectStatus() { return m_bEffectStatus; }
	void SetEffectStatus(bool bFlag) { m_bEffectStatus = bFlag; }


private:
	std::unique_ptr<CInstanceBase> m_pModel;
	std::unique_ptr<CGraphicImageInstance> m_background;
	std::unique_ptr<CGraphicRenderTargetTexture> m_renderTargetTexture;
	float m_modelRotation;
	bool m_pmodelRotationActive;
	bool m_visible;
	bool m_bShowModel;
	float m_bScale;
	bool m_bEffectStatus;
	bool m_bMotion;
	D3DXVECTOR3 m_v3AngularVelocity;
	D3DXVECTOR3 m_v3Eye;
	D3DXVECTOR3 m_v3Target;
	D3DXVECTOR3 m_v3Up;
};
#endif

