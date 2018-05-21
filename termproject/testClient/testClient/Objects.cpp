#include "stdafx.h"
#include "Objects.h"
#include "Renderer.h"
#include "GlobalVal.h"

Objects::Objects(float3 pos, float4 color, float size, float weight) :
	m_Pos(pos), m_Color(color), m_Size(size), m_Weight(weight), m_Live(false)
{
	m_texIndex = -1;
	m_RenderLevel = 1.0f;
	m_CurrXSeq = -1;
	m_CurrYSeq = -1;
	m_MaxXSeq = -1;
	m_MaxYSeq = -1;
	m_IsAnimate = false;
	m_aniTime = 0.0f;

	m_PrevTime = 0;
	m_LifeGuage = 1;
	m_isLifeGuage = false;
	//sprintf_s(m_buf, "%d/%d", (int)m_Life, (int)m_FullLife);
}

Objects::~Objects()
{
}


void Objects::setPos(float x, float y, float z)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_Pos.z = z;
}

void Objects::setPos(float3 pos)
{
	m_Pos = pos;
}

void Objects::setColor(float x, float y, float z, float w)
{
	m_Color.x = x;
	m_Color.y = y;
	m_Color.z = z;
	m_Color.w = w;
}

void Objects::setColor(float4 color)
{
	m_Color = color;
}

void Objects::Move(float ElapsedTime, float3 moveValue)
{
	m_Pos += moveValue;
}

void Objects::Animate()
{
	if (m_IsAnimate) {
		m_aniTime += 0.1f;
		if (m_aniTime >= 0.5f) {
			m_aniTime = 0;
			m_CurrXSeq++;
			if (m_CurrXSeq > m_MaxXSeq) {
				m_CurrXSeq = 0;
				m_CurrYSeq++;

				if (m_aniState == 1) {
					if (m_CurrYSeq > m_divide-1) {
						m_CurrYSeq = 0;

						if (m_Life == 1)
							m_Life = 0; //이펙트 효과 제거
					}
				}
				else if (m_aniState == -1) {
					if (m_CurrYSeq > m_MaxYSeq-1)
						m_CurrYSeq = m_divide;

					if (m_Life == 1)
						m_Life = 0; //이펙트 효과 제거
				}
			}
		}
	}
}

void Objects::setminusLife(int tmp)
{ 
	m_Life -= tmp; 
	m_LifeGuage = (float)m_Life / (float)m_FullLife;
	sprintf_s(m_buf, "%d/%d", (int)m_Life, (int)m_FullLife);
}

void Objects::OnPrepareRender()
{

}

void Objects::Render(Renderer& g_Renderer)
{
	if (m_Live) {
		float3 pos=float3((m_Pos.x - *GlobalVal::getInstance()->getxPos() * tileSize),
			(m_Pos.y - *GlobalVal::getInstance()->getyPos() * tileSize), 0);
		OnPrepareRender();
		if(m_texIndex==-1)
			g_Renderer.DrawSolidRect(pos.x, pos.y, pos.z, m_Size,
				m_Color.x, m_Color.y, m_Color.z, m_Color.w, m_RenderLevel);
		else {
			if (m_IsAnimate) {
				g_Renderer.DrawTexturedRectSeq(pos.x, pos.y, pos.z, m_Size, m_Color.x, m_Color.y, m_Color.z, m_Color.w,
					m_texIndex, m_CurrXSeq, m_CurrYSeq, m_MaxXSeq, m_MaxYSeq, m_RenderLevel);
			}
			else {
				g_Renderer.DrawTexturedRect(pos.x, pos.y, pos.z, m_Size,
					m_Color.x, m_Color.y, m_Color.z, m_Color.w, m_texIndex, m_RenderLevel);
			}
		}

		if (m_isLifeGuage) {
			g_Renderer.DrawSolidRectGauge(pos.x, pos.y + m_Size/2 + 15.0f, pos.z, 
				m_Size, 12, m_teamColor.x, m_teamColor.y, m_teamColor.z, m_teamColor.w, m_LifeGuage, 0.0f);

			g_Renderer.DrawTextW(pos.x-m_Size/2, pos.y + m_Size / 2 + 10.0f, GLUT_BITMAP_HELVETICA_12, 1, 1, 1, m_buf);
		}
	}
}

bool Objects::Update(float ElapsedTime)
{
	if (m_Live) {
		Animate();

		DWORD currTime = timeGetTime() *0.001f;

		if (m_Life <= 0)
			m_Live = false;

		return true;
	}
	return false;
}

