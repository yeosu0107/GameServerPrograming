#pragma once
#include <math.h>

const int namebuff = 10;

class Renderer;

enum TEAM {TEAM_1=0, TEAM_2, NONE};

class RENDER_LEVEL
{
public:
	const float RENDER_GOD					= 0.0f;
	const float RENDER_BUILDING				= 0.1f;
	const float RENDER_CHARACTER			= 0.2f;
	const float RENDER_PROJECTILE			= 0.3f;
};

class Objects
{
protected:
	float3 m_Pos;		//��ġ
	float4 m_Color;	//����
	float4 m_teamColor; //��Į��
	float m_Size;		//ũ��
	float m_Weight;	//����
	
	UINT m_texIndex;		//�ؽ���
	bool	m_IsAnimate;		//�ִϸ��̼��� �ִ°�?
	int		m_CurrXSeq;		//����x������
	int		m_CurrYSeq;		//����y������
	int		m_MaxXSeq;		//�ִ�x������
	int		m_MaxYSeq;		//�ִ�y������
	int		m_aniState;		//���� ����
	int		m_divide;			//�ִϸ��̼� ������
	float	m_aniTime;

	float m_RenderLevel; //������ ���� (�������� ���� �׷���)

	int m_id;

	bool m_Live;		//���ӻ� ǥ���Ǵ� ���� (true - ǥ���ض�, false -  ������ ����)

	bool m_isLifeGuage; //������������ ǥ�� ����
	float m_LifeGuage;	//������������
	int m_FullLife;		//�ִ� HP
	int m_Life;			//���� HP
	char m_buf[10];	//������������ �ؽ�Ʈ

	int m_type;			//������Ʈ Ÿ��

	DWORD m_PrevTime; 

public:
	Objects(float3 pos, float4 color, float size, float weight);
	//��ǥ, RGB , ������, ����, �̸�, �̵�����, �ӵ�, �����
	~Objects();

	//������ ����
	void setPos(float, float, float);
	void setPos(float3);
	void setColor(float, float, float, float);
	void setColor(float4);
	void setSize(float size) { 
		m_Size = size; 
	}
	void setWeight(float weight) { m_Weight = weight; }
	void setType(int type) { m_type = type; }

	void setLive(bool live) { m_Live = live; }

	void setminusLife(int tmp);
	void setID(int tmp) { m_id = tmp; }
	void setTexIndex(int tmp) { 	m_texIndex = tmp; }
	void setTexSeq(int x, int y) {
		m_IsAnimate = true;
		m_CurrXSeq = 0;
		m_CurrYSeq = 0;
		m_MaxXSeq = x;
		m_MaxYSeq = y;
	}

	void setRenderLevel(float tmp) { m_RenderLevel = tmp; }
	void setIsLifeGuage(bool tmp) { m_isLifeGuage = tmp; }

	//���� �� �ҷ�����
	float3 getPos() const { return m_Pos; }
	float getSize() const { return m_Size; }
	float getWeight() const { return m_Weight; }
	int getType() const { return m_type; }

	bool getLive() const { return m_Live; }
	int getLife() const { return m_Life; }

	int getID() const { return m_id; }
	int getTexIndex() const { return m_texIndex; }

	//������Ʈ ����
	void Move(float ElapsedTime, float3 moveValue);		//�ӵ����͸�ŭ �̵�

	virtual void Animate();					//�ִϸ���Ʈ

	virtual void OnPrepareRender();		//������ ���� �����ؾ� �� �͵�
	virtual void Render(Renderer& renderer);					//������

	virtual bool Update(float ElapsedTime);					//������Ʈ
};
