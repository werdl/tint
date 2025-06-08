
/*
 * Copyright (c) Abraham vd Merwe <abz@blio.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <curses.h>
#include <wchar.h>
#include <limits.h>
#include <sys/time.h>

#ifndef bool
#define bool int
#endif

#if !defined(false) || (false != 0)
#define false	0
#endif

#if !defined(true) || (true != 0)
#define true	1
#endif

#if !defined(FALSE) || (FALSE != false)
#define FALSE	false
#endif

#if !defined(TRUE) || (TRUE != true)
#define TRUE	true
#endif

/*
 * Error flags
 */

#if !defined(ERR) || (ERR != -1)
#define ERR		-1
#endif

#if !defined(OK) || (OK != 0)
#define OK		0
#endif

/*
 * Initialize random number generator
 */
void rand_init ()
{
#ifdef USE_RAND
   srand (time (NULL));
#else
   srandom (time (NULL));
#endif
}

/*
 * Generate a random number within range
 */
int rand_value (int range)
{
#ifdef USE_RAND
   return ((int) ((float) range * rand () / (RAND_MAX + 1.0)));
#else
   return (random () % range);
#endif
}

/*
 * Convert an str to long. Returns TRUE if successful,
 * FALSE otherwise.
 */
bool str2int (int *i,const char *str)
{
   char *endptr;
   *i = strtol (str,&endptr,0);
   if (*str == '\0' || *endptr != '\0' || *i == LONG_MIN || *i == LONG_MAX || *i < INT_MIN || *i > INT_MAX) return FALSE;
   return TRUE;
}

/*
 * Colors
 */

#define COLOR_BLACK     0                        /* Black */
#define COLOR_RED       1                        /* Red */
#define COLOR_GREEN     2                        /* Green */
#define COLOR_YELLOW    3                        /* Yellow */
#define COLOR_BLUE      4                        /* Blue */
#define COLOR_MAGENTA   5                        /* Magenta */
#define COLOR_CYAN      6                        /* Cyan */
#define COLOR_WHITE     7                        /* White */

/*
 * Attributes
 */

#define ATTR_OFF        0                        /* All attributes off */
#define ATTR_BOLD       1                        /* Bold On */
#define ATTR_DIM        2                        /* Dim (Is this really in the ANSI standard? */
#define ATTR_UNDERLINE  4                        /* Underline (Monochrome Display Only */
#define ATTR_BLINK      5                        /* Blink On */
#define ATTR_REVERSE    7                        /* Reverse Video On */
#define ATTR_INVISIBLE  8                        /* Concealed On */

/*
 * Init & Close
 */

/* Initialize screen */
void io_init ();

/* Restore original screen state */
void io_close ();

/*
 * Output
 */

/* Set color attributes */
void out_setattr (int attr);

/* Set color */
void out_setcolor (int fg,int bg);

/* Move cursor to position (x,y) on the screen. Upper corner of screen is (0,0) */
void out_gotoxy (int x,int y);

/* Put a character on the screen */
void out_putch (char ch);

/* Write a string to the screen */
void out_printf (char *format, ...);

/* Refresh screen */
void out_refresh ();

/* Get the screen width */
int out_width ();

/* Get the screen height */
int out_height ();

/* Beep */
void out_beep ();

/*
 * Input
 */

/* Read a character */
int in_getch ();

/* Set keyboard timeout in microseconds */
void in_timeout (int delay);

/* Empty keyboard buffer */
void in_flush ();


/* Number of colors defined in io.h */
#define NUM_COLORS	8

/* Number of attributes defined in io.h */
#define NUM_ATTRS	9

/* Cursor definitions */
#define CURSOR_INVISIBLE	0
#define CURSOR_NORMAL		1

/* Maps color definitions onto their real definitions */
static int color_map[NUM_COLORS];

/* Maps attribute definitions onto their real definitions */
static int attr_map[NUM_ATTRS];

/* Current attribute used on screen */
static int out_attr;

/* Current color used on screen */
static int out_color;

/* This is the timeout in microseconds */
static int in_timetotal;

/* This is the amount of time left to before a timeout occurs (in microseconds) */
static int in_timeleft;

/*
 * Init & Close
 */

/* Initialize screen */
void io_init ()
{
   initscr ();
   start_color ();
   curs_set (CURSOR_INVISIBLE);
   out_attr = A_NORMAL;
   out_color = COLOR_WHITE;
   noecho ();
   /* Map colors */
   color_map[COLOR_BLACK] = COLOR_BLACK;
   color_map[COLOR_RED] = COLOR_RED;
   color_map[COLOR_GREEN] = COLOR_GREEN;
   color_map[COLOR_YELLOW] = COLOR_YELLOW;
   color_map[COLOR_BLUE] = COLOR_BLUE;
   color_map[COLOR_MAGENTA] = COLOR_MAGENTA;
   color_map[COLOR_CYAN] = COLOR_CYAN;
   color_map[COLOR_WHITE] = COLOR_WHITE;
   /* Map attributes */
   attr_map[ATTR_OFF] = A_NORMAL;
   attr_map[ATTR_BOLD] = A_BOLD;
   attr_map[ATTR_DIM] = A_DIM;
   attr_map[ATTR_UNDERLINE] = A_UNDERLINE;
   attr_map[ATTR_BLINK] = A_BLINK;
   attr_map[ATTR_REVERSE] = A_REVERSE;
   attr_map[ATTR_INVISIBLE] = A_INVIS;

  keypad(stdscr, TRUE);
}

