#include <pebble.h>
#include "menu.h"
#include "main.h"

#define MENU_ROW_COUNT_COMMANDS 10
#define MENU_ROW_COUNT_SETTINGS 4

static Window *window;
static MenuLayer *s_menu;

static int16_t s_scroll_index;
static int16_t s_scroll_row_index = -1;
static AppTimer *s_scroll_timer;
static char tmp_menu_item[100];

int last_s_scroll_row_index = -1;
static int scroll_inc = 1;

int app_timeout_ms = 1000;

char** gsmenu_items;
int count;

static char s_subtext[1][32];



static uint16_t menu_num_sections(struct MenuLayer* menu, void* callback_context);
static uint16_t menu_num_rows(struct MenuLayer* menu, uint16_t section_index, void* callback_context);
static int16_t menu_cell_height(struct MenuLayer *menu, MenuIndex *cell_index, void *callback_context);
static int16_t menu_header_height(struct MenuLayer *menu, uint16_t section_index, void *callback_context);
static void menu_draw_row(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context);
static void menu_select(struct MenuLayer* menu, MenuIndex* cell_index, void* callback_context);
static void menu_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
static void menu_draw_header(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* callback_context);
static void scroll_timer_callback(void* data);



static uint16_t menu_num_sections(struct MenuLayer* menu, void* callback_context) {
  return 2;
}

static uint16_t menu_num_rows(struct MenuLayer* menu, uint16_t section_index, void* callback_context) {
   
    int ret = 0;
    switch(section_index){
       case 0:
           ret = MENU_ROW_COUNT_COMMANDS;
           break;
        case 1:
            ret = MENU_ROW_COUNT_SETTINGS;
    
   }
    return ret;
}

static int16_t menu_cell_height(struct MenuLayer *menu, MenuIndex *cell_index, void *callback_context) {
#ifdef PBL_ROUND
  if(menu_layer_is_index_selected(menu,cell_index))
    return MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT;
  else
    return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
#else
#ifdef PBL_PLATFORM_EMERY
  return 54;
#else
  return 38;
#endif
#endif
}

static int16_t menu_header_height(struct MenuLayer *menu, uint16_t section_index, void *callback_context) {
#ifdef PBL_ROUND
  return 0;
#else
  return 16;
#endif
}

static void menu_draw_header(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* callback_context) {
    
#ifndef PBL_ROUND
    GRect bounds = layer_get_bounds(cell_layer);
    graphics_context_set_text_color(ctx, GColorWhite);
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorBlue);
    #else
      graphics_context_set_fill_color(ctx, GColorBlack);
    #endif
    graphics_fill_rect(ctx,GRect(0,1,bounds.size.w,14),0,GCornerNone);

    switch(section_index){
        case 0:
            graphics_draw_text(ctx, "Last Sent", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0, -2, bounds.size.w, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            break;
        case 1:
            graphics_draw_text(ctx, "Settings", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0, -2, bounds.size.w, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            break;
    }
#endif
}


static void menu_cell_animated_draw(GContext* ctx, const Layer* cell_layer, char* text, char* subtext, bool animate){
  if(animate && s_scroll_index > 0 && strlen(text) > 16 ){
   /* if(((int16_t)strlen(text)-15-s_scroll_index)>0)
        text+=s_scroll_index;
     else
        s_scroll_index = 0;
        
     */ 
     if( last_s_scroll_row_index !=s_scroll_row_index){
          strcpy(tmp_menu_item,text);
          if(strlen(text) < 97){ 
             //strcat(tmp_menu_item,"                   ");
             strcat(tmp_menu_item,"...");
          }
          last_s_scroll_row_index = s_scroll_row_index;
      }
      
      char stmp = 0;
      stmp = tmp_menu_item[0];
      for(int i=0;i<(int)strlen(tmp_menu_item);i++){
         tmp_menu_item[i] = tmp_menu_item[i+1];
      }
      tmp_menu_item[strlen(tmp_menu_item)] = stmp;
      //strncpy(tmp_menu_item_display, tmp_menu_item,17);
      menu_cell_basic_draw(ctx,cell_layer,tmp_menu_item,subtext,NULL);
  }    
  else{       
      menu_cell_basic_draw(ctx,cell_layer,text,subtext,NULL);
  }
}

static void menu_draw_row(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context) {

  bool animate = false;
  char* text = NULL;
  char* subtext = "";
    
  switch(cell_index->section){
      case 0:
          animate = cell_index->row==s_scroll_row_index;
          text =gsmenu_items[cell_index->row];
          break;
      case 1:
          switch(cell_index->row){
              case 0:
                  text = "Speak Response";
                  if (read_persist_bool(SPEAK_AUDIO_KEY,true))
                      subtext = "Enabled";
                  else    
                      subtext = "Disabled";
                  break;
               case 1:
                  text = "Speak To Text";
                  if (read_persist_bool(SPEACH_TO_TEXT,true))
                      subtext = "Enabled";
                  else    
                      subtext = "Disabled";
                  break;
                case 2:
                  text = "Auto Volume";
                  if (read_persist_bool(AUTO_VOLUME,true))
                      subtext = "Enabled";
                  else    
                      subtext = "Disabled";
                  break;
        
                case 3:
                  text = "Volume Level";
                  int vol = read_persist_int(AUTO_VOLUME_LEVEL,10);
                  snprintf(s_subtext[0], sizeof(s_subtext[0]), "Level %d", vol);
    
                  subtext = s_subtext[0];
             
                 break;
          }
          break;
      
  }
  menu_cell_animated_draw(ctx, cell_layer, text, subtext, animate);
      
}

static void scroll_timer_callback(void *data){
   
    s_scroll_index+=scroll_inc;
   if(s_scroll_row_index==-1) s_scroll_row_index = 0;
    layer_mark_dirty(menu_layer_get_layer(s_menu));
    
    s_scroll_timer = app_timer_register(200,scroll_timer_callback,NULL);
}

static void menu_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context){
  //app_timeout_ms = 300;
  s_scroll_index=0;
  s_scroll_row_index = new_index.row;
  
  app_timer_cancel(s_scroll_timer);
  s_scroll_timer = app_timer_register(1000,scroll_timer_callback,NULL);  
  //app_timer_reschedule(s_scroll_timer,app_timeout_ms);
}

