// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.0
// LVGL version: 8.3.6
// Project name: CarUi

#include "../ui.h"

void ui_Screen2_screen_init(void)
{
    ui_Screen2 = lv_obj_create(NULL);

    lv_obj_clear_flag(ui_Screen2, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_Label1 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label1, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label1, -200);
    lv_obj_set_y(ui_Label1, 100);
    lv_obj_set_align(ui_Label1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label1, "100 %");

    ui_Bar1 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar1, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar1, 35);
    lv_obj_set_height(ui_Bar1, 80);
    lv_obj_set_x(ui_Bar1, -150);
    lv_obj_set_y(ui_Bar1, 47);
    lv_obj_set_align(ui_Bar1, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(ui_Bar1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar1, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Bar1, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar1, lv_color_hex(0x3030F0), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_Label2 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label2, -150);
    lv_obj_set_y(ui_Label2, 100);
    lv_obj_set_align(ui_Label2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label2, "400V");

    ui_Bar2 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar2, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar2, 35);
    lv_obj_set_height(ui_Bar2, 80);
    lv_obj_set_x(ui_Bar2, -200);
    lv_obj_set_y(ui_Bar2, 47);
    lv_obj_set_align(ui_Bar2, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(ui_Bar2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar2, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Bar2, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar2, lv_color_hex(0x3030F0), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_Label3 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label3, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label3, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label3, -200);
    lv_obj_set_y(ui_Label3, -15);
    lv_obj_set_align(ui_Label3, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label3, "10 °C");

    ui_Bar3 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar3, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar3, 35);
    lv_obj_set_height(ui_Bar3, 80);
    lv_obj_set_x(ui_Bar3, -200);
    lv_obj_set_y(ui_Bar3, -70);
    lv_obj_set_align(ui_Bar3, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(ui_Bar3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar3, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Bar3, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar3, lv_color_hex(0x3030F0), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar3, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_Label4 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label4, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label4, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label4, -150);
    lv_obj_set_y(ui_Label4, -15);
    lv_obj_set_align(ui_Label4, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label4, "10 °C");

    ui_Bar4 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar4, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar4, 35);
    lv_obj_set_height(ui_Bar4, 80);
    lv_obj_set_x(ui_Bar4, -150);
    lv_obj_set_y(ui_Bar4, -70);
    lv_obj_set_align(ui_Bar4, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(ui_Bar4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar4, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_Bar4, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Bar4, lv_color_hex(0x3030F0), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar4, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_Arc1 = lv_arc_create(ui_Screen2);
    lv_obj_set_width(ui_Arc1, 220);
    lv_obj_set_height(ui_Arc1, 220);
    lv_obj_set_align(ui_Arc1, LV_ALIGN_CENTER);
    lv_arc_set_value(ui_Arc1, 20);
    lv_arc_set_bg_angles(ui_Arc1, 180, 0);
    lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc1, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc1, false, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(ui_Arc1, 100, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc1, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc1, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(ui_Arc1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_Arc2 = lv_arc_create(ui_Screen2);
    lv_obj_set_width(ui_Arc2, 220);
    lv_obj_set_height(ui_Arc2, 220);
    lv_obj_set_align(ui_Arc2, LV_ALIGN_CENTER);
    lv_arc_set_bg_angles(ui_Arc2, 0, 180);
    lv_arc_set_mode(ui_Arc2, LV_ARC_MODE_REVERSE);
    lv_obj_set_style_arc_color(ui_Arc2, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc2, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc2, false, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(ui_Arc2, 100, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_Arc2, 50, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_Arc2, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_opa(ui_Arc2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_Label6 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label6, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label6, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label6, -5);
    lv_obj_set_y(ui_Label6, -5);
    lv_obj_set_align(ui_Label6, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label6, "100");

    ui_Label5 = lv_label_create(ui_Screen2);
    lv_obj_set_width(ui_Label5, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label5, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label5, 28);
    lv_obj_set_y(ui_Label5, 18);
    lv_obj_set_align(ui_Label5, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label5, "km/h");

    ui_Panel2 = lv_obj_create(ui_Screen2);
    lv_obj_set_width(ui_Panel2, 100);
    lv_obj_set_height(ui_Panel2, 30);
    lv_obj_set_x(ui_Panel2, 180);
    lv_obj_set_y(ui_Panel2, -60);
    lv_obj_set_align(ui_Panel2, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Panel2, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_border_width(ui_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Panel2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Panel2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Panel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label8 = lv_label_create(ui_Panel2);
    lv_obj_set_width(ui_Label8, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label8, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label8, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label8, "CAR_OFF");

    ui_Panel1 = lv_obj_create(ui_Screen2);
    lv_obj_set_width(ui_Panel1, 100);
    lv_obj_set_height(ui_Panel1, 30);
    lv_obj_set_x(ui_Panel1, 180);
    lv_obj_set_y(ui_Panel1, -100);
    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_Panel1, lv_color_hex(0xB39212), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_Panel1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Panel1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Label7 = lv_label_create(ui_Panel1);
    lv_obj_set_width(ui_Label7, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label7, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label7, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label7, "BOTS Open");

    ui_Bar6 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar6, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar6, 30);
    lv_obj_set_height(ui_Bar6, 120);
    lv_obj_set_x(ui_Bar6, 160);
    lv_obj_set_y(ui_Bar6, 50);
    lv_obj_set_align(ui_Bar6, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_Bar6, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Bar6, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar6, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_Bar5 = lv_bar_create(ui_Screen2);
    lv_bar_set_value(ui_Bar5, 25, LV_ANIM_OFF);
    lv_obj_set_width(ui_Bar5, 30);
    lv_obj_set_height(ui_Bar5, 120);
    lv_obj_set_x(ui_Bar5, 210);
    lv_obj_set_y(ui_Bar5, 50);
    lv_obj_set_align(ui_Bar5, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_Bar5, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Bar5, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Bar5, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
}
