#pragma once

#include"GameObject.h"

//ゲームシーンの管理者をカテゴリ分けするために、GameObjectを継承させる
class GameProcess :public GameObject
{
public:
	GameProcess(){}
	virtual ~GameProcess(){}

protected:
	bool KeyFlg = false;
};