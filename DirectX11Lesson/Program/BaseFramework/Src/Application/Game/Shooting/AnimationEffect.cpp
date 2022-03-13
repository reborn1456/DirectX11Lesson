﻿#include"AnimationEffect.h"

AnimationEffect::AnimationEffect(){}
AnimationEffect::~AnimationEffect(){}

void AnimationEffect::SetAnimationInfo(const std::shared_ptr<KdTexture>& rTex, float size, int spliteX, int spliteY, float angle)
{
	//ポリゴンの大きさと色を設定
	m_poly.Init(size, size, { 1,1,1,1 });
	
	//アニメーションの分割を設定
	m_poly.SetAnimationInfo(spliteX, spliteY);
	
	//アニメーションの位置を0コマ目にしておく
	m_poly.SetAnimationPos(0);
	
	//渡されたテクスチャを設定する
	m_poly.SetTexture(rTex);

	//Z軸の回転角度を覚える
	m_angleZ = angle;
}

void AnimationEffect::Update()
{
	//アニメーション進行
	m_poly.Animation(0.5f, false);

	//アニメ終了
	if (m_poly.IsAnimationEnd())
	{
		Destroy();
	}
}

void AnimationEffect::DrawEffect()
{
	//各方向の拡大率を取得
	float scaleX = m_mWorld.GetAxisX().Length();
	float scaleY = m_mWorld.GetAxisY().Length();
	float scaleZ = m_mWorld.GetAxisZ().Length();

	//ビルボード処理

	KdMatrix drawMat;								//拡大率を設定
	drawMat.CreateScalling(scaleX, scaleY, scaleZ);	//Z軸の回転角度を加える
	drawMat.RotateZ(m_angleZ * KdToRadians);

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====


	//座標は自分のものを使う
	drawMat.SetTranslation(m_mWorld.GetTranslation());

	drawMat.SetBillboard(SHADER.m_cb7_Camera.GetWork().mV);

	////カメラ行列の取得
	//KdMatrix camMat = SHADER.m_cb7_Camera.GetWork().mV;
	//camMat.Inverse();//カメラの逆行列を計算

	//////カメラの逆行列を描画する行列と合成
	//drawMat *= camMat;


	//////座標は自分のものを使う
	//drawMat.SetTranslation(m_mWorld.GetTranslation());

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

	
	//描画
	SHADER.m_effectShader.SetWorldMatrix(drawMat);
	SHADER.m_effectShader.WriteToCB();
	m_poly.Draw(0);
}

