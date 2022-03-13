#include"Aircraft.h"
#include"Missile.h"
#include"../Scene.h"

#include"../../Component/CameraComponent.h"
#include"../../Component/InputComponent.h"
#include"../../Component/ModelComponent.h"

#include"EffectObject.h"

// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void Aircraft::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);

	if (m_spCameraComponent)
	{
		m_spCameraComponent->OffsetMatrix().CreateTranslation(0.0f, 1.5f, -10.0f);
	}
	//プレイヤーなら
	if ((GetTag() & OBJECT_TAG::TAG_Player) != 0)
	{
		Scene::GetInstance().SetTargetCamera(m_spCameraComponent);

		//プレイヤー入力
		m_spInputComponent = std::make_shared<PlayerInputComponent>(*this);
	}
	else
	{
		//敵飛行機入力
		m_spInputComponent = std::make_shared<EnemyInputComponent>(*this);
		
		KdModel::Node* propNode = m_spModelComponent->FindNode("propeller");//文字列を元にプロペラノードの検索
		if (propNode)
		{
			propNode->m_localTransform.CreateTranslation(0.0f, 0.0f, 2.85f);//プレイヤーのプロペラだけを前に進める
		}
		m_propRotSpeed = 0.3f;//プロペラの回転速度
	}
	
	m_spActionState = std::make_shared<ActionFly>();
	
	//軌跡ポリゴン設定
	if (jsonObj["PropNum"].is_null() == false)
	{
		m_propNum = jsonObj["PropNum"].int_value();
	}

	m_propTrail.resize(m_propNum);
	std::shared_ptr<KdTexture> spPropTrailTexture = KdResFac.GetTexture("Data/Texture/sabelline.png");
	for (UINT i = 0; i < m_propTrail.size(); i++) {
		m_propTrail[i].SetTexture(spPropTrailTexture);
	}
}

// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void Aircraft::Update()
{

	if (m_spInputComponent)
	{
		m_spInputComponent->Update();
	}
	//移動前の座標
	m_prevPos = m_mWorld.GetTranslation();


	if (m_spActionState)
	{
		m_spActionState->Update(*this);
		m_spActionState->Update(*this);
	}

	if (m_spCameraComponent)
	{
		m_spCameraComponent->SetCameraMatrix(m_mWorld);
	}

	//プロペラの更新
	UpdatePropeller();

}

void Aircraft::UpdateMove()
{

	if (m_spInputComponent == nullptr) { return; }

	
	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::Axes::L);
	const Math::Vector2& inputRotate = m_spInputComponent->GetAxis(Input::Axes::R);

	//移動ベクトル作成
	KdVec3 move = { inputMove.x, 0.0f, inputMove.y };
	//回転ベクトル作成
	KdVec3 rotate = { -inputRotate.y, 0.0f, -inputRotate.x };
	
	//正規化(ベクトル長さを1にする)
	move.Normalize();

	//移動速度補正
	move *= m_speed;

	//移動行列作成
	KdMatrix moveMat;
	moveMat.CreateTranslation(move.x, move.y, move.z);

	//ワールド行列に合成
	//飛行機描画されるときに使われる行列 移動行列　回転移動を含めた行列
	m_mWorld = moveMat * m_mWorld;

	//回転行列作成
	////////////////////////////////////////////////
	//XMMatrixMultiplyで合成するときの引数の順番  //
	//第一引数が受け取り側(ベース)の行列          //
	//第二引数が送り側の行列                      //
	////////////////////////////////////////////////

	KdMatrix rotateMat;
	rotateMat.CreateRotationX(rotate.x * KdToRadians);
	rotateMat.RotateZ(rotate.z * KdToRadians);
	//ワールド行列に合成
	m_mWorld = rotateMat * m_mWorld;

	if ((GetTag() & OBJECT_TAG::TAG_Player) != 0)
	{
		if (inputMove.y != 0) {
			if (m_propRotSpeed == 0) {
				m_propRotSpeed = 0.05;
			}
			if (m_speed >= 1.5f) {
				m_speed = 1.5f;
			}
			else {
				m_speed += m_speed * 0.02;
			}
			if (m_propRotSpeed >= 0.8f) {
				m_propRotSpeed = 0.8f;
			}
			else {
				m_propRotSpeed += m_propRotSpeed * 0.05;
			}
		}
		else {
			m_speed = 0.2f;
			m_propRotSpeed = 0.0f;
		}
	}
}

