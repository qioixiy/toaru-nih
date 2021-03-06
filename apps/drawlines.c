/* This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2013-2018 K. Lange
 */
/*
 * drawlines
 *
 * Test application to draw lines to a window.
 */
#include <stdlib.h>
#include <assert.h>
#include <syscall.h>
#include <unistd.h>
#include <pthread.h>

#include <toaru/yutani.h>
#include <toaru/graphics.h>

#define TRACE_APP_NAME "drawlines"
#include <toaru/trace.h>

static int left, top, width, height;

static yutani_t * yctx;
static yutani_window_t * wina;
static gfx_context_t * ctx;
static int should_exit = 0;

void * draw_thread(void * garbage) {
	(void)garbage;
	while (!should_exit) {
		draw_line(ctx, rand() % width, rand() % width, rand() % height, rand() % height, rgb(rand() % 255,rand() % 255,rand() % 255));
		yutani_flip(yctx, wina);
		usleep(16666);
	}
	pthread_exit(0);
	return NULL;
}

int main (int argc, char ** argv) {
	left   = 100;
	top    = 100;
	width  = 500;
	height = 500;

	srand(time(NULL));

	TRACE("Starting drawlines.");
	yctx = yutani_init();
	TRACE("Creating a window.");
	wina = yutani_window_create(yctx, width, height);
	TRACE("Move.");
	yutani_window_move(yctx, wina, left, top);
	TRACE("Advertise icon.");
	yutani_window_advertise_icon(yctx, wina, "drawlines", "drawlines");

	TRACE("Init graphics.");
	ctx = init_graphics_yutani(wina);
	draw_fill(ctx, rgb(0,0,0));

	pthread_t thread;
	pthread_create(&thread, NULL, draw_thread, NULL);

	while (!should_exit) {
		yutani_msg_t * m = yutani_poll(yctx);
		if (m) {
			switch (m->type) {
				case YUTANI_MSG_KEY_EVENT:
					{
						struct yutani_msg_key_event * ke = (void*)m->data;
						if (ke->event.action == KEY_ACTION_DOWN && ke->event.keycode == 'q') {
							should_exit = 1;
							syscall_yield();
						}
					}
					break;
				case YUTANI_MSG_WINDOW_MOUSE_EVENT:
					{
						struct yutani_msg_window_mouse_event * me = (void*)m->data;
						if (me->command == YUTANI_MOUSE_EVENT_DOWN && me->buttons & YUTANI_MOUSE_BUTTON_LEFT) {
							yutani_window_drag_start(yctx, wina);
						}
					}
					break;
				case YUTANI_MSG_WINDOW_CLOSE:
				case YUTANI_MSG_SESSION_END:
					should_exit = 1;
					break;
				default:
					break;
			}
		}
		free(m);
	}

	yutani_close(yctx, wina);

	return 0;
}
