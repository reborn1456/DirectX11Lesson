#include"Scene.h"
#include"GameObject.h"
#include"../Component/CameraComponent.h"
//実装すること　　インスタンス化
#include"Shooting/Aircraft.h"
#include"EditorCamera.h"
#include"Shooting/Missile.h"

#include"Shooting/AnimationEffect.h"

//コンストラクタ
Scene::Scene()
{
}

//デストラクタ
Scene::~Scene()
{
}

//初期化
void Scene::Init()
{
	OutputDebugStringA("初期化関数\n");//デバッグ

	//Json===============================================
	//Jsonファイルを開く
	std::ifstream ifs("Data/test.json");
	if (ifs.fail()) { assert(0 && "Jsonファイルのパスが間違っています"); }

	//文字列rとして全読み込み
	std::string strJson((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	//文字列のJsonを解析(パース)する
	std::string err;
	json11::Json jsonObj = json11::Json::parse(strJson, err);
	if (err.size() > 0) { assert(0 && "読み込んだファイルのJson変換に失敗"); }

	//値アクセス
	{
		OutputDebugStringA(jsonObj["Name"].string_value().append("\n").c_str());
		//auto name = jsonObj["Name"].string_value();  本来取得するならこれだけでいい
		OutputDebugStringA(std::to_string(jsonObj["Hp"].int_value()).append("\n").c_str());

	}

	//配列アクセス
	{
		auto& pos = jsonObj["Position"].array_items();
		for (auto& p : pos)
		{
			OutputDebugStringA(std::to_string(p.number_value()).append("\n").c_str());
		}
		//並列添字アクセス
		OutputDebugStringA(std::to_string(pos[0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[1].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[2].number_value()).append("\n").c_str());

	}

	//Object取得
	{
		auto& object = jsonObj["monster"].object_items();
		OutputDebugStringA(object["name"].string_value().append("\n").c_str());
		OutputDebugStringA(std::to_string(object["hp"].int_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][1].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][2].number_value()).append("\n").c_str());

	}
	//Object配列取得
	{
		auto& objects = jsonObj["techniques"].array_items();
		for (auto& object : objects)
		{
			//共通の要素はチェック無しでアクセス
			OutputDebugStringA(object["name"].string_value().append("\n").c_str());
			OutputDebugStringA(std::to_string(object["atk"].int_value()).append("\n").c_str());
			OutputDebugStringA(std::to_string(object["hitrate"].number_value()).append("\n").c_str());

			//固有のパラメータはチェックしてからアクセス
			if (object["effect"].is_string())
			{
				OutputDebugStringA(object["effect"].string_value().append("\n").c_str());
			}
		}
	}
	//================================================================
	
	m_spsky = KdResourceFactory::GetInstance().GetModel("Data/Sky/Sky.gltf");

	//インスタンス化
	m_pCamera = new EditorCamera();

	Deserialize();

}
void Scene::Deserialize()
{
	LoadScene("Data/Scene/ShootingGame.json");
}

//解放
void Scene::Release()
{
	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = nullptr;
	}

	m_spObjects.clear();
}

//更新
void Scene::Update()
{
	OutputDebugStringA("更新関数\n");//デバッグ



	if (m_editorCameraEnable)
	{
		m_pCamera->Update();
	}

	auto selectObject = m_wpImguiSelectObj.lock();

	//範囲for(直接中身いじりたいときはautoのあとに&)
	for (auto spObject : m_spObjects)
	{
		if (spObject == selectObject) { continue; }
		spObject->Update();
	}

	
	//   ポインタの配列ver
	for (auto spObjectItr = m_spObjects.begin(); spObjectItr != m_spObjects.end();)
	{
		//寿命が尽きていたらリストから除外
		if ((*spObjectItr)->IsAlive() == false)
		{
			//次の要素返す
			spObjectItr = m_spObjects.erase(spObjectItr);
		}
		else {
			++spObjectItr;
		}
	}

	//シーンの遷移リクエストがあった場合、変更
	if (m_isReqestChangeScene)
	{
		ExecChangeScene();
	}

}

