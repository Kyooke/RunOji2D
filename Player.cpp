#include "Player.h"
#include "Ground.h"
#include "Engine/Model.h"
#include "Engine/Input.h"
#include "Engine/Camera.h"
#include "Engine/SphereCollider.h"

namespace
{
	const float MAX_SPEED = 0.2f;
	const float BASE_SPEED = 0.1f;
	const float ACCELERATION = 0.005f;
	const float FRICTION = 0.008f;
	const float BRAKE = 0.02f;
	const float TURN_FRAME = 10.0f;
	const float BLOCK_SIZE = 2.0f;
	const float JUMP_POWER = 0.4f;
	const float GRAVITY = 0.01f;

	const XMVECTOR P_MOVE[] = {
		{  1.0f, 0.0f, 0.0f }, 
		{ -1.0f, 0.0f, 0.0f }  
	};

	const float P_ANGLE[] = { -90.0f, 90.0f };
}

Player::Player(GameObject* parent)
	: GameObject(parent, "Player"), hModel_(-1), currentSpeed(0.0f),
	pstate(PLAYER_IDLE), pdirection(PLAYER_RIGHT), ground_(nullptr), 
	isGrounded(true), jumpVelocity(0.0f)
{
}

void Player::Initialize()
{
	hModel_ = Model::Load("Walking.fbx");
	transform_.position_ = { 15.0f, 5.0f, 0.5f }; 

	SphereCollider* collision = new SphereCollider(XMFLOAT3(0, 0.25f, 0), 0.5f);
	AddCollider(collision);
}

void Player::Update()
{
	if (pstate != PLAYER_TURN) pstate = PLAYER_IDLE;

	bool isBraking = HandleInput();

	if (UpdateTurn()) return;

	XMVECTOR pos = XMLoadFloat3(&transform_.position_);
	XMVECTOR move = XMVectorSet(0, 0, 0, 0);

	if (pstate == PLAYER_WALK)
	{
		float accel = isGrounded ? ACCELERATION : ACCELERATION * 0.5f;
		currentSpeed += accel;
		if (currentSpeed > MAX_SPEED) currentSpeed = MAX_SPEED;
		move = P_MOVE[pdirection];
		transform_.rotate_.y = P_ANGLE[pdirection];
	}
	else
	{
		if (currentSpeed > 0.0f)
		{
			float decel = isBraking ? BRAKE : FRICTION;
			currentSpeed -= decel;
			if (currentSpeed < 0.0f) currentSpeed = 0.0f;
			move = P_MOVE[pdirection];
		}
	}

	pos = pos + currentSpeed * move;
	XMStoreFloat3(&transform_.position_, pos);

	UpdateJump();

	ResolveWallCollision(pos, move);
}

void Player::Draw()
{
	Model::SetTransform(hModel_, transform_);
	Model::Draw(hModel_);
}

void Player::Release()
{
}

bool Player::HandleInput()
{
	bool isBraking = false;

	if (Input::IsKey(DIK_RIGHT))
	{
		if (pdirection == PLAYER_LEFT && currentSpeed > 0.0f) isBraking = true;
		else
		{
			pdirection = PLAYER_RIGHT;
			pstate = PLAYER_WALK;
		}
	}
	if (Input::IsKey(DIK_LEFT))
	{
		if (pdirection == PLAYER_RIGHT && currentSpeed > 0.0f) isBraking = true;
		else
		{
			pdirection = PLAYER_LEFT;
			pstate = PLAYER_WALK;
		}
	}

	if (Input::IsKeyDown(DIK_SPACE) && isGrounded)
	{
		jumpVelocity = JUMP_POWER;
		isGrounded = false;
	}

	return isBraking;
}

bool Player::UpdateTurn()
{
	return false;
}

