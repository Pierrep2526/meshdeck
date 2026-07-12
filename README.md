# MeshDeck OS

Your own standalone MeshCore firmware for the **LilyGo T-Deck** — a free, open
alternative to MeshOS, built on the MIT-licensed [MeshCore](https://github.com/meshcore-dev/MeshCore)
stack. No licence fee, no serial number, fully yours to modify.

Configured for **868 MHz UK/EU** (MeshCore standard preset 869.618 / BW 62.5 / SF 8).

## Features

**Chat**
- Speech-bubble chat UI at full 320x240 resolution, touch & trackball scrolling
- Tabbed channels + DM threads, colour-coded usernames
- Delivery reports (double ticks), send-failure marks, message timestamps
- Tap a message to quote-reply; tap a message containing a link to show it as a **QR code** for your phone
- Emoji rendering (smilies, heart, thumbs-up, fire...)
- Message history survives reboots (saved to flash)
- New-message popups, wake-on-message, alert tones with volume control

**Map**
- Built-in offline vector map - no tile files or SD card needed
- Worldwide coastlines + extra detail for UK/Europe, 285 city labels, 9 zoom levels
- Shows your contacts, repeaters (orange diamonds) and your own position live
- Pan with trackball or touch-drag, `+`/`-` zoom, `u` = UK view, `w` = world
- **SD map packs**: drop `.mdm` files in `/meshdeck-maps/` on an SD card for
  extra high-detail regions - a ready-made UK+Ireland pack is included in
  `sdcard/`, and `tools/gen_sdmap.py` builds packs for any region on Earth

**Mesh power tools**
- **Last heard** list: signal, RSSI, hops, age and distance of recent stations
- **Noise floor monitor**: realtime 80-second graph, min/max/avg, last-RX readout
- **Trace route** with per-hop SNR bars
- **Repeater manager**: separate repeater list, login, one-key `stats` / `neighbors` / `advert` / `clock sync`
- **Full terminal**: every mesh event streams into a colour terminal; send DMs,
  channel messages, repeater CLI commands, change radio settings - also mirrored
  to USB serial so you can use it from a computer
- Home screen with big clock, mesh signal bars, battery and unread badge

**Extras MeshOS doesn't have**
- **BLE companion mode still works** - pair the official MeshCore phone app any time (PIN 123456)
- Open source: every feature is editable C++
- Firmware update from SD card (`firmware.bin` on card → Settings → Update)

## Layout

```
firmware/              the firmware source (drops into MeshCore's examples/)
  main.cpp             entry point
  MyMesh.cpp/.h        MeshCore companion core + MeshDeck event hooks
  ui/                  the MeshDeck UI (screens, drivers, map data)
platformio.local.ini   build environment (MeshDeck_TDeck_868)
build.sh / flash.sh    one-command build + flash
webflasher/            your own browser-based flasher page (GitHub Pages)
sdcard/meshdeck-maps/  ready-made UK+Ireland high-detail map pack (uk.mdm)
.github/workflows/     cloud build + web flasher deploy (GitHub Actions)
tools/gen_mapdata.py   regenerates the built-in map from Natural Earth data
tools/gen_sdmap.py     builds SD map packs (.mdm) for any region
```

## Quick start

```
./build.sh     # compile (first run ~10 min)
./flash.sh     # flash over USB
```

Full beginner instructions, download-mode steps and a no-install cloud build
path are in **FLASHING.md**.

## Controls

| Input | Action |
|---|---|
| Trackball | move / scroll / pan |
| Trackball click | select / send |
| Trackball long-press | back |
| Keyboard | type; enter = send/confirm, backspace on empty = back |
| Touch | tap = select, drag = scroll/pan, swipe right = back |
| Home screen keys 1-9 | quick-launch apps |

## Credits & licences

- [MeshCore](https://github.com/meshcore-dev/MeshCore) mesh stack - MIT, (c) meshcore-dev contributors
- Map data: [Natural Earth](https://www.naturalearthdata.com/) - public domain
- QR encoding: [qrcodegen](https://github.com/nayuki/QR-Code-generator) - MIT, (c) Project Nayuki
- MeshDeck UI written for this project - MIT. Not affiliated with the MeshCore
  store or MeshOS; "MeshOS" is used only to describe feature parity.
