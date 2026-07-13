#include "AllScreens.h"
#include <helpers/TxtDataHelpers.h>
#include <ctype.h>

/*
 * Download map packs over WiFi. On enter the screen fetches maps/index.txt
 * from the flasher site, lists every available region, and lets you type to
 * filter and press enter to pull the selected .mdm onto the SD card.
 */

#define MD_ROW_H 18
#define MD_TOP   (STATUS_H + 4)
#define MD_VIS   ((SCREEN_H - MD_TOP - 16) / MD_ROW_H)

static bool ci_contains(const char* hay, const char* needle) {
  if (!needle[0]) return true;
  for (const char* h = hay; *h; h++) {
    const char* a = h; const char* b = needle;
    while (*a && *b && tolower((uint8_t)*a) == tolower((uint8_t)*b)) { a++; b++; }
    if (!*b) return true;
  }
  return false;
}

void MapDownloadScreen::enter() {
  _sel = 0; _top = 0; _flen = 0; _filter[0] = 0;
  _n = 0; _nf = 0; _status = nullptr;
  if (ui.wifiState() != 2) {
    _status = "Connect WiFi first (Settings > WiFi)";
    return;
  }
  ui.toast("Fetching map list...", C_CYAN);
  loadIndex();
}

void MapDownloadScreen::loadIndex() {
  static char buf[4096];
  int len = 0;
  const char* err = ui.downloadMapIndex(buf, sizeof(buf), len);
  if (err) { _status = err; return; }

  _n = 0;
  char* p = buf;
  while (*p && _n < MAPDL_MAX) {
    char* nl = strchr(p, '\n');
    if (nl) *nl = 0;
    char* bar = strchr(p, '|');
    if (bar && p[0]) {
      *bar = 0;
      StrHelper::strncpy(_codes[_n], p, sizeof(_codes[_n]));
      StrHelper::strncpy(_names[_n], bar + 1, sizeof(_names[_n]));
      // trim a trailing CR if the file has CRLF line endings
      int nlen = strlen(_names[_n]);
      if (nlen > 0 && _names[_n][nlen - 1] == '\r') _names[_n][nlen - 1] = 0;
      _n++;
    }
    if (!nl) break;
    p = nl + 1;
  }
  _status = _n ? nullptr : "No regions found";
  rebuildFilter();
}

void MapDownloadScreen::rebuildFilter() {
  _nf = 0;
  for (int i = 0; i < _n; i++)
    if (ci_contains(_names[i], _filter)) _filt[_nf++] = i;
  if (_sel >= _nf) _sel = _nf > 0 ? _nf - 1 : 0;
  if (_sel < 0) _sel = 0;
  _top = 0;
}

void MapDownloadScreen::downloadSel() {
  if (_nf == 0) return;
  int idx = _filt[_sel];
  const char* err = ui.downloadMapPack(_codes[idx]);
  if (err) {
    ui.toast(err, C_RED);
  } else {
    char msg[40];
    snprintf(msg, sizeof(msg), "%s map saved", _names[idx]);
    ui.toast(msg, C_GREEN);
  }
}

void MapDownloadScreen::draw() {
  GFXcanvas16& c = ui.cv();
  c.fillScreen(C_BG);
  ui.drawStatusBar("Download maps");
  c.setTextSize(1);

  if (_status) {
    c.setTextColor(C_YELLOW);
    c.setCursor(10, 60);
    c.print(_status);
    c.setTextColor(C_FG_FAINT);
    c.setCursor(10, SCREEN_H - 12);
    c.print("back to return");
    return;
  }

  if (_sel < _top) _top = _sel;
  if (_sel >= _top + MD_VIS) _top = _sel - MD_VIS + 1;

  for (int r = _top; r < _nf && r < _top + MD_VIS; r++) {
    int i = _filt[r];
    int y = MD_TOP + (r - _top) * MD_ROW_H;
    bool sel = r == _sel;
    if (sel) c.fillRoundRect(2, y - 1, SCREEN_W - 4, MD_ROW_H - 1, 4, C_BG_RAISED);
    c.setTextColor(sel ? C_FG : C_FG_DIM);
    c.setCursor(8, y + 4);
    c.print(_names[i]);
    if (sel) {
      c.setTextColor(C_ACCENT);
      c.setCursor(SCREEN_W - 8 - 8 * 6, y + 4);
      c.print("download");
    }
  }

  // filter / hint bar
  c.setTextColor(C_FG_FAINT);
  c.setCursor(6, SCREEN_H - 12);
  if (_flen > 0) {
    char line[48];
    snprintf(line, sizeof(line), "search: %s   (%d)", _filter, _nf);
    c.setTextColor(C_YELLOW);
    c.print(line);
  } else {
    c.print("type to search   enter = download");
  }
}

bool MapDownloadScreen::key(uint8_t k) {
  if (_status) return false;
  if (k == 0x0D) { downloadSel(); return true; }
  if (k == 0x08 || k == 0x7F) {          // backspace / delete
    if (_flen > 0) { _filter[--_flen] = 0; rebuildFilter(); }
    return true;
  }
  if (k >= 32 && k < 127 && _flen < (int)sizeof(_filter) - 1) {
    _filter[_flen++] = k;
    _filter[_flen] = 0;
    rebuildFilter();
    return true;
  }
  return false;
}

bool MapDownloadScreen::nav(NavEvent e) {
  if (_status) return false;
  switch (e) {
    case NAV_UP:     if (_sel > 0) _sel--; return true;
    case NAV_DOWN:   if (_sel < _nf - 1) _sel++; return true;
    case NAV_SELECT: downloadSel(); return true;
    default: return false;
  }
}

bool MapDownloadScreen::touch(const TouchEvent& e) {
  if (_status) return false;
  if (e.kind != TouchEvent::TAP) return false;
  if (e.y < MD_TOP) return false;
  int r = _top + (e.y - MD_TOP) / MD_ROW_H;
  if (r >= 0 && r < _nf) { _sel = r; downloadSel(); return true; }
  return false;
}
