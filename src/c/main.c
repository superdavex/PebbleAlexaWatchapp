#include <pebble.h>
#include "main.h"
#include "menu.h"


static Window *s_main_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_question_layer ;
static AppTimer *s_progress_timer;

static DictationSession *s_dictation_session;
static bool s_speaking_enabled;
GRect bounds;
static char s_last_text[1024];
static int APPTIMEOUT = 60000;


enum {
  MESSAGE_KEY_RequestData   = 1,
  MESSAGE_KEY_Speak_Audio = 2,
  MESSAGE_KEY_Text_To_Speech = 3,
  MESSAGE_KEY_Auto_Volume = 4,
  MESSAGE_KEY_Volume_Level = 5,
  MESSAGE_KEY_Favorite1 = 6,
  MESSAGE_KEY_Favorite2 = 7,
  MESSAGE_KEY_Favorite3 = 8,
  MESSAGE_KEY_Favorite4 = 9,
  MESSAGE_KEY_Favorite5 = 10

};

static int command_count=0;
static char* arrayLastCommands[10];
static char last_command_1[100];
static char last_command_2[100];
static char last_command_3[100];
static char last_command_4[100];
static char last_command_5[100];
static char last_command_6[100];
static char last_command_7[100];
static char last_command_8[100];
static char last_command_9[100];
static char last_command_10[100];

    

char* scrolltext; 

static void exit_timer_callback(void *data) {
 window_stack_pop_all(true);
}

void  bumpTimer(){
    if(s_progress_timer){
        app_timer_reschedule(s_progress_timer,APPTIMEOUT);
    }
    else{
        s_progress_timer = app_timer_register(APPTIMEOUT, exit_timer_callback, NULL);
    }
    
}
void setScrollText(char * message){   
  bumpTimer();
  text_layer_set_size(s_question_layer, GSize(bounds.size.w - 10, 2000));
  text_layer_set_text(s_question_layer, message);
        
  GSize max_size = text_layer_get_content_size(s_question_layer);
 max_size.h += 20;
  text_layer_set_size(s_question_layer, max_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, max_size.h + 20));

   
}

/************************************ App *************************************/

void sendMessage(char * message){
    bumpTimer();
    
    // Declare the dictionary's iterator
    DictionaryIterator *out_iter;

    // Prepare the outbox buffer for this message
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if(result == APP_MSG_OK) {
        int val =0;
        dict_write_cstring(out_iter, MESSAGE_KEY_RequestData,message);
        val = read_persist_bool(SPEAK_AUDIO_KEY,true);
        dict_write_int(out_iter, MESSAGE_KEY_Speak_Audio,&val,1,false);
        val = read_persist_bool(SPEACH_TO_TEXT,true);
        dict_write_int(out_iter, MESSAGE_KEY_Text_To_Speech,&val,1,false);
        val = read_persist_bool(AUTO_VOLUME,true);
        dict_write_int(out_iter, MESSAGE_KEY_Auto_Volume,&val,1,false);
        val = read_persist_int(AUTO_VOLUME_LEVEL,10);
        dict_write_int(out_iter, MESSAGE_KEY_Volume_Level,&val,1,false);

        dict_write_end (out_iter);
        
        // Send this message
        result = app_message_outbox_send();
        if(result != APP_MSG_OK) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
        }
    } else {
        // The outbox cannot be used right now
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    }
}

void send_last_command(int commandid){
    APP_LOG(APP_LOG_LEVEL_ERROR, "Command ID %d, %s", (int)commandid,arrayLastCommands[commandid]);
    
    scrolltext = realloc(scrolltext, strlen(arrayLastCommands[commandid]) + 50);
    snprintf(scrolltext,strlen(arrayLastCommands[commandid]) + 50,"Asking Alexa...\r\n\r\n%s",arrayLastCommands[commandid]);     
    setScrollText(scrolltext);

    sendMessage(arrayLastCommands[commandid]);
}

