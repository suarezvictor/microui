#include <SDL2/SDL.h>

#define bool int
#define false 0
#define true 1
#include <sim_fb.h>

#define FB_PITCH (4096*4)
static uint32_t pixbuf[4096][4096];
static fb_handle_t fb;

static inline uint32_t color2rgba(mu_Color color)
{
  return color.r | (color.g << 8) | (color.b << 16) | (color.a << 24);
}

static void
r_internal_fill_rect(unsigned char *pixels,
    const short x0, const short y0, const short x1, const short y1,
    const unsigned int col)
{
    // WARNING: no checking

    unsigned int i, n;
    unsigned int c[16];
    unsigned int *ptr;
    for (i = 0; i < sizeof(c) / sizeof(c[0]); i++)
        c[i] = col;

    pixels += y0 * FB_PITCH;
    for (int y = y0; y < y1; y++)
    {
        ptr = (unsigned int *)pixels + x0;

        n = x1 - x0;
        while (n > 16) { //TODO: alignment
            memcpy((void *)ptr, c, sizeof(c));
            n -= 16; ptr += 16;
        }
        for (i = 0; i < n; i++)
            ptr[i] = c[i];
        pixels += FB_PITCH;
    }
}


static w_width = 0, w_height = 0;
static mu_Rect scissors;
void r_init(int width, int height) {
  w_width = width;
  w_height = height;
  scissors.x = 0;
  scissors.y = 0;
  scissors.w = width;
  scissors.h = height;
  fb_init(width, height, 0, &fb);
}


static void flush(void) {
}

#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {

  int x0 = MAX(scissors.x, MIN(scissors.w, dst.x));
  int y0 = MAX(scissors.y, MIN(scissors.h, dst.y));
  int x1 = MAX(scissors.x, MIN(scissors.w, dst.x+dst.w));
  int y1 = MAX(scissors.y, MIN(scissors.h, dst.y+dst.h));

  r_internal_fill_rect((unsigned char*) pixbuf, x0, y0, x1, y1, color2rgba(color));
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, atlas[ATLAS_WHITE], color);
}

void r_set_clip_rect(mu_Rect rect) {
  flush();
  //glScissor(rect.x, w_height - (rect.y + rect.h), rect.w, rect.h);
}

void r_clear(mu_Color clr) {
/*
  flush();
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);*/
  r_draw_rect((mu_Rect) {0, 0, w_width, w_height }, clr);
}

void r_present(void) {
  fb_update(&fb, pixbuf, FB_PITCH);
}

#include <../src/sim_fb.c>

