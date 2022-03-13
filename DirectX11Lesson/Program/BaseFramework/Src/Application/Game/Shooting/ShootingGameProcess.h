#pragma once

#include"../GameProcess.h"

//シューティングゲームシーン管理と画面遷移を担当するクラス

//GameObject->GameProcess->ShootingGameProcess
class ShootinGameProcess :public GameProcess
{
public:
	ShootinGameProcess(){}
	virtual ~ShootinGameProcess(){}

	void Update() override;

private:
	//カメラターゲットの変更
	void ChangeCamTarget();

	//カメラのターゲットになっているキャラクターの名前
	std::string m_strCameraTarget = "Player";
};