void swap_persit_bool(int key){
    persist_write_bool(key,! persist_read_bool(key) );
}

bool read_persist_bool(int key, bool def){
    if (persist_exists(key)) 
        return persist_read_bool(key);
    else{
        persist_write_bool(key,def);
        return def;
    }
}
int read_persist_int(int key, bool def){
    if (persist_exists(key)) 
        return persist_read_int(key);
   else{
        persist_write_int(key,def);
        return def;
    }
}

void read_persist(){
    int count =10;
    int ret =0;
    ret =persist_read_string(FAVORITE_1_KEY,last_command_1,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_1,"Set the volume to 10."); // Default option
        
    ret =persist_read_string(FAVORITE_2_KEY,last_command_2,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_2,"What time is it."); // Default option
    
    ret =persist_read_string(FAVORITE_3_KEY,last_command_3,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_3,"What is the weather."); // Default option
    
    ret =persist_read_string(FAVORITE_4_KEY,last_command_4,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_4,"Tell me a joke."); // Default option
    
    ret =persist_read_string(FAVORITE_5_KEY,last_command_5,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_5,"What is my flash briefing."); // Default option
    
    ret =persist_read_string(FAVORITE_6_KEY,last_command_6,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_6,"What time is it"); // Default option
        
    ret =persist_read_string(FAVORITE_7_KEY,last_command_7,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_7,"Ask the house what the temperature outside is"); // Default option
    
    ret =persist_read_string(FAVORITE_8_KEY,last_command_8,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_8,"What is the weather."); // Default option
    
    ret =persist_read_string(FAVORITE_9_KEY,last_command_9,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_9,"Turn my light off."); // Default option
    
    ret =persist_read_string(FAVORITE_10_KEY,last_command_10,100);
    if( ret == E_DOES_NOT_EXIST ) 
        strcpy(last_command_10,"What is my flash briefing."); // Default option
    
    command_count=count;
    
    APP_LOG(APP_LOG_LEVEL_ERROR, "Read persist read %d", (int)command_count);
}
void write_persist(){
    persist_write_string(FAVORITE_6_KEY, last_command_6);
    persist_write_string(FAVORITE_7_KEY, last_command_7);
    persist_write_string(FAVORITE_8_KEY, last_command_8);
    persist_write_string(FAVORITE_9_KEY, last_command_9);
    persist_write_string(FAVORITE_10_KEY, last_command_10);
}

void swap_last_commands(char *cur_command){
    // Check if its already in the list
    if( strcmp(last_command_10,cur_command) != 0 && 
       strcmp(last_command_9,cur_command) != 0 && 
       strcmp(last_command_8,cur_command) != 0 && 
       strcmp(last_command_7,cur_command) != 0 && 
       strcmp(last_command_6,cur_command) != 0){
        
        strcpy(last_command_10,last_command_9);
        strcpy(last_command_9,last_command_8);
        strcpy(last_command_8,last_command_7);
        strcpy(last_command_7,last_command_6);
        strcpy(last_command_6,cur_command);
        
        command_count++;
        if(command_count > 10) command_count = 10;
        write_persist();
    }
}
/******************************* Dictation API ********************************/

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
    bumpTimer();
    if(status == DictationSessionStatusSuccess) {
        
      scrolltext = realloc(scrolltext, strlen(transcription) + 50);
      snprintf(scrolltext,strlen(transcription) + 50,"Asking Alexa...\r\n\r\n%s",transcription);    
      setScrollText(scrolltext);

       swap_last_commands(transcription);
        
       sendMessage(transcription); 
  } else {
    //setScrollText("Transcription Failed");
    //APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed.\n\nError ID:\n%d", (int)status);
  }
  s_speaking_enabled = true;
}

static void select_long_click_handler (ClickRecognizerRef recognizer, void *context){
  bumpTimer();
      
  if(s_speaking_enabled) {
    // Start voice dictation UI
    dictation_session_start(s_dictation_session);
    s_speaking_enabled = false;
  }
}

