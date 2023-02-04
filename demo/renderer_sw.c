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
  return color.b | (color.g << 8) | (color.r << 16) | (color.a << 24);
}

inline uint8_t alpha_mul8(uint8_t a, uint8_t c) { return a*c >> 8; }

static inline uint32_t alpha_mul(uint32_t color, uint8_t alpha)
{
  return alpha_mul8(color >> 0, alpha)
    | (alpha_mul8(color >> 8, alpha) << 8)
    | (alpha_mul8(color >> 16, alpha) << 16)
    | (alpha << 24);
}

static void
r_internal_fill_rect(unsigned char *pixels,
    const short x0, const short y0, const short x1, const short y1,
    const unsigned int col)
{
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


inline uint32_t blend_premul(uint32_t bg, uint32_t fg_premul)
{
  return fg_premul + alpha_mul(bg, 255 - (fg_premul >> 24)); //will result in alpha 255
}

int r_internal_fill_rect_alpha(uint8_t *target_pixels, int x0, int y0, int x1, int y1, uint32_t col)
{
  target_pixels += y0 * FB_PITCH + x0 * sizeof(col);
  for (int y = y0; y < y1; ++y)
  {
    uint32_t *target_pixel = (uint32_t *) target_pixels;	
	for (int x = x0; x < x1; ++x)
	{
        *target_pixel++ = blend_premul(*target_pixel, col);
	}
	target_pixels += FB_PITCH;
  }
  return 1;
}

int r_internal_blit_alpha8(int x0, int y0, int x1, int y1, uint32_t col,
  int src_x, int src_y, const uint8_t *src_pixels, unsigned src_width)
{
  const uint8_t *texture_pixels = &src_pixels[src_y * src_width + src_x];
  uint8_t *target_pixels = (unsigned char *) pixbuf;
  target_pixels += y0 * FB_PITCH + x0 * sizeof(col);
  for (int y = y0; y < y1; ++y)
  {
    const uint8_t *texel = texture_pixels;
    uint32_t *target_pixel = (uint32_t *) target_pixels;	
	for (int x = x0; x < x1; ++x)
	{
	    uint8_t alpha = *texel++;
	    uint32_t src_col = alpha_mul(col, alpha);
	    src_col = (src_col & 0xFFFFFF) | (alpha_mul(col >> 24, alpha) << 24); //set alpha as product
        *target_pixel++ = blend_premul(*target_pixel, src_col);
	}
	texture_pixels += src_width;
	target_pixels += FB_PITCH;
  }
  return 1;
}


static int w_width, w_height;
int scissors_x0, scissors_y0, scissors_x1, scissors_y1;
void r_init(int width, int height) {
  w_width = width;
  w_height = height;
  scissors_x0 = 0;
  scissors_y0 = 0;
  scissors_x1 = width;
  scissors_y1 = height;
  fb_init(width, height, 0, &fb);
}


static void flush(void) {
}

#ifndef MAX
#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#endif

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
  int x0 = MAX(scissors_x0, MIN(scissors_x1, dst.x));
  int y0 = MAX(scissors_y0, MIN(scissors_y1, dst.y));
  int x1 = MAX(scissors_x0, MIN(scissors_x1, dst.x+dst.w));
  int y1 = MAX(scissors_y0, MIN(scissors_y1, dst.y+dst.h));

  uint32_t c = alpha_mul(color2rgba(color), color.a);
  if(src.w == 0 || src.h == 0)
  {
    if((c >> 24) == 255)
      r_internal_fill_rect((unsigned char*) pixbuf, x0, y0, x1, y1, c);
    else
      r_internal_fill_rect_alpha((unsigned char*) pixbuf, x0, y0, x1, y1, c);
  }
  else
    r_internal_blit_alpha8(x0, y0, x1, y1, c, src.x, src.y, atlas_texture, ATLAS_WIDTH);
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, (mu_Rect) {0,0,0,0}, color);
}

void r_set_clip_rect(mu_Rect rect) {
  flush();
  scissors_x0 = rect.x;
  scissors_y0 = rect.y;
  scissors_x1 = rect.x+rect.w;
  scissors_y1 = rect.y + rect.h;
}

void r_clear(mu_Color color) {
  flush();
  color.a = 255;
  r_internal_fill_rect((unsigned char*) pixbuf, 0, 0, w_width, w_height, color2rgba(color));
}

void r_present(void) {
  fb_update(&fb, pixbuf, FB_PITCH);
}

#include <../src/sim_fb.c>