//描画
void Scene::Draw()
{
	OutputDebugStringA("描画関数\n");//デバッグ

	if (m_editorCameraEnable)
	{
		//エディターカメラをシェーダにセット
		m_pCamera->SetToShader();
	}
	else
	{
		std::shared_ptr<CameraComponent> spCamera = m_wpTargetCamera.lock();
		if (spCamera)
		{
			spCamera->SetToShader();
		}
	}

	//カメラ情報(ビュー行列、射影行列)を、各シェーダの定数バッファにセット
	//SHADER.m_cb7_Camera.Write();

	//ライトの情報をセット
	SHADER.m_cb8_Light.Write();

	//エフェクトシェーダを描画デバイスにセット
	SHADER.m_effectShader.SetToDevice();

	//スカイスフィア拡大
	Math::Matrix skyScale = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);

	SHADER.m_effectShader.SetWorldMatrix(skyScale);

	//モデルの描画(メッシュ情報とマテリアルの情報を渡す)
	if (m_spsky)
	{
		SHADER.m_effectShader.DrawMesh(m_spsky->GetMesh(0).get(), m_spsky->GetMaterials());
	}
	//不透明物描画
	SHADER.m_standardShader.SetToDevice();

	//範囲for
	for (auto pObject : m_spObjects)
	{
		pObject->Draw();
	}

	//半透明物描画
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());
	 

		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteDisable, 0);
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullNone);
		

		for (auto spObj : m_spObjects)
		{
			spObj->DrawEffect();
		}

		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteEnable, 0);
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullBack);


	 


	//デバッグライン描画
	//  　　 不透明描画　　　描画準備
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());
	{
		AddDebugLine(Math::Vector3(), Math::Vector3(0.0f, 10.0f, 0.0f));

		AddDebugSphereLine(Math::Vector3(5.0f, 5.0f, 0.0f), 2.0f);

		AddDebugCoordinateAxisLine(Math::Vector3(0.0f, 5.0f, 5.0f), 3.0f);


		//Zバッファ使用OFF・書き込みOFF
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZDisable_ZWriteDisable, 0);

		if (m_debugLines.size() >= 1) {//点が1以上のとき
			//                      　　　　　　単位行列(拡大率以外0の行列)
			SHADER.m_effectShader.SetWorldMatrix(Math::Matrix());
			//                                 描画する点の配列  　　点の描画方法(今回は線)   
			SHADER.m_effectShader.DrawVertices(m_debugLines, D3D_PRIMITIVE_TOPOLOGY_LINELIST);

			m_debugLines.clear();
		}
		//Zバッファ使用ON・書き込みON
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteEnable, 0);

	}
}
//軸描画
void Scene::AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale)
{
	KdEffectShader::Vertex ver;
	ver.Color = Math::Color{ 1,1,1,1 };
	ver.UV = { 0.0f,0.0f };

	//x軸(赤)
	ver.Color = Math::Color{ 1.0f,0.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.x += 1.0f * scale;
	m_debugLines.push_back(ver);

	//y軸(緑)
	ver.Color = Math::Color{ 0.0f,1.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.y += 1.0f * scale;
	m_debugLines.push_back(ver);

	//z軸(青)
	ver.Color = Math::Color{ 0.0f,0.0f,1.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.z += 1.0f * scale;
	m_debugLines.push_back(ver);


}

//ImGui更新
void Scene::ImGuiUpdate()
{
	auto selectObject = m_wpImguiSelectObj.lock();
	
	if (ImGui::Begin("Scene"))
	{
		ImGui::Checkbox("EditorCamera", &m_editorCameraEnable);

		//オブジェクトリストの描画
		if (ImGui::CollapsingHeader("Object List", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto&& rObj : m_spObjects)
			{
				//選択オブジェクトと一致するオブジェクトかどうか
				bool selected = (rObj == selectObject);

				ImGui::PushID(rObj.get());

				if (ImGui::Selectable(rObj->GetName(), selected))
				{
					m_wpImguiSelectObj = rObj;
				}

				ImGui::PopID();
			}
		}
	}

	ImGui::End();

	if (ImGui::Begin("Inspector"))
	{
		if (selectObject)
		{
			//オブジェクトリストで選択したゲームオブジェクトの情報を描画
			selectObject->ImGuiUpdate();
		}
	}
	ImGui::End();

	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	ImGuiPrefabFactory();
	// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====


}

// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====
void Scene::ImGuiPrefabFactory()
{
	//ログウィンドウ展開
	m_Editor_Log.ImGuiUpdate("Log Window");

	if (ImGui::Begin("PrefabFactory"))
	{
		//Jsonパス入力
		ImGui::InputText("JsonName", &m_jsonName);
		if (ImGui::Button("Create"))
		{
			json11::Json& jsonObj = (KdResFac.GetJSON(m_jsonName));

			if (jsonObj.is_null()) {	
				//ファイル名間違ってたら
				m_Editor_Log.AddLog(u8"%s Prefab生成失敗", m_jsonName.c_str());
				return;
			}
				auto newGameObj = CreateGameObject(jsonObj["ClassName"].string_value());
				newGameObj->Deserialize(jsonObj);
				AddObject(newGameObj);

				m_Editor_Log.AddLog(u8"%s Prefab生成完了", m_jsonName.c_str());
			
		}
		//エクスプローラー開く
		if (ImGui::Button(u8"Jsonパス取得"))
		{
			if (m_Win.OpenFileDialog(m_jsonPath, "ファイルを開く", "jsonファイル\0*.json\0"))
			{
				m_Editor_Log.AddLog(u8"読み込んだJsonファイルのパス=%s", m_jsonPath.c_str());
				m_jsonName = m_jsonPath;
			}
			ImGui::LabelText("%s", m_jsonPath.c_str());
		}
	}
	ImGui::End();
}
// 3D課題変更箇所===== ===== ===== ===== ===== ===== ===== ===== ===== =====

void Scene::Reset()
{
	m_spObjects.clear();			//メインのGameObjectリストをクリア
	m_wpImguiSelectObj.reset();		//ImGuiが選んでいるオブジェクトをクリア
	m_wpTargetCamera.reset();		//カメラのターゲットになっているキャラクタのリセット
	m_spsky = nullptr;				//空のクリア
}

void Scene::LoadScene(const std::string& sceneFilename)
{

	Reset();//各項目のクリア

	//JSON読み込み
	json11::Json json = KdResFac.GetJSON(sceneFilename);
	if (json.is_null())
	{
		assert(0 && "[LoadScene]jsonファイル読み込み失敗");
		return;
	}

	//オブジェクトリスト取得
	auto& objectDataList = json["GameObjects"].array_items();

	//オブジェクト生成ループ
	for (auto&& objJsonData : objectDataList)
	{
		//オブジェクト作成
		auto newGameObj = CreateGameObject(objJsonData["ClassName"].string_value());

		//プレハブ指定ありの場合は、プレハブ側のものをベースにこのJSONをマージする
		KdMergePrefab(objJsonData);

		//オブジェクトのデシリアライズ
		newGameObj->Deserialize(objJsonData);

		//リストへ追加
		AddObject(newGameObj);
	}
}


void Scene::AddObject(std::shared_ptr<GameObject> spObject)
{
	if (spObject == nullptr) { return; }
	m_spObjects.push_back(spObject);
}

std::shared_ptr<GameObject> Scene::FindObjectWithName(const std::string& name)
{
	for (auto&& obj : m_spObjects)
	{
		if (obj->GetName() == name) { return obj; }
	}

	//見つからなかったらnullが帰る
	return nullptr;
}

//デバッグライン描画
void Scene::AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color)
{
	//ラインの開始頂点
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };
	ver.Pos = p1;
	m_debugLines.push_back(ver);

	//ラインの終端頂点
	ver.Pos = p2;
	m_debugLines.push_back(ver);

}

void Scene::AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color)
{
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };

	static constexpr int kDetail = 32;
	for (UINT i = 0; i < kDetail + 1; i++)
	{
		//XZ平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		//XY平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		//YZ平面
		ver.Pos = pos;
		ver.Pos.y += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.y += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

	}
}
void Scene::RequestChangeScene(const std::string& fileName)
{
	//次のシーンへのファイル名を覚える
	m_nextSceneFilename = fileName;

	//リクエストがあったことを覚える
	m_isReqestChangeScene = true;
}
void Scene::ExecChangeScene()
{
	LoadScene(m_nextSceneFilename);//シーンの遷移

	m_isReqestChangeScene = false;//リクエスト状況のリセット
}