/* Restore original screen state */
void io_close ()
{
   echo ();
   attrset (A_NORMAL);
   clear ();
   curs_set (CURSOR_NORMAL);
   refresh ();
   endwin ();
}

/*
 * Output
 */

/* Set color attributes */
void out_setattr (int attr)
{
   out_attr = attr_map[attr];
}

/* Set color */
void out_setcolor (int fg,int bg)
{
   out_color = (color_map[bg] << 3) + color_map[fg];
   init_pair (out_color,color_map[fg],color_map[bg]);
   attrset (COLOR_PAIR (out_color) | out_attr);
}

/* Move cursor to position (x,y) on the screen. Upper corner of screen is (0,0) */
void out_gotoxy (int x,int y)
{
   move (y,x);
}

/* Put a character on the screen */
void out_putch (char ch)
{
   addch (ch);
}

/* Put a unicode character on the screen */
/* Put a string on the screen */
void out_printf (char *format, ...)
{
   va_list ap;
   va_start (ap,format);
   vwprintw (stdscr,format,ap);
   va_end (ap);
}

/* Refresh screen */
void out_refresh ()
{
   refresh ();
}

/* Get the screen width */
int out_width ()
{
   return COLS;
}

/* Get the screen height */
int out_height ()
{
   return LINES;
}

/* Beep */
void out_beep ()
{
   beep ();
}

/*
 * Input
 */

/* Read a character. Please note that you MUST call in_timeout() before in_getch() */
int in_getch ()
{
   struct timeval starttv,endtv;
   int ch;
   timeout (in_timeleft / 1000);
   gettimeofday (&starttv,NULL);
   ch = getch ();
   gettimeofday (&endtv,NULL);
   /* Timeout? */
   if (ch == ERR)
	 in_timeleft = in_timetotal;
   /* No? Then calculate time left */
   else
	 {
		endtv.tv_sec -= starttv.tv_sec;
		endtv.tv_usec -= starttv.tv_usec;
		if (endtv.tv_usec < 0)
		  {
			 endtv.tv_usec += 1000000;
			 endtv.tv_sec--;
		  }
		in_timeleft -= endtv.tv_usec;
		if (in_timeleft <= 0) in_timeleft = in_timetotal;
	 }
   return ch;
}

/* Set keyboard timeout in microseconds */
void in_timeout (int delay)
{
   /* ncurses timeout() function works with milliseconds, not microseconds */
   in_timetotal = in_timeleft = delay;
}

/* Empty keyboard buffer */
void in_flush ()
{
   flushinp ();
}

const char scorefile[] = "/var/games/tint.scores";
/*
 * Macros
 */

/* Number of shapes in the game */
#define NUMSHAPES	7

/* Number of blocks in each shape */
#define NUMBLOCKS	4

/* Number of rows and columns in board */
#define NUMROWS	23
#define NUMCOLS	13

/* Wall id - Arbitrary, but shouldn't have the same value as one of the colors */
#define WALL 16

/*
 * Type definitions
 */

typedef int board_t[NUMCOLS][NUMROWS];

typedef struct
{
   int x,y;
} block_t;

typedef struct
{
   int color;
   int type;
   bool flipped;
   block_t block[NUMBLOCKS];
} shape_t,shapes_t[NUMSHAPES];

typedef struct
{
   int moves;
   int rotations;
   int dropcount;
   int efficiency;
   int droppedlines;
   int currentdroppedlines;
} status_t;

typedef struct engine_struct
{
   bool shadow;                                     /* show shadow */
   int curx,cury,curx_shadow,cury_shadow;			/* coordinates of current piece */
   int curshape,nextshape;							/* current & next shapes */
   int score;										/* score */
   int bag_iterator;								/* iterator for randomized bag */
   int bag[NUMSHAPES];								/* pointer to bag of shapes */
   shapes_t shapes;									/* shapes */
   board_t board;									/* board */
   status_t status;									/* current status of shapes */
   void (*score_function)(struct engine_struct *);	/* score function */
} engine_t;

typedef enum { ACTION_LEFT, ACTION_ROTATE, ACTION_RIGHT, ACTION_DROP, ACTION_DOWN } action_t;

/*
 * Global variables
 */

extern const shapes_t SHAPES;

/*
 * Functions
 */

/*
 * Initialize specified tetris engine
 */
void engine_init (engine_t *engine,void (*score_function)(engine_t *));

/*
 * Perform the given action on the specified tetris engine
 */
void engine_move (engine_t *engine,action_t action);

/*
 * Evaluate the status of the specified tetris engine
 *
 * OUTPUT:
 *   1 = shape moved down one line
 *   0 = shape at bottom, next one released
 *  -1 = game over (board full)
 */
int engine_evaluate (engine_t *engine);

/*
 * Global variables
 */

const shapes_t SHAPES =
{
   { COLOR_CYAN,    0, FALSE, { {  1,  0 }, {  0,  0 }, {  0, -1 }, { -1, -1 } } },
   { COLOR_GREEN,   1, FALSE, { {  1, -1 }, {  0, -1 }, {  0,  0 }, { -1,  0 } } },
   { COLOR_YELLOW,  2, FALSE, { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  0,  1 } } },
   { COLOR_BLUE,    3, FALSE, { { -1, -1 }, {  0, -1 }, { -1,  0 }, {  0,  0 } } },
   { COLOR_MAGENTA, 4, FALSE, { { -1,  1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } },
   { COLOR_WHITE,   5, FALSE, { {  1,  1 }, {  1,  0 }, {  0,  0 }, { -1,  0 } } },
   { COLOR_RED,     6, FALSE, { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  2,  0 } } }
};

