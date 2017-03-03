#include <pebble.h>



#define FAVORITE_1_KEY 1
#define FAVORITE_2_KEY 2
#define FAVORITE_3_KEY 3
#define FAVORITE_4_KEY 4
#define FAVORITE_5_KEY 5
#define FAVORITE_6_KEY 6
#define FAVORITE_7_KEY 7
#define FAVORITE_8_KEY 8
#define FAVORITE_9_KEY 9
#define FAVORITE_10_KEY 10

#define SPEAK_AUDIO_KEY 11
#define SPEACH_TO_TEXT 12
#define AUTO_VOLUME 13
#define AUTO_VOLUME_LEVEL 14

void sendMessage(char * message);
void send_last_command(int commandid);

bool read_persist_bool(int key, bool def);
int read_persist_int(int key, bool def);
void swap_persit_bool(int key);


    
    