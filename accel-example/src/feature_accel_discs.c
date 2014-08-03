#include "pebble.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50
#define BUFFER_SIZE 10


typedef struct Vec2d {
  double x;
  double y;
} Vec2d;

typedef struct Disc {
  Vec2d pos;
  Vec2d vel;
  double mass;
  double radius;
} Disc;

AccelData lastData;

static Disc discs[NUM_DISCS];

static double next_radius = 3;

static Window *window;

static GRect window_frame;

static Layer *disc_layer;

static AppTimer *timer;

static TextLayer* text_layer;

static int16_t buff[BUFFER_SIZE];

static int i;

static char message[40];

static int best;

 void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
   // incoming message received
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }



static AccelData accelDataDiff(AccelData d1, AccelData d2) {
  AccelData diff;
  diff.x = d2.x - d1.x;
  diff.y = d2.y - d1.y;
  diff.z = d2.z - d2.z;
  diff.timestamp = (d2.timestamp + d1.timestamp) / 2;
  return diff;
}

static float get(int16_t* buff, int size){

  float total = 0;

  int lastposc = -1;
  int thisposc = 0;

  int dummy = 0;
  //for getting actual values
  for(int p = 0; p < BUFFER_SIZE; ++p){
    if(buff[p] > 0) 
      ++thisposc;
    if(p % size == 0){

      if (dummy % 2 == 0){
        int diff = abs(lastposc - thisposc);
        total += (float) diff / (float) size;
      }

      dummy++;
      lastposc = thisposc;
    }
  }

  return total;
}

// O(n^2)
static int get_best_beat(int16_t* buff){
  //for adjusting size
  int bestsize = -1;
  int bestval = BUFFER_SIZE;
  for(int it = 1; it < BUFFER_SIZE / 2; ++it){
    float val = get(buff, it);

    //snprintf(message, 40, "%d", (int) (val * 1000));
    //text_layer_set_text(text_layer, message);

    if (val < bestval){
      bestval = val;
      bestsize = it;
    }
  }

  //snprintf(message, 40, "%d", (int) (bestval * 1000));
  //|text_layer_set_text(text_layer, message);

  return bestsize;
}

static void send_to_phone(uint8_t* buffer){

  // Byte array + k:
  //static const uint32_t SOME_DATA_KEY = 0xb00bf00b;
  //static const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  // CString + key:
  //static const uint32_t SOME_STRING_KEY = 0;
  //static const char *string = "Hello World";
  // Calculate the buffer size that is needed for the final Dictionary:
  //const uint8_t key_count = 2;
  //const uint32_t size = dict_calc_buffer_size(key_count, sizeof(data),
  //                                            strlen(string) + 1);
  // Stack-allocated buffer in which to create the Dictionary:
  //uint8_t buffer[100];
  // Iterator variable, keeps the state of the creation serialization process:
  //DictionaryIterator iter;
  // Begin:
  //dict_write_begin(&iter, buffer, sizeof(buffer));
  // Write the Data:
  //dict_write_data(&iter, SOME_DATA_KEY, data, sizeof(data));
  // Write the CString:
  //dict_write_cstring(&iter, SOME_STRING_KEY, message);
  // End:
  DictionaryIterator *iter;

  dict_write_begin(&iter, buffer, sizeof(buffer));



  //app_message_outbox_begin(&iter);

  dict_write_data(&iter, 1, buffer , sizeof(buffer));

  //Tuplet value = TupletInteger(1, num);
  //dict_write_tuplet(iter, &value);


  app_message_outbox_send();

}

static void timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };

  accel_service_peek(&accel);

  AccelData delta = accelDataDiff(accel, lastData);  //LOOKINTO

  /*
  float mag = fsqrt(delta.x + delta.y + delta.z);

  void priq_push(pri_queue q, void *data, int pri)
  priq_push(q, mag, ftoi(mag));

  snprintf(buff, 20, "%i, %i, %i", accel.x, accel.y, accel.z);
*/

  buff[i++] = delta.x;
  //int best = -8;
  //++best;
  

  if(i >= BUFFER_SIZE){
    i = 0;
    int total = 0;
    for(int it = 0; it < BUFFER_SIZE; ++it){
      total += abs(buff[it]);
    }

    float average = total / BUFFER_SIZE;
    snprintf(message, 20, "%d", (int)(average * 1000));
    text_layer_set_text(text_layer, message);

    send_to_phone((int) (average * 1000));
  }



  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);

  text_layer = text_layer_create(
    (GRect) { 
      .origin = { 0, 0 }, 
      .size = { frame.size.w, frame.size.h } 
    }
  );

  i = 0;
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));


}

static void window_unload(Window *window) {
  layer_destroy(disc_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  accel_data_service_subscribe(0, NULL);

  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);

   app_message_register_inbox_received(in_received_handler);
   app_message_register_inbox_dropped(in_dropped_handler);
   app_message_register_outbox_sent(out_sent_handler);
   app_message_register_outbox_failed(out_failed_handler);

   const uint32_t inbound_size = 64;
   const uint32_t outbound_size = 64;
   app_message_open(inbound_size, outbound_size);


}


static void deinit(void) {
  accel_data_service_unsubscribe();

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
