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
#define PROBLEM_RADIUS 3500
#define AGRO_RADIUS 5000
#define MENACE_RADIUS 7200

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
	int		index; //entity_index of hero
	int		id; //entity_id of hero
	int		shield_life; //countdown for shield : 12 at start, -1 per turn; while > 0 = immune to spells
	int		is_controlled; //0 if not controlled by a spell; 1 if controlled by a spell
	int		x; //pos x of hero
	int		y; //pos y of hero
};

//t_entity	*entity = malloc(sizeof(t_entity) * base.entity_count);
struct s_entity
{
	int		index; //entity index in list
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

/*/////////////////////////////////////////////////////////////////////////////
		FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////
int	count_entity_in_range(t_base *base, t_entity *entity, int mark_x, int mark_y, int radius)
{
	int	in_range = 0;

	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0)
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
		if (entity[i].type == 0)
		{
			if (target == -1 || (is_in_range(entity[i].nx, entity[i].ny, base->x, base->y, AGRO_RADIUS)))
			{
				if (target == -1)
					target = entity[i].index;
				else
					target = get_index_closest_from_base(base, entity[target], entity[i]);
			}
		}
	}
	return (target);
}

int	get_target_closest_from_hero(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;

	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 0)
		{
			if (target == -1 || (is_in_range(entity[i].nx, entity[i].ny, hero->x, hero->y, VIEW_RADIUS)))
			{
				if (target == -1)
					target = entity[i].index;
				else
					target = get_index_closest_from_hero(hero, entity[target], entity[i]);
			}
		}
	}
	return (target);
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

void	update_heroes_data(t_hero *hero, t_entity *entity)
{
	hero->id = entity->id;
	hero->index = entity->index;
	hero->x = entity->x;
	hero->y = entity->y;
	hero->shield_life = entity->shield_life;
	hero->is_controlled = entity->is_controlled;
	entity->nx = -1;
	entity->ny = -1;
}

/*/////////////////////////////////////////////////////////////////////////////
		INSTRUCTIONS
*//////////////////////////////////////////////////////////////////////////////
int	move_to_base(t_base *base, int x, int y, char *str)
{
	dprintf(2, "ACTION %s\n", str);
	if (!base->corner)
	{
		printf("MOVE %d %d %s\n", base->x + x, base->y + y, str);
		return (0);
	}
	printf("MOVE %d %d %s\n", base->x - x, base->y - y, str);
	return (0);
}

int	move_to_enemy_base(t_base *base, int x, int y, char *str)
{
	dprintf(2, "ACTION %s\n", str);
	if (!base->corner)
	{
		printf("MOVE %d %d %s\n", base->ex - x, base->ey - y, str);
		return (0);
	}
	printf("MOVE %d %d %s\n", base->ex + x, base->ey + y, str);
	return (0);
}

int	spell_wind(t_base *base, int x, int y, char *str)
{
	if (base->mana >= 10)
	{
		base->mana = base->mana - 10;
		dprintf(2, "ACTION %s\n", str);
		printf("SPELL WIND %d %d %s\n", x, y, str);
		return (0);
	}
	return (1);
}

int	spell_control(t_base *base, t_entity entity, int x, int y, char *str)
{
	if (base->mana >= 10 && entity.type != 1)
	{
		base->mana = base->mana - 10;
		dprintf(2, "ACTION %s\n", str);
		printf("SPELL CONTROL %d %d %d %s\n", entity.id, x, y, str);
		return (0);
	}
	return (1);
}

int	spell_shield(t_base *base, t_entity entity, t_hero *hero, char *str, int def)
{
	if (!def)
	{
		if (base->mana >= 10 && !hero->shield_life)
		{
			base->mana = base->mana - 10;
			dprintf(2, "ACTION %s\n", str);
			printf("SPELL SHIELD %d %s\n", hero->id, str);
			return (0);
		}
	}
	else
	{
		if (base->mana >= 10 && !entity.shield_life)
		{
			base->mana = base->mana - 10;
			dprintf(2, "ACTION %s\n", str);
			printf("SPELL SHIELD %d %s\n", entity.id, str);
			return (0);
		}
	}
	return (1);
}

int	move_to_target(t_entity entity, char *str)
{
	// dprintf(2, "\nhero id = %d\n", hero->id);
	// dprintf(2, "index = %d | id = %d | type = %d\n", entity.index, entity.id, entity.type);
	// dprintf(2, "x = %d | y = %d | distance = %f\n", entity.x, entity.y, get_distance(entity.x, entity.y, base->x, base->y, AGRO_RADIUS));
	// dprintf(2, "shield_life = %d | is_controlled = %d | health = %d\n", entity.shield_life, entity.is_controlled, entity.health);
	// dprintf(2, "vx = %d | vy = %d\n", entity.vx, entity.vy);
	// dprintf(2, "near_base = %d | threat_for = %d\n", entity.near_base, entity.threat_for);
	// dprintf(2, "nx = %d | ny = %d\n", entity.nx, entity.ny);
	dprintf(2, "ACTION %s\n", str);
	printf("MOVE %d %d %s\n", entity.nx, entity.ny, str);
	return (0);
}

/*/////////////////////////////////////////////////////////////////////////////
		ROUTINES
*//////////////////////////////////////////////////////////////////////////////
//Farm routine
int	farm_routine(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;

	if (count_entity_in_range(base, entity, base->x, base->y, MENACE_RADIUS) >= 2)
		target = get_target_closest_from_base(base, entity);
	else
	{
		target = get_target_closest_from_base(base, entity);
		if (target == -1 || !is_in_range(entity[target].x, entity[target].y, hero->x, hero->y, VIEW_RADIUS))
			target = get_target_closest_from_hero(base, entity, hero);
	}
	if (target != -1 && (is_in_range(entity[target].x, entity[target].y, hero->x, hero->y, VIEW_RADIUS) || is_in_range(entity[target].x, entity[target].y, base->x, base->y, AGRO_RADIUS)))
		return (move_to_target(entity[target], "GRRR"));
	for (int i = 0; i < base->entity_count; i++)
	{
		if (entity[i].type == 2)
		{
			if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && !entity[i].shield_life && base->mana >= 10)
				return (spell_control(base, entity[i], base->ex, base->ey, "OUST"));
		}
		else if (entity[i].type == 0)
		{
			if (is_in_range(entity[i].nx, entity[i].ny, base->x, base->y, MENACE_RADIUS) && entity[i].threat_for == 1)
				return (move_to_target(entity[i], "GRRR"));
		}
	}
	return (1);
}

