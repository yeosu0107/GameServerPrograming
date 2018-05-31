#pragma once

class CollisionMap
{
private:
	int map[300][300];
public:
	CollisionMap();
	~CollisionMap();

	void LoadCollisionFile();

	int (*getCollisionMap())[300] { return map; }
};