#pragma once
#include <queue>
#include <vector>

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
	vector<int> expTable;
public:
	CsvMap();
	~CsvMap();

	void LoadmapFile(const char* fileName);
	void LoadSpawnPoint(const char* fileName);
	void LoadExpTable(const char* fileName);

	int (*getCollisionMap())[300] { return map; }
	queue<spawnPoint> getSpawnPoint() { return spawn; }
	vector<int> getExpTable() { return expTable; }
};