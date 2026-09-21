#pragma once
#include "Engine/GameObject.h"
#include <vector>

class Ground;

class Player : public GameObject
{
public:
	// プレイヤーステートと方向の列挙型
	enum PlayerState
	{
		PLAYER_IDLE,
		PLAYER_WALK,
		PLAYER_TURN
	};

	enum PlayerDirection
	{
		PLAYER_RIGHT,
		PLAYER_LEFT
	};

	Player(GameObject* parent);
	~Player() {}

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Release() override;

	bool HandleInput();
	bool UpdateTurn();
	void UpdateJump();
	void ResolveWallCollision(XMVECTOR& pos, const XMVECTOR& move);

	void SetGround(Ground* ground);

	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3 pos);

private:
	int hModel_;
	float currentSpeed;
	PlayerState pstate;
	PlayerDirection pdirection;

	Ground* ground_; // ※Player.cppで使っている名前に統一
	bool isGrounded;
	float jumpVelocity;

	std::vector<std::vector<int>> gmap;
};