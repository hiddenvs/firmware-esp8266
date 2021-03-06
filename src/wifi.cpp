/* 
  Cryptoclock ESP8266
  Copyright (C) 2018 www.cryptoclock.net <info@cryptoclock.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "parameter_store.hpp"
#include "wifi.hpp"
#include "utils.hpp"

extern ParameterStore g_parameters;
extern WiFiCore *g_wifi;

WiFiCore::WiFiCore(DisplayT *display) :
  m_wifimanager(WiFiManager()), m_display(display)
{
  m_wifimanager.setSaveConfigCallback(&saveCallback);

  AP_list::addAPsToWiFiManager(&m_wifimanager);
  addParametersFromGlobal();

  m_ev_conn = WiFi.onStationModeConnected(onConnect);
  m_ev_disconn = WiFi.onStationModeDisconnected(onDisconnect);
  m_ev_gotip = WiFi.onStationModeGotIP(onGotIP);
}

void WiFiCore::setAPCallback(void (*func)(WiFiManager*))
{
  m_wifimanager.setAPCallback(func);
}

void WiFiCore::connectToWiFiOrFallbackToAP(void)
{
  if (!m_wifimanager.autoConnect()) {
    DEBUG_SERIAL.println(F("[WiFiCore] Failed to connect and hit timeout"));
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
}

void WiFiCore::addParametersFromGlobal()
{
  m_parameters.clear();
  m_parameters.reserve(g_parameters.all_items().size());

  g_parameters.iterateAllParameters([this](const ParameterItem* item) {
    if (!item->name.startsWith("__")) {// reserved parameter, don't display
      auto *wifi_param = new WiFiManagerParameter(item->name.c_str(), item->description.c_str(), item->value.c_str(), item->field_length);
      m_parameters.push_back(wifi_param);
      m_wifimanager.addParameter(wifi_param);
    }
  });
}

void WiFiCore::startAP(const String& ssid_name, unsigned long timeout)
{
  m_wifimanager.setTimeout(timeout);
  if (!m_wifimanager.startConfigPortal(ssid_name.c_str())) {
    DEBUG_SERIAL.println(F("[WiFiCore] Failed to start AP and hit timeout"));
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  m_wifimanager.setTimeout(0);
}

void WiFiCore::resetSettings()
{
  m_wifimanager.resetSettings();
}

void WiFiCore::updateParametersFromAP(WiFiManager *manager)
{
  // save parameters
  for (auto *wifi_param : m_parameters) {
    String id = wifi_param->getID();
    String new_value = wifi_param->getValue();
    auto *param = g_parameters.findByName(id);
    if (param)
      param->value = new_value;
  }
}

void WiFiCore::saveCallback(void)
{
  DEBUG_SERIAL.println(F("[WiFiCore] Save callback called"));

  Utils::eeprom_BEGIN();
  // save APs
  auto manager = g_wifi->getWiFiManager();
  AP_list::saveAPsToEEPROM(manager);

  // save parameters
  g_wifi->updateParametersFromAP(manager);
  g_parameters.storeToEEPROMwithoutInit();

  Utils::eeprom_END();
}

void WiFiCore::onConnect(WiFiEventStationModeConnected event_info)
{
  DEBUG_SERIAL.printf_P(PSTR("[WiFiCore] Connected to SSID: %s channel %i\n"),
    event_info.ssid.c_str(), event_info.channel);
}

void WiFiCore::onDisconnect(WiFiEventStationModeDisconnected event_info)
{
  DEBUG_SERIAL.printf_P(PSTR("[WiFiCore] Disconnected from SSID: %s\n"), event_info.ssid.c_str());
  DEBUG_SERIAL.printf_P(PSTR("[WiFiCore] Reason: %d\n"), event_info.reason);
}

void WiFiCore::onGotIP(WiFiEventStationModeGotIP ipInfo)
{
  DEBUG_SERIAL.printf_P(PSTR("[WiFiCore] Got IP: %s Gateway: %s, Mask: %s\n"),
    ipInfo.ip.toString().c_str(), ipInfo.gw.toString().c_str(), ipInfo.mask.toString().c_str()
  );
}
