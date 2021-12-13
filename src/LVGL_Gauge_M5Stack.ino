//=====================================================================
// GAUGE   : 2021.06.15 : macsbug
//---------------------------------------------------------------------
// Development environment : Arduino IDE 1.8.15
// LVGL    : v7.11.0-dev : https://docs.lvgl.io/7.11/
// Document: https://docs.lvgl.io/latest/en/html/widgets/
// Tabview : https://docs.lvgl.io/latest/en/html/widgets/tabview.html
// Label   : https://docs.lvgl.io/latest/en/html/widgets/label.html
// Guage   : https://docs.lvgl.io/latest/en/html/widgets/gauge.html
// L_Meter : https://docs.lvgl.io/latest/en/html/widgets/linemeter.html
// Forum   : https://forum.lvgl.io
// ESP32   : LVGL : https://programmersought.com/article/10128234373/
// HARD    : M5Stack.h
//    https://github.com/m5stack/M5Stack/blob/master/src/M5Stack.h#L19   
// BLOG    : How to run LVGL on M5Stack ( ESP32 )
//         : https://macsbug.wordpress.com/2021/06/18/
// BLOG    : ButtonA on M5Stack does not work properly
//         : https://macsbug.wordpress.com/2021/05/02/
// BLOG    : LVGL with M5Stack Part 2
//   https://macsbug.wordpress.com/2021/07/03/lvgl-with-m5stack-part-2/
//=====================================================================

#pragma GCC optimize ("Ofast")
#include <M5Core2.h>
#include <lvgl.h>
int screenWidth  = 320;
int screenHeight = 240;
#define LVGL_TICK_PERIOD 30
#include <Ticker.h>
Ticker tick;
#include <driver/adc.h>
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

static lv_obj_t * gauge;
uint32_t lastMillis = 0, pointer = 0, inc = 1;
//int adc1_ch7 = 35; // GPIO 35 Input


// Initialize the display =============================================
void Initialize_the_display(){
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = screenWidth;
  disp_drv.ver_res  = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer   = &disp_buf;
  lv_disp_drv_register(&disp_drv);
}
//=====================================================================

// Display flushing ===================================================
void my_disp_flush(
  lv_disp_drv_t *disp,const lv_area_t *area,lv_color_t *color_p){
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  M5.Lcd.startWrite();
  M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
  M5.Lcd.pushColors(&color_p->full, w*h, true);
  M5.Lcd.endWrite();
  lv_disp_flush_ready(disp);
}
//=====================================================================


//=====================================================================
void loop() {
  lv_task_handler(); delay(5);
  
  if(millis() - lastMillis > 20 ){
    lastMillis = millis();
    pointer = pointer + inc;
    if ( pointer > 100 ){ inc = -1;}
    if ( pointer <   1 ){ inc =  1;}
  //pointer = analogRead(adc1_ch7);       // Read GPIO 35 
    lv_gauge_set_value(gauge, 0, pointer);// next gauge pointer
  } 
}

//=====================================================================
void setup() {
  M5.begin(true, true, true, true);      // LCD,SD,Serial,I2C
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLUE);
  //pinMode(adc1_ch7,INPUT);             // Read GPIO
  //adc_power_acquire(); // ADC Power ON, A Button(39) error prevention
  // WiFi.Begin(); WiFi.setSleep(false); // A Button error prevention
  lv_init();
  Initialize_the_display(); // Initialize the display, Display flushing
  //-------------------------------------------------------------------

  // Create a gauge ---------------------------------------------------
  static lv_color_t needle_colors[1];
  needle_colors[0] = LV_COLOR_BLUE;
  gauge = lv_gauge_create(lv_scr_act(), NULL);
  lv_gauge_set_needle_count(gauge, 1, needle_colors);
  lv_obj_set_size(gauge, 230, 230);
  lv_obj_align(gauge, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_gauge_set_value(gauge, 0, 0); // value
  //-------------------------------------------------------------------
}
//=====================================================================


//=====================================================================
