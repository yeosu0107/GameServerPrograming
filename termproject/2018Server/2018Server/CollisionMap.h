#pragma once
#include <queue>

struct spawnPoint
{
	int type;
	int xPos;
	int yPos;
	spawnPoint(int t, int x, int y) {
		type = t;
		xPos = x;
		yPos = y;
	}
};

class CsvMap
{
private:
	int map[300][300];
	queue<spawnPoint> spawn;
public:
	CsvMap();
	~CsvMap();

	void LoadmapFile(const char* fileName);
	void LoadSpawnPoint(const char* fileName);
	int (*getCollisionMap())[300] { return map; }
	queue<spawnPoint> getSpawnPoint() { return spawn; }
};