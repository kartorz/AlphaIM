#include "GtkIMPreedit.h"

/*
 'w'  longger press, lots of state:0x00 and a 0x40000000
{ ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x0 } +       //key press.
  ....
  ProcessKeyEvent val: 0x77,  keycode:0x11,  state:0x40000000  //key release
*/

GtkIMPreedit::GtkIMPreedit(ICManager* icm):QtIMPreedit(icm)
{
}