static void menu_select(struct MenuLayer* menu, MenuIndex* cell_index, void* callback_context) {
    int vol =0;
    
  switch( cell_index->section){
      case 0:
          send_last_command(cell_index->row);
           window_stack_pop(true);
          break;
      case 1:
          switch(cell_index->row){
              case 0:
                  swap_persit_bool(SPEAK_AUDIO_KEY);
                  break;
               case 1:
                  swap_persit_bool(SPEACH_TO_TEXT);
                  break;
                case 2:
                  swap_persit_bool(AUTO_VOLUME);
                  break;
                case 3:
                  vol = read_persist_int(AUTO_VOLUME_LEVEL,10);
                  if( vol ==10)
                      vol = 0;
                  else
                      vol++;
                  persist_write_int(AUTO_VOLUME_LEVEL,vol);
                 break;
          }
      layer_mark_dirty(menu_layer_get_layer(menu));
      break;
  }
       
 
    
  
}

static void window_load(Window* window) {

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
       
  // Create the menu layer
  s_menu = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_num_sections,
    .get_num_rows = menu_num_rows,
    .get_cell_height = menu_cell_height,
    .draw_row = menu_draw_row,
    .select_click = menu_select,
    .get_header_height = menu_header_height,
    .draw_header = menu_draw_header,
    .selection_changed = menu_selection_changed,
  });
  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu, window);
    
#ifdef PBL_COLOR
  menu_layer_pad_bottom_enable(s_menu,false);
  menu_layer_set_highlight_colors(s_menu,GColorBlue,GColorWhite);
#endif
#ifdef PBL_ROUND
  menu_layer_set_center_focused(s_menu,true);
#endif
  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(s_menu));
 
}

void menu_show(char* smenu_items[],int count){
 // init_settings_window(smenu_items,count);
  gsmenu_items = smenu_items;
  s_scroll_row_index  = -1;
  s_scroll_index=0;
  last_s_scroll_row_index = -1;
  s_scroll_timer = app_timer_register(1000,scroll_timer_callback,NULL);  
  window_stack_push(window, true);
}

static void window_appear(Window* window) {
  layer_mark_dirty(menu_layer_get_layer(s_menu));
}

static void window_unload(Window* window) {
  app_timer_cancel(s_scroll_timer);
  menu_layer_destroy(s_menu);
    
}

void window_disappear(Window *window) {
   menu_layer_destroy(s_menu);
}

void menu_init() {
  window = window_create();
    
  window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear
  });
      
}
void menu_deinit() {
  window_destroy(window);
}
