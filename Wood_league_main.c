#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define WIDTH 17630
#define HEIGHT 9000

typedef struct s_entity		t_entity;
typedef struct s_target		t_target;
typedef struct s_hero		t_hero;

struct s_entity
{
	int	id;
	int	type;
	int	x;
	int	y;
	int	shield_life; //not defined
	int	is_controlled; //not defined
	int	health;
	int	vx;
	int	vy;
	int	near_base;
	int	threat_for;
};

struct s_target
{
	int	x;
	int	y;
    int health;
};

struct	s_hero
{
	int	id;
	int	health;
	int	mana;
};

int	main(void)
{
	int	base_x;
	int	base_y;
	scanf("%d%d", &base_x, &base_y);
	int heroes_per_player;
	scanf("%d", &heroes_per_player);
	
	while (1)
	{
		t_hero		*hero = malloc(sizeof(t_hero) * heroes_per_player);
		for (int i = 0; i < 2; i++)
		{
			int	health;
			int	mana;
			scanf("%d%d", &health, &mana);
			hero[i].health = health;
			hero[i].mana = mana;
			hero[i].id = i;
		}
		int	entity_count;
		scanf("%d", &entity_count);
		t_entity	*entity = malloc(sizeof(t_entity) * entity_count);
		for (int i = 0; i < entity_count; i++)
		{
			int id;
			int type;
			int	x;
			int	y;
			int shield_life;
			int	is_controlled;
			int	health;
			int	vx;
			int	vy;
			int	near_base;
			int	threat_for;
			scanf("%d%d%d%d%d%d%d%d%d%d%d", &id, &type, &x, &y, &shield_life, &is_controlled, &health, &vx, &vy, &near_base, &threat_for);
			entity[i].id = id;
			entity[i].type = type;
			entity[i].x = x;
			entity[i].y = y;
			entity[i].shield_life = shield_life;
			entity[i].is_controlled = is_controlled;
			entity[i].health = health;
			entity[i].vx = vx;
			entity[i].vy = vy;
			entity[i].near_base = near_base;
			entity[i].threat_for = threat_for;
		}
		for (int i = 0; i < heroes_per_player; i++)
		{
			t_target	target;
			target.x = -100000;
			target.y = -100000;
            target.health = -100000;
			if (base_x < (WIDTH / 2))
			{
				for (int a = 0; a < entity_count; a++)
				{
					if (entity[a].type == 0 && entity[a].x < (WIDTH / 4) && entity[a].y < (HEIGHT / 2.5))
					{
						if (target.x == -100000 || (entity[a].health < target.health && entity[a].x <= target.x && entity[a].y <= target.y))
						{
							target.x = entity[a].x + (entity[a].vx * 2);
							target.y = entity[a].y + (entity[a].vy * 2);
                            target.health = entity[a].health;
						}
					}
				}
			}
			else
			{
				for (int a = 0; a < entity_count; a++)
				{
					if (entity[a].type == 0 && (base_x - entity[a].x) < (WIDTH / 4) && (base_y - entity[a].y) < (HEIGHT / 2.5))
					{
						if (target.x == -100000 || (entity[a].health < target.health && (base_x - entity[a].x) <= (base_x - target.x) && (base_y - entity[a].y) <= (base_y - target.y)))
						{
							target.x = entity[a].x + (entity[a].vx * 2);
							target.y = entity[a].y + (entity[a].vy * 2);
                            target.health = entity[a].health;
						}
					}
				}
			}
			if (target.x != -100000 && (target.x < WIDTH || target.y < HEIGHT))
				printf("MOVE %d %d\n", target.x, target.y);
			else
			{
				if (base_x < WIDTH / 2)
					printf("MOVE %d %d\n", base_x + 1500, base_y + 1500);
				else
					printf("MOVE %d %d\n", base_x - 1500, base_y - 1500);
			}
		}
		free (entity);
		free (hero);
	}
	return 0;
}
