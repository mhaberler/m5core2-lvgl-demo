//=====================================================================
// LVGL : How-to
//      : M5Core2 slow fps. Scrol slider breaks when moving horizontally
// 2 Dec,2020
// https://forum.lvgl.io
//  /t/m5core2-slow-fps-scrol-slider-breaks-when-moving-horizontally/3931
// Arduino IDE 1.8.15
// Check : 2021.06.13 : macsbug
// https://macsbug.wordpress.com/2021/06/18/how-to-run-lvgl-on-m5stack-esp32/
//=====================================================================

#include <M5Core2.h>
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
uint32_t startTime, frame = 0; // For frames-per-second estimate

//=====================================================================
/*Read the touchpad*/
bool my_touchpad_read(lv_indev_drv_t * indev_driver,
                      lv_indev_data_t * data){
  TouchPoint_t pos = M5.Touch.getPressPoint();
  bool touched = ( pos.x == -1 ) ? false : true;
  if(!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR; 
    /*Set the coordinates*/
    data->point.x = pos.x;
    data->point.y = pos.y;
  }
  return false; 
//Return `false` because we are not buffering and no more data to read
}
//=====================================================================
/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, 
     const lv_area_t *area, lv_color_t *color_p){
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}
//=====================================================================
void setup(){
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
  //-------------------------------------------------------------------
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  lv_init();
  startTime = millis();
  
  //-------------------------------------------------------------------
  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = 320;
  disp_drv.ver_res  = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer   = &disp_buf;
  lv_disp_drv_register(&disp_drv);
  
  //-------------------------------------------------------------------
  /*Initialize the (dummy) input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  
  //-------------------------------------------------------------------
  /*Create a Tab view object*/
  lv_obj_t *tabview;
  tabview = lv_tabview_create(lv_scr_act(), NULL);
  
  //-------------------------------------------------------------------
  /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");
  
  //-------------------------------------------------------------------
  /*Add content to the tabs*/
  lv_obj_t * label = lv_label_create(NULL, NULL);
  static lv_anim_path_t path_overshoot;
  lv_anim_path_init(  &path_overshoot);
  lv_anim_path_set_cb(&path_overshoot, lv_anim_path_overshoot);

  static lv_anim_path_t path_ease_out;
  lv_anim_path_init(  &path_ease_out);
  lv_anim_path_set_cb(&path_ease_out, lv_anim_path_ease_out);

  static lv_anim_path_t path_ease_in_out;
  lv_anim_path_init(  &path_ease_in_out);
  lv_anim_path_set_cb(&path_ease_in_out, lv_anim_path_ease_in_out);
  
  //-------------------------------------------------------------------
  /*Gum-like button*/
  static lv_style_t style_gum;
  lv_style_init(&style_gum);
  lv_style_set_transform_width(   &style_gum, LV_STATE_PRESSED,  10);
  lv_style_set_transform_height(  &style_gum, LV_STATE_PRESSED, -10);
  lv_style_set_value_letter_space(&style_gum, LV_STATE_PRESSED,   5);
  lv_style_set_transition_path(   &style_gum, LV_STATE_DEFAULT, &path_overshoot);
  lv_style_set_transition_path(   &style_gum, LV_STATE_PRESSED, &path_ease_in_out);
  lv_style_set_transition_time(   &style_gum, LV_STATE_DEFAULT, 250);
  lv_style_set_transition_delay(  &style_gum, LV_STATE_DEFAULT, 100);
  lv_style_set_transition_prop_1( &style_gum, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
  lv_style_set_transition_prop_2( &style_gum, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);
  lv_style_set_transition_prop_3( &style_gum, LV_STATE_DEFAULT, LV_STYLE_VALUE_LETTER_SPACE);

  lv_obj_t * btn1 = lv_btn_create(tab1, NULL);
  lv_obj_align(    btn1, NULL, LV_ALIGN_CENTER, 0, -20);
  lv_obj_add_style(btn1, LV_BTN_PART_MAIN, &style_gum);
  
  //-------------------------------------------------------------------
  /*Instead of creating a label add a values string*/
  lv_obj_set_style_local_value_str(btn1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Gum");
  
  //-------------------------------------------------------------------
  /*Halo on press*/
  static lv_style_t style_halo;
  lv_style_init(&style_halo);
  lv_style_set_transition_time(  &style_halo, LV_STATE_PRESSED, 400);
  lv_style_set_transition_time(  &style_halo, LV_STATE_DEFAULT,   0);
  lv_style_set_transition_delay( &style_halo, LV_STATE_DEFAULT, 200);
  lv_style_set_outline_width(    &style_halo, LV_STATE_DEFAULT,   0);
  lv_style_set_outline_width(    &style_halo, LV_STATE_PRESSED,  20);
  lv_style_set_outline_opa(      &style_halo, LV_STATE_DEFAULT, LV_OPA_COVER);
    /*Just to be sure, the theme might use it*/
  lv_style_set_outline_opa(      &style_halo, LV_STATE_FOCUSED, LV_OPA_COVER);
  lv_style_set_outline_opa(      &style_halo, LV_STATE_PRESSED, LV_OPA_TRANSP);
  lv_style_set_transition_prop_1(&style_halo, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
  lv_style_set_transition_prop_2(&style_halo, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_WIDTH);

  lv_obj_t * btn2 = lv_btn_create(tab1, NULL);
  lv_obj_align(                     btn2, NULL, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_style(                 btn2, LV_BTN_PART_MAIN, &style_halo);
  lv_obj_set_style_local_value_str( btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Halo");
  
  //-------------------------------------------------------------------
  /*Ripple on press*/
  static lv_style_t style_ripple;
  lv_style_init(&style_ripple);
  lv_style_set_transition_time(  &style_ripple, LV_STATE_PRESSED, 300);
  lv_style_set_transition_time(  &style_ripple, LV_STATE_DEFAULT,   0);
  lv_style_set_transition_delay( &style_ripple, LV_STATE_DEFAULT, 300);
  lv_style_set_bg_opa(           &style_ripple, LV_STATE_DEFAULT,   0);
  lv_style_set_bg_opa(           &style_ripple, LV_STATE_PRESSED, LV_OPA_80);
  lv_style_set_border_width(     &style_ripple, LV_STATE_DEFAULT,   0);
  lv_style_set_outline_width(    &style_ripple, LV_STATE_DEFAULT,   0);
  lv_style_set_transform_width(  &style_ripple, LV_STATE_DEFAULT, -20);
  lv_style_set_transform_height( &style_ripple, LV_STATE_DEFAULT, -20);
  lv_style_set_transform_width(  &style_ripple, LV_STATE_PRESSED,   0);
  lv_style_set_transform_height( &style_ripple, LV_STATE_PRESSED,   0);

  lv_style_set_transition_path(  &style_ripple, LV_STATE_DEFAULT, &path_ease_out);
  lv_style_set_transition_prop_1(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_BG_OPA);
  lv_style_set_transition_prop_2(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
  lv_style_set_transition_prop_3(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);

  lv_obj_t * btn3 = lv_btn_create(tab1, NULL);
  lv_obj_align(                    btn3, NULL, LV_ALIGN_CENTER, 0, 140);
  lv_obj_add_style(                btn3, LV_BTN_PART_MAIN, &style_ripple);
  lv_obj_set_style_local_value_str(btn3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Ripple");

  label = lv_label_create(tab2, NULL);
  lv_label_set_text(label, "Second tab");

  label = lv_label_create(tab3, NULL);
  lv_label_set_text(label, "Third tab");
}
//=====================================================================
void loop(){
  lv_task_handler(); /* let the GUI do its work */
  /*
  // Show approximate frame rate
  if(!(++frame & 255)) { // Every 256 frames...
    uint32_t elapsed = (millis() - startTime) / 1000; // Seconds
    if(elapsed) {
      M5.Lcd.setCursor(278, 232);
      M5.Lcd.print(frame / elapsed); M5.Lcd.println(" fps");
    }
  }
  */
  delay(5);
}
//=====================================================================
