//メッシュデータ,マテリアルデータ保持するクラス
#pragma once

class KdModel
{
public:
	KdModel();
	~KdModel();
	//　　　　変数の中身変えませんよ
	bool Load(const std::string& filename);//GLTFファイル読み込む(&は参照型nullを入れない)

	//アクセサ constで固定して外部に持ち出す
	//const std::shared_ptr<KdMesh> GetMesh()const { return m_spMesh; }
	//　　　　　　　配列に使いたい型
	const std::shared_ptr<KdMesh> GetMesh(UINT index)const
	{
		return index < m_originalNodes.size() ? m_originalNodes[index].m_spMesh : nullptr;
	}
	
	const std::vector<KdMaterial>& GetMaterials()const { return m_materials; }

	//ノード：モデルを形成するメッシュを扱うための最少単位
	struct Node
	{
		std::string					m_name;				//ノード名
		KdMatrix					m_localTransform;	//変換行列(原点からどれだけ離れているか)
		std::shared_ptr<KdMesh>		m_spMesh;			//メッシュ情報
	};

	//文字列を元にノードの検索
	inline Node* FindNode(const std::string& name)
	{
		for (auto&& node : m_originalNodes)
		{
			if (node.m_name == name) { return &node; }
		}
		return nullptr;
	}

	//ノード配列取得
	const std::vector<Node>& GetOriginalNodes()const { return m_originalNodes; }

private:

	void Release();							//解放
	std::vector<Node>	m_originalNodes;	//データのノード配列

	//メンバのm,ポインタのp,小文字始まり
	//std::shared_ptr<KdMesh> m_spMesh = nullptr;//ここで初期化

	//マテリアル配列(vector配列増やしてくれる)
	std::vector<KdMaterial> m_materials;


};