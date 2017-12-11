#include <Arduino.h>
#include "display_u8g2.hpp"

namespace Display {
void U8G2Matrix::displayText(const String& value, int x, int y, bool immediate)
{
  if (m_rotation==U8G2_R0)
    y += 8;
  else if (m_rotation==U8G2_R2)
    y += 16;

  if (immediate) clearBuffer();
  m_display->drawStr(x,y,value.c_str());
  if (immediate) sendBuffer();
}

void U8G2Matrix::displayNumber(int number, int length=0, int position=0, bool zero_fill=false)
{
  displayText(String(number), 0, 0, true);
}

void U8G2Matrix::displayBitmap(const unsigned char *bitmap, const int x, const int y, const int w, const int h)
{
  m_display->drawXBM(x,y,w,h, bitmap);
}

void U8G2Matrix::displayBitmapP(const unsigned char *bitmap, const int x, const int y, const int w, const int h)
{
  m_display->drawXBMP(x,y,w,h, bitmap);
}

void U8G2Matrix::drawGlyph(uint16_t glyph, Coords coords)
{
  if (m_rotation==U8G2_R0)
    coords.y += 8;
  else if (m_rotation==U8G2_R2)
    coords.y += 16;

  m_display->drawGlyph(coords.x, coords.y, glyph);
}

void U8G2Matrix::fill()
{
  m_display->drawBox(0, 0, m_width, m_height);
}

void U8G2Matrix::clearBuffer(void)
{
  m_display->clearBuffer();
}

void U8G2Matrix::sendBuffer(void)
{
  m_display->sendBuffer();
}

int U8G2Matrix::getTextWidth(const String& text)
{
  return m_display->getStrWidth(text.c_str());
}

void U8G2Matrix::setFont(const uint8_t* font)
{
  if (font==nullptr)
    m_display->setFont(m_default_font);
  else
    m_display->setFont(font);
}

font_t U8G2Matrix::getDefaultFont(void)
{
  return m_default_font;
}

void U8G2Matrix::setBrightness(uint8_t brightness)
{
  m_display->setContrast(brightness * (64.0/100.0));
}

void U8G2Matrix::setDrawColor(const uint8_t color)
{
  m_display->setDrawColor(color);
}


int U8G2Matrix::getDisplayWidth() { return m_width; }
int U8G2Matrix::getDisplayHeight() { return m_height; }
int U8G2Matrix::getCurrentFontHeight()
{
  return m_display->getAscent();
}
}
