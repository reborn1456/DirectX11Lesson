#include"EffectObject.h"

void EffectObject::Update()
{
	if (m_alive == false) { return; }

	if (--m_lifeSpan <= 0)
	{
		Destroy();

		return;
	}
	{
		//徐々に大きくなっていく
		m_scale += m_lifeSpan * 0.001f;

		//色を爆発っぽい色
		m_color.R(m_lifeSpan / 30.0f);
		m_color.G(m_lifeSpan / 60.0f);

	}
}
#include"../Scene.h"
void EffectObject::Draw()
{
	if (m_alive == false) { return; }

	//デバッグ表示で爆発の球を書く
	Scene::GetInstance().AddDebugSphereLine(m_mWorld.GetTranslation(), m_scale, m_color);


}