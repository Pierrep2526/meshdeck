#include "AllScreens.h"
#include <helpers/TxtDataHelpers.h>
#include <WiFi.h>

/*
 * WiFi join screen (reachable from a home tile). Three rows: network name,
 * password, and connect/disconnect. Text entry uses the same number/symbol
 * layer as the rest of the UI: roll the trackball up while editing for digits,
 * or press Alt+C.
 */

#define W_ROW_Y0 82
#define W_ROW_H  30

void WifiScreen::enter() {
  _editing = false;
  _sel = 0;
}

void WifiScreen::draw() {
  GFXcanvas16& c = ui.cv();
  c.fillScreen(C_BG);
  ui.drawStatusBar("WiFi");
  c.setTextSize(1);
  int ws = ui.wifiState();

  // big status banner
  c.setCursor(12, STATUS_H + 12);
  c.setTextSize(2);
  c.setTextColor(ws == 2 ? C_GREEN : ws == 1 ? C_YELLOW : C_FG_DIM);
  c.print(ws == 2 ? "Connected" : ws == 1 ? "Connecting..." : "Not connected");
  c.setTextSize(1);
  if (ws == 2) {
    char ip[28];
    snprintf(ip, sizeof(ip), "IP %s", WiFi.localIP().toString().c_str());
    c.setTextColor(C_FG_FAINT);
    c.setCursor(14, STATUS_H + 32);
    c.print(ip);
  }

  const char* labels[3] = { "Network (SSID)", "Password", ws == 0 ? "Connect" : "Disconnect" };
  for (int i = 0; i < 3; i++) {
    int y = W_ROW_Y0 + i * W_ROW_H;
    bool sel = i == _sel;
    if (sel) c.fillRoundRect(4, y - 2, SCREEN_W - 8, W_ROW_H - 4, 5, C_BG_RAISED);
    c.setTextColor(sel ? C_FG : C_FG_DIM);
    c.setCursor(12, y + 6);
    c.print(labels[i]);

    if (_editing && i == _sel) {
      _edit[_elen] = 0;
      c.fillRect(SCREEN_W - 150, y, 146, 16, C_BG);
      c.drawRect(SCREEN_W - 150, y, 146, 16, C_ACCENT);
      c.setTextColor(C_YELLOW);
      c.setCursor(SCREEN_W - 144, y + 4);
      c.print(_edit);
      c.fillRect(SCREEN_W - 144 + _elen * 6 + 1, y + 3, 2, 10, C_ACCENT);
      if (ui.symShift()) { c.setTextColor(C_CYAN); c.setCursor(SCREEN_W - 26, y + 4); c.print("123"); }
    } else {
      char v[24];
      if (i == 0)      ellipsize(v, 16, ui.wifiSsid()[0] ? ui.wifiSsid() : "(not set)");
      else if (i == 1) strcpy(v, ui.wifiPass()[0] ? "****" : "(none)");
      else             strcpy(v, ">");
      c.setTextColor(sel ? C_ACCENT : C_FG_FAINT);
      c.setCursor(SCREEN_W - 12 - strlen(v) * 6, y + 6);
      c.print(v);
    }
  }

  // hint
  c.setCursor(6, SCREEN_H - 10);
  if (_editing) {
    c.setTextColor(ui.symShift() ? C_CYAN : C_FG_FAINT);
    c.print(ui.symShift() ? "123 mode (ball down=letters)  enter=save"
                          : "roll ball up for 123/symbols  enter=save");
  } else {
    c.setTextColor(C_FG_FAINT);
    c.print("up/down = choose   enter = edit / connect");
  }
}

void WifiScreen::select() {
  if (_sel == 0) {
    _editing = true;
    StrHelper::strncpy(_edit, ui.wifiSsid(), sizeof(_edit));
    _elen = strlen(_edit);
  } else if (_sel == 1) {
    _editing = true;
    StrHelper::strncpy(_edit, ui.wifiPass(), sizeof(_edit));
    _elen = strlen(_edit);
  } else {
    if (ui.wifiState() == 0) {
      if (!ui.wifiSsid()[0]) { ui.toast("Set a network name first", C_YELLOW); return; }
      ui.wifiConnect();
    } else {
      ui.wifiOff();
    }
  }
}

void WifiScreen::applyEdit() {
  _edit[_elen] = 0;
  if (_sel == 0) {
    StrHelper::strncpy(ui.wifiSsid(), _edit, 33);
    ui.saveWifi();
    ui.toast("Network saved");
  } else if (_sel == 1) {
    StrHelper::strncpy(ui.wifiPass(), _edit, 65);
    ui.saveWifi();
    ui.toast("Password saved");
  }
  _editing = false;
}

bool WifiScreen::key(uint8_t k) {
  if (_editing) {
    if (k == 0x0D) { applyEdit(); return true; }
    if (k == 0x08 || k == 0x7F) {
      if (_elen > 0) _elen--;
      else _editing = false;
      return true;
    }
    if (k >= 32 && k < 127 && _elen < (int)sizeof(_edit) - 2) { _edit[_elen++] = k; return true; }
    return true;
  }
  if (k == 0x0D) { select(); return true; }
  return false;
}

bool WifiScreen::nav(NavEvent e) {
  if (_editing) {
    if (e == NAV_SELECT) { applyEdit(); return true; }
    if (e == NAV_BACK) { _editing = false; return true; }
    if (e == NAV_UP   && !ui.symShift()) ui.toggleSym();   // up = numbers
    if (e == NAV_DOWN &&  ui.symShift()) ui.toggleSym();   // down = letters
    return true;
  }
  switch (e) {
    case NAV_UP:     if (_sel > 0) _sel--; return true;
    case NAV_DOWN:   if (_sel < 2) _sel++; return true;
    case NAV_SELECT: select(); return true;
    default: return false;
  }
}

bool WifiScreen::touch(const TouchEvent& e) {
  if (_editing) return false;
  if (e.kind != TouchEvent::TAP) return false;
  if (e.y < W_ROW_Y0) return false;
  int r = (e.y - W_ROW_Y0) / W_ROW_H;
  if (r >= 0 && r < 3) { _sel = r; select(); return true; }
  return false;
}
