﻿#include"InputComponent.h"

//コンストラクター：オーナーの設定・ボタンの初期化
InputComponent::InputComponent(GameObject& owner):m_owner(owner)
{
	//操作軸の初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}
	

	m_buttons.fill(FREE);
}

void InputComponent::PushButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BTN_MAX);
	//押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	//ENTER反転とAND=ENTERPRISEだけをOFF
		
	}
	//押していない
	else
	{
		m_buttons[no] |= ENTER;		//ENTERをOR=ENTERをON
		m_buttons[no] |= STAY;		//STAYをOR=STAYをON
	}

}
void InputComponent::ReleaseButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BTN_MAX);
	//押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	//ENTER反転とAND=ENTERだけをOFF
		m_buttons[no] &= ~STAY;		//STAY反転とAND=STAYだけをOFF
		m_buttons[no] |= EXIT;		//EXITをOR=EXITをON

	}
	//押していない
	else
	{
		m_buttons[no] &= ~EXIT;		//EXIT反転とAND=EXITだけをOFF

	}

}

void PlayerInputComponent::Update()
{
	//操作軸の初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}
	
	//[左の軸値]入力処理
	if (GetAsyncKeyState(VK_UP) & 0x8000) { m_axes[Input::Axes::L].y = 1.0f; }
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) { m_axes[Input::Axes::L].y = -1.0f; }
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { m_axes[Input::Axes::L].x = 1.0f; }
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) { m_axes[Input::Axes::L].x = -1.0f; }
	//[右の軸値]入力処理
	if (GetAsyncKeyState('W') & 0x8000) { m_axes[Input::Axes::R].y = 1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { m_axes[Input::Axes::R].x = -1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { m_axes[Input::Axes::R].y = -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { m_axes[Input::Axes::R].x = 1.0f; }

	//ボタン入力処理
	if (GetAsyncKeyState('Z')) { PushButton(Input::Buttons::A); }
	else { ReleaseButton(Input::Buttons::A); }

	if (GetAsyncKeyState('X')) { PushButton(Input::Buttons::B); }
	else { ReleaseButton(Input::Buttons::B); }

	if (GetAsyncKeyState('C')) { PushButton(Input::Buttons::X); }
	else { ReleaseButton(Input::Buttons::X); }

	if (GetAsyncKeyState('V')) { PushButton(Input::Buttons::Y); }
	else { ReleaseButton(Input::Buttons::Y); }

	if (GetAsyncKeyState('O')) { PushButton(Input::Buttons::L1); }
	else { ReleaseButton(Input::Buttons::L1); }

	if (GetAsyncKeyState('E')) { PushButton(Input::Buttons::R1); }
	else { ReleaseButton(Input::Buttons::R1); }

}

void EnemyInputComponent::Update()
{
	m_axes[Input::Axes::L].y = 1.0f;
	m_axes[Input::Axes::R].y = 1.0f;
}

void ActionPlayerInputComponent::Update()
{
	//角度の初期化
	for (auto& rAxis : m_axes)
	{
		rAxis = { 0.0f,0.0f };
	}
	//[左の軸値]入力処理
	if (GetAsyncKeyState('W') & 0x8000) { m_axes[Input::Axes::L].y = 1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { m_axes[Input::Axes::L].x = -1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { m_axes[Input::Axes::L].y = -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { m_axes[Input::Axes::L].x = 1.0f; }

	//[右の軸値](マウス)入力処理
	POINT nowMousePos;
	GetCursorPos(&nowMousePos);//マウスの現在位置の取得
	m_axes[Input::R].x=(float)(nowMousePos.x - m_prevMousePos.x);
	m_axes[Input::R].y=(float)(nowMousePos.y - m_prevMousePos.y);
	m_prevMousePos = nowMousePos;//今回のマウス位置を覚える

	

	//ボタン入力
	//ジャンプ処理
	if (GetAsyncKeyState('E')) { PushButton(Input::Buttons::A); }
	else { ReleaseButton(Input::Buttons::A); }
}