static void select_single_click_handler (ClickRecognizerRef recognizer, void *context){
  bumpTimer();

    arrayLastCommands[0] = last_command_1;
    arrayLastCommands[1] = last_command_2;
    arrayLastCommands[2] = last_command_3;
    arrayLastCommands[3] = last_command_4;
    arrayLastCommands[4] = last_command_5;
    arrayLastCommands[5] = last_command_6;
    arrayLastCommands[6] = last_command_7;
    arrayLastCommands[7] = last_command_8;
    arrayLastCommands[8] = last_command_9;
    arrayLastCommands[9] = last_command_10;
    
    APP_LOG(APP_LOG_LEVEL_ERROR, "Count %d", (int)command_count);
    
    menu_show(arrayLastCommands,command_count);    
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 500,select_long_click_handler,NULL);  
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);
    
  // Initialize the scroll layer
  s_scroll_layer = scroll_layer_create(bounds);
 
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks){.click_config_provider=click_config_provider});
 
  s_question_layer = text_layer_create(max_text_bounds);
    
  text_layer_set_font(s_question_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    
  // Add the layers for display
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_question_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_question_layer);
  scroll_layer_destroy(s_scroll_layer);
}

void process_tuple(Tuple *t){
  //Get key
  int key = t->key;
 
  //Get integer value, if present
 // int value = t->value->int32;
 
  //Get string value, if present
 // char string_value[1024];
 // strcpy(string_value, t->value->cstring);
 
  //Decide what to do
  switch(key) {
    case MESSAGE_KEY_RequestData:
      //ac in use received
      //snprintf(ac_buffer, sizeof("In Use: couldbereallylongname"), "In Use: %s W", string_value);
      setScrollText(t->value->cstring);
      break;
    case MESSAGE_KEY_Favorite1:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Received Favorite %s", t->value->cstring);
          strcpy(last_command_1 , t->value->cstring);
          persist_write_string(FAVORITE_1_KEY, last_command_1);
          break;
     case MESSAGE_KEY_Favorite2:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Received Favorite %s", t->value->cstring);
          strcpy(last_command_2 , t->value->cstring);
          persist_write_string(FAVORITE_2_KEY, last_command_2);
          break;
       case MESSAGE_KEY_Favorite3:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Received Favorite %s", t->value->cstring);
          strcpy(last_command_3, t->value->cstring);
          persist_write_string(FAVORITE_3_KEY, last_command_3);
          break;
       case MESSAGE_KEY_Favorite4:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Received Favorite %s", t->value->cstring);
          strcpy(last_command_4 , t->value->cstring);
          persist_write_string(FAVORITE_4_KEY, last_command_4);
          break;
       case MESSAGE_KEY_Favorite5:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Received Favorite %s", t->value->cstring);
          strcpy(last_command_5 , t->value->cstring);
          persist_write_string(FAVORITE_5_KEY, last_command_5);
          break;      
  }
    
}
    
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    //Get data
    Tuple *t = dict_read_first(iterator);
    while(t != NULL){
        process_tuple(t);
         
        //Get next
        t = dict_read_next(iterator);
    }
    
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
  dictation_session_enable_confirmation(s_dictation_session, false);
    
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // Open AppMessage
    const int inbox_size = 1024;
    const int outbox_size = 1024;
    app_message_open(inbox_size, outbox_size);

   // Intial allocation
   scrolltext =malloc(1);
          
   read_persist();
    
        
  setScrollText("Ask Alexa");
  s_speaking_enabled = true;
    
 
    
  if  (launch_reason() == APP_LAUNCH_QUICK_LAUNCH){
      // Start voice dictation Up    
      dictation_session_start(s_dictation_session);
       s_speaking_enabled = false;
    }

    menu_init();
    
    bumpTimer();
}

static void deinit() {
  // Free the last session data
  dictation_session_destroy(s_dictation_session);

  window_destroy(s_main_window);
  free(scrolltext);
  menu_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
