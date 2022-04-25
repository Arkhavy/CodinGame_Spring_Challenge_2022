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
#define MIDDLE_RADIUS 10000

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
	int		nx; //pos x + vector x
	int		ny; //pos x + vector y
	int		n2x; //pos x + (vector x * 2)
	int		n2y; //pos y + (vector y * 2)
	int		n3x; //pos x + (vector x * 3)
	int		n3y; //pos y + (vector y * 3)
	int		n4x; //pos x + (vector x * 4)
	int		n4y; //pos y + (vector y * 4)
};

/*/////////////////////////////////////////////////////////////////////////////
		FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////

//Distance maths
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
//////////////////////////////

//Other functions
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
//////////////////////////////

//Initialization of data
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
	hero->id = entity->id;
	hero->x = entity->x;
	hero->y = entity->y;
	hero->shield_life = entity->shield_life;
	hero->is_controlled = entity->is_controlled;
	entity->nx = -1;
	entity->ny = -1;
	entity->n2x = -1;
	entity->n2y = -1;
	entity->n3x = -1;
	entity->n3y = -1;
	entity->n4x = -1;
	entity->n4y = -1;
}

void	update_entity_pos(t_base *base, t_entity *entity)
{
	if (entity->type == 0) //monster init nx ny
	{
		entity->nx = entity->x + entity->vx;
		entity->ny = entity->y + entity->vy;
		entity->n2x = entity->x + (entity->vx * 2);
		entity->n2y = entity->y + (entity->vy * 2);
		entity->n3x = entity->x + (entity->vx * 3);
		entity->n3y = entity->y + (entity->vy * 3);
		entity->n4x = entity->x + (entity->vx * 4);
		entity->n4y = entity->y + (entity->vy * 4);
		dprintf(2, "nx = %d | ny = %d | ", entity->nx, entity->ny);
		dprintf(2, "n2x = %d | n2y = %d | ", entity->n2x, entity->n2y);
		dprintf(2, "n3x = %d | n3y = %d | ", entity->n3x, entity->n3y);
		dprintf(2, "n4x = %d | n4y = %d\n", entity->n4x, entity->n4y);
	}
	else //enemy hero init nx ny
	{
		entity->nx = -1;
		entity->ny = -1;
		entity->n2x = -1;
		entity->n2y = -1;
		entity->n3x = -1;
		entity->n3y = -1;
		entity->n4x = -1;
		entity->n4y = -1;
		dprintf(2, "\n");
	}
}
//////////////////////////////

//Instructions
int	move_to_base(t_base *base, int x, int y)
{
	if (!base->corner)
		return (printf("MOVE %d %d GRAOU\n", base->x + x, base->y + y) * 0);
	return (printf("MOVE %d %d GRAOU\n", base->x - x, base->y - y) * 0);
}

int	move_to_enemy_base(t_base *base, int x, int y)
{
	if (!base->corner)
		return (printf("MOVE %d %d GRAOU\n", base->ex - x, base->ey - y) * 0);
	return (printf("MOVE %d %d GRAOU\n", base->ex + x, base->ey + y) * 0);
}

int	spell_wind(t_base *base, int x, int y)
{
	if (base->mana >= 10)
	{
		base->mana -= 10;
		return (printf("SPELL WIND %d %d OUAF\n", x, y) * 0);
	}
	return (1);
}

int	spell_control(t_base *base, t_entity *entity, int x, int y)
{
	if (base->mana >= 10 && entity->type != 1)
	{
		base->mana -= 10;
		entity->is_controlled = 1;
		return (printf("SPELL CONTROL %d %d %d WOLOLO\n", entity->id, x, y) * 0);
	}
	return (1);
}

int	spell_shield(t_base *base, t_entity *entity, t_hero *hero)
{
	if (base->mana >= 10 && !entity->shield_life)
	{
		base->mana -= 10;
		if (entity->type == 2)
		{
			hero->shield_life = 12;
			return (printf("SPELL SHIELD %d MEOW\n", hero->id) * 0);
		}
		entity->shield_life = 12;
		return (printf("SPELL SHIELD %d MEOW\n", entity->id) * 0);
	}
	return (1);
}

int	move_to_target(t_base *base, t_entity entity, t_hero *hero)
{
	// dprintf(2, "\nhero id = %d\n", hero->id);
	// dprintf(2, "index = %d | id = %d | type = %d\n", entity.index, entity.id, entity.type);
	// dprintf(2, "x = %d | y = %d | distance = %f\n", entity.x, entity.y, get_distance(entity.x, entity.y, base->x, base->y, AGRO_RADIUS));
	// dprintf(2, "shield_life = %d | is_controlled = %d | health = %d\n", entity.shield_life, entity.is_controlled, entity.health);
	// dprintf(2, "vx = %d | vy = %d\n", entity.vx, entity.vy);
	// dprintf(2, "near_base = %d | threat_for = %d\n", entity.near_base, entity.threat_for);
	// dprintf(2, "nx = %d | ny = %d\n", entity.nx, entity.ny);
	return (printf("MOVE %d %d GRRR\n", entity.nx, entity.ny) * 0);
}
//////////////////////////////

/*/////////////////////////////////////////////////////////////////////////////
		HEROES
