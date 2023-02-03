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

inline uint8_t alpha_mul(uint8_t a, uint8_t c) { return a*c >> 8; }

inline uint32_t blend(uint32_t bg, uint32_t fg, uint8_t alpha)
{
  alpha = alpha_mul(alpha, fg >> 24);
  uint8_t r = alpha_mul(alpha, fg >> 16);
  uint8_t g = alpha_mul(alpha, fg >> 8);
  uint8_t b = alpha_mul(alpha, fg >> 0);
  alpha = 255 - alpha;
  r += alpha_mul(alpha, bg >> 16);
  g += alpha_mul(alpha, bg >> 8);
  b += alpha_mul(alpha, bg >> 0);
  return b | (g << 8) | (r << 16);
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
		uint8_t src_alpha = *texel++;
        *target_pixel++ = blend(*target_pixel, col, src_alpha);
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

  uint32_t c = color2rgba(color);
  if(src.w == 0 || src.h == 0)
    r_internal_fill_rect((unsigned char*) pixbuf, x0, y0, x1, y1, c);
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

void r_clear(mu_Color clr) {
  flush();
  r_internal_fill_rect((unsigned char*) pixbuf, 0, 0, w_width, w_height, color2rgba(clr));
}

void r_present(void) {
  fb_update(&fb, pixbuf, FB_PITCH);
}

#include <../src/sim_fb.c>

