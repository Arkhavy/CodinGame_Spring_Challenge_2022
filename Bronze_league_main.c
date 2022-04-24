#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define WIDTH 17630
#define HEIGHT 9000
#define WIND_RADIUS 1280
#define VIEW_RADIUS 2200
#define AGRO_RADIUS 5000
#define MENACE_RADIUS 7500
#define MOVE_RADIUS 1600

typedef struct s_base	t_base;
typedef struct s_hero	t_hero;
typedef struct s_entity	t_entity;

struct s_base
{
	int		entity_count;
	int		x;
	int		y;
	int		heroes_per_player;
	int		health;
	int		mana;
	int		ehealth;
	int		emana;
	int		ey;
	int		ex;
	int		time;
	int		corner;
};

struct s_hero
{
	int		id;
	int		shield_life;
	int		is_controlled;
	int		x;
	int		y;
};

struct s_entity
{
	int		id;
	int		type;
	int		shield_life;
	int		is_controlled;
	int		health;
	int		near_base;
	int		threat_for;
	int		x;
	int		y;
	int		vx;
	int		vy;
	int		nx;
	int		ny;
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

void	update_heroes_data(t_base base, t_hero *hero, t_entity entity)
{
	if (!base.time)
		hero->id = entity.id;
	hero->x = entity.x;
	hero->y = entity.y;
	hero->shield_life = entity.shield_life;
	hero->is_controlled = entity.is_controlled;
}

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
		if (!base.corner) //base init
		{
			scanf("%d%d", &base.health, &base.mana);
			scanf("%d%d", &base.ehealth, &base.emana);
			// dprintf(2, "h = %d | m = %d | ", base.health, base.mana);
			// dprintf(2, "eh = %d | em = %d\n", base.ehealth, base.emana);
		}
		else
		{
			scanf("%d%d", &base.ehealth, &base.emana);
			scanf("%d%d", &base.health, &base.mana);
			// dprintf(2, "h = %d | m = %d | ", base.health, base.mana);
			// dprintf(2, "eh = %d | em = %d\n", base.ehealth, base.emana);
		}
		scanf("%d", &base.entity_count);
		// dprintf(2, "entity count = %d\n", base.entity_count);
		entity = malloc(sizeof(t_entity) * base.entity_count);
		for (int i = 0; i < base.entity_count; i++) //entity init
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			// dprintf(2, "count %d | ", i);
			// dprintf(2, "id = %d | type = %d | ", entity[i].id, entity[i].type);
			// dprintf(2, "x = %d | y = %d | ", entity[i].x, entity[i].y);
			// dprintf(2, "shield_life = %d | is_controlled = %d | health = %d | ", entity[i].shield_life, entity[i].is_controlled, entity[i].health);
			// dprintf(2, "vx = %d | vy = %d | ", entity[i].vx, entity[i].vy);
			// dprintf(2, "near_base = %d | threat_for = %d | ", entity[i].near_base, entity[i].threat_for);
			if (entity[i].type == 1) //hero init
			{
				update_heroes_data(base, &hero[a], entity[i]);
				a++;
				// dprintf(2, "\n");
			}
			else
			{
				entity[i].nx = entity[i].x + entity[i].vx;
				entity[i].ny = entity[i].y + entity[i].vy;
				// dprintf(2, "nx = %d | ny = %d\n", entity[i].nx, entity[i].ny);
			}
		}
		for (int i = 0; i < base.heroes_per_player; i++)
		{
			if (i == 0)
				hero_zero(&base, entity, hero[0]);
			else if (i == 1)
				hero_one(&base, entity, hero[1]);
			else if (i == 2)
				hero_two(&base, entity, hero[2]);
			else
				printf("WAIT ZZZZ\n");
		}
		free (entity);
		base.time++;
	}
	free (hero);
	return (0);
}