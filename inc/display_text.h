#pragma once
#include <stdint.h>
#include "esp_lcd_types.h"


void draw_text_5x7(esp_lcd_panel_handle_t panel,
                          int x, int y, const char *s,
                          uint16_t fg565, uint16_t bg565,
                          int scale);

