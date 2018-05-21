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
	float3 m_Pos;		//위치
	float4 m_Color;	//색깔
	float4 m_teamColor; //팀칼라
	float m_Size;		//크기
	float m_Weight;	//무게
	
	UINT m_texIndex;		//텍스쳐
	bool	m_IsAnimate;		//애니메이션이 있는가?
	int		m_CurrXSeq;		//현재x시퀸스
	int		m_CurrYSeq;		//현재y시퀸스
	int		m_MaxXSeq;		//최대x시퀸스
	int		m_MaxYSeq;		//최대y시퀸스
	int		m_aniState;		//방향 상태
	int		m_divide;			//애니메이션 나누기
	float	m_aniTime;

	float m_RenderLevel; //랜더링 순서 (작을수록 위에 그려짐)

	int m_id;

	bool m_Live;		//게임상에 표현되는 여부 (true - 표현해라, false -  지워진 상태)

	bool m_isLifeGuage; //라이프게이지 표시 유무
	float m_LifeGuage;	//라이프게이지
	int m_FullLife;		//최대 HP
	int m_Life;			//현재 HP
	char m_buf[10];	//라이프게이지 텍스트

	int m_type;			//오브젝트 타입

	DWORD m_PrevTime; 

public:
	Objects(float3 pos, float4 color, float size, float weight);
	//좌표, RGB , 사이즈, 질량, 이름, 이동방향, 속도, 생명력
	~Objects();

	//변수값 저장
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

	//변수 값 불러오기
	float3 getPos() const { return m_Pos; }
	float getSize() const { return m_Size; }
	float getWeight() const { return m_Weight; }
	int getType() const { return m_type; }

	bool getLive() const { return m_Live; }
	int getLife() const { return m_Life; }

	int getID() const { return m_id; }
	int getTexIndex() const { return m_texIndex; }

	//오브젝트 제어
	void Move(float ElapsedTime, float3 moveValue);		//속도벡터만큼 이동

	virtual void Animate();					//애니메이트

	virtual void OnPrepareRender();		//랜더링 전에 동작해야 할 것들
	virtual void Render(Renderer& renderer);					//랜더링

	virtual bool Update(float ElapsedTime);					//업데이트
};