/*
 * Functions
 */

/* This rotates a shape */
static void real_rotate (shape_t *shape,bool clockwise)
{
   int i,tmp;
   if (clockwise)
	 {
		for (i = 0; i < NUMBLOCKS; i++)
		  {
			 tmp = shape->block[i].x;
			 shape->block[i].x = -shape->block[i].y;
			 shape->block[i].y = tmp;
		  }
	 }
   else
	 {
		for (i = 0; i < NUMBLOCKS; i++)
		  {
			 tmp = shape->block[i].x;
			 shape->block[i].x = shape->block[i].y;
			 shape->block[i].y = -tmp;
		  }
	 }
}

/* Rotate shapes the way tetris likes it (= not mathematically correct) */
static void fake_rotate (shape_t *shape)
{
   switch (shape->type)
	 {
	  case 0:	/* Just rotate this one anti-clockwise and clockwise */
		if (shape->flipped) real_rotate (shape,TRUE); else real_rotate (shape,FALSE);
		shape->flipped = !shape->flipped;
		break;
	  case 1:	/* Just rotate these two clockwise and anti-clockwise */
	  case 6:
		if (shape->flipped) real_rotate (shape,FALSE); else real_rotate (shape,TRUE);
		shape->flipped = !shape->flipped;
		break;
	  case 2:	/* Rotate these three anti-clockwise */
	  case 4:
	  case 5:
		real_rotate (shape,FALSE);
		break;
	  case 3:	/* This one is not rotated at all */
		break;
	 }
}

/* Draw a shape on the board */
static void drawshape (board_t board,shape_t *shape,int x,int y)
{
   int i;
   for (i = 0; i < NUMBLOCKS; i++) board[x + shape->block[i].x][y + shape->block[i].y] = shape->color;
}

/* Erase a shape from the board */
static void eraseshape (board_t board,shape_t *shape,int x,int y)
{
   int i;
   for (i = 0; i < NUMBLOCKS; i++) board[x + shape->block[i].x][y + shape->block[i].y] = COLOR_BLACK;
}

/* Check if shape is allowed to be in this position */
static bool allowed (board_t board,shape_t *shape,int x,int y)
{
   int i,occupied = FALSE;
   for (i = 0; i < NUMBLOCKS; i++) if (board[x + shape->block[i].x][y + shape->block[i].y]) occupied = TRUE;
   return (!occupied);
}

/* Set y coordinate of shadow */
static void place_shadow_to_bottom (board_t board,shape_t *shape,int x_shadow,int *y_shadow,int y) {
   while (allowed(board,shape,x_shadow,y+1)) y++;
   *y_shadow = y;
}

/* Move the shape left if possible */
static bool shape_left (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   bool result = FALSE;
   eraseshape (*board,shape,engine->curx,engine->cury);
   if (engine->shadow) eraseshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   if (allowed (*board,shape,engine->curx - 1,engine->cury))
	 {
        engine->curx--;
        result = TRUE;
        if (engine->shadow)
        {
            engine->curx_shadow--;
            place_shadow_to_bottom(*board,shape,engine->curx_shadow,&engine->cury_shadow,engine->cury);
        }
	 }
   if (engine->shadow) drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   drawshape (*board,shape,engine->curx,engine->cury);
   return result;
}

/* Move the shape right if possible */
static bool shape_right (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   bool result = FALSE;
   eraseshape (*board,shape,engine->curx,engine->cury);
   if (engine->shadow) eraseshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   if (allowed (*board,shape,engine->curx + 1,engine->cury))
	 {
		engine->curx++;
		result = TRUE;
		if (engine->shadow)
		{
            engine->curx_shadow++;
            place_shadow_to_bottom(*board,shape,engine->curx_shadow,&engine->cury_shadow,engine->cury);
		}
	 }
   if (engine->shadow) drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   drawshape (*board,shape,engine->curx,engine->cury);
   return result;
}

/* Rotate the shape if possible */
static bool shape_rotate (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   bool result = FALSE;
   shape_t test;
   eraseshape (*board,shape,engine->curx,engine->cury);
   if (engine->shadow) eraseshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   memcpy (&test,shape,sizeof (shape_t));
   fake_rotate (&test);
   if (allowed (*board,&test,engine->curx,engine->cury))
	 {
		memcpy (shape,&test,sizeof (shape_t));
		result = TRUE;
		if (engine->shadow) place_shadow_to_bottom(*board,shape,engine->curx_shadow,&engine->cury_shadow,engine->cury);
	 }
   if (engine->shadow) drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   drawshape (*board,shape,engine->curx,engine->cury);
   return result;
}

/* Move the shape one row down if possible */
static bool shape_down (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   bool result = FALSE;
   eraseshape (*board,shape,engine->curx,engine->cury);
   if (engine->shadow) eraseshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   if (allowed (*board,shape,engine->curx,engine->cury + 1))
	 {
		engine->cury++;
		result = TRUE;
		if (engine->shadow) place_shadow_to_bottom(*board,shape,engine->curx_shadow,&engine->cury_shadow,engine->cury);
	 }
   if (engine->shadow) drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   drawshape (*board,shape,engine->curx,engine->cury);
   return result;
}

