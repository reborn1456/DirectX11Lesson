#include"KdCollision.h"

using namespace DirectX;
//XMMATRIXは、DirectX::XMMATRIXということ！

bool KdRayToMesh(const XMVECTOR& rRayPos, const XMVECTOR& rRayDir, float maxDistance, const KdMesh& rMesh, const KdMatrix& rMatrix,KdRayResult& rResult)
{

	//モデルの逆行列でレイを変換
	XMMATRIX invMat = XMMatrixInverse(0, rMatrix);//高速化

	//レイの判定開始位置を逆変換
	XMVECTOR rayPos = XMVector3TransformCoord(rRayPos, invMat);

	//発射方向は正規化されていないと正しく判定できないので正規化
	XMVECTOR rayDir= XMVector3TransformNormal(rRayDir, invMat);

	//逆行列に拡縮が入っていると
	//レイが当たった距離にも拡縮が反映されてしまうので
	//判定用の最大距離にも拡縮を反映させておく
	float dirLength = XMVector3Length(rayDir).m128_f32[0];
	float rayCheckRange = maxDistance * dirLength;
	
	rayDir = XMVector3Normalize(rayDir);

	//-----------------------------------------------------
	//ブロードウェイズ
	//	比較的軽量なAABB vs レイな判定で
	//	あきらかにヒットしない場合は、これ以降の判定を中止
	//-----------------------------------------------------
	{
		//AABB vs レイ
		float AABBdist = FLT_MAX;
		if (rMesh.GetBoundingBox().Intersects(rayPos, rayDir, AABBdist) == false) { return false; }

		//最大距離以降なら範囲外なので中止
		if (AABBdist > rayCheckRange) { return false; }

	}

	//-----------------------------------------------------
	//ナローウェイズ
	//	レイ vs 全ての面
	//-----------------------------------------------------

	bool ret = false;                 //当たったかどうか
	float closestDist = FLT_MAX;      //当たった面との距離

	//面情報の取得
	const KdMeshFace* pFaces = &rMesh.GetFaces()[0];//面情報の先頭を取得
	UINT faceNum = rMesh.GetFaces().size();

	//すべての面(三角形)と当たり判定
	for (UINT faceIdx = 0; faceIdx < faceNum; ++faceIdx)
	{
		//三角形を構成する3つの頂点のIdx
		const UINT* idx = pFaces[faceIdx].Idx;

		//レイと三角形の当たり判定
		float triDist = FLT_MAX;
		bool bHit = DirectX::TriangleTests::Intersects(
			//		rInfo.m_pos,	//発射場所
			rayPos,			//変更
			rayDir,			//発射方向

			//判定する三角形の頂点情報
			rMesh.GetVertexPositions()[idx[0]],
			rMesh.GetVertexPositions()[idx[1]],
			rMesh.GetVertexPositions()[idx[2]],

			triDist//当たった場合の距離
		);

		//ヒットしていなかったらスキップ
		if (bHit == false) { continue; }

		//最大距離以内か
		if (triDist <= rayCheckRange)
		{
			ret = true;  //当たったとする

			//当たり判定でとれる距離は拡縮に影響ないので、実際の長さを計算する
			triDist /= dirLength;
			if (triDist < closestDist)
			{
				closestDist = triDist;    //距離を更新
			}
		}
	}

	rResult.m_distance = closestDist;
	rResult.m_hit = ret;

	//3D課題変更箇所///////////////////////////////////
	//当たった場所の座標
	rResult.rHitPos = rRayPos;
	rResult.rHitPos = rResult.rHitPos + (rRayDir * rResult.m_distance);
	//3D課題変更箇所///////////////////////////////////

	
	return ret;

}