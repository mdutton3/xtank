/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** radar.c
*/

/*
$Author: lidl $
$Id: radar.c,v 1.1.1.1 1995/02/01 00:25:37 lidl Exp $
*/

#include "malloc.h"
#include "xtank.h"
#include "graphics.h"
#include "gr.h"
#include "map.h"
#include "vehicle.h"
#include "globals.h"
#include "bullet.h"
#include "proto.h"
#ifdef SOUND
#include "sound.h"
#endif /*SOUND*/

extern Map real_map;

/* Names for radar blip life numbers */
#define RADAR_born	23
#define RADAR_size5	20
#define RADAR_size4	17
#define RADAR_size3	14
#define RADAR_size2	11
#define RADAR_size1	8
#define RADAR_dead	5

#define draw_number(v,loc) \
  draw_char(loc,(char) ('0' + v->number),v->color)

/*
** Machine dependent values here based on MAP_BOX_SIZE = 9.
*/
static Coord radar_sweeper[24] =
{
	{63, -7},
	{63, 7},
	{58, 24},
	{49, 39},
	{39, 49},
	{24, 58},
	{7, 63},
	{-7, 63},
	{-24, 58},
	{-39, 49},
	{-49, 39},
	{-58, 24},
	{-63, 7},
	{-63, -7},
	{-58, -24},
	{-49, -39},
	{-39, -49},
	{-24, -58},
	{-7, -63},
	{7, -63},
	{24, -58},
	{39, -49},
	{49, -39},
	{58, -24}
};

static Coord radar_swept[24][8] =
{
	{
		{1, 0},
		{2, 0},
		{3, 0},
		{4, 0},
		{5, 0},
		{6, 0},
		{7, 0}},
	{
		{3, 1},
		{4, 1},
		{5, 1},
		{6, 1},
		{7, 1},
		{5, 2},
		{6, 2},
		{7, 2}},
	{
		{2, 1},
		{3, 2},
		{4, 2},
		{4, 3},
		{5, 3},
		{6, 3},
		{6, 4}},
	{
		{1, 1},
		{2, 2},
		{3, 3},
		{4, 4},
		{5, 4},
		{4, 5},
		{5, 5}},
	{
		{1, 2},
		{2, 3},
		{2, 4},
		{3, 4},
		{3, 5},
		{3, 6},
		{4, 6}},
	{
		{1, 3},
		{1, 4},
		{1, 5},
		{1, 6},
		{1, 7},
		{2, 5},
		{2, 6},
		{2, 7}},

	{
		{0, 1},
		{0, 2},
		{0, 3},
		{0, 4},
		{0, 5},
		{0, 6},
		{0, 7}},
	{
		{-1, 3},
		{-1, 4},
		{-1, 5},
		{-1, 6},
		{-1, 7},
		{-2, 5},
		{-2, 6},
		{-2, 7}},
	{
		{-1, 2},
		{-2, 3},
		{-2, 4},
		{-3, 4},
		{-3, 5},
		{-3, 6},
		{-4, 6}},
	{
		{-1, 1},
		{-2, 2},
		{-3, 3},
		{-4, 4},
		{-5, 4},
		{-4, 5},
		{-5, 5}},
	{
		{-2, 1},
		{-3, 2},
		{-4, 2},
		{-4, 3},
		{-5, 3},
		{-6, 3},
		{-6, 4}},
	{
		{-3, 1},
		{-4, 1},
		{-5, 1},
		{-6, 1},
		{-7, 1},
		{-5, 2},
		{-6, 2},
		{-7, 2}},

	{
		{-1, 0},
		{-2, 0},
		{-3, 0},
		{-4, 0},
		{-5, 0},
		{-6, 0},
		{-7, 0}},
	{
		{-3, -1},
		{-4, -1},
		{-5, -1},
		{-6, -1},
		{-7, -1},
		{-5, -2},
		{-6, -2},
		{-7, -2}},
	{
		{-2, -1},
		{-3, -2},
		{-4, -2},
		{-4, -3},
		{-5, -3},
		{-6, -3},
		{-6, -4}},
	{
		{-1, -1},
		{-2, -2},
		{-3, -3},
		{-4, -4},
		{-5, -4},
		{-4, -5},
		{-5, -5}},
	{
		{-1, -2},
		{-2, -3},
		{-2, -4},
		{-3, -4},
		{-3, -5},
		{-3, -6},
		{-4, -6}},
	{
		{-1, -3},
		{-1, -4},
		{-1, -5},
		{-1, -6},
		{-1, -7},
		{-2, -5},
		{-2, -6},
		{-2, -7}},

	{
		{0, -1},
		{0, -2},
		{0, -3},
		{0, -4},
		{0, -5},
		{0, -6},
		{0, -7}},
	{
		{1, -3},
		{1, -4},
		{1, -5},
		{1, -6},
		{1, -7},
		{2, -5},
		{2, -6},
		{2, -7}},
	{
		{1, -2},
		{2, -3},
		{2, -4},
		{3, -4},
		{3, -5},
		{3, -6},
		{4, -6}},
	{
		{1, -1},
		{2, -2},
		{3, -3},
		{4, -4},
		{5, -4},
		{4, -5},
		{5, -5}},
	{
		{2, -1},
		{3, -2},
		{4, -2},
		{4, -3},
		{5, -3},
		{6, -3},
		{6, -4}},
	{
		{3, -1},
		{4, -1},
		{5, -1},
		{6, -1},
		{7, -1},
		{5, -2},
		{6, -2},
		{7, -2}}
};

