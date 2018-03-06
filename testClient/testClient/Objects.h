#pragma once
#include <math.h>

const int namebuff = 10;

class Renderer;

struct float3
{
	float x, y, z;
	float3(float a, float b, float c) {
		x = a; y = b; z = c;
	}
	float3 operator+=(const float3& num) {
		x = x + num.x;
		y = y + num.y;
		z = z + num.z;
		return float3(x, y, z);
	}
	float3 operator-=(const float3& num) {
		x = x - num.x;
		y = y - num.y;
		z = z - num.z;
		return float3(x, y, z);
	}
	float3 operator*(const float num) {
		x *= num;
		y *= num;
		z *= num;
		return float3(x, y, z);
	}
};

struct float4
{
	float x, y, z, w;
	float4(float a, float b, float c, float d) {
		x = a; y = b; z = c; w = d;
	}
};

class Objects
{
protected:
	float3 m_Pos;		//��ġ
	float4 m_Color;	//����
	float m_Width;	//����
	float m_Height;	//����
	float m_Weight;	//����

	bool m_Live;		//���ӻ� ǥ���Ǵ� ���� (true - ǥ���ض�, false -  ������ ����)
	char* m_Name;	//�̸�
	
	float3 m_moveDir;		//�̵����� (x,y,z)
	float m_moveSpeed;	//�̵��ӵ�

	
public:
	Objects();
	Objects(float x, float y, float z, float r, float g, float b, float a, float width, float height, float weight,
		char* name, float mx, float my, float mz, float speed);
	Objects(float3 pos, float4 color, float width, float height, float weight, char* name, float3 dir, float speed);
	~Objects();

	//������ ����
	void setPos(float, float, float);
	void setPos(float3);
	void setColor(float, float, float, float);
	void setColor(float4);
	//void setSize(float size) { m_Size = size; }
	void setWeight(float weight) { m_Weight = weight; }

	void setLive(bool live) { m_Live = live; }
	void setName(char* name) { m_Name = name; }

	void setSpeed(float speed) { m_moveSpeed = speed; }
	void setMoveDir(float, float, float);
	void setMoveDir(float3);


	//���� �� �ҷ�����
	float3 getPos() const { return m_Pos; }
	float4 getColor() const { return m_Color; }
	//float getSize() const { return m_Size; }
	float getWeight() const { return m_Weight; }

	bool getLive() const { return m_Live; }
	char* getName() const { return m_Name; }

	float getSpeed() const { return m_moveSpeed; }
	float3 getMoveDir() const { return m_moveDir; }

	//������Ʈ ����
	void Move();								//�̵��������� �̵��ӵ���ŭ �̵�
	void Move(float3 moveValue);		//�ӵ����͸�ŭ �̵�
	virtual void Animate();					//�ִϸ���Ʈ
	virtual void CrashCheck();				//�浹üũ

	virtual void OnPrepareRender();		//������ ���� �����ؾ� �� �͵�
	virtual void Render(Renderer& renderer);					//������

	virtual void Update();					//������Ʈ
};