#include"ActionGameProcess.h"
#include"../Scene.h"

void ActionGameProcess::Update()
{
	

	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		if (KeyFlg) {
			Scene::GetInstance().RequestChangeScene("Data/Scene/ShootingGame.json");
			KeyFlg = false;
		}
	}
	else
	{
		KeyFlg = true;
	}
}