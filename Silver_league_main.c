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

typedef struct s_master	t_master;
typedef struct s_base	t_base;
typedef struct s_entity	t_entity;
typedef struct s_hero	t_hero;
typedef struct s_pos	t_pos;

struct s_master
{
	int		entity_count; //number of visible entities
	int		heroes_per_player; //number of heroes available per player
	int		time; //number of turns since start : max = 220
	t_base	ally; //ally base
	t_base	enemy; //enemy base
};

struct s_base
{
	int		health; //health of base
	int		mana; //mana of base
	int		x; //pos x of base
	int		y; //pos y of base
	int		corner; //corner of base : 0 = top left; 1 = bottom right
	int		h_index[3]; //list of heroes index
	int		h_id[3]; //list of heroes id
};

struct s_entity
{
	int		index; //index of entity in the list
	int		id; //id of entity
	int		type; //type of entity : 0 = monster; 1 = ally hero; 2 = enemy hero
	int		shield_life; //countdown for shield : 12 at start; -1 per turn; while > 0 = immune to spells
	int		is_controlled; //0 = not controlled by a spell; 1 = controlled by a spell
	int		health; //current health of entity
	int		near_base; //0 = not targeting a base; 1 = targeting a base
	int		threat_for; //depending on trajectory : 0 = not a threat; 1 = threat for ally base; 2 = threat for enemy base
	t_pos	pos; //pos of entity
};

struct s_pos
{
	int		x; //pos x of entity
	int		y; //pos y of entity
	int		vx; //vector x of entity
	int		vy; //vector y of entity
	int		nx[5]; //next x of entity : pos x + (vx * (i + 1))
	int		ny[5]; //next y of entity : pos y + (vy * (i + 1))
};

/*/////////////////////////////////////////////////////////////////////////////
		MATHS FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////

/*/////////////////////////////////////////////////////////////////////////////
		INSTRUCTIONS FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////

//move <x> <y>
//wait
//wind <x> <y>
//control <id> <x> <y>
//shield <id>

/*/////////////////////////////////////////////////////////////////////////////
		HEROES FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////

/*/////////////////////////////////////////////////////////////////////////////
		INIT FUNCTIONS
*//////////////////////////////////////////////////////////////////////////////
void	init_base_position(t_master *master)
{
	if (master->ally.x == 0 && master->ally.y == 0)
	{
		master->enemy.x = WIDTH;
		master->enemy.y = HEIGHT;
		master->ally.corner = 0;
		master->enemy.corner = 1;
	}
	else
	{
		master->enemy.x = 0;
		master->enemy.y = 0;
		master->ally.corner = 1;
		master->enemy.corner = 0;
	}
	master->time = 0;
}

void	init_entity_next_pos(t_entity *entity)
{
	if (entity->type == 0)
	{
		for (int i = 0; i < 5; i++)
		{
			entity->pos.nx[i] = entity->pos.x + (entity->pos.vx * (i + 1));
			entity->pos.ny[i] = entity->pos.y + (entity->pos.vy * (i + 1));
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			entity->pos.nx[i] = -1;
			entity->pos.ny[i] = -1;
		}
	}
}
/*/////////////////////////////////////////////////////////////////////////////
		MAIN
*//////////////////////////////////////////////////////////////////////////////

int	main(void)
{
	t_master	master;
	t_entity	*entity;
	int			a;
	int			b;

	scanf("%d%d%d", &master.ally.x, &master.ally.y, &master.heroes_per_player);
	init_base_position(&master);
	while (42)
	{
		a = 0;
		b = 0;
		scanf("%d%d", &master.ally.health, &master.ally.mana);
		scanf("%d%d", &master.enemy.health, &master.enemy.mana);
		scanf("%d", &master.entity_count);
		entity = malloc(sizeof(t_entity) * master.entity_count);
		for (int i = 0; i < master.entity_count; i++)
		{
			entity[i].index = i;
			scanf("%d%d", &entity[i].id, &entity[i].type);
			scanf("%d%d", &entity[i].pos.x, &entity[i].pos.y);
			scanf("%d%d%d", &entity[i].shield_life, &entity[i].is_controlled, &entity[i].health);
			scanf("%d%d", &entity[i].pos.vx, &entity[i].pos.vy);
			scanf("%d%d", &entity[i].near_base, &entity[i].threat_for);
			init_entity_next_pos(&entity[i]);
			if (entity[i].type == 1)
			{
				master.ally.h_index[a] = entity[i].index;
				master.ally.h_id[a] = entity[i].id;
				a++;
			}
			else if (entity[i].type == 2)
			{
				master.enemy.h_index[b] = entity[i].index;
				master.enemy.h_id[b] = entity[i].id;
				b++;
			}
		}
	}
	return (0);
}