#include <assert.h>
#include "renderer.h"
#include "atlas.inl"

#include "renderer_sdl.c"

//-----------------------------------------------------------------------------------------------
// Portable functions
//-----------------------------------------------------------------------------------------------
static int r_draw_char(char chr, mu_Vec2 pos, mu_Color color) {
  mu_Rect src = atlas[ATLAS_FONT + chr];
  mu_Rect dst = { pos.x, pos.y, src.w, src.h };
  push_quad(dst, src, color);
  return src.w;
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    pos.x += r_draw_char(chr, pos, color);
  }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}


int r_get_text_height(void) {
  return 18;
}