/* Check if shape can move down (= in the air) or not (= at the bottom */
/* of the board or on top of one of the resting shapes) */
static bool shape_bottom (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   bool result = FALSE;
   eraseshape (*board,shape,engine->curx,engine->cury);
   if (engine->shadow) eraseshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   result = !allowed (*board,shape,engine->curx,engine->cury + 1);
   if (engine->shadow) drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
   drawshape (*board,shape,engine->curx,engine->cury);
   return result;
}

/* Drop the shape until it comes to rest on the bottom of the board or */
/* on top of a resting shape */
static int shape_drop (engine_t *engine)
{
   board_t *board = &engine->board;
   shape_t *shape = &engine->shapes[engine->curshape];
   eraseshape (*board,shape,engine->curx,engine->cury);
   int droppedlines = 0;

   if (engine->shadow) {
       drawshape (*board,shape,engine->curx_shadow,engine->cury_shadow);
       droppedlines = engine->cury_shadow - engine->cury;
       engine->cury = engine->cury_shadow;
       return droppedlines;
   }

   while (allowed (*board,shape,engine->curx,engine->cury + 1))
	 {
		engine->cury++;
		droppedlines++;
	 }
   drawshape (*board,shape,engine->curx,engine->cury);
   return droppedlines;
}

/* This removes all the rows on the board that is completely filled with blocks */
static int droplines (board_t board)
{
   int i,x,y,ny,status,droppedlines;
   board_t newboard;
   /* initialize new board */
   memset (newboard,0,sizeof (board_t));
   for (i = 0; i < NUMCOLS; i++) newboard[i][NUMROWS - 1] = newboard[i][NUMROWS - 2] = WALL;
   for (i = 0; i < NUMROWS; i++) newboard[0][i] = newboard[NUMCOLS - 1][i] = newboard[NUMCOLS - 2][i] = WALL;
   /* ... */
   ny = NUMROWS - 3;
   droppedlines = 0;
   for (y = NUMROWS - 3; y > 0; y--)
	 {
		status = 0;
		for (x = 1; x < NUMCOLS - 2; x++) if (board[x][y]) status++;
		if (status < NUMCOLS - 3)
		  {
			 for (x = 1; x < NUMCOLS - 2; x++) newboard[x][ny] = board[x][y];
			 ny--;
		  }
		else droppedlines++;
	 }
   memcpy (board,newboard,sizeof (board_t));
   return droppedlines;
}

/* shuffle int array */
void shuffle (int *array, size_t n)
{
   size_t i;
   for (i = 0; i < n - 1; i++)
   {
      int range = (int)(n - i);
      size_t j = i + rand_value(range);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
   }
}

/*
 * Initialize specified tetris engine
 */
void engine_init (engine_t *engine,void (*score_function)(engine_t *))
{
   int i;
   engine->shadow = FALSE;
   engine->score_function = score_function;
   /* intialize values */
   engine->curx = 5;
   engine->cury = 1;
   engine->curx_shadow = 5;
   engine->cury_shadow = 1;
   engine->bag_iterator = 0;
   /* create and randomize bag */
   for (int j = 0; j < NUMSHAPES; j++) engine->bag[j] = j;
   shuffle (engine->bag,NUMSHAPES);
   engine->curshape = engine->bag[engine->bag_iterator%NUMSHAPES];
   engine->nextshape = engine->bag[(engine->bag_iterator+1)%NUMSHAPES];
   engine->bag_iterator++;
   engine->score = 0;
   engine->status.moves = engine->status.rotations = engine->status.dropcount = engine->status.efficiency = engine->status.droppedlines = 0;
   /* initialize shapes */
   memcpy (engine->shapes,SHAPES,sizeof (shapes_t));
   /* initialize board */
   memset (engine->board,0,sizeof (board_t));
   for (i = 0; i < NUMCOLS; i++) engine->board[i][NUMROWS - 1] = engine->board[i][NUMROWS - 2] = WALL;
   for (i = 0; i < NUMROWS; i++) engine->board[0][i] = engine->board[NUMCOLS - 1][i] = engine->board[NUMCOLS - 2][i] = WALL;
}

/*
 * Perform the given action on the specified tetris engine
 */
void engine_move (engine_t *engine,action_t action)
{
   switch (action)
	 {
		/* move shape to the left if possible */
	  case ACTION_LEFT:
        if (shape_left (engine)) engine->status.moves++;
		break;
		/* rotate shape if possible */
	  case ACTION_ROTATE:
		if (shape_rotate (engine)) engine->status.rotations++;
		break;
		/* move shape to the right if possible */
	  case ACTION_RIGHT:
	    if (shape_right (engine)) engine->status.moves++;
		break;
		/* move shape to the down if possible */
	  case ACTION_DOWN:
		if (shape_down (engine)) engine->status.moves++;
		break;
		/* drop shape to the bottom */
	  case ACTION_DROP:
		engine->status.dropcount += shape_drop (engine);
	 }
}

/*
 * Evaluate the status of the specified tetris engine
 *
 * OUTPUT:
 *   1 = shape moved down one line
 *   0 = shape at bottom, next one released
 *  -1 = game over (board full)
 */
