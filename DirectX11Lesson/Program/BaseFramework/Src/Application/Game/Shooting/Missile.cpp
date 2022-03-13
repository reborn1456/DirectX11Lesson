#include"Missile.h"
#include"../Scene.h"
#include"Aircraft.h"
#include"EffectObject.h"
#include"Application/main.h"
#include"../../Component/ModelComponent.h"
#include"AnimationEffect.h"

void Missile::Deserialize(const json11::Json& jsonObj)
{
	///秒数
	m_lifeSpan = APP.m_maxFps * 10;
	
	if (jsonObj.is_null()) { return; }

	GameObject::Deserialize(jsonObj);
	
	if (jsonObj["Speed"].is_null() == false) 
	{
		m_speed = jsonObj["Speed"].number_value();
	}

	//煙のテクスチャ
	m_trailSmoke.SetTexture(KdResFac.GetTexture("Data/Texture/smokeline2.png"));
}
void Missile::Update()
{


	if (m_alive == false) { return; }

	if (--m_lifeSpan <= 0)
	{
		Destroy();
	}

	//移動前の座標
	m_prevPos = m_mWorld.GetTranslation();


	//ターゲットをshared_ptr化
	auto target = m_wpTarget.lock();

	if (target)
	{

		//自身からターゲットへのベクトル
		KdVec3 vTarget = target->GetMatrix().GetTranslation() - m_mWorld.GetTranslation();

		//単位ベクトル化：地震からターゲットへ向かう長さ１のベクトル
		vTarget.Normalize();

		//自分のZ方向(前方向)
		KdVec3 vZ = m_mWorld.GetAxisZ();

		//拡大率が入っていると計算がおかしくなるため単位ベクトル化
		vZ.Normalize();

		//今の前方向のベクトルをターゲット方向のベクトルに1.0度向ける
		vZ.Complement(vTarget, 1.0f);

		//求めた新しい軸をミサイルの前方向にセットする
		m_mWorld.SetAxisZ(vZ);

	}

		KdVec3 move = m_mWorld.GetAxisZ();
		move.Normalize();

		move *= (float)m_speed;

		//移動前の座標を覚える
		m_prevPos = m_mWorld.GetTranslation();

		m_mWorld.Move(move);

		

	//	//※※※※※回転軸作成(この軸で回転する)※※※※※
	//	KdVec3 vRotAxis = KdVec3::Cross(vZ, vTarget);

	//	//	0ベクトルなら回転しない
	//	if (vRotAxis.LengthSquared() != 0)
	//	{

	//		//自分のZ方向ベクトルと自信からターゲットへ向かうベクトルの内積
	//		float d = KdVec3::Dot(vZ, vTarget);

	//		//誤差で-1～1以外になる可能性大なので、クランプする
	//		if (d > 1.0f)d = 1.0f;
	//		else if (d < -1.0f)d = -1.0f;

	//		//自分の前方向ベクトルと自身からターゲットへ向かうベクトル間の角度(radian)
	//		float radian = acos(d);

	//		//角度制限１フレームにつき最大で1度以上回転しない
	//		if (radian > 1.0f * KdToRadians)
	//		{
	//			radian = 1.0f * KdToRadians;
	//		}

	//		//※※※※※radian(ここまでで回転角度が求まった)※※※※※
	//		KdMatrix mRot;
	//		mRot.CreateRotationAxis(vRotAxis, radian);

	//		auto pos = m_mWorld.GetTranslation();
	//		m_mWorld.SetTranslation({ 0,0,0 });
	//		m_mWorld *= mRot;
	//		m_mWorld.SetTranslation(pos);
	//	}
	//}


	UpdateCollision();

	//軌跡の更新
	UpdateTrail();


}

void Missile::UpdateCollision()
{
	//一回の移動量と移動方向を計算
	KdVec3 moveVec = m_mWorld.GetTranslation() - m_prevPos;//動く前->今の場所のベクトル
	float moveDistance = moveVec.Length();//一回の移動量

	//動いていないなら判定しない
	if (moveDistance == 0.0f) { return; }


	//球判定情報の作成
	SphereInfo info;
	info.m_pos = m_mWorld.GetTranslation();
	info.m_radius = m_colRadius;



	//発射した主人のshared_ptr取得
	auto spOwner = m_wpOwner.lock();

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//自分自身は無視
		if (obj.get() == this) { continue; }
		//発射した主人も無視
		if (obj.get() == spOwner.get()) { continue; }

		//TAG_Characterとは球判定を行う
		if (obj->GetTag() & TAG_Character) {
			if (obj->HitCheckBySphere(info))
			{
				//std::dynamic_pointer_cast=基底クラス型をダウンキャストするときに使う。失敗するとnullptrが帰る
				//重たい、多発する場合は設計がミスっている
				//改善したい人は先生まで相談
				std::shared_ptr<Aircraft> aircraft = std::dynamic_pointer_cast<Aircraft>(obj);
				if (aircraft)
				{
					aircraft->OnNotify_Damage(m_attackPow);
				}

				//3D課題変更箇所///////////////////////////////////
				m_ExPos = m_mWorld.GetTranslation();
				//3D課題変更箇所///////////////////////////////////

				//当たったら
				Explosion();
				Destroy();
			}
		}
	}

	//レイ情報の作成
	RayInfo rayInfo;
	rayInfo.m_pos = m_prevPos;			//ひとつ前の場所から
	rayInfo.m_dir = moveVec;			//動いた方向に向かって
	rayInfo.m_maxRange = moveDistance;	//動いた分だけ判定を行う

	rayInfo.m_dir.Normalize();

	KdRayResult rayResult;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		//TAG_StageObjectとはレイ判定を行う
		if (obj->GetTag() & TAG_StagetObject) {
			if (obj->HitCheckByRay(rayInfo, rayResult))
			{
				//3D課題変更箇所///////////////////////////////////
				m_ExPos = rayResult.rHitPos;
				//3D課題変更箇所///////////////////////////////////

				//当たったら
				Explosion();
				Destroy();

			}
		}
	}

}

void Missile::Explosion()
{
	//アニメーションエフェクトをインスタンス化
	std::shared_ptr<AnimationEffect> effect = std::make_shared<AnimationEffect>();
	
	//爆発のテクスチャとアニメーション情報を渡す
	effect->SetAnimationInfo(
		KdResFac.GetTexture("Data/Texture/Explosion00.png"), 10.0f, 5, 5, rand()%360);
	
	//3D課題変更箇所///////////////////////////////////
	KdMatrix mat;
	mat.SetTranslation(m_ExPos);

	//場所をミサイル(自分)の位置に合わせる
	effect->SetMatrix(mat);
	//3D課題変更箇所///////////////////////////////////

	//リストに追加
	Scene::GetInstance().AddObject(effect);
}
void Missile::UpdateTrail()
{
	//軌跡の座標を先頭に追加
	m_trailSmoke.AddPoint(m_mWorld);

	//軌跡の数制限(100以前の軌跡を消去する)
	if (m_trailSmoke.GetNumPoints() > 100)
	{
		m_trailSmoke.DelPoint_Back();
	}
}
void Missile::DrawEffect()
{
	if (!m_alive) { return; }

	SHADER.m_effectShader.SetWorldMatrix(KdMatrix());

	SHADER.m_effectShader.WriteToCB();
	
	m_trailSmoke.DrawBillboard(0.5f);
}

