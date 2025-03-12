#include "../../gateware-main/Gateware.h"
#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_

namespace GAME
{
	///*** Tags ***///
	struct Player {};
	struct Enemy {};
	struct Bullet {};
	struct Collidable {};
	struct Obstacle {};
	struct Destroy {};
	struct GameOver {};

	///*** Components ***///
	struct Transform {
		GW::MATH::GMATRIXF matrix;
	};

	struct GameManager {};

	struct Firing {
		float cooldown;

	};

	struct Velocity {
		GW::MATH::GVECTORF direction;
	};

	struct Collider {
		GW::MATH::GOBBF obb;
	};

	struct Health
	{
		int health;
		int maxHealth;

	};

	struct Shatter
	{
		int initialShatterCount;
		int shatterAmount;
		float shatterScale;
	};

	struct Invuln
	{
		float cooldown;
	};

	void UpdatePlayer(entt::registry& registry, entt::entity entity);

}// namespace GAME
#endif // !GAME_COMPONENTS_H_