int engine_evaluate (engine_t *engine)
{
   if (shape_bottom (engine))
	 {
		/* update status information */
		int dropped_lines = droplines(engine->board);
		engine->status.droppedlines += dropped_lines;
		engine->status.currentdroppedlines = dropped_lines;
		/* increase score */
		engine->score_function (engine);
		engine->curx -= 5;
		engine->curx = abs (engine->curx);
		engine->curx_shadow -= 5;
		engine->curx_shadow = abs (engine->curx_shadow);
		engine->status.rotations = 4 - engine->status.rotations;
		engine->status.rotations = engine->status.rotations > 0 ? 0 : engine->status.rotations;
		engine->status.efficiency += engine->status.dropcount + engine->status.rotations + (engine->curx - engine->status.moves);
		engine->status.efficiency >>= 1;
		engine->status.dropcount = engine->status.rotations = engine->status.moves = 0;
		/* intialize values */
		engine->curx = 5;
		engine->cury = 1;
		engine->curx_shadow = 5;
		engine->cury_shadow = 1;
		engine->curshape = engine->bag[engine->bag_iterator%NUMSHAPES];
		/* shuffle bag before first item in bag would be reused */
		if ((engine->bag_iterator+1) % NUMSHAPES == 0) shuffle(engine->bag, NUMSHAPES);
		engine->nextshape = engine->bag[(engine->bag_iterator+1)%NUMSHAPES];
		engine->bag_iterator++;
		/* initialize shapes */
		memcpy (engine->shapes,SHAPES,sizeof (shapes_t));
		/* return games status */
		return allowed (engine->board,&engine->shapes[engine->curshape],engine->curx,engine->cury) ? 0 : -1;
	 }
   shape_down (engine);
   return 1;
}

/*
 * Macros
 */

/* Upper left corner of board */
#define XTOP ((out_width () - NUMROWS - 3) >> 1)
#define YTOP ((out_height () - NUMCOLS - 9) >> 1)

/* Maximum digits in a number (i.e. number of digits in score, */
/* number of blocks, etc. should not exceed this value */
#define MAXDIGITS 5

/* Number of levels in the game */
#define MINLEVEL	1
#define MAXLEVEL	9

/* This calculates the time allowed to move a shape, before it is moved a row down */
#define DELAY (1000000 / (level + 2))

/* The score is multiplied by this to avoid losing precision */
#define SCOREFACTOR 2

/* This calculates the stored score value */
#define SCOREVAL(x) (SCOREFACTOR * (x))

/* This calculates the real (displayed) value of the score */
#define GETSCORE(score) ((score) / SCOREFACTOR)

static bool shownext;
static bool dottedlines;
static bool shadow;
static int level = MINLEVEL - 1,shapecount[NUMSHAPES];
static char blockchar = ' ';

/*
 * Functions
 */

/* This function is responsible for increasing the score appropriately whenever
 * a block collides at the bottom of the screen (or the top of the heap */
static void score_function (engine_t *engine)
{
   int score = SCOREVAL (level * (engine->status.dropcount + 1));
   score += SCOREVAL ((level + 10) * engine->status.currentdroppedlines * engine->status.currentdroppedlines);

   if (shownext) score /= 2;
   if (dottedlines) score /= 2;

   engine->score += score;
}

/* Draw the board on the screen */
static void drawboard (board_t board)
{
   int x,y;
   out_setattr (ATTR_OFF);
   for (y = 1; y < NUMROWS - 1; y++) for (x = 0; x < NUMCOLS - 1; x++)
	 {
		out_gotoxy (XTOP + x * 2,YTOP + y);
		switch (board[x][y])
		  {
			 /* Wall */
		   case WALL:
			 out_setattr (ATTR_BOLD);
			 out_setcolor (COLOR_BLUE,COLOR_BLACK);
			 out_putch ('<');
			 out_putch ('>');
			 out_setattr (ATTR_OFF);
			 break;
			 /* Background */
		   case 0:
			 if (dottedlines)
			   {
				  out_setcolor (COLOR_BLUE,COLOR_BLACK);
				  out_putch ('.');
				  out_putch (' ');
			   }
			 else
			   {
				  out_setcolor (COLOR_BLACK,COLOR_BLACK);
				  out_putch (' ');
				  out_putch (' ');
			   }
			 break;
			 /* Block */
		   default:
			 out_setcolor (COLOR_BLACK,board[x][y]);
			 out_putch (blockchar);
			 out_putch (blockchar);
		  }
	 }
   out_setattr (ATTR_OFF);
}

/* Show the next piece on the screen */
static void drawnext (int shapenum,int x,int y)
{
   int i;
   block_t ofs[NUMSHAPES] =
	 { { 1,  0 }, { 1,  0 }, { 1, -1 }, { 2,  0 }, { 1, -1 }, { 1, -1 }, { 0, -1 } };
   out_setcolor (COLOR_BLACK,COLOR_BLACK);
   for (i = y - 2; i < y + 2; i++)
	 {
		out_gotoxy (x - 2,i);
		out_printf ("        ");
	 }
   out_setcolor (COLOR_BLACK,SHAPES[shapenum].color);
   for (i = 0; i < NUMBLOCKS; i++)
	 {
		out_gotoxy (x + SHAPES[shapenum].block[i].x * 2 + ofs[shapenum].x,
					y + SHAPES[shapenum].block[i].y + ofs[shapenum].y);
		out_putch (' ');
		out_putch (' ');
	 }
}

