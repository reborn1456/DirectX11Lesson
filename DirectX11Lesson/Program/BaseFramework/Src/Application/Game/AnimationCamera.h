#pragma once
#include"GameObject.h"

class AnmationCamera :public GameObject
{
public:
	AnmationCamera() { m_name = "AnimationCamera"; }
	
	//二点間の保管を行う
	void Update()override;

	//情報の設定
	void Set(const std::shared_ptr<GameObject>& start, const std::shared_ptr<GameObject>& end, float spd = 0.01f)
	{
		//カメラコンポーネントが無かったら帰る
		if (!start->GetCameraComponent() || !end->GetCameraComponent()) { return; }

		m_progress = 0.0f;
		m_wpStart = start;
		m_wpEnd = end;
		m_speed = spd;
	}

private:
	//スタート地点のキャラクター
	std::weak_ptr < GameObject> m_wpStart;
	
	//ゴール地点のキャラクター
	std::weak_ptr < GameObject> m_wpEnd;
	
	//進行具合(0～1)
	float m_progress = 0.0f;
	
	//進行スピード
	float m_speed = 0.01f;
};