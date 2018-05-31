#include "stdafx.h"
#include "CollisionMap.h"
#include "StringTokenizer.h"
#include <fstream>

CollisionMap::CollisionMap()
{
	LoadCollisionFile();
}

CollisionMap::~CollisionMap()
{
}

void CollisionMap::LoadCollisionFile()
{
	ifstream in("myMap_collision.csv");

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