/* Draw the background */
static void drawbackground ()
{
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (4,YTOP + 7);   out_printf ("H E L P");
   out_gotoxy (1,YTOP + 9);   out_printf ("p: Pause");
   out_gotoxy (1,YTOP + 10);  out_printf ("j: Left");
   out_gotoxy (1,YTOP + 11);  out_printf ("l: Right");
   out_gotoxy (1,YTOP + 12);  out_printf ("k: Rotate");
   out_gotoxy (1,YTOP + 13);  out_printf ("s: Draw next");
   out_gotoxy (1,YTOP + 14);  out_printf ("d: Toggle lines");
   out_gotoxy (1,YTOP + 15);  out_printf ("a: Speed up");
   out_gotoxy (1,YTOP + 16);  out_printf ("q: Quit");
   out_gotoxy (2,YTOP + 17);  out_printf ("SPACE: Drop");
   out_gotoxy (3,YTOP + 19);  out_printf ("Next:");
}

static int getsum ()
{
   int i,sum = 0;
   for (i = 0; i < NUMSHAPES; i++) sum += shapecount[i];
   return (sum);
}

/* This show the current status of the game */
static void showstatus (engine_t *engine)
{
   static const int shapenum[NUMSHAPES] = { 4, 6, 5, 1, 0, 3, 2 };
   char tmp[MAXDIGITS + 1];
   int i,sum = getsum ();
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (1,YTOP + 1);   out_printf ("Your level: %d",level);
   out_gotoxy (1,YTOP + 2);   out_printf ("Full lines: %d",engine->status.droppedlines);
   out_gotoxy (2,YTOP + 4);   out_printf ("Score");
   out_setattr (ATTR_BOLD);
   out_setcolor (COLOR_YELLOW,COLOR_BLACK);
   out_printf ("  %d",GETSCORE (engine->score));
   if (shownext) drawnext (engine->nextshape,3,YTOP + 22);
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 12,YTOP + 1);
   out_printf ("STATISTICS");
   out_setcolor (COLOR_BLACK,COLOR_MAGENTA);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 3);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 4);
   out_printf ("  ");
   out_setcolor (COLOR_MAGENTA,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 3);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[0]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 3);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_RED);
   out_gotoxy (out_width () - MAXDIGITS - 13,YTOP + 5);
   out_printf ("        ");
   out_setcolor (COLOR_RED,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 5);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[1]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 5);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_WHITE);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 7);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 13,YTOP + 8);
   out_printf ("  ");
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 7);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[2]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 7);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_GREEN);
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 9);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 11,YTOP + 10);
   out_printf ("    ");
   out_setcolor (COLOR_GREEN,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 9);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[3]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 9);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_CYAN);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 11);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 15,YTOP + 12);
   out_printf ("    ");
   out_setcolor (COLOR_CYAN,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 11);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[4]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 11);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_BLACK,COLOR_BLUE);
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 13);
   out_printf ("    ");
   out_gotoxy (out_width () - MAXDIGITS - 9,YTOP + 14);
   out_printf ("    ");
   out_setcolor (COLOR_BLUE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 13);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[5]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 13);
   out_printf ("%s",tmp);
   out_setattr (ATTR_OFF);
   out_setcolor (COLOR_BLACK,COLOR_YELLOW);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 15);
   out_printf ("      ");
   out_gotoxy (out_width () - MAXDIGITS - 15,YTOP + 16);
   out_printf ("  ");
   out_setcolor (COLOR_YELLOW,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 3,YTOP + 15);
   out_putch ('-');
   snprintf (tmp,MAXDIGITS + 1,"%d",shapecount[shapenum[6]]);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 15);
   out_printf ("%s",tmp);
   out_setcolor (COLOR_WHITE,COLOR_BLACK);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 17);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch ('-');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 18);
   out_printf ("Sum          :");
   snprintf (tmp,MAXDIGITS + 1,"%d",sum);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 18);
   out_printf ("%s",tmp);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 20);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch (' ');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 20);
   out_printf ("Score ratio  :");
   snprintf (tmp,MAXDIGITS + 1,"%d",GETSCORE (engine->score) / sum);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 20);
   out_printf ("%s",tmp);
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 21);
   for (i = 0; i < MAXDIGITS + 16; i++) out_putch (' ');
   out_gotoxy (out_width () - MAXDIGITS - 17,YTOP + 21);
   out_printf ("Efficiency   :");
   snprintf (tmp,MAXDIGITS + 1,"%d",engine->status.efficiency);
   out_gotoxy (out_width () - strlen (tmp) - 1,YTOP + 21);
   out_printf ("%s",tmp);
}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

/* Header for scorefile */
#define SCORE_HEADER	"Tint 0.02b (c) Abraham vd Merwe - Scores"

/* Header for score title */
static const char scoretitle[] = "\n\t   TINT HIGH SCORES\n\n\tRank   Score        Name\n\n";

/* Length of a player's name */
#define NAMELEN 20

/* Number of scores allowed in highscore list */
#define NUMSCORES 10

typedef struct
{
   char name[NAMELEN];
   int score;
   time_t timestamp;
} score_t;

static void getname (char *name)
{
   struct passwd *pw = getpwuid (geteuid ());

   fprintf (stderr,"Congratulations! You have a new high score.\n");
   fprintf (stderr,"Enter your name [%s]: ",pw != NULL ? pw->pw_name : "");

   fgets (name,NAMELEN - 1,stdin);
   name[strlen (name) - 1] = '\0';

   if (!strlen (name) && pw != NULL)
	 {
		strncpy (name,pw->pw_name,NAMELEN);
		name[NAMELEN - 1] = '\0';
	 }
}

