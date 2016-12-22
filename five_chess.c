#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>

//#define DEBUG

#define COMPUTER_FIRST 			1
#define HUMAN_FIRST 			2
#define BLANK 					0
#define COMPUTER 				1
#define HUMAN 					2
#define UP 						1
#define DOWN 					2
#define LEFT 					3
#define RIGHT 					4

#define ROW						23
#define COL						80

#define CHAR_COMPUTER 			'O'
#define CHAR_HUMAN 				'*'

struct point {
	int x, y;
};

void play();
int in_map(int y, int x);
int have_opposite(int y, int x, int flag, int dir, int idx);
int have_self(int y, int x, int flag, int dir, int idx, int num);
void cal_value(int y, int x, int flag);
void computer_go();
void find_max_value(int flag, int *max_value);
void undo();
int win(int y, int x, int flag);
void draw_chess(int y, int x, int flag);
void clear_cursor(struct point cursor);
void draw_cursor(struct point cursor);
int same_position();
void move_cursor(int dir);
void game_over();
void init_game();
void init();

struct point cmp, human, cmp_human, old, new;
int mode;
int map[ROW][COL];
int value_cmp[ROW][COL], value_human[ROW][COL];
int max_cmp, max_human;

struct point weiyi[4][11] = {
	{-5,0,		-4,0,		-3,0,		-2,0,		-1,0,		0,0, 
	 1,0, 		2,0,		3,0,		4,0, 		5,0},

	{-5,-5, 	-4,-4,		-3,-3,		-2,-2,		-1,-1,		0,0,
	 1,1, 		2,2,		3,3,		4,4, 		5,5},

	{0,-5, 		0,-4,		0,-3,		0,-2,		0,-1,		0,0,
	 0,1,		0,2,		0,3,		0,4, 		0,5},

	{5,-5, 		4,-4,		3,-3,		2,-2,		1,-1,		0,0,
	 -1,1,		-2,2,		-3,3,		-4,4, 		-5,5}
};

int main()
{
	init();
	play();
	return 0;
}

void play()
{
	int ch;

	while (1) {
		ch = getch();
		switch (ch) {
			case 'W':
			case 'w':
				move_cursor(UP);
				break;
			case 'S':
			case 's':
				move_cursor(DOWN);
				break;
			case 'A':
			case 'a':
				move_cursor(LEFT);
				break;
			case 'D':
			case 'd':
				move_cursor(RIGHT);
				break;
			case 'U':
			case 'u':
				undo();
				break;
			case ' ':
				draw_chess(human.y, human.x, HUMAN);
				computer_go();
				break;
			case 'R':
			case 'r':
				init_game();
				break;
			case 'Q':
			case 'q':
				game_over();
				break;
			default:
				break;
		}
	}
}

int in_map(int y, int x)
{
	return ((y >= 0) && (y <= ROW) && 
			(x >= 0) && (x <= COL));
}

int have_opposite(int y, int x, int flag, int dir, int idx)
{
	int x0, y0, i;

	for (i = idx; i < idx+5; i ++) {
		y0 = y + weiyi[dir][i].y;
		x0 = x + weiyi[dir][i].x;

		if (!in_map(y0, x0))
			return 1;

		if ( (COMPUTER == flag) && (HUMAN == map[y0][x0]) )
			return 1;
		else if ( (HUMAN == flag) && (COMPUTER == map[y0][x0]) )
			return 1;
	}

	return 0;
}

int have_self(int y, int x, int flag, int dir, int idx, int num)
{
	int x0, y0, i, cnt = 0, have = 0;

	for (i = idx; i < idx+5; i ++) {
		y0 = y + weiyi[dir][i].y;
		x0 = x + weiyi[dir][i].x;

		if (!in_map(y0, x0))
			break;

		if ( (COMPUTER == flag) && (HUMAN != map[y0][x0]) ) {
			if (COMPUTER == map[y0][x0]) {
				if (have > num) 
					break;
				have ++;
				cnt ++;
			}
			else if (BLANK == map[y0][x0]) {
				cnt ++;
			}
		}
		else if ((HUMAN == flag) && (COMPUTER != map[y0][x0])) {
			if (HUMAN == map[y0][x0]) {
				if (have > num) 
					break;
				have ++;
				cnt ++;
			}
			else if (BLANK == map[y0][x0]) {
				cnt ++;
			}
		}
	}

	if ((5 == cnt) && (num == have))
		return 1;
	else
		return 0;
}

