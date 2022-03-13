#include "../Scene.h"
#include "Human.h"

#include "../../Component/CameraComponent.h"
#include "../../Component/InputComponent.h"


const float Human::s_allowToStepHeight = 0.8f;
const float Human::s_landingHeight = 0.1f;

void Human::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);//共通部分のDeserialize

	//カメラコンポーネントの設定
	if (m_spCameraComponent)
	{
		m_spCameraComponent->OffsetMatrix().CreateTranslation(0.0f, 1.5f, -5.0f);
		m_spCameraComponent->OffsetMatrix().RotateX(25.0f * KdToRadians);
	}

	//プレイヤーであればInputComponentを作成
	if ((GetTag() & TAG_Player) != 0)
	{
		Scene::GetInstance().SetTargetCamera(m_spCameraComponent);
		m_spInputComponent = std::make_shared< ActionPlayerInputComponent>(*this);
	}
}

void Human::Update()
{
	//InputCompの更新
	if (m_spInputComponent) 
	{
		m_spInputComponent->Update();
	}

	//移動前の座標を覚える
	m_prevPos = m_pos;

	UpdateCamera();

	//入力による移動の更新
	UpdateMove();

	//重力をキャラクターのYの移動方向に加える
	m_force.y -= m_gravity;

	//移動力をキャラクターの座標に足しこむ
	m_pos.x += m_force.x;
	m_pos.y += m_force.y;
	m_pos.z += m_force.z;

	//座標の更新を行った後に当たり判定
	UpdateCollision();

	//ワールド行列を合成する
	m_mWorld.CreateRotationX(m_rot.x);
	m_mWorld.RotateY(m_rot.y);
	m_mWorld.RotateZ(m_rot.z);
	m_mWorld.Move(m_pos);
	

	//CaeraCompの更新
	if (m_spCameraComponent) 
	{
		KdMatrix trans;
		trans.CreateTranslation(m_pos.x, m_pos.y, m_pos.z);
		m_spCameraComponent->SetCameraMatrix(trans);
		
	}
	
	

}

void Human::UpdateMove()
{
	//カメラの方向に移動方法が依存するのでカメラなかったら帰る
	if (!m_spCameraComponent) { return; }

	//入力情報の取得
	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::Axes::L);

	//カメラの右方向*レバーの左右入力=キャラクターの左右の移動方向
	KdVec3 moveSide = m_spCameraComponent->GetCameraMatrix().GetAxisX() * inputMove.x;
	
	//カメラの前方向*レバーの前後入力=キャラクターの前後の移動方向
	KdVec3 moveForward = m_spCameraComponent->GetCameraMatrix().GetAxisZ() * inputMove.y;

	//上下方向への移動成分はカットしていく
	moveForward.y = 0.0f;

	//移動ベクトルの計算
	KdVec3 moveVec = moveSide+moveForward;

	//正規化
	moveVec.Normalize();

	//キャラクターの回転処理
	UpdateRotate(moveVec);

	//移動速度に合わせる
	moveVec *= m_moveSpeed;

	//場所に加算
	m_force.x = moveVec.x;
	m_force.z = moveVec.z;
}

void Human::UpdateCamera()
{
	if (!m_spCameraComponent) { return; }
	
	//マウスの横入力=キャラクターを中心に回転するカメラの移動
	const Math::Vector2& inputCam= m_spInputComponent->GetAxis(Input::Axes::R);
	m_spCameraComponent->OffsetMatrix().RotateY(inputCam.x * m_camRotSpeed * KdToRadians);
	

}

//rMoveDir:移動方向
void Human::UpdateRotate(const KdVec3& rMoveDir)
{
	//移動していなければ帰る
	if (rMoveDir.LengthSquared() == 0.0f) { return; }

	//今キャラクターの方向ベクトル
	KdVec3 nowDir = m_mWorld.GetAxisZ();
	nowDir.Normalize();

	//キャラクターの今向いている方向の角度を求める(ラジアン角)
	float nowRadian = atan2(nowDir.x, nowDir.z);

	//移動方向へのベクトルの角度を求める(ラジアン角)
	float targetRadian = atan2(rMoveDir.x, rMoveDir.z);

	float rotateRadian = targetRadian - nowRadian;

	//atan2の結果=-π～π(-180度～180度)

	//180どの角度で数値の切れ目がある
	if (rotateRadian > M_PI)
	{
		rotateRadian -= 2 * float(M_PI);
	}
	else if (rotateRadian < -M_PI)
	{
		rotateRadian += 2 * float(M_PI);
	}

	//一回の回転角度をrotateAngle度以内に収める(クランプ)
	rotateRadian = std::clamp(rotateRadian, -m_rotateAngle * KdToRadians, m_rotateAngle * KdToRadians);

	m_rot.y += rotateRadian;

}

void Human::UpdateCollision()
{
	float distanceFromGround = FLT_MAX;

	//下方向への判定を行い、着地した
	if (CheckGround(distanceFromGround))
	{
		//地面の上にy座標を移動
		m_pos.y += s_allowToStepHeight - distanceFromGround;
		
		//地面があるので、y方向への移動力は0に
		//m_force.y = 0.0f;
		if (m_spInputComponent->GetButton(Input::Buttons::A))
		{
			if (m_canJump) {
				m_force.y = 0.2f;
				m_canJump = false;
			}
		}
		else
		{
			m_canJump = true;
		}
	}
}

bool Human::CheckGround(float& rDistDistance)
{
	//レイ判定情報
	RayInfo rayInfo;
	rayInfo.m_pos = m_pos;	//キャラクターの位置を発射地点に

	//キャラの足元からレイを発射すると地面と当たらないので少し持ち上げる(乗り越えられる段差の高さ分だけ)
	rayInfo.m_pos.y += s_allowToStepHeight;

	//落下中かもしれないので、1フレーム前の座標分も持ち上げる
	rayInfo.m_pos.y += m_prevPos.y - m_pos.y;

	//地面方向へのレイ
	rayInfo.m_dir = { 0.0f,-1.0f,0.0f };

	//レイの結果格納用
	rayInfo.m_maxRange = FLT_MAX;
	KdRayResult finalRayResult;

	//全員とレイ判定
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身は無理
		if (obj.get() == this) { continue; }

		//ステージと当たり判定(背景オブジェクト以外に乗るときは変更の可能性あり)
		if (!(obj->GetTag() & (TAG_StagetObject))) { continue; }
		KdRayResult rayResult;

		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			//最も当たったところまでの距離が短いものを保持する
			if (rayResult.m_distance < finalRayResult.m_distance)
			{
				finalRayResult = rayResult;
			}
		}
	}

	//補正分の長さを結果に反映＆着地判定
	float distanceFromGround = FLT_MAX;

	//足元にステージオブジェクトがあった
	if (finalRayResult.m_hit)
	{
		//地面との距離を算出
		distanceFromGround = finalRayResult.m_distance - (m_prevPos.y - m_pos.y);
	}

	//上方向にかかっていた場合
	if (m_force.y > 0.0f)
	{
		//着地禁止
		m_isGround = false;
	}
	else
	{
		//地面からの距離が(歩いて乗り越えられる高さ＋地面から足が離れても着地する高さ)未満であれば着地とみなす
		m_isGround = (distanceFromGround < (s_allowToStepHeight + s_landingHeight));
	}

	//地面との距離を格納
	rDistDistance = distanceFromGround;

	//着地したかどうかを返す
	return m_isGround;

}

