#pragma once
#include"KdMath.h"

//3Dベクトル
//KdVec3にXMFLOAT3を取り込む(継承)
//　　派生クラス　　　　　　基底クラス
class KdVec3:public DirectX::XMFLOAT3
{
	//==========カスタマイズ(ラッピング)===============================-

public:

	//追加-------------------------------------------->

	//指定行列でVectorを変換する
	KdVec3& TransformCoord(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformCoord(*this, m);
		return* this;
	}
	//指定(回転)行列でVectorを変換する
	KdVec3& TransformNormal(const DirectX::XMMATRIX& m)
	{
		*this = XMVector3TransformNormal(*this, m);
		return* this;
	}
	
	//<--------------------------------------------追加

	//デフォルトコンストラクタ(引数が無いコンストラクタ)
	KdVec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	//座標指定付きコンストラクタ
	KdVec3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	//XMVECTORから代入してきたとき
	KdVec3(const DirectX::XMVECTOR& v)
	{
		//保存用に変換して代入(XMVECTORからXMFLOATへ戻している)
		//        出力先(XMFLOAT3*にアップキャストされた),出力元
		DirectX::XMStoreFloat3(this, v);//this=自分自身のポインタ
		//下記と同意だが、上記のSIMD命令を使用したほうが高速
		//x=v.m128_f32[0];  m128(データサイズ)_f32(どう使っているのか)
		//y=v.m128_f32[1];
		//z=v.m128_f32[2];
	}

	//XMVECTORへ変換
	operator DirectX::XMVECTOR()const { return DirectX::XMLoadFloat3(this); }

	//Math::Vector3と互換性を持つための関数 (キャストしてる)
	operator Math::Vector3&(){ return *(Math::Vector3*)this; }

	//算術演算子　乗算(*) operator独自の処理(オペレータオーバーロード)
	KdVec3& operator*=(float s)
	{
		*this = DirectX::XMVectorScale(*this, s);
		return *this;
	}

	//自分を正規化
	void Normalize()
	{
		*this = DirectX::XMVector3Normalize(*this);
	}

	//長さ
	float Length() const
	{
		return DirectX::XMVector3Length(*this).m128_f32[0];
	}
	
	//長さの二乗(ルートの計算が無くて高速なので使うことが多い)
	float LengthSquared() const
	{
		return DirectX::XMVector3LengthSq(*this).m128_f32[0];
	}

	//内積
	static float Dot(const KdVec3& v1, const KdVec3& v2)
	{
		return DirectX::XMVector2Dot(v1, v2).m128_f32[0];
	}
	//外積
	static KdVec3 Cross(const KdVec3& v1, const KdVec3& v2)
	{
		return DirectX::XMVector3Cross(v1, v2);
	}

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

	inline void Complement(const KdVec3& vTo, float rot) 
	{
		//自身の方向ベクトル
		KdVec3 me = *this;

		//回転軸取得
		KdVec3 vRotAxis = KdVec3::Cross(me, vTo);

		if (vRotAxis.LengthSquared() != 0) {

			float d = KdVec3::Dot(me, vTo);

			if (d > 1.0f)d = 1.0f;
			else if (d < -1.0f)d = -1.0f;

			//ターゲットへの角度
			float radian = acos(d);

			if (radian > rot* KdToRadians) 
			{
				radian = rot * KdToRadians;
			}

			//回転軸作成(軸と角度渡す)
			DirectX::XMMATRIX Rotmat = DirectX::XMMatrixRotationAxis(vRotAxis, radian);
			
			TransformNormal(Rotmat);

			////行列をベクトルに変換
			//KdVec3 vec = TransformCoord(Rotmat);

			//*this = vec;

		}
	}
	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

};

//4X4の行列
class KdMatrix :public DirectX::XMFLOAT4X4
{
public:

	//デフォルトコンストラクタ単位行列化
	KdMatrix()
	{
		*this = DirectX::XMMatrixIdentity();
	}
	//XMMATRIXから代入してきた
	KdMatrix(const DirectX::XMMATRIX& m)
	{
		DirectX::XMStoreFloat4x4(this, m);
	}
	//XMFLOAT4X4,Math::Matrixから代入してきた
	KdMatrix(const DirectX::XMFLOAT4X4& m)
	{
		//速度求めるとき
		//メモリ用域ごとコピー
		memcpy_s(this,sizeof(float)*16,&m,sizeof(XMFLOAT4X4));
	}

	//XMMATRIXへ変換(キャスト)
	operator DirectX::XMMATRIX() const
	{
		return DirectX::XMLoadFloat4x4(this);
	}

	