void Player::UpdateJump()
{
	gmap = ground_->GetMapData();
	if (!gmap.empty())
	{
		int mapWidth = (int)gmap[0].size();
		int mapHeight = (int)gmap.size();
		XMFLOAT3 wpos = transform_.position_;

		int mapX = (int)((wpos.x + BLOCK_SIZE / 2.0f) / BLOCK_SIZE);

		if (isGrounded)
		{
			int mapY = mapHeight - 1 - (int)((wpos.y - 0.1f) / 1.0f);

			if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight)
			{
				if (gmap[mapY][mapX] == 0)
				{
					isGrounded = false;
					jumpVelocity = 0.0f;
				}
			}

			if (isGrounded)
			{
				return;
			}
		}
	}

	if (!gmap.empty() && jumpVelocity > 0.0f)
	{
		int mapWidth = (int)gmap[0].size();
		int mapHeight = (int)gmap.size();
		XMFLOAT3 wpos = transform_.position_;
		int mapX = (int)((wpos.x + BLOCK_SIZE / 2.0f) / BLOCK_SIZE);

		int headMapY = mapHeight - 1 - (int)((wpos.y + 1.2f) / 1.0f);

		if (mapX >= 0 && mapX < mapWidth && headMapY >= 0 && headMapY < mapHeight)
		{
			if (gmap[headMapY][mapX] == 1)
			{
				float blockBottomY = (float)(mapHeight - 1 - headMapY);

				if (transform_.position_.y + 1.0f >= blockBottomY)
				{
					transform_.position_.y = blockBottomY - 1.0f;
					jumpVelocity = 0.0f;
				}
			}
		}
	}

	transform_.position_.y += jumpVelocity;
	jumpVelocity -= GRAVITY;

	if (!gmap.empty())
	{
		int mapWidth = (int)gmap[0].size();
		int mapHeight = (int)gmap.size();
		XMFLOAT3 wpos = transform_.position_;
		int mapX = (int)((wpos.x + BLOCK_SIZE / 2.0f) / BLOCK_SIZE);

		if (jumpVelocity <= 0.0f)
		{
			int mapY = mapHeight - 1 - (int)(wpos.y / 1.0f);

			if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight)
			{
				if (gmap[mapY][mapX] == 1)
				{
					float blockTopY = (float)(mapHeight - 1 - mapY) * 1.0f;

					if (wpos.y <= blockTopY + 0.5f && wpos.y >= blockTopY - 0.5f)
					{
						transform_.position_.y = blockTopY + 0.5f;
						jumpVelocity = 0.0f;
						isGrounded = true;
						return;
					}
				}
			}
		}
	}
}

void Player::ResolveWallCollision(XMVECTOR& pos, const XMVECTOR& move)
{
	gmap = ground_->GetMapData();
	if (gmap.empty()) return;

	int mapWidth = (int)gmap[0].size();
	int mapHeight = (int)gmap.size();
	XMFLOAT3 wpos = transform_.position_;

	float checkOffsetX = 0.0f;
	if (pdirection == PLAYER_LEFT)  checkOffsetX = -1.0f;
	if (pdirection == PLAYER_RIGHT) checkOffsetX = 1.0f;

	int mapX = (int)((wpos.x + checkOffsetX + BLOCK_SIZE / 2.0f) / BLOCK_SIZE);
	int mapY = mapHeight - 1 - (int)((wpos.y + 1.0f) / 1.0f);

	if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight)
	{
		if (gmap[mapY][mapX] == 1 && (pdirection == PLAYER_LEFT || pdirection == PLAYER_RIGHT))
		{
			pos = pos - currentSpeed * move;

			XMFLOAT3 newPos;
			XMStoreFloat3(&newPos, pos);
			newPos.y = transform_.position_.y;
			transform_.position_ = newPos;

			currentSpeed = 0.0f;
		}
	}
}

void Player::SetGround(Ground* ground)
{
	ground_ = ground;
}

XMFLOAT3 Player::GetPosition()
{
	return transform_.position_;
}

void Player::SetPosition(XMFLOAT3 pos)
{
	transform_.position_ = pos;
	jumpVelocity = 0.0f;
	isGrounded = true;
	currentSpeed = 0.0f;
}