//Patrol routine
int	patrol_routine(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;

	// dprintf(2, "entity_count %d | mana %d | hx %d | hy %d\n", base->entity_count, base->mana, hero->x, hero->y);
	// dprintf(2, "bx %d | by %d | bex %d | bey %d\n", base->x, base->y, base->ex, base->ey);
	if (base->time > 25 && count_entity_in_range(base, entity, hero->x, hero->y, WIND_RADIUS) >= 2 && is_in_range(hero->x, hero->y, base->ex, base->ey, MENACE_RADIUS) && base->mana >= 10)
		return (spell_wind(base, base->ex, base->ey, "OUAF"));
	for (int i = 0; i < base->entity_count; i++)
	{
		// dprintf(2, "entity id %d | entity type %d | entity x %d | entity y %d\n", entity[i].id, entity[i].type, entity[i].x, entity[i].y);
		if (entity[i].type == 0)
		{
			if (entity[i].threat_for == 2 || is_in_range(entity[i].x, entity[i].y, base->ex, base->ey, MENACE_RADIUS))
			{
				if (base->time > 25 && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && base->mana >= 10)
					return (spell_wind(base, base->ex, base->ey, "OUAF"));
				else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS))
					return (move_to_target(entity[i], "AGROU"));
			}
			else if (entity[i].threat_for == 1)
			{
				if (base->time > 25 && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && !entity[i].shield_life && base->mana >= 10)
					return (spell_control(base, entity[i], base->ex, base->ey, "WOLOLO"));
				return (move_to_target(entity[i], "GRRR"));
			}
		}
		if (entity[i].type == 2)
		{
			if (base->time > 25 && !entity[i].shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && is_in_range(entity[i].x, entity[i].y, base->ex, base->ey, AGRO_RADIUS) && base->mana >= 10)
				return (spell_wind(base, base->x, base->y, "OUST"));
			else if (base->time > 25 && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, MOVE_RADIUS) && base->mana >= 10)
				return (spell_shield(base, entity[i], hero, "MEOW", 0));
			else if (base->time > 25 && !entity[i].shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && is_in_range(entity[i].x, entity[i].y, base->ex, base->ey, MENACE_RADIUS) && base->mana >= 10)
				return (spell_control(base, entity[i], base->x, base->y, "LOL"));
		}
	}
	target = get_target_closest_from_hero(base, entity, hero);
	if (target != -1 && is_in_range(entity[target].x, entity[target].y, hero->x, hero->y, VIEW_RADIUS))
		return (move_to_target(entity[target], "GRAOU"));
	return (1);
}

