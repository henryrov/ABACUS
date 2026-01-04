#ifndef INTERFACE_H
#define INTERFACE_H

enum selection_side_e
{
  SIDE_LEFT,
  SIDE_RIGHT
};

void interface_draw_result();
void interface_draw_input();
void interface_draw_selection(enum selection_side_e side);
void interface_draw_battery();
void interface_draw_error();

#endif
