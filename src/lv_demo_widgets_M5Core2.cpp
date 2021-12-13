//=====================================================================
// M5Core2 lv_demo_widgets : Program porting : 2021.06.12 : macsbug
// IDE     : Arduino IDE 1.8.15
// Boards  : Boards Manager : ESP32 by Espresso Systems version 1.06
// HARD    : M5Stack Core2
// Library : TFT-eSPI   : https://github.com/Bodmer/TFT_eSPI
// Library : Manager    : lv_arduino by Pavel Brychta version 3.0.1
// Library : lv_arduino : https://github.com/lvgl/lv_arduino
// Sketch  : lv_demo_widgets
//  https://github.com/lvgl/lv_examples/tree/master/src/lv_demo_widgets
// LVGL    : https://lvgl.io : Forum : https://forum.lvgl.io
// LGFX    : https://github.com/lovyan03/LovyanGFX/
//        blob/master/examples/HowToUse/2_spi_setting/2_spi_setting.ino
// BLOG    : How to run LVGL on M5Stack ( ESP32 )
// https://macsbug.wordpress.com/2021/06/18/how-to-run-lvgl-on-m5stack-esp32/
// BLOG    : LVGL with M5Stack Part 2
// https://macsbug.wordpress.com/2021/07/03/lvgl-with-m5stack-part-2/
// BLOG    : CO2 monitor with M5Stack and LVGL
// https://macsbug.wordpress.com/2021/07/05/co2-monitor-with-m5stack-and-lvgl/
//=====================================================================

#pragma GCC optimize("Ofast")
#include "lv_demo_widgets.h"
#include <Arduino.h>
#include <M5Core2.h>
#include <SPI.h>
#include <Wire.h>
#include <lvgl.h>
TFT_eSPI tft = TFT_eSPI();
int screenWidth = 320;
int screenHeight = 240;
#define LVGL_TICK_PERIOD 30
#include <Ticker.h>
Ticker tick;
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//=====================================================================
// Read the touchpad --------------------------------------------------
bool my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  TouchPoint_t pos = M5.Touch.getPressPoint();
  bool touched = (pos.x == -1) ? false : true;
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = pos.x;
    data->point.y = pos.y;
    Serial.print(pos.x);
    Serial.print(" : ");
    Serial.println(pos.y);
  }
  return false;
}


// Display flushing 
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  M5.Lcd.startWrite();
  M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
  M5.Lcd.pushColors(&color_p->full, w * h, true);
  M5.Lcd.endWrite();
  lv_disp_flush_ready(disp);
}

// Initialize the display 
void Initialize_the_display() {
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
}

// Interrupt driven periodic handler
static void lv_tick_handler(void) { lv_tick_inc(LVGL_TICK_PERIOD); }

//=====================================================================
void setup() {
  M5.begin(true, true, true, true);
  tft.begin();
  tft.setRotation(1);
  M5.Axp.SetLcdVoltage(2800);
  M5.Axp.SetLcdVoltage(3300);
  M5.Axp.SetBusPowerMode(0);
  M5.Axp.SetCHGCurrent(AXP192::kCHG_190mA);
  M5.Axp.SetLDOEnable(3, true);
  delay(150);
  M5.Axp.SetLDOEnable(3, false);
  M5.Axp.SetLed(1);
  delay(100);
  M5.Axp.SetLed(0);
  M5.Axp.SetLDOVoltage(3, 3300);
  M5.Axp.SetLed(1);
  lv_init();
  Initialize_the_display();
  lv_demo_widgets();
}



void loop() {
  lv_tick_handler();
  lv_task_handler(); // msec,loop,Timer interrupt or in an Operation
  delay(5);
}