/*/////////////////////////////////////////////////////////////////////////////
		HEROES
*//////////////////////////////////////////////////////////////////////////////

//Hero zero AI
int	hero_zero(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;
	int	skip = 0;

	if (count_entity_in_range(base, entity, base->x, base->y, MENACE_RADIUS) > 0)
		skip = 1;
	if (base->time < 50 && skip == 0)
		return (farm_routine(base, entity, hero));
	else
	{
		if (count_entity_in_range(base, entity, hero->x, hero->y, WIND_RADIUS) >= 2 && base->mana >= 10)
			return (spell_wind(base, base->ex, base->ey, "OUAF"));
		for (int i = 0; i < base->entity_count; i++)
		{
			if (entity[i].type == 2)
			{
				if (is_in_range(entity[i].x, entity[i].y, base->x, base->y, AGRO_RADIUS) && !is_in_range(hero->x, hero->y, base->x, base->y, AGRO_RADIUS))
					return (move_to_base(base, 3000, 3000, "GROU"));
				if (!hero->shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && base->emana >= 10)
					return (spell_shield(base, entity[i], hero, "MEOW", 0));
				else if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && base->mana >= 10 && !entity[i].shield_life)
					return (spell_wind(base, base->ex, base->ey, "OUAF"));
			}
			else if (entity[i].type == 0)
			{
				if (is_in_range(entity[i].x, entity[i].y, base->x, base->y, PROBLEM_RADIUS))
				{
					if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && base->mana >= 10 && entity[i].shield_life == 0)
						return (spell_wind(base, base->ex, base->ey, "AAAAA"));
					target = get_target_closest_from_base(base, entity);
					if (target != -1 && entity[target].threat_for == 1)
						return (move_to_target(entity[target], "GRAA"));
					return (move_to_target(entity[i], "GRRR"));
				}
				else if (count_entity_in_range(base, entity, base->x, base->y, MENACE_RADIUS) >= 2)
				{
					target = get_target_closest_from_base(base, entity);
					if (target != -1 && entity[target].threat_for == 1)
						return (move_to_target(entity[target], "GRRR"));
				}
				else if (!is_in_range(entity[i].x, entity[i].y, base->x, base->y, AGRO_RADIUS) && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && entity[i].shield_life == 0 && base->mana >= 10 && entity[i].threat_for != 2)
					return (spell_control(base, entity[i], base->ex, base->ey, "WOLOLO"));
				else if (is_in_range(entity[i].x, entity[i].y, base->x, base->y, MENACE_RADIUS))
				{
					target = get_target_closest_from_base(base, entity);
					if (target != -1 && entity[target].threat_for == 1)
						return (move_to_target(entity[target], "GRRR"));
					return (move_to_target(entity[i], "GRRR"));
				}
			}
		}
	}
	return (1);
}

//Hero one AI
int	hero_one(t_base *base, t_entity *entity, t_hero *hero)
{
	int	target = -1;
	int	skip = 0;

	if (count_entity_in_range(base, entity, base->x, base->y, MENACE_RADIUS) > 0)
		skip = 1;
	if (base->time < 75 && skip == 0)
		return (farm_routine(base, entity, hero));
	else
	{
		if (count_entity_in_range(base, entity, base->x, base->y, AGRO_RADIUS) >= 2)
		{
			target = get_target_closest_from_base(base, entity);
			if (target != -1 && entity[target].threat_for == 1)
				return (move_to_target(entity[target], "GROU"));
		}
		for (int i = 0; i < base->entity_count; i++)
		{
			if (entity[i].type == 2)
			{
				if (is_in_range(entity[i].x, entity[i].y, base->x, base->y, AGRO_RADIUS) && !is_in_range(hero->x, hero->y, base->x, base->y, AGRO_RADIUS))
					return (move_to_base(base, 3000, 3000, "GROU"));
				if (!hero->shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && base->emana >= 10)
					return (spell_shield(base, entity[i], hero, "MEOW", 0));
				else if (!entity[i].shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && base->mana >= 10)
					return (spell_wind(base, base->ex, base->ey, "OUAF"));
				// else if (!entity[i].shield_life && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && base->mana >= 10)
				// 	return (spell_control(base, entity[i], base->ex, base->ey, "WOLOLO"));
			}
			else if (entity[i].type == 0)
			{
				if (count_entity_in_range(base, entity, base->x, base->y, MENACE_RADIUS) >= 2)
				{
					target = get_target_closest_from_base(base, entity);
					if (target != -1 && entity[target].threat_for != 2)
						return (move_to_target(entity[target], "GRAOU"));
				}
				if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && !is_in_range(entity[i].x, entity[i].y, base->x, base->y, AGRO_RADIUS) && entity[i].threat_for != 2 && base->mana >= 10 && !entity[i].shield_life)
					return (spell_control(base, entity[i], base->ex, base->ey, "WOLOLO"));
				else if (is_in_range(entity[i].x, entity[i].y, base->x, base->y, MENACE_RADIUS))
					return (move_to_target(entity[i], "GRRR"));
			}
		}
	}
	return (1);
}