static void err1 ()
{
   fprintf (stderr,"Error creating %s\n",scorefile);
   exit (EXIT_FAILURE);
}

static void err2 ()
{
   fprintf (stderr,"Error writing to %s\n",scorefile);
   exit (EXIT_FAILURE);
}

void showplayerstats (engine_t *engine)
{
   fprintf (stderr,
			"\n\t   PLAYER STATISTICS\n\n\t"
			"Score       %11d\n\t"
			"Efficiency  %11d\n\t"
			"Score ratio %11d\n",
			GETSCORE (engine->score),engine->status.efficiency,GETSCORE (engine->score) / getsum ());
}

static void createscores (int score)
{
   FILE *handle;
   int i,j;
   score_t scores[NUMSCORES];
   char header[strlen (SCORE_HEADER)+1];
   if (score == 0) return;	/* No need saving this */
   for (i = 1; i < NUMSCORES; i++)
	 {
		strcpy (scores[i].name,"None");
		scores[i].score = -1;
		scores[i].timestamp = 0;
	 }
   getname (scores[0].name);
   scores[0].score = score;
   scores[0].timestamp = time (NULL);
   if ((handle = fopen (scorefile,"w")) == NULL) err1 ();
   strcpy (header,SCORE_HEADER);
   i = fwrite (header,strlen (SCORE_HEADER),1,handle);
   if (i != 1) err2 ();
   for (i = 0; i < NUMSCORES; i++)
	 {
		j = fwrite (scores[i].name,strlen (scores[i].name) + 1,1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1) err2 ();
	 }
   fclose (handle);

   fprintf (stderr,"%s",scoretitle);
   fprintf (stderr,"\t  1* %7d        %s\n\n",score,scores[0].name);
}

static int cmpscores (const void *a,const void *b)
{
   int result;
   result = (int) ((score_t *) a)->score - (int) ((score_t *) b)->score;
   /* a < b */
   if (result < 0) return 1;
   /* a > b */
   if (result > 0) return -1;
   /* a = b */
   result = (time_t) ((score_t *) a)->timestamp - (time_t) ((score_t *) b)->timestamp;
   /* a is older */
   if (result < 0) return -1;
   /* b is older */
   if (result > 0) return 1;
   /* timestamps is equal */
   return 0;
}

static void savescores (int score)
{
   FILE *handle;
   int i,j,ch;
   score_t scores[NUMSCORES];
   char header[strlen (SCORE_HEADER)+1];
   time_t tmp = 0;
   if ((handle = fopen (scorefile,"r")) == NULL)
	 {
		createscores (score);
		return;
	 }
   i = fread (header,strlen (SCORE_HEADER),1,handle);
   if ((i != 1) || (strncmp (SCORE_HEADER,header,strlen (SCORE_HEADER)) != 0))
	 {
		createscores (score);
		return;
	 }
   for (i = 0; i < NUMSCORES; i++)
	 {
		j = 0;
		while ((ch = fgetc (handle)) != '\0')
		  {
			 if ((ch == EOF) || (j >= NAMELEN - 2))
			   {
				  createscores (score);
				  return;
			   }
			 scores[i].name[j++] = (char) ch;
		  }
		scores[i].name[j] = '\0';
		j = fread (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1)
		  {
			 createscores (score);
			 return;
		  }
		j = fread (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1)
		  {
			 createscores (score);
			 return;
		  }
	 }
   fclose (handle);
   if (score > scores[NUMSCORES - 1].score)
	 {
		getname (scores[NUMSCORES - 1].name);
		scores[NUMSCORES - 1].score = score;
		scores[NUMSCORES - 1].timestamp = tmp = time (NULL);
	 }
   qsort (scores,NUMSCORES,sizeof (score_t),cmpscores);
   if ((handle = fopen (scorefile,"w")) == NULL) err2 ();
   strcpy (header,SCORE_HEADER);
   i = fwrite (header,strlen (SCORE_HEADER),1,handle);
   if (i != 1) err2 ();
   for (i = 0; i < NUMSCORES; i++)
	 {
		j = fwrite (scores[i].name,strlen (scores[i].name) + 1,1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].score),sizeof (int),1,handle);
		if (j != 1) err2 ();
		j = fwrite (&(scores[i].timestamp),sizeof (time_t),1,handle);
		if (j != 1) err2 ();
	 }
   fclose (handle);

   fprintf (stderr,"%s",scoretitle);
   i = 0;
   while ((i < NUMSCORES) && (scores[i].score != -1))
	 {
		j = scores[i].timestamp == tmp ? '*' : ' ';
		fprintf (stderr,"\t %2d%c %7d        %s\n",i + 1,j,scores[i].score,scores[i].name);
		i++;
	 }
   fprintf (stderr,"\n");
}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

static void showhelp ()
{
   fprintf (stderr,"USAGE: tint [-h] [-l level] [-n] [-d] [-b char]\n");
   fprintf (stderr,"  -h           Show this help message\n");
   fprintf (stderr,"  -l <level>   Specify the starting level (%d-%d)\n",MINLEVEL,MAXLEVEL);
   fprintf (stderr,"  -n           Draw next shape\n");
   fprintf (stderr,"  -d           Draw vertical dotted lines\n");
   fprintf (stderr,"  -b <char>    Use this character to draw blocks instead of spaces\n");
   fprintf (stderr,"  -s           Draw shadow of shape\n");
   exit (EXIT_FAILURE);
}

