#include"EditorCamera.h"
#include"../main.h"

EditorCamera::EditorCamera() 
{
	m_mCam = Math::Matrix::CreateTranslation(0.0f, 0.0f, -5.0f);
	m_mCam *= Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(20));
	m_mCam *= Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(20));

	//射影行列　　　　　　　　　　　　　　　　　　　　　　視野角
	m_mProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60),
		//　　　　　　　　　アスペクト比　最近接距離　再速報距離
		D3D.GetBackBuffer()->GetAspectRatio(), 0.01f, 5000.0f);

}
EditorCamera::~EditorCamera()
{
}
void EditorCamera::Update() 
{
	POINT nowPos;
	GetCursorPos(&nowPos);

	float ratio = 1.0f;
	//shiftで大きく
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { ratio = 5.f; }
	//controlで小さく
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) { ratio = 0.2f; }

	//マウスの右ボタン
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		//マウスの前回からの移動量
		float deltaX = (float)(nowPos.x - m_prevMousePos.x) * 0.5f * ratio;
		float deltaY = (float)(nowPos.y - m_prevMousePos.y) * 0.5f * ratio;
	
		//カメラY軸回転
		Math::Matrix mRotateY;
		mRotateY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(deltaX));
		m_mCam *= mRotateY;

		//カメラX軸回転
		Math::Matrix mRotateX;
		mRotateX = DirectX::XMMatrixRotationAxis(m_mCam.Right(), DirectX::XMConvertToRadians(deltaY));
		m_mCam *= mRotateX;

	}

	//ホイールドラッグ
	if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) 
	{
		//マウスの前回からの移動量
		float deltaX = (float)(nowPos.x - m_prevMousePos.x) * 0.05f * ratio;
		float deltaY = (float)(nowPos.y - m_prevMousePos.y) * 0.05f * ratio;

		m_viewPos -= m_mCam.Right() * deltaX;
		m_viewPos += m_mCam.Up() * deltaY;

	}

	//ホイール回転
	if (APP.m_window.GetMouseWheelVal())
	{
		float wheelVal = (float)APP.m_window.GetMouseWheelVal() * 0.01 * ratio;
		Math::Matrix mDistance;//                 X    Y      Z
		mDistance = DirectX::XMMatrixTranslation(0.f, 0.f, wheelVal);

		m_mCam = mDistance* m_mCam;
	}

	m_prevMousePos = nowPos;

}

void EditorCamera::SetToShader()
{
	//カメラ行列の作成
	Math::Matrix mCam = m_mCam;

	//view=カメラが見ている場所(視点)/mCam=viewからどれくらい離れているか
	//mCam*view=カメラの場所、これをシェーダにセット
	//カメラ行列に注視点の座標を合成する(注視点と注視点からの角度や距離)
	mCam *= DirectX::XMMatrixTranslation(m_viewPos.x, m_viewPos.y, m_viewPos.z);

	//カメラの座標をシェーダにセット
	SHADER.m_cb7_Camera.Work().CamPos = mCam.Translation();

	//カメラのビュー行列をシェーダにセット
	SHADER.m_cb7_Camera.Work().mV = mCam.Invert();

	//カメラの射影行列をセット
	SHADER.m_cb7_Camera.Work().mP = m_mProj;

	//カメラ情報(ビュー行列、射影行列を描くシェーダの定数バッファへセット)
	SHADER.m_cb7_Camera.Write();
}