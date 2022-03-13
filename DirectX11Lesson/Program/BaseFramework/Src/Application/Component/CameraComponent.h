#pragma once
#include"../Game/GameObject.h"

//=========================================
//カメラコンポーネントクラス
//=========================================

class CameraComponent
{
public:
	//コンストラクター　オーナーの設定と射影行列の作成
	CameraComponent(GameObject& owner);
	
	~CameraComponent();

	//オフセット行列取得
	inline KdMatrix& OffsetMatrix() { return m_mOffset; }
	
	//カメラ行列取得
	inline const KdMatrix& GetCameraMatrix() { return m_mCam; }
	
	//ビュー行列取得
	inline const KdMatrix& GetViewMatrix() { return m_mView; }

	//カメラ行列・ビュー行列設定(行列ｍと行列Offsetが合成され、最終的なカメラ行列になる)
	void SetCameraMatrix(const KdMatrix& m);

	//カメラ情報(ビュー・射影行列など)をシェーダーへセット
	void SetToShader();

protected:

	//オフセット行列	
	KdMatrix    m_mOffset;         //追従カメラの相対座標行列
	//カメラ行列	
	KdMatrix    m_mCam;          
	//ビュー行列	
	KdMatrix    m_mView;          
	//射影行列
	KdMatrix    m_mProj;           //追従カメラの射影行列
	
	GameObject& m_owner;

};