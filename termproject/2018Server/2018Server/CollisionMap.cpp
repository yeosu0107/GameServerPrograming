#include "stdafx.h"
#include "CollisionMap.h"
#include "StringTokenizer.h"
#include <fstream>

CsvMap::CsvMap()
{
	LoadmapFile("myMap_collision.csv");
	LoadSpawnPoint("myMap_spawn.csv");
}

CsvMap::~CsvMap()
{
}

void CsvMap::LoadmapFile(const char* fileName)
{
	ifstream in(fileName);

	string delim = ",";
	StringTokenizer st = StringTokenizer("");
	string line;

	int xPos = 0, yPos = 0;
	while (getline(in, line)) {
		st = StringTokenizer(line, delim);
		while (st.countTokens() != 0) {
			map[yPos][xPos] = atoi(st.nextToken().c_str());
			xPos++;
		}
		xPos = 0;
		yPos++;
	}
}

void CsvMap::LoadSpawnPoint(const char * fileName)
{
	ifstream in(fileName);

	string delim = ",";
	StringTokenizer st = StringTokenizer("");
	string line;

	int xPos = 0, yPos = 0;
	while (getline(in, line)) {
		st = StringTokenizer(line, delim);
		while (st.countTokens() != 0) {
			switch (atoi(st.nextToken().c_str())) {
			case 794:
				spawn.emplace(spawnPoint(0, xPos, yPos));
				break;
			case 1018:
				spawn.emplace(spawnPoint(1, xPos, yPos));
				break;
			case 923:
				spawn.emplace(spawnPoint(2, xPos, yPos));
				break;
			case 762:
				spawn.emplace(spawnPoint(3, xPos, yPos));
				break;
			}
			xPos++;
		}
		xPos = 0;
		yPos++;
	}
}