void cal_value(int y, int x, int flag)
{
	int i, j, k, x0, y0, x1, y1, x2, y2, x3, y3;
	int one_five = 0, max = 0, sum = 0;

	for (i = 0; i < 4; i ++) {
		max = 0;

		for (j = 1; j < 6; j ++) {
			one_five = 0;

			if ( have_opposite(y, x, flag, i, j) )
				one_five = 0;
			if ( have_self(y, x, flag, i, j, 1) )
				one_five = 0;
			if ( have_self(y, x, flag, i, j, 2) )
				one_five = 1;
			if ( have_self(y, x, flag, i, j, 3) ) {
				y0 = y + weiyi[i][j].y;
				x0 = x + weiyi[i][j].x;
				y1 = y + weiyi[i][j+4].y;
				x1 = x + weiyi[i][j+4].x;
				y2 = y + weiyi[i][j+1].y;
				x2 = x + weiyi[i][j+1].x;
				y3 = y + weiyi[i][j+3].y;
				x3 = x + weiyi[i][j+3].x;
				if ((BLANK == map[y0][x0]) && 
						(BLANK == map[y1][x1])) 
					one_five = 20;
				else if ( ((BLANK == map[y0][x0]) && 
							(BLANK == map[y2][x2])) || 
						((BLANK == map[y1][x1]) && 
						 (BLANK == map[y3][x3])))
					one_five = 10;
				else
					one_five = 5;
			}
			if ( have_self(y, x, flag, i, j, 4) ) {
				y0 = y + weiyi[i][j].y;
				x0 = x + weiyi[i][j].x;
				y1 = y + weiyi[i][j+5].y;
				x1 = x + weiyi[i][j+5].x;
				y2 = y + weiyi[i][j-1].y;
				x2 = x + weiyi[i][j-1].x;
				y3 = y + weiyi[i][j+4].y;
				x3 = x + weiyi[i][j+4].x;
				if ( ((in_map(y1, x1)) && 
						(BLANK == map[y0][x0]) && 
						(BLANK == map[y1][x1])) || 
						(in_map(y2, x2) && 
						 (BLANK == map[y2][x2]) && 
						 (BLANK == map[y3][x3])) ) 
					one_five = 400;
				else if ((BLANK == map[y0][x0]) || 
						(BLANK == map[y3][x3]))
					one_five = 200;
				else
					one_five = 100;
			}
			if ( have_self(y, x, flag, i, j, 5) ) {
				one_five = 2000;
			}

			if (one_five > max)
				max = one_five;
		}

		sum += max;
	}

	if (COMPUTER == flag) 
		value_cmp[y][x] = sum;
	else if (HUMAN == flag)
		value_human[y][x] = sum;
}

void computer_go()
{
	int i, j;

	max_cmp = max_human = 0;
	memset(&cmp, 0, sizeof(struct point));
	memset(&cmp_human, 0, sizeof(struct point));
	memset(value_cmp, 0, sizeof(int)*ROW*COL);
	memset(value_human, 0, sizeof(int)*ROW*COL);

	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			if (BLANK == map[i][j]) {
				map[i][j] = COMPUTER;
				cal_value(i, j, COMPUTER);
				map[i][j] = HUMAN;
				cal_value(i, j, HUMAN);
				map[i][j] = BLANK;
			}
		}
	}

	find_max_value(COMPUTER, &max_cmp);
	find_max_value(HUMAN, &max_human);

	if (max_cmp > max_human) {
		new = (struct point){cmp.x, cmp.y};
		draw_chess(cmp.y, cmp.x, COMPUTER);
		old = (struct point){new.x, new.y};
	}
	else if (max_cmp <= max_human) {
		new = (struct point){cmp_human.x, cmp_human.y};
		draw_chess(cmp_human.y, cmp_human.x, COMPUTER);
		old = (struct point){new.x, new.y};
	}
	else {
	}

#ifdef DEBUG
	attroff(A_STANDOUT);
	move(ROW, 0);
	printw("                             ");
	move(ROW, 0);
	printw("cmp:%d, human:%d", max_cmp, max_human);
	refresh();
#endif
}

void find_max_value(int flag, int *max_value)
{
	int i, j, x, y, max = 0;

	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			if ((COMPUTER == flag) && (value_cmp[i][j] > max)) {
				max = value_cmp[i][j];
				x = j;
				y = i;
			}
			else if ((HUMAN == flag) && (value_human[i][j] > max)) {
				max = value_human[i][j];
				x = j;
				y = i;
			}
		}
	}

	*max_value = max;
	if (COMPUTER == flag) {
		cmp.x = x;
		cmp.y = y;
	}
	else if (HUMAN == flag) {
		cmp_human.x = x;
		cmp_human.y = y;
	}
}

