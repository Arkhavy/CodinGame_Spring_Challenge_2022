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
typedef struct s_pos	t_pos;

struct s_base
{
	int		entity_count;
	int		x;
	int		y;
	int		heroes_per_player;
	int		mana;
	int		health;
	int		ey;
	int		ex;
	int		time;
	int		corner;
	int		radius;
};

struct s_hero
{
	int		id;
	int		shield_life;
	int		radius;
	t_pos	pos;
};

struct s_entity
{
	int		id;
	int		type;
	int		shield_life;
	int		is_controlled;
	int		max_health;
	int		health;
	int		near_base;
	int		threat_for;
	t_pos	pos;
};

struct s_pos
{
	int		x;
	int		y;
	int		vx;
	int		vy;
	int		next_x;
	int		next_y;
};

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
	base->radius = 5000;
}

void	update_heroes_data(t_base base, t_hero *hero, t_entity entity)
{
	if (!base.time)
	{
		hero->id = entity.id;
		hero->pos.vx = -1;
		hero->pos.vy = -1;
		hero->pos.next_x = -1;
		hero->pos.next_y = -1;
		hero->radius = 2200;
	}
	hero->pos.x = entity.pos.x;
	hero->pos.y = entity.pos.y;
	hero->shield_life = entity.shield_life;
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
	init_heroes_position(base, &hero);
	while (42)
	{
		a = 0;
		for (int i = 0; i < 2; i++) //base initialization
			scanf("%d%d", &base.health, &base.mana);
		scanf("%d", &base.entity_count);
		for (int i = 0; i < base.entity_count; i++) //entity initialization
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].pos.x, &entity[i].pos.y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].pos.vx, &entity[i].pos.vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			if (entity[i].type == 1)
			{
				update_heroes_data(base, &hero[a], entity[i]);
				a++;
			}
		}
		for (int i = 0; i < base.heroes_per_player; i++)
		{
			printf("WAIT\n");
		}
	}
}