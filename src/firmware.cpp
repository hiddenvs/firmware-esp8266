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

#include "config_common.hpp"
#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include "firmware.hpp"

void Firmware::update(const String &update_url)
{
  if (update_url.length() < 3 || update_url=="") {
    DEBUG_SERIAL.println(F("[Update] Update url empty, skipping"));
    return;
  }

  String url = "http://" + update_url + "/esp/update?md5=" + ESP.getSketchMD5() + "&model=" + String(X_MODEL_NUMBER) + "&version=" + String(FIRMWARE_VERSION);
  //t_httpUpdate_return ret = ESPhttpUpdate.update(url,"","AF B9 78 3B E6 1D 70 AE E7 97 0A 50 D8 7B 1C 89 83 90 32 30");
  DEBUG_SERIAL.printf_P(PSTR("Update URL: '%s'\n"),url.c_str());
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);

  switch(ret) {
    case HTTP_UPDATE_FAILED:
      DEBUG_SERIAL.println(F("[update] Update failed."));
      DEBUG_SERIAL.printf_P(PSTR("[update] HTTP_UPDATE_FAILED Error (%d): %s\n"), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_SERIAL.println(F("[update] Update no Update."));
      break;
    case HTTP_UPDATE_OK:
      DEBUG_SERIAL.println(F("[update] Update ok.")); // may not called we reboot the ESP
      break;
  }
}
