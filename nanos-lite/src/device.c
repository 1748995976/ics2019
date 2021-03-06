#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  //_yield();
  for(int i=0;i<len;i++){
    _putc(((char *)buf)[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};
extern void set_pcb_id(int id);
size_t events_read(void *buf, size_t offset, size_t len) {
  //_yield();
  //sprintf(buf,"t %d\n",uptime());
  //Log("here");
  //uint32_t time = uptime();
  //Log("time: %d",time);
  //sprintf(buf,"t %d\n",uptime());
  //sprintf(buf,"t \n");
  int key = read_key();
  //Log("key: %x",key);
  bool down =false;
  if(key & 0x8000){
    key ^=0x8000;
    down = true;
    if(key == _KEY_1)set_pcb_id(1);
    else if(key == _KEY_2)set_pcb_id(2);
    else if(key == _KEY_3)set_pcb_id(3);
  }
  if(key == _KEY_NONE){
    //uint32_t time = uptime();
    unsigned long time = uptime();
    sprintf(buf,"t %d\n",time);
  }else{
    //sprintf(buf,"%s %s\n",down ? "kd":"ku",keyname[key]);
    sprintf(buf,"%s %s\n",down ? "kd":"ku",keyname[key]);
  }

  return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  //Log("dispinfo_read\n");
  //if(len+offset>128)len=128-offset;
  if(len+offset>strlen(dispinfo))len=strlen(dispinfo)-offset;
  strncpy(buf, &dispinfo[offset], len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  //Log("fb_write\n");
  //_yield();
  
  int x,y;
  offset = offset>>2;
  y = offset / screen_width();
  x = offset % screen_width();
  //printf("%d %d\n",x,y);
  int lenth = len>>2;
  int len1,len2=0,len3=0;

  len1 = lenth<= (screen_width()-x) ? lenth : screen_width()-x;
  draw_rect((uint32_t*)buf,x,y,len1,1);

  if((lenth>len1)&&((lenth-len1)>screen_width())){
    len2 = lenth-len1;
    draw_rect((uint32_t*)buf+len1,0,y+1,screen_width(),len2/screen_width());
  }

  if(lenth-len1-len2>0){
    len3 = lenth-len1-len2;
    draw_rect((uint32_t*)buf+len1+len2,0,y+len2/screen_width()+1,len3,1);
  }
	//draw_rect((uint32_t *)buf,x,y,lenth,1);
  return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  draw_sync();
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();
  //Log("here\n");

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
