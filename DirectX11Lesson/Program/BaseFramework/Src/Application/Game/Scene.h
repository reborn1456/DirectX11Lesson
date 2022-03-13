#pragma once
#include"ImGuiHelper.h"

//前方宣言
class EditorCamera;
class GameObject;
class CameraComponent;

class Scene
{
public:

	static Scene& GetInstance()
	{
		static Scene instance;
		return instance;
	}

	~Scene();//デストラクタ

	void Init();
	void Deserialize();

	void RequestChangeScene(const std::string& filename);//シーン変更のリクエストを受け取り

	void Release();
	void Update();
	void Draw();


	void AddObject(std::shared_ptr<GameObject> spObject);//渡されたミサイルを追加

	//指定された名前で検索をかけて合致した最初のオブジェクトを返す
	std::shared_ptr<GameObject> FindObjectWithName(const std::string& name);

	// GameObjectのリストを返す
	const std::list<std::shared_ptr<GameObject>>GetObjects()const { return m_spObjects; }

	inline void SetTargetCamera(std::shared_ptr<CameraComponent> spCamera) { m_wpTargetCamera = spCamera; }

	void ImGuiUpdate();//ImGuiの更新


	//デバッグライン描画
	void AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color = { 1,1,1,1 });

	//デバッグスフィア描画
	void AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color = { 1,1,1,1 });

	//軸描画
	void AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale = 1.0f);
	 
private:

	Scene();//コンストラクタ

	void LoadScene(const std::string& sceneFilename);

	//シーン遷移=====
	void ExecChangeScene();					//シーンを実際に変更する処理
	void Reset();							//シーンをまたぐときにリセットする処理

	std::string m_nextSceneFilename = "";	//次のシーンのJsonファイル名
	bool m_isReqestChangeScene = false;		//シーン遷移のリクエストがあったか
	//===============

	std::shared_ptr<KdModel> m_spsky = nullptr;               //スカイスフィア(世界)
	EditorCamera* m_pCamera = nullptr;  //カメラ
	bool     m_editorCameraEnable = true;

	//シェア参照カウンタ、勝手にデリートしてくれる
	std::list<std::shared_ptr<GameObject>> m_spObjects;

	//Imguiで選択されたオブジェクト
	std::weak_ptr<GameObject> m_wpImguiSelectObj;

	//ターゲットのカメラ
	std::weak_ptr <CameraComponent> m_wpTargetCamera;

	//デバッグライン描画用頂点配列
	std::vector<KdEffectShader::Vertex> m_debugLines;

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	void ImGuiPrefabFactory();

	std::string			m_jsonName = "";
	std::string			m_jsonPath = "";
	ImGuiLogWindow		m_Editor_Log;
	KdWindow			m_Win;
	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

};