void undo()
{
}

int win(int y, int x, int flag)
{
	int i, j, k, x0, y0, cnt = 0;

	for(i = 0; i < 4; i ++) {
		for (j = 1; j < 6; j ++) {
			cnt = 0;

			for (k = j; k < j+5; k ++) {
				y0 = y + weiyi[i][k].y;
				x0 = x + weiyi[i][k].x;

				if (!in_map(y0, x0))
					break;

				if (COMPUTER == flag) {
					if (COMPUTER == map[y0][x0]) {
						cnt ++;
						if (cnt >= 5)
							return 1;
					}
					else {
						break;
					}
				}
				else if (HUMAN == flag) {
					if (HUMAN == map[y0][x0]) {
						cnt ++;
						if (cnt >= 5)
							return 1;
					}
					else {
						break;
					}
				}
			}
		}
	}

	return 0;
}

void draw_chess(int y, int x, int flag)
{
	if (BLANK == map[y][x]) {
		clear_cursor(old);
		move(y, x);

		if (COMPUTER == flag) {
			addch(CHAR_COMPUTER);
			map[y][x] = COMPUTER;
		}
		else if (HUMAN == flag) {
			addch(CHAR_HUMAN);
			map[y][x] = HUMAN;
		}

		refresh();
		draw_cursor(new);
	}

	if (win(y, x, flag)) {
		attroff(A_STANDOUT);
		move(ROW, 0);
		printw("                             ");
		move(ROW, 0);
		if (COMPUTER == flag)
			printw("Computer Win");
		else if (HUMAN == flag)
			printw("Human Win");
		refresh();
		getch();
		init_game();
	}
}

void clear_cursor(struct point cursor)
{
	char ch;
	move(cursor.y, cursor.x);
	attroff(A_STANDOUT);
	ch = (char)inch();
	addch(ch);
	refresh();
}

void draw_cursor(struct point cursor)
{
	char ch;
	move(cursor.y, cursor.x);
	ch = (char)inch();
	attron(A_STANDOUT);
	addch(ch);
	refresh();
}

int same_position()
{
	return ((cmp.x == human.x) && (cmp.y == human.y));
}

void move_cursor(int dir)
{
	if (!same_position())
		clear_cursor(human);
	switch (dir) {
		case UP:
			if (human.y > 0)
			human.y --;
			break;
		case DOWN:
			if (human.y < ROW-1)
				human.y ++;
			break;
		case LEFT:
			if (human.x > 0)
				human.x --;
			break;
		case RIGHT:
			if (human.x < COL-1)
				human.x ++;
			break;
		default:
			break;

	}
	if (!same_position())
			draw_cursor(human);
}
/*
void select_mode()
{
	int ch, flag;

	attroff(A_STANDOUT);
	clear();
	move(0, 10);
	printw("Select Mode");
	move(3, 10);
	printw("1. Computer First (1)");
	move(5, 10);
	printw("2. Human First (2)");
	move(7, 10);
	printw("3. Quit (3)");

	flag = 1;
	while(flag) {
		ch = getch();
		switch (ch) {
			case '1':
				mode = COMPUTER_FIRST;
				flag = 0;
				break;
			case '2':
				mode = HUMAN_FIRST;
				flag = 0;
				break;
			case '3':
				game_over();
				flag = 0;
				break;
			default:
				break;
		}
	}
}
*/
void game_over()
{
	endwin();
	exit(0);
}

void init_game()
{
//	select_mode();
	clear();
	memset(map, 0, sizeof(int)*ROW*COL);
	cmp = (struct point){40, 10};
	old = (struct point){40, 10};
	new = (struct point){40, 10};
	human = (struct point){40, 11};
//	if (COMPUTER_FIRST == mode)  
		draw_chess(cmp.y, cmp.x, COMPUTER);
//	else 
//		draw_chess(human.y, human.x, HUMAN);
	draw_cursor(cmp);
//	draw_cursor(human);
	attroff(A_STANDOUT);
	move(ROW, 0);
	printw("                             ");
	refresh();
	max_cmp = max_human = 0;
	memset(&cmp, 0, sizeof(struct point));
	memset(&cmp_human, 0, sizeof(struct point));
	memset(value_cmp, 0, sizeof(int)*ROW*COL);
	memset(value_human, 0, sizeof(int)*ROW*COL);

}

void init()
{
	initscr();
	clear();
	cbreak();
	noecho();
	curs_set(0);
	srand(time(0));

	init_game();
}
