#include "textManager.h"

const char MAX_TEXT = 5;

void textManager::insert(DWORD time, WCHAR * msg)
{
	textMSG tmp(time, msg);
	m_message.push(tmp);
}

void textManager::draw()
{
	char size = m_message.size();

	if (size > MAX_TEXT) {
		char pop = size - MAX_TEXT;
		for (int i = 0; i < pop; ++i)
			m_message.pop();
		size = m_message.size();
	}
	
	for (int i = 0; i < size; ++i) {
		if (m_message.front().time < GetTickCount() - 2000) {
			m_message.pop();
			continue;
		}
		int tmp = size - i;
		textMSG text = m_message.front();
		m_message.pop();
		Draw_Text_D3D(text.msg, 10, screen_height - 32 * tmp - 64, D3DCOLOR_ARGB(255, 255 - tmp*20, 255 - tmp*20, 255));
		m_message.emplace(text);
	}

	/*if (bob->message_time > GetTickCount() - 2000)
		Draw_Text_D3D(bob->message, static_cast<int>(pos.x), static_cast<int>(pos.y), D3DCOLOR_ARGB(255, 200, 200, 255));*/
}
