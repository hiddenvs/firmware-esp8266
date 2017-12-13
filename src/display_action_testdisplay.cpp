#include "config_common.hpp"
#include "display_action_testdisplay.hpp"

#include "display.hpp"
#include "display_u8g2.hpp"

namespace Display {
namespace Action {
void TestDisplay::tick(DisplayT *display, double elapsed_time)
{
  m_elapsed_time += elapsed_time;
}

void TestDisplay::draw(DisplayT *display, Coords coords)
{
  display->setFont(u8g2_font_u8glib_4_tr);
  display->setDrawColor(1);

  int w = display->getDisplayWidth();
  int h = display->getDisplayHeight();

  int offset = (int) (m_elapsed_time*2) % 8;
  int brightness = (int) (m_elapsed_time*2) % 16;

  // modes
  switch(m_current_mode % 5) {
    case 0:
      display->setBrightness(255);
      display->fill({0,0});
      break;
    case 1: // walking columns
      display->setBrightness(255);
      for (int x=offset; x<w; x+=8)
        display->drawLine({x,0},{x,h});
      break;
    case 2: // walking rows
      display->setBrightness(255);
      for (int y=offset; y<h; y+=8)
        display->drawLine({0,y},{w,y});
      break;
    case 3: // walking diagonals
      display->setBrightness(255);
      for (int x=offset; x<w; x+=8)
        display->drawLine({x+8,0},{x,h});
      display->drawLine({offset,0},{0,offset});
      break;
    case 4: // brightness
      {
        display->setBrightness(brightness*16);
        Coords coords{1,-2};
        String text("Bright " + String(brightness));
        display->displayText(text, coords);
      }
      break;
    default:
      break;
  }
}

void TestDisplay::nextMode() { ++m_current_mode; }

}
}
