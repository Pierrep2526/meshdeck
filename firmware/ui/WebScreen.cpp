#include "UITask.h"
#include "Theme.h"
#include <WiFi.h>
#include <WebServer.h>

/*
 * Remote screen: a tiny web server that mirrors the T-Deck's framebuffer to a
 * browser over WiFi and feeds clicks/keystrokes back as input. All our own
 * code (no GPL sources) so MeshDeck stays MIT.
 *
 *   GET /                -> the viewer page (canvas + JS)
 *   GET /screen.rgb565   -> raw RGB565 framebuffer (width*height*2 bytes)
 *   GET /tap?x=&y=       -> inject a screen tap
 *   GET /key?c=          -> inject a key (ASCII code)
 *   GET /nav?e=          -> inject a nav event (1=up 2=down 3=left 4=right)
 */

static const char PAGE[] PROGMEM = R"HTML(<!doctype html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>MeshDeck remote</title>
<style>
body{background:#0d0f12;color:#cdd6de;font-family:system-ui,sans-serif;text-align:center;margin:0;padding:10px}
h3{font-weight:600;margin:6px}
canvas{border:2px solid #2b3138;border-radius:6px;image-rendering:pixelated;
 width:320px;height:240px;max-width:96vw;touch-action:none;cursor:pointer}
p{color:#8a949c;font-size:13px}
</style></head><body>
<h3>MeshDeck &mdash; remote screen</h3>
<canvas id="s" width="320" height="240"></canvas>
<p>click to tap &middot; type to enter text &middot; arrow keys to navigate</p>
<script>
const c=document.getElementById('s'),g=c.getContext('2d'),W=320,H=240;
const img=g.createImageData(W,H),d=img.data;
async function frame(){
 try{
  const r=await fetch('/screen.rgb565?'+Date.now());
  const b=new Uint8Array(await r.arrayBuffer());
  for(let i=0,j=0;i<W*H;i++,j+=2){
   const v=b[j]|(b[j+1]<<8);
   d[i*4]  =((v>>11)&31)*255/31|0;
   d[i*4+1]=((v>>5)&63)*255/63|0;
   d[i*4+2]=(v&31)*255/31|0;
   d[i*4+3]=255;
  }
  g.putImageData(img,0,0);
 }catch(e){}
 setTimeout(frame,250);
}
frame();
c.addEventListener('click',e=>{
 const r=c.getBoundingClientRect();
 const x=Math.round((e.clientX-r.left)*W/r.width);
 const y=Math.round((e.clientY-r.top)*H/r.height);
 fetch('/tap?x='+x+'&y='+y);
});
document.addEventListener('keydown',e=>{
 const nav={ArrowUp:1,ArrowDown:2,ArrowLeft:3,ArrowRight:4};
 if(nav[e.key]!==undefined){fetch('/nav?e='+nav[e.key]);e.preventDefault();return;}
 const sp={Enter:13,Escape:27,Backspace:8,Tab:9};
 let code=sp[e.key]!==undefined?sp[e.key]:(e.key.length===1?e.key.charCodeAt(0):0);
 if(code){fetch('/key?c='+code);e.preventDefault();}
});
</script></body></html>)HTML";

void UITask::startRemoteScreen() {
  if (_remote_on) return;
  if (wifiState() != 2) { toast("Connect WiFi first", C_YELLOW); return; }

  _web = new WebServer(80);

  _web->on("/", [this]() {
    _web->send_P(200, "text/html", PAGE);
  });

  _web->on("/screen.rgb565", [this]() {
    GFXcanvas16& c = cv();
    size_t n = (size_t)c.width() * (size_t)c.height() * 2;
    _web->setContentLength(n);
    _web->send(200, "application/octet-stream", "");
    _web->client().write((const uint8_t*)c.getBuffer(), n);
  });

  _web->on("/tap", [this]() {
    _inj_x = (int16_t)_web->arg("x").toInt();
    _inj_y = (int16_t)_web->arg("y").toInt();
    _inj_tap = true;
    _web->send(200, "text/plain", "ok");
  });

  _web->on("/key", [this]() {
    _inj_key = (uint8_t)_web->arg("c").toInt();
    _web->send(200, "text/plain", "ok");
  });

  _web->on("/nav", [this]() {
    int e = _web->arg("e").toInt();
    if (e >= 1 && e <= 6) _inj_nav = e;
    _web->send(200, "text/plain", "ok");
  });

  _web->begin();
  _remote_on = true;
  snprintf(_remote_url, sizeof(_remote_url), "http://%s/", WiFi.localIP().toString().c_str());
  toast("Remote screen ON", C_GREEN);
}

void UITask::stopRemoteScreen() {
  if (_web) { _web->stop(); delete _web; _web = nullptr; }
  _remote_url[0] = 0;
  if (_remote_on) toast("Remote screen off", C_YELLOW);
  _remote_on = false;
}

const char* UITask::remoteScreenURL() { return _remote_url; }