*//////////////////////////////////////////////////////////////////////////////

//Farm routine
int	farm_routine(t_base *base, t_entity *entity, t_hero *hero)
{
	return (0);
}
//////////////////////////////

//Patrol routine
int	patrol_routine(t_base *base, t_entity *entity, t_hero *hero)
{
}
//////////////////////////////

//Hero zero AI
int	hero_zero(t_base *base, t_entity *entity, t_hero *hero)
{
	return (0);
}
//////////////////////////////

//Hero one AI
int	hero_one(t_base *base, t_entity *entity, t_hero *hero)
{
	return (0);
}
//////////////////////////////

//Hero two AI
int	hero_two(t_base *base, t_entity *entity, t_hero *hero)
{
	return (0);
}
//////////////////////////////

/*/////////////////////////////////////////////////////////////////////////////
		MAIN
*//////////////////////////////////////////////////////////////////////////////

int	main(void)
{
	t_base		base;
	t_entity	*entity;
	t_hero		*hero;
	int			a;

	scanf("%d%d%d", &base.x, &base.y, &base.heroes_per_player);
	init_base_position(&base);
	hero = malloc(sizeof(t_hero) * base.heroes_per_player);
	while (42)
	{
		a = 0;
		//base init
		scanf("%d%d", &base.health, &base.mana);
		scanf("%d%d", &base.ehealth, &base.emana);
		scanf("%d", &base.entity_count);
		// dprintf(2, "h = %d | m = %d | ", base.health, base.mana);
		// dprintf(2, "eh = %d | em = %d\n", base.ehealth, base.emana);
		// dprintf(2, "entity count = %d\n", base.entity_count);
		entity = malloc(sizeof(t_entity) * base.entity_count);
		for (int i = 0; i < base.entity_count; i++) //entity init
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			dprintf(2, "count %d | ", i);
			dprintf(2, "id = %d | type = %d | ", entity[i].id, entity[i].type);
			dprintf(2, "x = %d | y = %d | distance = %f | ", entity[i].x, entity[i].y, get_distance(entity[i].x, entity[i].y, base.x, base.y, AGRO_RADIUS));
			dprintf(2, "shield_life = %d | is_controlled = %d | health = %d | ", entity[i].shield_life, entity[i].is_controlled, entity[i].health);
			dprintf(2, "vx = %d | vy = %d | ", entity[i].vx, entity[i].vy);
			dprintf(2, "near_base = %d | threat_for = %d | ", entity[i].near_base, entity[i].threat_for);
			entity[i].index = i;
			if (entity[i].type == 1) //hero init
			{
				update_heroes_data(base, &hero[a], &entity[i]);
				a++;
				dprintf(2, "\n");
			}
			else
				update_entity_pos(&base, &entity[i]);
		}
		for (int i = 0; i < base.heroes_per_player; i++)
		{
			if (i == 0)
			{
				// printf("WAIT ZZZZ\n");
				if (hero_zero(&base, entity, &hero[0]))
					move_to_base(&base, 10000, 2000);
			}
			else if (i == 1)
			{
				printf("WAIT ZZZZ\n");
				// if (hero_one(&base, entity, &hero[1]))
				// 	move_to_base(&base, 3500, 2500);
			}
			else if (i == 2)
			{
				printf("WAIT ZZZZ\n");
				// if (hero_two(&base, entity, &hero[2]))
				// 	move_to_base(&base, 1000, 5000);
			}
			else
				move_to_base(&base, 3500, 3500);
		}
		free (entity);
		base.time++;
	}
	free (hero);
	return (0);
}