#include"kdModel.h"
#include"KdGLTFLoader.h"

//コンストラクタ
KdModel::KdModel()//m_pMesh(nullptr)
{

}
//デストラクタ
KdModel::~KdModel()
{
	Release();

	//if (m_spMesh)
	//{
	//	m_spMesh.reset();
	//}
}
void KdModel::Release()
{
	m_materials.clear();
	m_originalNodes.clear();
}
//ロード関数
bool KdModel::Load(const std::string& filename)
{
	//ファイルの完全パスを取得
	std::string fileDir = KdGetDirFromPath(filename);

	//GLTFの読み込み
	std::shared_ptr<KdGLTFModel>spGltfModel = KdLoadGLTFModel(filename);
	if (spGltfModel == nullptr) { return false; }

	//ノード格納場所のメモリ確保
	m_originalNodes.resize(spGltfModel->Nodes.size());

	//メッシュの受け取り
	for (UINT i = 0; i < spGltfModel->Nodes.size(); i++)
	{
		//入力元ノード
		const KdGLTFNode& rSrcNode = spGltfModel->Nodes[i];

		//出力先ノード
		Node& rDstNode = m_originalNodes[i];

		//ノード情報のセット
		rDstNode.m_name = rSrcNode.Name;
		rDstNode.m_localTransform = rSrcNode.LocalTransform;

		//ノードの内容がメッシュだったら
		if (rSrcNode.IsMesh)
		{
			rDstNode.m_spMesh = std::make_shared<KdMesh>();

			if (rDstNode.m_spMesh)
			{
				rDstNode.m_spMesh->Create(
					rSrcNode.Mesh.Vertices, rSrcNode.Mesh.Faces, rSrcNode.Mesh.Subsets);
			}
		}
	}
	//マテリアル配列を受け取れるサイズのメモリを確保
	m_materials.resize(spGltfModel->Materials.size());

	for (UINT i = 0; i < m_materials.size(); i++)
	{
		//src=sourceの略　　　　 渡す側
		//dst=destinationの略　　受け取る側

		const KdGLTFMaterial& rSrcMaterial = spGltfModel->Materials[i];
		KdMaterial& rDstMaterial = m_materials[i];

		//名前 受け取る
		rDstMaterial.Name = rSrcMaterial.Name;

		//基本色
		rDstMaterial.BaseColor = rSrcMaterial.BaseColor;
		rDstMaterial.BaseColorTex = std::make_shared<KdTexture>();

		//テクスチャを読み込み
		rDstMaterial.BaseColorTex = KdResFac.GetTexture(fileDir + rSrcMaterial.BaseColorTexture);
		if (rDstMaterial.BaseColorTex == nullptr)
		{
			//白の画面読み込み
			rDstMaterial.BaseColorTex = D3D.GetWhiteTex();
		}
	}

	return true;

}