#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <chipmunk.h>

extern "C" {
#include <SDL.h>
#include <cairo.h>

#include <stdio.h>
}

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

char WINDOW_NAME[] = "Hello, World!";
SDL_Window * gWindow = NULL;

void die(string message) {
    printf("%s\n", message);
    exit(1);
}

void init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) die("SDL");
    if (! SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) die("texture");

    gWindow = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if (gWindow == NULL) die("window");
}

void close()
{
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}

default_random_engine gen;
uniform_real_distribution<double> dist(0.0, 1.0);

double random(double min, double max) {
    return dist(gen) * (max - min) + min;
}

cpSpace * space;

int BLIT_READY;

double mouse_angle = 0.0;
bool mouse_down = false;

void drawstuff(cairo_t * cr) {
    // 0,0 at center of window and 1,1 at top right
    cairo_scale(cr, SCREEN_WIDTH/2.0, -SCREEN_HEIGHT/2.0);
    cairo_translate(cr, 1, -1);

    while (true) {
        // clear screen
        cairo_rectangle(cr, -1, -1, 2, 2);
        cairo_set_source_rgb(cr, 0.5, 1, 0.5);
        cairo_fill(cr);

	cairo_save(cr);

	cairo_rotate(cr, mouse_angle);

	// guy
	cairo_new_sub_path(cr);
	cairo_arc(cr, 0, 0, 0.1, 0, 2*M_PI);
	cairo_set_source_rgb(cr, 0,1,0);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, 0.01);
	cairo_set_source_rgb(cr, 0,0,0.5);
	cairo_stroke(cr);
	cairo_new_sub_path(cr);
	cairo_arc(cr, -0.05, 0.05, 0.025, 0, 2*M_PI);
	cairo_new_sub_path(cr);
	cairo_arc(cr, 0.05, 0.05, 0.025, 0, 2*M_PI);
	cairo_set_source_rgb(cr, 0.5,0.5,1);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, 0.01);
	cairo_set_source_rgb(cr, 0,0,0.5);
	cairo_stroke(cr);

	// axe
	cairo_save(cr);
	if (mouse_down) cairo_rotate(cr, M_PI/4);

	cairo_move_to(cr, 0.1, 0);
	cairo_line_to(cr, 0.25, 0.15);
	cairo_set_line_width(cr, 0.02);
	cairo_set_source_rgb(cr, 150.0/255.0, 83.0/255.0, 83.0/255.0);
	cairo_stroke(cr);
	cairo_new_sub_path(cr);
	cairo_arc(cr, 0.25, 0.15, 0.1, M_PI/4, M_PI+M_PI/4);
	cairo_set_source_rgb(cr, 246.0/255.0, 187.0/255.0, 66.0/255.0);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, 0.02);
	cairo_set_source_rgb(cr, 248.0/255.0, 200.0/255.0, 104.0/255.0);
	cairo_stroke(cr);

	cairo_restore(cr);

	cairo_restore(cr);

        SDL_Event e;
        e.type = BLIT_READY;
        SDL_PushEvent(& e);

        this_thread::sleep_for(chrono::milliseconds(20));
    }
}

int main(int nargs, char * args[])
{
    init();

    SDL_Surface * sdlsurf = SDL_CreateRGBSurface(
        0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
        0x00FF0000, // red
        0x0000FF00, // green
        0x000000FF, // blue
        0); // alpha

    //TODO make sure sdlsurf is locked or doesn't need locking

    cairo_surface_t * csurf = cairo_image_surface_create_for_data(
        (unsigned char *) sdlsurf->pixels,
        CAIRO_FORMAT_RGB24,
        sdlsurf->w,
        sdlsurf->h,
        sdlsurf->pitch);

    cairo_t * cr = cairo_create(csurf);

    BLIT_READY = SDL_RegisterEvents(1);
    thread drawthread(drawstuff, cr);

    SDL_Surface * wsurf = SDL_GetWindowSurface(gWindow);

    bool done = false;
    while (! done)
    {
        SDL_Event e;
        SDL_WaitEvent(& e); //TODO check for error

        if (e.type == SDL_QUIT) done = true;
        else if (e.type == BLIT_READY) {
            SDL_BlitSurface(sdlsurf, NULL, wsurf, NULL);
            SDL_UpdateWindowSurface(gWindow);
        }
	else if (e.type == SDL_MOUSEMOTION) {
            double x = e.motion.x;
            double y = e.motion.y;
            cairo_device_to_user(cr, & x, & y);

            mouse_angle = cpvtoangle(cpv(x,y)) - M_PI/2;
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
	    mouse_down = true;
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
	    mouse_down = false;
	}
    }

    drawthread.detach();

    close();

    return 0;
}