	//Math::Matrixと互換性を持つための関数
	operator Math::Matrix& () { return *(Math::Matrix*)this; }

	//代入演算子 乗算
	KdMatrix& operator*=(const KdMatrix& m)
	{
		*this = DirectX::XMMatrixMultiply(*this, m);
		return *this;
	}

	//作成=======================================================

	//移動行列作成
	void CreateTranslation(float x, float y, float z)
	{
		*this = DirectX::XMMatrixTranslation(x, y, z);
	}

	//X軸回転行列
	void CreateRotationX(float angle)
	{
		*this = DirectX::XMMatrixRotationX(angle);
	}

	//拡縮行列作成
	void CreateScalling(float x, float y, float z)
	{
		*this = DirectX::XMMatrixScaling(x, y, z);
	}

	//指定軸回転行列作成
	void CreateRotationAxis(const KdVec3& axis, float angle)
	{
		*this = DirectX::XMMatrixRotationAxis(axis, angle);
	}

	//透視影行列の作成
	KdMatrix& CreateProjectionPerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		*this = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
		return *this;
	}

	///操作============================================================

	//Z軸回転
	void RotateZ(float angle)
	{
		//行列に追加したいので*
		*this *= DirectX::XMMatrixRotationZ(angle);
	}

	//X軸回転
	void RotateX(float angle)
	{
		*this *= DirectX::XMMatrixRotationX(angle);
	}

	//Y軸回転
	void RotateY(float angle)
	{
		*this *= DirectX::XMMatrixRotationY(angle);
	}

	//逆行列にする
	void Inverse()
	{
		*this = DirectX::XMMatrixInverse(nullptr, *this);
	}

	//拡縮
	void Scale(float x, float y, float z)
	{
		*this *= DirectX::XMMatrixScaling(x, y, z);
	}
	//拡縮
	void testScale(float s)
	{
		*this *= DirectX::XMMatrixScaling(s, s, s);
	}

	//移動関数
	void Move(const KdVec3& v)
	{ 
		_41 += v.x;
		_42 += v.y;
		_43 += v.z;
	}

	//プロパティ==================================================

	//X軸取得
	KdVec3 GetAxisX()const { return{ _11,_12,_13 }; }
	//Y軸取得
	KdVec3 GetAxisY()const { return{ _21,_22,_23 }; }
	//Z軸取得
	KdVec3 GetAxisZ()const { return{ _31,_32,_33 }; }

	//X軸セット
	void SetAxisX(const KdVec3& v)
	{
		_11 = v.x;
		_12 = v.y;
		_13 = v.z;
	}
	//Y軸セット
	void SetAxisY(const KdVec3& v)
	{
		_21 = v.x;
		_22 = v.y;
		_23 = v.z;
	}
	//Z軸セット
	void SetAxisZ(const KdVec3& v)
	{
		_31 = v.x;
		_32 = v.y;
		_33 = v.z;
	}

	//座標取得
	KdVec3 GetTranslation()const { return{ _41,_42,_43 }; }

	void SetTranslation(const KdVec3& v)
	{
		_41 = v.x;
		_42 = v.y;
		_43 = v.z;
	}

	KdVec3 GetAngle() const
	{
		KdMatrix mat = *this;

		//各軸を取得
		KdVec3 axisX = mat.GetAxisX();
		KdVec3 axisY = mat.GetAxisY();
		KdVec3 axisZ = mat.GetAxisZ();

		//各軸を正規化
		axisX.Normalize();
		axisY.Normalize();
		axisZ.Normalize();

		//マトリックスを正規化
		mat.SetAxisX(axisX);
		mat.SetAxisY(axisY);
		mat.SetAxisZ(axisZ);

		KdVec3 angles;
		angles.x = atan2f(mat.m[1][2], mat.m[2][2]);
		angles.y = atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2]));
		angles.z = atan2f(mat.m[0][1], mat.m[0][0]);

		return angles;
	}
	
	float GetScale()const {
		KdMatrix mat = *this;

		float scales;

		scales = mat.GetAxisX().Length();
		

		

		return scales;
	}

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

	inline void SetBillboard(const KdMatrix& mat) {

		auto CamMat = mat;

		CamMat.Inverse();
		auto vPos = this->GetTranslation();

		*this *= CamMat;

		this->SetTranslation(vPos);

	}
	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

};

//KdMatrix同士の合成
inline KdMatrix operator* (const KdMatrix& M1, const KdMatrix& M2)
{
	using namespace DirectX;
	return XMMatrixMultiply(M1, M2);
}