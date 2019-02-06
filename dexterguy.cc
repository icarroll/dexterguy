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

void drawstuff(cairo_t * cr) {
    // 0,0 at center of window and 1,1 at top right
    cairo_scale(cr, SCREEN_WIDTH/2.0, -SCREEN_HEIGHT/2.0);
    cairo_translate(cr, 1, -1);

    // set up physics
    cpVect gravity = cpv(0, -1);
    space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);

    // walls
    //TODO simplify this
    /*
    cpShape * ceiling = cpSegmentShapeNew(cpSpaceGetStaticBody(space),
                                          cpv(-1,1), cpv(1, 1), 0);
    cpShapeSetFriction(ceiling, 0);
    cpSpaceAddShape(space, ceiling);
    cpShape * leftwall = cpSegmentShapeNew(cpSpaceGetStaticBody(space),
                                           cpv(-1,-1), cpv(-1, 1), 0);
    cpShapeSetFriction(leftwall, 0);
    cpSpaceAddShape(space, leftwall);
    cpShape * rightwall = cpSegmentShapeNew(cpSpaceGetStaticBody(space),
                                           cpv(1,-1), cpv(1, 1), 0);
    cpShapeSetFriction(rightwall, 0);
    cpSpaceAddShape(space, rightwall);
    cpShape * floor = cpSegmentShapeNew(cpSpaceGetStaticBody(space),
                                        cpv(-1,-1), cpv(1, -1), 0);
    cpShapeSetFriction(floor, 0);
    cpSpaceAddShape(space, floor);
    */

    // static body
    cpBody * body0 = cpSpaceAddBody(space, cpBodyNewStatic());
    cpBodySetPosition(body0, cpv(0, 1));

    // set font
    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 0.2);
    cpFloat pad = 0.02;

    // "Hello,"
    cairo_text_extents_t te1;
    cairo_text_extents(cr, "Hello,", & te1);
    cpFloat mass1 = 1;
    cpFloat moment1 = cpMomentForBox(mass1, te1.width, te1.height);
    cpBody * body1 = cpSpaceAddBody(space, cpBodyNew(mass1, moment1/2));
    //cpShape * shape1 = cpSpaceAddShape(space, cpBoxShapeNew(body1, te1.width, te1.height, 0.001));
    //cpShapeSetElasticity(shape1, 0.999999);
    cpBodySetPosition(body1, cpv(0.5, 0.75));
    //cpShapeSetFriction(shape1, 0.0);

    /*
    vector<cpVect> dots = {cpv(0.25-te1.width/2,0.25-te1.height/2),
                           cpv(0.25-te1.width/2,0.25+te1.height/2),
                           cpv(0.25+te1.width/2,0.25+te1.height/2),
                           cpv(0.25+te1.width/2,0.25-te1.height/2)};
    */

    // "World!"
    cairo_text_extents_t te2;
    cairo_text_extents(cr, "World!", & te2);
    cpFloat mass2 = 1;
    cpFloat moment2 = cpMomentForBox(mass2, te2.width, te2.height);
    cpBody * body2 = cpSpaceAddBody(space, cpBodyNew(mass2, moment2/2));
    //cpShape * shape2 = cpSpaceAddShape(space, cpBoxShapeNew(body2, te2.width, te2.height, 0.001));
    //cpShapeSetElasticity(shape2, 0.999999);
    cpBodySetPosition(body2, cpv(0.5, 0.333));
    //cpShapeSetFriction(shape2, 0.0);

    // springs
    cpSpaceAddConstraint(space, cpDampedSpringNew(body0, body1, cpv(0,0), cpv(0,0.01), 0.667, 30, 0.0001));
    cpSpaceAddConstraint(space, cpDampedSpringNew(body1, body2, cpv(0,-0.01), cpv(0,0.01), 0.667, 30, 0.0001));

    while (true) {
        for (int n=0 ; n<20 ; n+=1) cpSpaceStep(space, 1/1000.0);
        //cpSpaceStep(space, 20 / 1000.0);

        // clear screen
        cairo_rectangle(cr, -1, -1, 2, 2);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_fill(cr);

        // Hello,
        cpVect pos1 = cpBodyGetPosition(body1);
        cpFloat rot1 = cpBodyGetAngle(body1);

        cairo_save(cr);

        cairo_move_to(cr, pos1.x, pos1.y);
        cairo_scale(cr, 1, -1);
        cairo_rotate(cr, rot1);
        cairo_rel_move_to(cr, -te1.width/2, te1.height/2);
        cairo_text_path(cr, "Hello,");
        cairo_set_source_rgb(cr, 0,0,1);
        cairo_fill_preserve(cr);
        cairo_set_line_width(cr, 0.001);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);

        cairo_restore(cr);

        /*
        // Hello box
        cairo_save(cr);

        cairo_move_to(cr, pos1.x, pos1.y);
        cairo_scale(cr, 1, -1);
        cairo_rotate(cr, rot1);
        cairo_rel_move_to(cr, -te1.width/2+te1.x_bearing, te1.height/2+te1.y_bearing);
        cairo_rel_move_to(cr, -pad, -pad);
        cairo_rel_line_to(cr, te1.width+2*pad, 0);
        cairo_rel_line_to(cr, 0, te1.height+2*pad);
        cairo_rel_line_to(cr, -te1.width-2*pad, 0);
        cairo_close_path(cr);
        cairo_set_line_width(cr, 0.001);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);

        cairo_restore(cr);
        */

        // World!
        cpVect pos2 = cpBodyGetPosition(body2);
        cpFloat rot2 = cpBodyGetAngle(body2);

        cairo_save(cr);

        cairo_move_to(cr, pos2.x, pos2.y);
        cairo_scale(cr, 1, -1);
        cairo_rotate(cr, rot2);
        cairo_rel_move_to(cr, -te2.width/2, te2.height/2);
        cairo_text_path(cr, "World!");
        cairo_set_source_rgb(cr, 0,1,0);
        cairo_fill_preserve(cr);
        cairo_set_line_width(cr, 0.001);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);

        cairo_restore(cr);

        /*
        // World box
        cairo_save(cr);

        cairo_move_to(cr, pos2.x, pos2.y);
        cairo_scale(cr, 1, -1);
        cairo_rotate(cr, rot2);
        cairo_rel_move_to(cr, -te2.width/2+te2.x_bearing, te2.height/2+te2.y_bearing);
        cairo_rel_move_to(cr, -pad, -pad);
        cairo_rel_line_to(cr, te2.width+2*pad, 0);
        cairo_rel_line_to(cr, 0, te2.height+2*pad);
        cairo_rel_line_to(cr, -te2.width-2*pad, 0);
        cairo_close_path(cr);
        cairo_set_line_width(cr, 0.001);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);

        cairo_restore(cr);
        */

        /*
        for (cpVect dot : dots) {
            cairo_new_sub_path(cr);
            cairo_arc(cr, dot.x, dot.y, 0.01, 0, 2*M_PI);
        }
        cairo_set_source_rgb(cr, 1,0,0);
        cairo_fill(cr);
        */

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
    }

    drawthread.detach();

    close();

    return 0;
}
