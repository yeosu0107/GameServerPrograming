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
	float3 m_Pos;		//위치
	float4 m_Color;	//색깔
	float m_Width;	//넓이
	float m_Height;	//높이
	float m_Weight;	//무게

	bool m_Live;		//게임상에 표현되는 여부 (true - 표현해라, false -  지워진 상태)
	char* m_Name;	//이름
	
	float3 m_moveDir;		//이동방향 (x,y,z)
	float m_moveSpeed;	//이동속도

	
public:
	Objects();
	Objects(float x, float y, float z, float r, float g, float b, float a, float width, float height, float weight,
		char* name, float mx, float my, float mz, float speed);
	Objects(float3 pos, float4 color, float width, float height, float weight, char* name, float3 dir, float speed);
	~Objects();

	//변수값 저장
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


	//변수 값 불러오기
	float3 getPos() const { return m_Pos; }
	float4 getColor() const { return m_Color; }
	//float getSize() const { return m_Size; }
	float getWeight() const { return m_Weight; }

	bool getLive() const { return m_Live; }
	char* getName() const { return m_Name; }

	float getSpeed() const { return m_moveSpeed; }
	float3 getMoveDir() const { return m_moveDir; }

	//오브젝트 제어
	void Move();								//이동방향으로 이동속도만큼 이동
	void Move(float3 moveValue);		//속도벡터만큼 이동
	virtual void Animate();					//애니메이트
	virtual void CrashCheck();				//충돌체크

	virtual void OnPrepareRender();		//랜더링 전에 동작해야 할 것들
	virtual void Render(Renderer& renderer);					//랜더링

	virtual void Update();					//업데이트
};