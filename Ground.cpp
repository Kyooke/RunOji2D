#include "Ground.h"
#include "Engine/Model.h"
#include "Engine/CsvReader.h"

namespace
{
	using std::vector;
	const float GROUND_WIDTH = 20.0f;
	const float GROUND_Y = 10.0f;
	const float GROUND_Z = 1.0f;
	const float GROUND_ROTATE_X = -90.0f;
	const float BLOCK_INTERVAL_X = 2.0f;
	const float BLOCK_INTERVAL_Y = 1.0f;
}

Ground::Ground(GameObject* parent)
	: GameObject(parent, "Ground"), hModel_(-1), mapWidth_(-1), mapHeight_(-1)
{
	CsvReader csvData;
	if (!csvData.Load("map.csv")) {
		return; 
	}

	mapWidth_ = (int)csvData.GetWidth();   // 列数
	mapHeight_ = (int)csvData.GetHeight(); // 行数（/2 を一旦外して安全に全高を取得）

	if (mapWidth_ <= 0 || mapHeight_ <= 0) {
		return;
	}

	// mapData_を初期化
	mapData_ = vector<vector<int>>(mapHeight_, vector<int>(mapWidth_, 0));

	for (int y = 0; y < mapHeight_; y++)
	{
		for (int x = 0; x < mapWidth_; x++)
		{
			// CsvReaderの仕様通り (x = 列, y = 行) の順で取得
			mapData_[y][x] = csvData.GetValue(x, y);
		}
	}
}

void Ground::Initialize()
{
	hModel_ = Model::Load("jimen3.fbx");
	hModelt_ = Model::Load("BrickG.fbx");
}

void Ground::Update()
{
}

void Ground::Draw()
{
	for (int i = 0; i < 3; i++) {
		transform_.position_ = { GROUND_WIDTH / 2.0f + GROUND_WIDTH * i, GROUND_Y, GROUND_Z };
		transform_.rotate_ = { GROUND_ROTATE_X, 0.0f, 0.0f };
		Model::SetTransform(hModel_, transform_);
		Model::Draw(hModel_);
	}

	for (int j = 0; j < mapHeight_; j++) {
		for (int i = 0; i < mapWidth_; i++) {
			if (mapData_[j][i] == 1) {
				Transform tr;
				tr.position_ = { i * BLOCK_INTERVAL_X, (mapHeight_ - 1 - j) * BLOCK_INTERVAL_Y, 0.0f };
				Model::SetTransform(hModelt_, tr);
				Model::Draw(hModelt_);
			}
		}
	}
}

void Ground::Release()
{
}