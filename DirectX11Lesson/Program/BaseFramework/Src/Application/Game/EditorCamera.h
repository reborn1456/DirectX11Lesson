﻿#pragma once
class EditorCamera {
public:
	EditorCamera();
	~EditorCamera();

	void Update();
	void SetToShader();//ビュー行列・射影行列をシェーダーへセット

private:

	Math::Vector3 m_viewPos;//注視点カメラが見てる座標
	Math::Matrix  m_mCam;   //注視点からの行列
	Math::Matrix  m_mProj;  //射影行列

	POINT         m_prevMousePos = { 0,0 };//前フレームのマウス座標

};