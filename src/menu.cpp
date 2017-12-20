#include "config_common.hpp"

#include "menu.hpp"
#include "display_action_menu.hpp"
#include <EEPROM.h>

#include "display_u8g2.hpp"

using std::shared_ptr;

void Menu::start(DisplayT *display, button_callback_t changeMode)
{
  m_finished = false;
  display->prependAction(std::make_shared<Display::Action::MenuWrapper>(this, Coords{0,0}, u8g2_font_u8glib_4_tr));
  m_end_of_menu_callback = changeMode;

  // load item values from parameters
  for (auto item: m_items) {
    auto parameter = m_parameters->findByName(item->getName());
    if (parameter)
      item->setValue(parameter->value);
  }
}

void Menu::onLongPress()
{
  auto item = (*m_current);
  if (item->isActive()) {
    item->onLongPress();

    auto parameter = m_parameters->findByName(item->getName());
    if (parameter)
      parameter->value = item->getValue();

    saveParameters();
  } else {
    item->activate();
  }
}

void Menu::end()
{
  m_finished = true;
  m_current = m_items.begin();
  if (m_end_of_menu_callback)
    m_end_of_menu_callback();
}

void Menu::onShortPress()
{
  if ((*m_current)->isActive()) {
    (*m_current)->onShortPress();
  } else {
    m_current++;
    if (m_current==m_items.end())
      end();
  }
}

void Menu::saveParameters()
{
  // FIXME: prepsat
  EEPROM.begin(2048);
  m_parameters->storeToEEPROM();
  EEPROM.commit();
  EEPROM.end();
}

void Menu::draw(DisplayT *display, const Coords& coords)
{
  (*m_current)->draw(display, coords);
}

shared_ptr<MenuItem> Menu::getMenuItem(const String& name)
{
  for (auto item : m_items)
    if (item->getName() == name)
      return item;

  return nullptr;
}


void MenuItemNumericRange::draw(DisplayT *display, const Coords& coords)
{
  if (isActive()) {
    String text = m_display_name_short + " " + String(m_current);
    display->displayText(text, coords);
  } else {
    display->displayText(m_display_name, coords);
  }
}

const String MenuItemNumericRange::getValue() const
{
  String s = String(m_current);
  return s;
}

void MenuItemNumericRange::setValue(const String& value)
{
  m_current = value.toInt();
}

void MenuItemNumericRange::onShortPress()
{
  if (++m_current>m_end)
    m_current = m_start;

  if (m_onchange_cb)
    m_onchange_cb(getValue());
}

void MenuItemNumericRange::onLongPress()
{
  deactivate();
}


void MenuItemBoolean::draw(DisplayT *display, const Coords& coords)
{
  if (isActive()) {
    String text = m_display_name_short + (m_current ? " On" : " Off");
    display->displayText(text, coords);
  } else {
    display->displayText(m_display_name, coords);
  }
}

void MenuItemBoolean::onShortPress()
{
  m_current = !m_current;

  if (m_onchange_cb)
    m_onchange_cb(getValue());
}

void MenuItemBoolean::onLongPress()
{
  deactivate();
}

const String MenuItemBoolean::getValue() const
{
  String s = m_current ? "1" : "0";
  return s;
}

void MenuItemBoolean::setValue(const String& value)
{
  m_current = (value == "1");
}
