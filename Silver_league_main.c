/*/////////////////////////////////////////////////////////////////////////////
		INCLUDES & DEFINES
*//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define WIDTH 17630
#define HEIGHT 9000
#define WIND_RADIUS 1280
#define MOVE_RADIUS 1600
#define VIEW_RADIUS 2200
#define PROBLEM_RADIUS 2500
#define AGRO_RADIUS 5000
#define MENACE_RADIUS 7500

/*/////////////////////////////////////////////////////////////////////////////
		TYPEDEF & STRUCTURES
*//////////////////////////////////////////////////////////////////////////////


typedef struct s_base	t_base;
typedef struct s_hero	t_hero;
typedef struct s_entity	t_entity;

//base and game data
struct s_base
{
	int		entity_count; //number of visible entities
	int		x; //pos x of ally base
	int		y; //pos y of ally base
	int		heroes_per_player; //number of heroes available per player
	int		health; //health of ally base
	int		mana; //mana of ally base
	int		ex; //pos x of enemy base
	int		ey; //pos y of enemy base
	int		ehealth; //health of enemy base
	int		emana; //mana of enemy base
	int		time; //number of turns since start
	int		corner; //position of ally base : top left = 0; bottom right = 1
};

//t_hero	*hero = malloc(sizeof(t_hero) * base.heroes_per_player);
struct s_hero
{
	int		id; //entity_id of hero
	int		shield_life; //countdown for shield : 12 at start, -1 per turn; while > 0 = immune to spells
	int		is_controlled; //0 if not controlled by a spell; 1 if controlled by a spell
	int		x; //pos x of hero
	int		y; //pos y of hero
};

//t_entity	*entity = malloc(sizeof(t_entity) * base.entity_count);
struct s_entity
{
	int		index; //index of entity in list
	int		id; //entity id
	int		type; //entity type : 0 = monster; 1 = ally hero; 2 = enemy hero
	int		shield_life; //countdown for shield : 12 at start, -1 per turn; while > 0 = immune to spells
	int		is_controlled; //0 if not controlled by a spell; 1 if controlled by a spell
	int		health; //current health of entity
	int		near_base; //0 if not targeting a base; 1 if targeting a base
	int		threat_for; //depending on trajectory : 0 = not a threat; 1 = threat for ally base; 2 = threat for enemy base
	int		x; //pos x of entity
	int		y; //pos y of entity
	int		vx; //vector x of entity
	int		vy; //vector y of entity
	int		nx[5]; //pos x + (vector x * (j + 1));
	int		ny[5]; //pos x + (vector y * (j + 1));
};

/*/////////////////////////////////////////////////////////////////////////////
		MATHS
*//////////////////////////////////////////////////////////////////////////////

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

float	get_distance(int target_x, int target_y, int mark_x, int mark_y, int radius)
{
	float	distance;
	float	distance_sqrt;

	distance_sqrt = sqrtf(square_distance(target_x, target_y, mark_x, mark_y));
	distance = distance_sqrt - radius;
	return (distance);
}

int	get_index_closest_from_base(t_base *base, t_entity entity_one, t_entity entity_two)
{
	float	distance_one = get_distance(entity_one.nx, entity_one.ny, base->x, base->y, AGRO_RADIUS);
	float	distance_two = get_distance(entity_two.nx, entity_two.ny, base->x, base->y, AGRO_RADIUS);

	if (distance_one <= distance_two)
		return (entity_one.index);
	return (entity_two.index);
}

int	get_index_closest_from_hero(t_hero *hero, t_entity entity_one, t_entity entity_two)
{
	float	distance_one = get_distance(entity_one.nx, entity_one.ny, hero->x, hero->y, VIEW_RADIUS);
	float	distance_two = get_distance(entity_two.nx, entity_two.ny, hero->x, hero->y, VIEW_RADIUS);

	if (distance_one <= distance_two)
		return (entity_one.index);
	return (entity_two.index);
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

int	get_target_closest_from_base(t_base *base, t_entity *entity)
{
	int	target = -1;

	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0 && (target == -1 || (is_in_range(entity[i].nx, entity[i].ny, base->x, base->y, AGRO_RADIUS))))
		{
			if (target == -1)
				target = entity[i].index;
			else
				target = get_index_closest_from_base(base, entity[target], entity[i]);
		}
	}
	return (target);
}

int	get_target_closest_from_hero(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;

	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0 && (target == -1 || (is_in_range(entity[i].nx, entity[i].ny, hero->x, hero->y, VIEW_RADIUS))))
		{
			if (target == -1)
				target = entity[i].index;
			else
				target = get_index_closest_from_hero(hero, entity[target], entity[i]);
		}
	}
	return (target);
}

