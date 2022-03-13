#include "AnimationCamera.h"

#include"./Scene.h"
#include"../Component/CameraComponent.h"

void AnmationCamera::Update()
{
	//補完する対象がどちらもあるか
	auto start = m_wpStart.lock();
	auto end = m_wpEnd.lock();
	if (!start || !end)
	{
		//補完する必要なし
		m_alive = false;//自身も破棄
		return;
	}

	//補完する行列の取得
	auto& mStart = start->GetCameraComponent()->GetCameraMatrix();
	auto& mEnd = end->GetCameraComponent()->GetCameraMatrix();

	//座標の補完
	const KdVec3& vStart = mStart.GetTranslation();
	const KdVec3& vEnd = mEnd.GetTranslation();
	KdVec3 vTo = vEnd - vStart;	//ゴール地点へのベクトル
	KdVec3 vNow = vStart + vTo * m_progress;	//進行具合を噛みして座標を求める

	//開始地点と終了地点のクォータニオンを生成(行列->回転)
	DirectX::XMVECTOR pSt = DirectX::XMQuaternionRotationMatrix(mStart);
	DirectX::XMVECTOR pEd = DirectX::XMQuaternionRotationMatrix(mEnd);

	//クォータニオンを使って回転の補完 (変数名...途中)
	DirectX::XMVECTOR qOTW = DirectX::XMQuaternionSlerp(pSt, pEd, m_progress);

	//クォータニオンを回転行列に変換(回転->行列)
	KdMatrix mRot = DirectX::XMMatrixRotationQuaternion(qOTW);

	//カメラに設定する行列
	KdMatrix mCam = mRot;

	//クォータニオンは回転しか管理しないので座標も含める
	mCam.SetTranslation(vNow);

	//カメラへセット
	m_spCameraComponent->SetCameraMatrix(mCam);

	//進行具合の更新
	m_progress += m_speed;
	if (m_progress > 1.0f)
	{
		m_progress = 1.0f;
		m_alive = false;	//自身を破棄

		//ゴール地点まで補完し終わったので、カメラターゲットをゴール地点のキャラクターにする
		Scene::GetInstance().SetTargetCamera(end->GetCameraComponent());
	}

}