void Aircraft::UpdateShoot()
{
	if (m_spInputComponent == nullptr) { return; }

	if (m_spInputComponent->GetButton(Input::Buttons::A))
	{
		if (m_canShoot)
		{
			std::shared_ptr<Missile> spMissile = std::make_shared<Missile>();
			if (spMissile)
			{
				spMissile->Deserialize(KdResFac.GetJSON("Data/Scene/Missile.json"));
				KdMatrix mLuntch;
				mLuntch.CreateRotationX((rand() % 120 - 60.0f) * KdToRadians);
				mLuntch.RotateY((rand() % 120 - 60.0f) * KdToRadians);
				mLuntch *= m_mWorld;
				
				spMissile->SetMatrix(mLuntch);
				
				//クラスを引数で渡すときは参照かポインタ
				//spMissile->SetMatrix(m_mWorld);

				spMissile->SetOwner(shared_from_this());

				Scene::GetInstance().AddObject(spMissile);

				//一番近いオブジェクトとの距離を格納する変数
				float minDistance = FLT_MAX;

				//誘導する予定のターゲットGameObject
				std::shared_ptr<GameObject> spTarget = nullptr;

				//全ゲームオブジェクトのリストからミサイルが当たる対象を探す
				for (auto object : Scene::GetInstance().GetObjects())
				{

					//発射した飛行機自身は無視
					if (object.get() == this) { continue; }
					
					if ((object->GetTag() & TAG_AttackHit))
					{

						//	(ターゲットの座標  -  自分の座標)の長さの二乗
						float distance = KdVec3(object->GetMatrix().GetTranslation() - m_mWorld.GetTranslation()).LengthSquared();

						//一番近いオブジェクトとの距離よりも近ければ
						if (distance < minDistance)
						{
							//誘導する予定のターゲットを今チェックしたGameObjectに置き換え
							spTarget = object;

							//一番近いオブジェクトとの距離を今のものに更新
							minDistance = distance;
						}

					}
					
				}
					spMissile->SetTarget(spTarget);
			}
			m_canShoot = false;
		}
	}
	else
	{
		m_canShoot = true;
	}

	m_laser = (m_spInputComponent->GetButton(Input::Buttons::B) != InputComponent::FREE);

}