//Hero two AI
int	hero_two(t_base *base, t_entity *entity, t_hero *hero)
{
	if (base->time < 100)
		return (patrol_routine(base, entity, hero));
	else
	{
		for (int i = 0; i < base->entity_count; i++)
		{
			if (entity[i].type == 0)
			{
				if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && entity[i].threat_for != 2 && base->mana >= 10 && !entity[i].shield_life)
					return (spell_control(base, entity[i], base->ex, base->ey, "WOLOLO"));
				if (is_in_range(entity[i].x, entity[i].y, base->ex, base->ey, MENACE_RADIUS))
				{
					if (is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, WIND_RADIUS) && !entity[i].shield_life && base->mana >= 10)
						return (spell_wind(base, base->ex, base->ey, "WOUF"));
					return (move_to_target(entity[i], "GRAOU"));
				}
			}
			else if (entity[i].type == 2)
			{
				if (!entity[i].shield_life && base->mana >= 10 && is_in_range(entity[i].x, entity[i].y, hero->x, hero->y, VIEW_RADIUS) && is_in_range(entity[i].x, entity[i].y, base->ex, base->ey, PROBLEM_RADIUS))
					return (spell_control(base, entity[i], base->x, base->y, "LOL"));
			}
		}
	}
	return (1);
}

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
	while (42) //loop start
	{
		a = 0;
		scanf("%d%d", &base.health, &base.mana);
		scanf("%d%d", &base.ehealth, &base.emana);
		scanf("%d", &base.entity_count);
		// dprintf(2, "h = %d | m = %d | ", base.health, base.mana);
		// dprintf(2, "eh = %d | em = %d\n", base.ehealth, base.emana);
		// dprintf(2, "entity count = %d\n\n", base.entity_count);
		entity = malloc(sizeof(t_entity) * base.entity_count);
		for (int i = 0; i < base.entity_count; i++) //entity init
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			entity[i].index = i;
			if (entity[i].type == 1) //hero init
			{
				update_heroes_data(&hero[a], &entity[i]);
				a++;
			}
			else if (entity[i].type == 0) //monster init nx ny
			{
				entity[i].nx = entity[i].x + entity[i].vx;
				entity[i].ny = entity[i].y + entity[i].vy;
			}
			else //enemy hero init nx ny
			{
				entity[i].nx = -1;
				entity[i].ny = -1;
			}
			// dprintf(2, "count %d | ", i);
			// dprintf(2, "id = %d | type = %d | ", entity[i].id, entity[i].type);
			// dprintf(2, "x = %d | y = %d | distance = %f\n", entity[i].x, entity[i].y, get_distance(entity[i].x, entity[i].y, base.x, base.y, AGRO_RADIUS));
			// dprintf(2, "shield_life = %d | is_controlled = %d | health = %d | ", entity[i].shield_life, entity[i].is_controlled, entity[i].health);
			// dprintf(2, "vx = %d | vy = %d\n", entity[i].vx, entity[i].vy);
			// dprintf(2, "near_base = %d | threat_for = %d | ", entity[i].near_base, entity[i].threat_for);
			// dprintf(2, "nx = %d | ny = %d\n\n", entity[i].nx, entity[i].ny);
		}
		if (hero_zero(&base, entity, &hero[0]))
			move_to_base(&base, 6000, 2300, "ZZZ0");
		if (hero_one(&base, entity, &hero[1]))
			move_to_base(&base, 2300, 6000, "ZZZ1");
		if (hero_two(&base, entity, &hero[2]))
			move_to_enemy_base(&base, 2200, 8000, "ZZZ2");
		free (entity);
		base.time++;
	} //loop end
	free (hero);
	return (0);
}