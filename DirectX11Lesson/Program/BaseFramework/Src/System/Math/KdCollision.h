#pragma once
//レイ判定をしたときの結果情報
struct KdRayResult
{
	float  m_distance = FLT_MAX;//当たったとこまでの距離
	bool   m_hit = false;//当たったかどうか
	KdVec3 rHitPos = {};
};

//レイに当たり判定
bool KdRayToMesh(
	const DirectX::XMVECTOR& rRayPos, 
	const DirectX::XMVECTOR& rRayDir, 
	float maxDistance, const KdMesh& rMesh, 
	const KdMatrix& rMatrix,
	KdRayResult& rResult
);