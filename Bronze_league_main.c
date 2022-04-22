#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define WIDTH 17630
#define HEIGHT 9000

typedef struct s_base	t_base;
typedef struct s_hero	t_hero;
typedef struct s_entity	t_entity;

struct s_base
{
	int		entity_count;
	int		x; //x of base
	int		y; //y of base
	int		heroes_count; //number of heroes
	int		health; //health of base
	int		mana; //mana of hero
	int		e_x; //x of enemy base
	int		e_y; //y of enemy base
	int		corner; //0 if ally base is top left, 1 if ally base is bottom right
	int		time; //time since start
};

struct s_hero
{
	int	id; //id of hero
	int	x; //x of hero
	int	y; //y of hero
	int	shield_life; //shield_life of hero
};

struct s_entity
{
	int	id; //Unique identifier for each entity
	int	type; //0 = monster, 1 = ally hero, 2 = enemy hero
	int	x; //x of entity
	int	y; //y of entity
	int	shield_life; //count down until shield spell fades
	int	is_controlled; //1 if controlled by a spell, 0 otherwise
	int	health; //remaining health
	int	vx; //direction x of entity
	int	vy; //direction y of entity
	int	near_base; //0 if no target, 1 if targeting a base
	int	threat_for; //0 if not a threat, 1 if threat for ally base, 2 if threat for enemy base
};

int	square(int nb)
{
	return (nb * nb);
}

int	square_distance(int	target_x, int target_y, int mark_x, int mark_y)
{
	int	distance_x = square(target_x - mark_x);
	int	distance_y = square(target_y - mark_y);
	return (distance_x + distance_y);
}

int	is_in_range(int target_x, int target_y, int mark_x, int mark_y, int radius)
{
	float	distance;
	float	distance_sqrt;

	distance_sqrt = sqrtf(square_distance(target_x, target_y, mark_x, mark_y));
	distance = distance_sqrt - radius;
	if (distance <= 0)
		return (1);
	return (0);
}

int	move_to_base(t_base *base, int x, int y)
{
	int	dest_x;
	int	dest_y;

	if (!base->corner)
	{
		dest_x = base->x + x;
		dest_y = base->y + y;
	}
	else
	{
		dest_x = base->x - x;
		dest_y = base->y - y;
	}
	return (printf("MOVE %d %d GRRR\n", dest_x, dest_y));
}

int	move_to_enemy_base(t_base *base, int x, int y)
{
	int	dest_x;
	int	dest_y;

	if (!base->corner)
	{
		dest_x = base->e_x - x;
		dest_y = base->e_y - y;
	}
	else
	{
		dest_x = base->e_x + x;
		dest_y = base->e_y + y;
	}
	return (printf("MOVE %d %d GRRR\n", dest_x, dest_y));
}

int	count_entity_in_range(t_base *base, t_entity *entity, int mark_x, int mark_y, int radius)
{
	int	in_range = 0;

	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0 || entity[i].type == 2)
		{
			if (is_in_range(entity[i].x, entity[i].y, mark_x, mark_y, radius))
				in_range++;
		}
	}
	return (in_range);
}

int	hero_zero(t_base *base, t_entity *entity, t_hero *hero)
{
	if (count_entity_in_range(base, entity, hero->x, hero->y, 1280) >= 2 && base->mana >= 10)
		return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0)
		{
			if (is_in_range(entity[i].x + entity[i].vx, entity[i].y + entity[i].vy, base->x, base->y, 4000))
			{
				if (is_in_range(entity[i].x + entity[i].vx, entity[i].y + entity[i].vy, base->x, base->y, 2500) && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
					return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
				return (printf("MOVE %d %d GRRR\n", entity[i].x + entity[i].vx, entity[i].y + entity[i].vy));
			}
		}
		else if (entity[i].type == 2)
		{
			if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
		}
	}
	return (move_to_base(base, 1500, 1500));
}

int	hero_one(t_base *base, t_entity *entity, t_hero *hero)
{
	if (count_entity_in_range(base, entity, hero->x, hero->y, 1280) >= 2 && base->mana >= 10)
		return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0)
		{
			if (is_in_range(entity[i].x + entity[i].vx, entity[i].y + entity[i].vy, base->x, base->y, 5000))
			{
				if (is_in_range(entity[i].x + entity[i].vx, entity[i].y + entity[i].vy, base->x, base->y, 2500) && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
					return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
				return (printf("MOVE %d %d GRRR\n", entity[i].x + entity[i].vx, entity[i].y + entity[i].vy));
			}
		}
		else if (entity[i].type == 2)
		{
			if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
		}
	}
	return (move_to_base(base, 3500, 3500));
}

int hero_two(t_base *base, t_entity *entity, t_hero *hero)
{
	if (!is_in_range(hero->x, hero->y, base->e_x, base->e_y, 8000))
		return (move_to_enemy_base(base, 3500, 2500));
	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0)
		{
			if (count_entity_in_range(base, entity, hero->x, hero->y, 1280) >= 2 && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
			else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->e_x, base->e_y));
			else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 2200))
				return (printf("MOVE %d %d GRRR\n", entity[i].x + entity[i].vx, entity[i].y + entity[i].vy));
		}
		if (entity[i].type == 2)
		{
			if (count_entity_in_range(base, entity, hero->x, hero->y, 1280) >= 2 && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->x, base->y));
			else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 1280) && base->mana >= 10)
				return (printf("SPELL WIND %d %d OUAF\n", base->x, base->y));
			else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, 2200))
				return (printf("MOVE %d %d GRRR\n", entity[i].x + entity[i].vx, entity[i].y + entity[i].vy));
		}
	}
	if (hero->shield_life > 0 && base->mana >= 10)
		return (printf("SPELL SHIELD %d MEOW\n", hero->id));
	return (move_to_enemy_base(base, 3500, 2500));
}

int	main(void)
{
	t_base		base;
	t_entity	*entity;
	t_hero		*hero;
	int			b;

	scanf("%d%d%d", &base.x, &base.y, &base.heroes_count);
	if (base.x == 0 && base.y == 0)
	{
		base.e_x = 17630;
		base.e_y = 9000;
		base.corner = 0;
	}
	else
	{
		base.e_x = 0;
		base.e_y = 0;
		base.corner = 1;
	}
	hero = malloc(sizeof(t_hero) * base.heroes_count);
	base.time = 0;
	while (1)
	{
		for (int i = 0; i < 2; i++)
		{
			hero[i].id = i;
			scanf("%d%d", &base.health, &base.mana);
		}
		scanf("%d", &base.entity_count);
		entity = malloc(sizeof(t_entity) * base.entity_count);
		b = 0;
		for (int i = 0; i < base.entity_count; i++)
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			if (entity[i].type == 1 && b < base.heroes_count)
			{
				hero[b].id = entity[i].id;
				hero[b].x = entity[i].x;
				hero[b].y = entity[i].y;
				hero[b].shield_life = entity[i].shield_life;
				b++;
			}
		}
		for (int a = 0; a < base.heroes_count; a++)
		{
			if (a == 0)
				hero_zero(&base, entity, &hero[0]);
			else if (a == 1)
				hero_one(&base, entity, &hero[1]);
			else if (a == 2)
				hero_two(&base, entity, &hero[2]);
			else
				printf("WAIT ZZZZ\n");
		}
		free (entity);
		base.time++;
	}
	free (hero);
	return (0);
}