void Aircraft::UpdateCollision()
{

	if (m_laser)
	{
		RayInfo rayInfo;
		rayInfo.m_pos = m_prevPos;				//移動する前の地点から
		rayInfo.m_dir = m_mWorld.GetAxisZ();	//自分の向いている方向に
		rayInfo.m_dir.Normalize();				
		rayInfo.m_maxRange = m_laserRange;		//レーザーの射程分判定
	
		//レイの判定結果
		KdRayResult rayResult;

		for (auto& obj : Scene::GetInstance().GetObjects())
		{
			//自分自身は無視
			if (obj.get() == this) { continue; }

			//背景オブジェクトとキャラが対象
			if (!(obj->GetTag() & (TAG_StagetObject | TAG_Character))) { continue; }

			//判定実行
			if (obj->HitCheckByRay(rayInfo, rayResult))
			{
				//当たったのであれば爆発をインスタンス化
				std::shared_ptr<EffectObject> effectObj = std::make_shared<EffectObject>();

				//相手の飛行機へダメージ通知
				//ミサイルやレーザーの攻撃力はJsonに入れておく


				if (effectObj)
				{
					//キャラクターのリストに爆発の追加
					Scene::GetInstance().AddObject(effectObj);

					//レーザーのヒット位置=レイの発射位置＋(レイの発射方向ベクトル*レイが当たった地点までの距離)
					KdVec3 hitPos(rayInfo.m_pos);
					hitPos = hitPos + (rayInfo.m_dir * rayResult.m_distance);

					//爆発エフェクトの行列を計算
					KdMatrix mMat;
					mMat.CreateTranslation(hitPos.x, hitPos.y, hitPos.z);
					effectObj->SetMatrix(mMat);
				}
			}

		}
	
	}

	//一回の移動量と移動方向を計算
	KdVec3 moveVec = m_mWorld.GetTranslation() - m_prevPos;//動く前->今の場所のベクトル
	float moveDistance = moveVec.Length();//一回の移動量

	//動いていないなら判定しない
	if (moveDistance == 0.0f) { return; }

	//球判定情報の作成
	SphereInfo info;
	info.m_pos = m_mWorld.GetTranslation();
	info.m_radius = m_colRadius;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身は無視
		if (obj.get() == this) { continue; }

		//characterと当たり判定するのでそれ以外は無視
		if (!(obj->GetTag() & TAG_Character)) { continue; }

		//当たり判定
		if (obj->HitCheckBySphere(info))
		{
			Scene::GetInstance().AddDebugSphereLine(
				m_mWorld.GetTranslation(), 2.0f, { 1.0f,0.0f,0.0f,1.0f }
			);
			//移動する前の位置に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}

	//レイによる当たり判定	
	//レイ情報の作成
	RayInfo rayInfo;
	rayInfo.m_pos = m_prevPos;			//ひとつ前の場所から
	rayInfo.m_dir = moveVec;			//動いた方向に向かって
	rayInfo.m_maxRange = moveDistance;	//動いた分だけ判定を行う

	rayInfo.m_dir.Normalize();

	KdRayResult rayResult;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身は無視
		if (obj.get() == this) { continue; }

		//背景タグ以外は無視
		if (!(obj->GetTag() & TAG_StagetObject)) { continue; }

		//判定実行
		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			//移動する前の位置フレーム前に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}
}

// 3DS課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void Aircraft::UpdatePropeller()
{
	//プロペラを回転させる
	KdModel::Node* propNode = m_spModelComponent->FindNode("propeller");//文字列を元にプロペラノードの検索
	if (propNode)
	{
		propNode->m_localTransform.RotateZ(m_propRotSpeed);//プロペラだけ回る
		
		//プロペラの中心座標(world)
		KdMatrix propCenterMat;
		propCenterMat *= propNode->m_localTransform * m_mWorld;

		float betweenPropRadian = (360.0f / m_propNum)* KdToRadians;

		for (UINT i = 0; i < m_propTrail.size(); i++) {
			
			m_propTrail[i].AddPoint(propCenterMat);

			//プロペラの外側座標(world)
			KdMatrix propOuterMat;
			//そこからY軸へずらした位置(モデルのスケールが変わると通用しない)
			propOuterMat.CreateTranslation(0.0f, 1.8f, 0.0f);

			propOuterMat.RotateZ(betweenPropRadian * i);
			propOuterMat *= propCenterMat;
			
			//Strip描画するために2つでペア追加
			m_propTrail[i].AddPoint(propOuterMat);

			//30個より多く登録されてたら
			if (m_propTrail[i].GetNumPoints() > 30)
			{
				//Strip描画するため2つで1ペア消す
				m_propTrail[i].DelPoint_Back();
				m_propTrail[i].DelPoint_Back();
			}
		}
	}
}
// 3DS課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

void Aircraft::Draw()
{
	GameObject::Draw();//基底クラスのDrawを呼び出す

	////プロペラ	
	//if (m_spPropeller)
	//{
	//	m_spPropeller->Draw();
	//}

	//レーザー描写
	if (m_laser)
	{
		//レーザの終点を決める
		KdVec3 laserStart(m_prevPos);
		KdVec3 laserEnd;
		KdVec3 laserDir(m_mWorld.GetAxisZ());

		laserDir.Normalize();//拡大が入ってると1以上になるので正規化

		laserDir *= m_laserRange;//レーザーの射程分方向ベクトルを伸ばす

		laserEnd = laserStart + laserDir;//レーザーの終点は発射位置ベクトル＋レーザー分の長さ分

		Scene::GetInstance().AddDebugLine(m_prevPos, laserEnd, { 0.0f,1.0f,1.0f,1.0f });

	}
}

// 3DS課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void Aircraft::DrawEffect()
{
	D3D.GetDevContext()->OMSetBlendState(SHADER.m_bs_Add, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

	SHADER.m_effectShader.SetWorldMatrix(KdMatrix());
	SHADER.m_effectShader.WriteToCB();
	
	for (UINT i = 0; i < m_propTrail.size(); i++) {
		m_propTrail[i].DrawStrip();
	}
	D3D.GetDevContext()->OMSetBlendState(SHADER.m_bs_Alpha, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);
}
// 3DS課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

void Aircraft::OnNotify_Damage(int damage)
{
	m_hp -= damage; //相手の攻撃力分、HPを減らす

	//HPが0になったら消える
	if (m_hp <= 0)
	{
		m_spActionState = std::make_shared<ActionCrash>();
	}
}


void Aircraft::ActionFly::Update(Aircraft& owner)
{
	owner.UpdateMove();
	
	owner.UpdateCollision();
	
	owner.UpdateShoot();
}

void Aircraft::ActionCrash::Update(Aircraft& owner)
{
	if (!(--m_timer))
	{
		owner.Destroy();
	}

	KdMatrix rotation;
	rotation.CreateRotationX(0.08f);
	rotation.RotateY(0.055f);
	rotation.RotateZ(0.03f);

	owner.m_mWorld = rotation * owner.m_mWorld;
	
	owner.m_mWorld.Move(KdVec3(0.0f, -0.2f, 0.0f));


}
