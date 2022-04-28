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
		for (int i = 0; i < base.entity_count; i++)
		{
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].x, &entity[i].y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].vx, &entity[i].vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			entity[i].index = i;
			if (entity[i].type == 1)
			{
				update_heroes_data(base, &hero[a], &entity[i]);
				a++;
			}
			else if (entity[i].type == 0)
			{
				for (int j = 0; j < 5; j++)
				{
					entity[i].nx[j] = entity[i].x + (entity[i].vx * (j + 1));
					entity[i].ny[j] = entity[i].y + (entity[i].vy * (j + 1));
				}
			}
			else
			{
				for (int j = 0; j < 5; j++)
				{
					entity[i].nx[j] = -1;
					entity[i].ny[j] = -1;
				}
			}
		}
	}
}