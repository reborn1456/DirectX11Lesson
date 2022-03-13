#include"ShootingGameProcess.h"
#include"../Scene.h"

#include"./Application/Game/AnimationCamera.h"

void ShootinGameProcess::Update()
{
	
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		if (KeyFlg) {
			Scene::GetInstance().RequestChangeScene("Data/Scene/ActionGame.json");
			KeyFlg = false;
		}
	}
	else
	{
		KeyFlg = true;
	}

	if (GetAsyncKeyState('C') & 0x8000)
	{
		ChangeCamTarget();
	}
}

void ShootinGameProcess::ChangeCamTarget()
{
	//現在アニメーション中か調べる
	auto animCam = Scene::GetInstance().FindObjectWithName("AnimationCamera");
	if (animCam) { return; }

	//今のカメラのターゲットになっているキャラクターを探す
	auto now = Scene::GetInstance().FindObjectWithName(m_strCameraTarget);
	if (!now) { return; }

	//新しいカメラターゲットの名前を設定
	if (m_strCameraTarget == "Player")
	{
		m_strCameraTarget = "Enemy00";
	}
	else
	{
		m_strCameraTarget = "Player";
	}

	//新しいターゲットのキャラクターを検索して探す
	auto target = Scene::GetInstance().FindObjectWithName(m_strCameraTarget);
	if (!target) { return; }

	//補完用のキャラクターを作成する
	auto newAimCam = std::make_shared<AnmationCamera>();
	newAimCam->Set(now, target);//情報の設定

	//補完用キャラクターをカメラのターゲットにする
	Scene::GetInstance().SetTargetCamera(newAimCam->GetCameraComponent());
	Scene::GetInstance().AddObject(newAimCam);

}