static void parse_options (int argc,char *argv[])
{
   int i = 1;
   while (i < argc)
	 {
		/* Help? */
		if (strcmp (argv[i],"-h") == 0)
		  showhelp ();
		/* Level? */
		else if (strcmp (argv[i],"-l") == 0)
		  {
			 i++;
			 if (i >= argc || !str2int (&level,argv[i])) showhelp ();
			 if ((level < MINLEVEL) || (level > MAXLEVEL))
			   {
				  fprintf (stderr,"You must specify a level between %d and %d\n",MINLEVEL,MAXLEVEL);
				  exit (EXIT_FAILURE);
			   }
		  }
		/* Show next? */
		else if (strcmp (argv[i],"-n") == 0)
		  shownext = TRUE;
		else if(strcmp(argv[i],"-d")==0)
		  dottedlines = TRUE;
		else if(strcmp(argv[i], "-b")==0)
		  {
		    i++;
		    if (i >= argc || strlen(argv[i]) < 1) showhelp();
		    blockchar = argv[i][0];
		  }
		else if (strcmp (argv[i],"-s") == 0)
            shadow = TRUE;
		else
		  {
			 fprintf (stderr,"Invalid option -- %s\n",argv[i]);
			 showhelp ();
		  }
		i++;
	 }
}

static void choose_level ()
{
   char buf[NAMELEN];

   do
	 {
		fprintf (stderr,"Choose a level to start [%d-%d]: ",MINLEVEL,MAXLEVEL);
		fgets (buf,NAMELEN - 1,stdin);
		buf[strlen (buf) - 1] = '\0';
	 }
   while (!str2int (&level,buf) || level < MINLEVEL || level > MAXLEVEL);
}

static bool evaluate (engine_t *engine)
{
    bool finished = FALSE;
    switch (engine_evaluate (engine))
    {
        /* game over (board full) */
        case -1:
            if ((level < MAXLEVEL) && ((engine->status.droppedlines / 10) > level)) level++;
            finished = TRUE;
            break;
            /* shape at bottom, next one released */
        case 0:
            if ((level < MAXLEVEL) && ((engine->status.droppedlines / 10) > level))
            {
                level++;
                in_timeout (DELAY);
            }
            shapecount[engine->curshape]++;
            break;
            /* shape moved down one line */
        case 1:
            break;
    }
    return finished;
}

          /***************************************************************************/
          /***************************************************************************/
          /***************************************************************************/

int main (int argc,char *argv[])
{
   bool finished;
   int ch;
   engine_t engine;
   /* Initialize */
   rand_init ();							/* must be called before engine_init () */
   engine_init (&engine,score_function);	/* must be called before using engine.curshape */
   finished = shownext = shadow = FALSE;
   memset (shapecount,0,NUMSHAPES * sizeof (int));
   shapecount[engine.curshape]++;
   parse_options (argc,argv);				/* must be called after initializing variables */
   engine.shadow = shadow;
   if (level < MINLEVEL) choose_level ();
   io_init ();
   drawbackground ();
   in_timeout (DELAY);
   /* Main loop */
   do
	 {
		/* draw shape */
		showstatus (&engine);
		drawboard (engine.board);
		out_refresh ();
		/* Check if user pressed a key */
		if ((ch = in_getch ()) != ERR)
		  {
			 switch (ch)
			   {
				case 'j':
				case KEY_LEFT:
				  engine_move (&engine,ACTION_LEFT);
				  break;
				case 'k':
				case KEY_UP:
				case '\n':
				  engine_move (&engine,ACTION_ROTATE);
				  break;
				case 'l':
				case KEY_RIGHT:
				  engine_move (&engine,ACTION_RIGHT);
				  break;
				case KEY_DOWN:
				  engine_move (&engine,ACTION_DROP);
				  break;
				case ' ':
				  engine_move (&engine,ACTION_DROP);
				  finished = evaluate(&engine);          /* prevent key press after drop */
				  break;
				  /* show next piece */
				case 's':
				  shownext = TRUE;
				  break;
				  /* toggle dotted lines */
				case 'd':
				  dottedlines = !dottedlines;
				  break;
				  /* next level */
				case 'a':
				  if (level < MAXLEVEL)
					{
					   level++;
					   in_timeout (DELAY);
					}
				  else out_beep ();
				  break;
				  /* quit */
				case 'q':
				  finished = TRUE;
				  break;
				  /* pause */
				case 'p':
				  out_setcolor (COLOR_WHITE,COLOR_BLACK);
				  out_gotoxy ((out_width () - 34) / 2,out_height () - 2);
				  out_printf ("Paused - Press any key to continue");
				  while ((ch = in_getch ()) == ERR) ;	/* Wait for a key to be pressed */
				  in_flush ();							/* Clear keyboard buffer */
				  out_gotoxy ((out_width () - 34) / 2,out_height () - 2);
				  out_printf ("                                  ");
				  break;
				  /* unknown keypress */
				default:
				  out_beep ();
			   }
			 in_flush ();
		  }
		else
		  finished = evaluate(&engine);
	 }
   while (!finished);
   /* Restore console settings and exit */
   io_close ();
   /* Don't bother the player if he want's to quit */
   if (ch != 'q')
	 {
		showplayerstats (&engine);
		savescores (GETSCORE (engine.score));
	 }
   exit (EXIT_SUCCESS);
}

