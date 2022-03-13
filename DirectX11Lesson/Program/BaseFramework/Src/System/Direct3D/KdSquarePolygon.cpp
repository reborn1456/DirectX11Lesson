#include"KdSquarePolygon.h"

void KdSquarePolygon::Init(float w, float h, const Math::Vector4& _color)
{
	//���_���W�̐ݒ�(���A������������ŗ���)
	m_vertex[0].pos = { -w / 2,-h / 2,0 };	//����
	m_vertex[1].pos = { -w / 2,h / 2,0 };	//����
	m_vertex[2].pos = { w / 2,-h / 2,0 };	//�E��
	m_vertex[3].pos = { w / 2,h / 2,0 };	//�E��

	m_vertex[0].color = _color;
	m_vertex[1].color = _color;
	m_vertex[2].color = _color;
	m_vertex[3].color = _color;

	//UV���W�̐ݒ�
	m_vertex[0].UV = { 0,1 };//����
	m_vertex[1].UV = { 0,0 };//����
	m_vertex[2].UV = { 1,1 };//�E��
	m_vertex[3].UV = { 1,0 };//�E��

}
void KdSquarePolygon::Draw(int setTextutreNo)
{
	//�e�N�X�`�����ݒ肳��Ă�����DirectX���ɋ�����
	if (m_texture) {
		D3D.GetDevContext()->PSSetShaderResources(setTextutreNo, 1, m_texture->GetSRViewAddress());
	}
	else
	{
		//������Ύj�Y�e�N�X�`������t����
		D3D.GetDevContext()->PSSetShaderResources(setTextutreNo, 1, D3D.GetWhiteTex()->GetSRViewAddress());
	}

	//�w�肵�����_�z���`��
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, &m_vertex, sizeof(Vertex));
}

void KdSquarePolygon::SetAnimationPos(float no)
{
	int x = (int)no % m_animSplitX;
	int y = (int)no / m_animSplitX;

	float w = 1.0f / m_animSplitX;
	float h = 1.0f / m_animSplitX;

	m_vertex[0].UV = { x * w,	(y + 1) * h };	//����
	m_vertex[1].UV = { x * w,	y * h };		//����
	m_vertex[2].UV = { (x + 1) * w,(y + 1) * h };//�E��
	m_vertex[3].UV = { (x + 1) * w,y * h };		//�E��

	//�A�j���[�V�����̈ʒu���X�V
	m_animPos = no;
}

void KdSquarePolygon::Animation(float speed, bool loop)
{
	//�X�s�[�h���A�j���[�V�����ʒu�����߂�
	m_animPos += speed;

	//�I������
	if (m_animPos >= (m_animSplitX * m_animSplitY))
	{
		//���[�v���邩�ǂ���
		if (loop)
		{
			m_animPos = 0;//�����߂�
		}
		//���[�v���Ȃ�
		else
		{
			//�Ō�̃R�}�Ŏ~�܂�
			m_animPos = (float)(m_animSplitX * m_animSplitY) - 1;
		}
	}
	//UV���W�̍X�V
	SetAnimationPos(m_animPos);
}

bool KdSquarePolygon::IsAnimationEnd()
{
	//�I������
	if (m_animPos >= (m_animSplitX * m_animSplitY) - 1) { return true; }

	return false;
}