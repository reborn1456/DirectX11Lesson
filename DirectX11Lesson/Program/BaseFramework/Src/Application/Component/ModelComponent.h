#pragma once

#include"../Game/GameObject.h"

//==========================================--
//モデルコンポーネント
//===========================================
class ModelComponent
{
public:
	ModelComponent(GameObject&owner):m_owner(owner){}

	//有効フラグ
	bool IsEnable()const { return m_enable; }
	void SetEnable(bool enable) { m_enable = enable; }

	//モデルの取得
	//inline const std::shared_ptr<KdModel>& GetModel()const { return m_spModel; }
	const std::vector <KdModel::Node>& GetNodes() const { return m_coppiedNodes; }

	//メッシュ取得
	inline const std::shared_ptr<KdMesh> GetMesh(UINT index)const
	{
		//if (m_spModel == nullptr) { return nullptr; }
		//return m_spModel->GetMesh(index);

		if (index >= m_coppiedNodes.size()) { return nullptr; }
		return m_coppiedNodes[index].m_spMesh;

	}
	//ノード
	inline KdModel::Node* FindNode(const std::string& name)
	{
		//if (!m_spModel) { return nullptr; }
		//return m_spModel->FindNode(name);

		for (auto&& node : m_coppiedNodes)
		{
			if (node.m_name == name)
			{
				return &node;
			}
		}
		return nullptr;
	}

	//モデルセット	
	void SetModel(const std::shared_ptr<KdModel>& model);

	//StanderdSharderで描画
	void Draw();

private:

	std::vector<KdModel::Node> m_coppiedNodes;//個別管理のため、オリジナルからコピーして保持する配列

	//有効
	bool m_enable = true;

	//モデルデータの参照
	std::shared_ptr<KdModel> m_spModel;

	GameObject& m_owner;
};