static int radar_num_swept[24] =
{
	7, 8, 7, 7, 7, 8, 7, 8, 7, 7, 7, 8, 7, 8, 7, 7, 7, 8, 7, 8, 7, 7, 7, 8
};


SpecialStatus special_radar(v, record, action)
Vehicle *v;
char *record;
unsigned int action;
{
	Radar *r;
	Blip *b;
	Coord *offset;
	unsigned int vehicle_flags;
	int init_x, init_y;
	int x, y;
	int i;

#ifndef NO_CAMO
	int veh;
	Vehicle *sv;

#endif /* !NO_CAMO */

	r = (Radar *) record;

	switch (action) {
	  case SP_update:
		  /* If there are any blips, decrement their lives and remove the
	   dead */
		  i = 0;
		  while (i < r->num_blips) {
			  b = &r->blip[i];
			  b->old_view = b->view;
			  switch (--b->life) {
				case RADAR_size5:
				case RADAR_size4:
				case RADAR_size3:
				case RADAR_size2:
				case RADAR_size1:
					b->view++;
					break;
				case RADAR_dead:
					if (i < r->num_blips - 1)
						*b = r->blip[r->num_blips - 1];
					r->num_blips--;
					i--;
					break;
			  }
			  i++;
		  }

		  /* Check the swept boxes for any vehicles */
		  init_x = v->loc->grid_x;
		  init_y = v->loc->grid_y;
		  offset = radar_swept[r->pos];

		  for (i = 0; i < radar_num_swept[r->pos]; i++) {
			  /* Make sure blip is inside the grid */
			  x = init_x + offset[i].x;
			  if (x < 0 || x >= GRID_WIDTH)
				  continue;
			  y = init_y + offset[i].y;
			  if (y < 0 || y >= GRID_HEIGHT)
				  continue;

			  vehicle_flags = real_map[x][y].flags & ANY_VEHICLE;

			  /* If there is a vehicle in this box, make a blip */
			  if (vehicle_flags) {

#ifndef NO_CAMO

#define S_THRESH 10.0

				  /*
             * This "crosses off" a vehicle out of the copy
             * of vehicle-flags obtained above if they
             * are being stealthy.
             */

				  for (veh = 0; veh < num_veh_alive; ++veh) {
					  sv = live_vehicles[veh];

					  if (sv->rcs < sv->normal_rcs)
						  clrflag(vehicle_flags, (VEHICLE_0 << sv->number));

					  if (v->loc->grid_x != sv->loc->grid_x
						  || v->loc->grid_y != sv->loc->grid_y) {

						  sv->illum[v->number].gx = v->loc->grid_x;
						  sv->illum[v->number].gy = v->loc->grid_y;

						  if (sv->rcs < sv->normal_rcs) {
							  sv->illum[v->number].color = GREEN;
						  } else
							  sv->illum[v->number].color = RED;
					  }
				  }

				  if (vehicle_flags) {
#endif /* !NO_CAMO */
#ifdef SOUND
					  play_owner(v, BLIP_SOUND);
#endif /*SOUND*/
					  b = &r->blip[r->num_blips++];
					  b->x = grid2map(x) + MAP_BOX_SIZE / 4;
					  b->y = grid2map(y) + MAP_BOX_SIZE / 4;
					  b->life = RADAR_born;
					  b->view = 0;
					  b->flags = vehicle_flags;
#ifndef NO_CAMO
				  }
#endif /* !NO_CAMO */
			  }
		  }

		  /* Increment the position counter modulo 24 */
		  if (++r->pos == 24)
			  r->pos = 0;

		  /* Save the old sweeper position, and compute the new one */
		  r->old_start_x = r->start_x;
		  r->old_start_y = r->start_y;
		  r->old_end_x = r->end_x;
		  r->old_end_y = r->end_y;
		  r->start_x = grid2map(v->loc->grid_x) + MAP_BOX_SIZE / 2;
		  r->start_y = grid2map(v->loc->grid_y) + MAP_BOX_SIZE / 2;
		  r->end_x = r->start_x + radar_sweeper[r->pos].x;
		  r->end_y = r->start_y + radar_sweeper[r->pos].y;
		  break;
	  case SP_redisplay:
		  /* redraw the sweeper line */
		  draw_line(MAP_WIN, r->old_start_x, r->old_start_y,
					r->old_end_x, r->old_end_y, DRAW_XOR, CUR_COLOR);
		  draw_line(MAP_WIN, r->start_x, r->start_y,
					r->end_x, r->end_y, DRAW_XOR, CUR_COLOR);

		  /* redraw the blips that have changed size */
		  for (i = 0; i < r->num_blips; i++) {
			  b = &r->blip[i];

			  /* Erase old view */
			  if (b->life < RADAR_born)
				  draw_filled_square(MAP_WIN, b->x, b->y, 6 - b->old_view,
									 DRAW_XOR, CUR_COLOR);

			  /* Draw new view */
			  if (b->life > RADAR_dead + 1)
				  draw_filled_square(MAP_WIN, b->x, b->y, 6 - b->view,
									 DRAW_XOR, CUR_COLOR);
		  }
		  break;
	  case SP_draw:
	  case SP_erase:
		  /* draw/erase the sweeper line */
		  draw_line(MAP_WIN, r->start_x, r->start_y,
					r->end_x, r->end_y, DRAW_XOR, CUR_COLOR);

		  /* draw/erase the blips */
		  for (i = 0; i < r->num_blips; i++) {
			  b = &r->blip[i];
#ifdef NO_BROKEN_BLIP_FIX
			  if (b->view < 5)
#else /* NO_BROKEN_BLIP_FIX */
			  if (b->view < 6 && b->life != RADAR_dead + 1)
#endif /* NO_BROKEN_BLIP_FIX */
				  draw_filled_square(MAP_WIN, b->x, b->y, 6 - b->view,
									 DRAW_XOR, CUR_COLOR);
		  }
		  break;
	  case SP_activate:
		  if (v->special[(SpecialType) NEW_RADAR].status == SP_on)
			  return SP_off;
		  /* clear the blips left over from before */
		  r->num_blips = 0;

		  /* compute the starting location of the sweeper */
		  r->start_x = grid2map(v->loc->grid_x) + (int) MAP_BOX_SIZE / 2;
		  r->start_y = grid2map(v->loc->grid_y) + (int) MAP_BOX_SIZE / 2;
		  r->end_x = r->start_x + radar_sweeper[r->pos].x;
		  r->end_y = r->start_y + radar_sweeper[r->pos].y;
		  /* make the old radar position the same as the newly computed one */
		  r->old_start_x = r->start_x;
		  r->old_start_y = r->start_y;
		  r->old_end_x = r->end_x;
		  r->old_end_y = r->end_y;

		  return SP_on;
		  break;
	  case SP_deactivate:
		  break;
	}
}

/*
** Puts numbers of vehicles on the map, and moves them around.
*/
full_radar(status)
unsigned int status;
{
	Vehicle *v;
	int i;

	if (status == ON) {
		for (i = 0; i < num_veh_alive; i++) {
			v = live_vehicles[i];
			draw_number(v, v->loc);
		}
	} else if (status == REDISPLAY) {
		for (i = 0; i < num_veh_alive; i++) {
			v = live_vehicles[i];

			/* Only draw the new number location if he is alive */
			if (tstflag(v->status, VS_is_alive)) {
				if (!(grid_equal(v->loc, v->old_loc))) {
					draw_number(v, v->old_loc);
					draw_number(v, v->loc);
				}
			} else
				draw_number(v, v->old_loc);
		}
	}
}

/*
** Xors the character c on the full map at location loc.
*/
draw_char(loc, c, color)
Loc *loc;
char c;
int color;
{
	int x, y;
	char buf[2];

	x = grid2map(loc->grid_x) + MAP_BOX_SIZE / 2;
	y = grid2map(loc->grid_y) + MAP_BOX_SIZE / 2 - 4;
	buf[0] = c;
	buf[1] = '\0';
	draw_text(MAP_WIN, x, y, buf, S_FONT, DRAW_XOR, color);
}