/*/////////////////////////////////////////////////////////////////////////////
		FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////

//Instructions
int	move_to_base(t_base *base, int x, int y, char *str)
{
	if (!base->corner)
		return (printf("MOVE %d %d %s\n", base->x + x, base->y + y, str) * 0);
	return (printf("MOVE %d %d %s\n", base->x - x, base->y - y, str) * 0);
}

int	move_to_enemy_base(t_base *base, int x, int y, char *str)
{
	if (!base->corner)
		return (printf("MOVE %d %d %s\n", base->ex - x, base->ey - y, str) * 0);
	return (printf("MOVE %d %d %s\n", base->ex + x, base->ey + y, str) * 0);
}

int	spell_wind(t_base *base, int x, int y, char *str)
{
	if (base->mana >= 10)
	{
		base->mana -= 10;
		return (printf("SPELL WIND %d %d %s\n", x, y, str) * 0);
	}
	return (1);
}

int	spell_control(t_base *base, t_entity *entity, int x, int y, char *str)
{
	if (base->mana >= 10 && entity->type != 1)
	{
		base->mana -= 10;
		entity->is_controlled = 1;
		return (printf("SPELL CONTROL %d %d %d %s\n", entity->id, x, y, str));
	}
	return (1);
}

int	spell_shield(t_base *base, t_entity *entity, t_hero *hero, char *str)
{
	if (base->mana >= 10 && !entity->shield_life)
	{
		base->mana -= 10;
		if (entity->type == 2)
		{
			hero->shield_life = 12;
			return (printf("SPELL SHIELD %d %s\n", hero->id, str) * 0);
		}
		entity->shield_life = 12;
		return (printf("SPELL SHIELD %d %s\n", entity->id, str) * 0);
	}
	return (1);
}

int	move_to_target(t_base *base, t_entity entity, t_hero *hero, char *str)
{
	// dprintf(2, "\nhero id = %d\n", hero->id);
	// dprintf(2, "index = %d | id = %d | type = %d\n", entity.index, entity.id, entity.type);
	// dprintf(2, "x = %d | y = %d | distance = %f\n", entity.x, entity.y, get_distance(entity.x, entity.y, base->x, base->y, AGRO_RADIUS));
	// dprintf(2, "shield_life = %d | is_controlled = %d | health = %d\n", entity.shield_life, entity.is_controlled, entity.health);
	// dprintf(2, "vx = %d | vy = %d\n", entity.vx, entity.vy);
	// dprintf(2, "near_base = %d | threat_for = %d\n", entity.near_base, entity.threat_for);
	// dprintf(2, "nx = %d | ny = %d\n", entity.nx, entity.ny);
	return (printf("MOVE %d %d %s\n", entity.nx, entity.ny, str) * 0);
}

/*/////////////////////////////////////////////////////////////////////////////
		INITIALIZATION
*//////////////////////////////////////////////////////////////////////////////

void	init_base_position(t_base *base)
{
	if (base->x == 0 && base->y == 0)
	{
		base->ex = WIDTH;
		base->ey = HEIGHT;
		base->corner = 0;
	}
	else
	{
		base->ex = 0;
		base->ey = 0;
		base->corner = 1;
	}
	base->time = 0;
}

void	update_heroes_data(t_base base, t_hero *hero, t_entity *entity)
{
	if (!base.time)
		hero->id = entity->id;
	hero->x = entity->x;
	hero->y = entity->y;
	hero->shield_life = entity->shield_life;
	hero->is_controlled = entity->is_controlled;
	for (int j = 0; j < 5; j++)
	{
		entity->nx[j] = -1;
		entity->ny[j] = -1;
	}
}

/*/////////////////////////////////////////////////////////////////////////////
		HEROES
*//////////////////////////////////////////////////////////////////////////////



/*/////////////////////////////////////////////////////////////////////////////
		MAIN
*//////////////////////////////////////////////////////////////////////////////

int	main(void)
{
	t_base		base;
	t_entity	*entity;
	t_hero		*hero;
	int			a;

	scanf ("%d%d%d", &base.x, &base.y, &base.heroes_per_player);
	init_base_position(&base);
	hero = malloc(sizeof(t_hero) * base.heroes_per_player);
	while (42)
	{
		a = 0;
		scanf("%d%d", &base.health, &base.mana);
		scanf("%d%d", &base.ehealth, &base.emana);
		scanf("%d", base.entity_count);
		entity = malloc(sizeof(t_entity) * base.entity_count);
		for (int i = 0; i < base.entity_count; i++) //entity init
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			entity[i].index = i;
			if (entity[i].type == 1) //ally heroes init
			{
				update_heroes_data(base, &hero[a], &entity[i]);
				a++;
			}
			else if (entity[i].type == 0) //monster init
			{
				for (int j = 0; j < 5; j++)
				{
					entity[i].nx[j] = entity[i].x + (entity[i].vx * (j + 1));
					entity[i].ny[j] = entity[i].y + (entity[i].vy * (j + 1));
				}
			}
			else //enemy heroes init
			{
				for (int j = 0; j < 5; j++)
				{
					entity[i].nx[j] = -1;
					entity[i].ny[j] = -1;
				}
			}
		}
		if (hero_zero(&base, entity, &hero[0]))
			move_to_base(&base, 5000, 1000, "WUT 0");
		if (hero_one(&base, entity, &hero[1]))
			move_to_base(&base, 3500, 2500, "WUT 1");
		if (hero_two(&base, entity, &hero[2]))
			move_to_base(&base, 1000, 5000, "WUT 2");
		free (entity);
		base.time++;
	}
	free (hero);
	return (0);
}