#ifndef _FRAME_H_
#define _FRAME_H_

#include "canvas.h"

size_t register_behavior(canvas_t *canvas);
size_t create_frame(canvas_t *const canvas, vec2_t pos, vec2_t box);
size_t create_styled_frame(canvas_t *const canvas, vec2_t pos, vec2_t box, style_t style);
void destroy_frame(canvas_t *const canvas, const size_t frame_id);

void render_frame(const canvas_t *const canvas, display_t *const display, const size_t frame_id);

#endif// _FRAME_H_
