#include "pebble.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50
#define QUEUE_SIZE 500
#define PERIOD_QUEUE_SIZE 251

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

typedef struct node node; // Necessary in C, harmless (but non-idiomatic) in C++

typedef struct node {
  node * link;
  float magnitude;
} node;

typedef struct queue{
  node* head;
  node* rear;
  int size;
} queue;

AccelData lastData;

AccelData thisData;

static Disc discs[NUM_DISCS];

static double next_radius = 3;

static Window *window;

static GRect window_frame;

static Layer *disc_layer;

static AppTimer *timer;

static TextLayer* text_layer;

//static int16_t buff[BUFFER_SIZE];

static int i;

static char message[40];

static int best;

static float mean;

static float tot;

static queue* qu;
static queue* periodQueue;

static float totalmean;

static int first;

static int canChange;

static float periodArray;


static time_t lastTime;

static time_t thisTime;

float my_sqrt(const float num) {
  const uint MAX_STEPS = 40;
  const float MAX_ERROR = 0.001;
  
  float answer = num;
  float ans_sqr = answer * answer;
  uint step = 0;
  while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
    answer = (answer + (num / answer)) / 2;
    ans_sqr = answer * answer;
  }
  return answer;
}


static float largest;
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

static float get(int16_t* buffer, int size){

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
static int get_best_beat(int16_t* buffer){
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

static void send_to_phone(int num){

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

/*
  uint8_t translated [BUFFER_SIZE];
  for(int it = 0; it < BUFFER_SIZE; ++it){
    translated[it] = (uint8_t) (buffer[it] / 256);
  }

  */
  DictionaryIterator iter;

  dict_write_begin(&iter, translated, sizeof(translated));



  //app_message_outbox_begin(&iter);

  //dict_write_data(&iter, 1, translated , sizeof(translated));

  Tuplet value = TupletInteger(1, num);
  dict_write_tuplet(iter, &value);


  app_message_outbox_send();

}

// - (float)sample:(NSMutableArray*)data {
//     float mean = 0;
//     largest = 0;
//     for (NSNumber *pt in data) {
//         if (pt.floatValue > largest) largest = pt.floatValue;
//         mean += pt.floatValue;
//     }
//     mean = mean / data.count;
//     totalmean = mean;
//     float workingDev = 0;
//     for (NSNumber *pt in data) {
//         workingDev += pow(pt.floatValue - mean, 2);
//     }
//     workingDev = workingDev / data.count;
//     return sqrtf(workingDev);
// }

static float sample(float* data){
  mean = 0;
  largest = 0;
  for(node* curr = q->head; curr != NULL; curr = curr->link){
    if (curr->magnitude > largest) 
      largest = curr->magnitude;
    mean += curr->magnitude;
  }

  mean = mean / QUEUE_SIZE;
  totalmean = mean;
  float workingDev = 0;

  for(node* curr = q->head; curr != NULL; curr = curr->link){
    workingDev += (curr->magnitude - mean) * (curr->magnitude - mean);
  }

  workingDev = workingDev / QUEUE_SIZE;
  return my_sqrt(workingDev);
}


static void add_to_queue(queue* q, float magnitude, int size){
  node* n = malloc(sizeof(float));
  n->magnitude = magnitude;
  n->link = NULL;
  if(q->head == NULL){
    q->head = n;
    q->rear = n;
  }

  if(q->size >= size){
    node* tmp = q->head;
    q->head = q->head->link;
    free(tmp);
  }
  else{
    q->size++;
  }
  q->rear->link = n;
  q->rear = n;

}

static void sort(float* array, int size){
  for(int outer = 0; outer < size; outer++){
    for(int inner = 0; inner < (size - outer) - 1; inner++){
      float temp;
      if(array[inner] > array[outer+1]){
        temp = array[inner];
        array[inner] = array[inner+1];
        array[inner+1] = temp;
      }
    }
  }
}

static void timer_callback(void *data) {
  //AccelData thisData = (AccelData) { .x = 0, .y = 0, .z = 0 };

  accel_service_peek(&thisData);

  if(first){
    lastData = thisData;
    first = 1;
    return;
  }

  AccelData delta = accelDataDiff(thisData, lastData);  //LOOKINTO

  float magnitude = my_sqrt((delta.x * delta.x) + (delta.y * delta.y) + (delta.z * delta.z));

  tot += magnitude;
  if(i % 4 == 0){
    add_to_queue(magnitude, qu, BUFFER_SIZE );
    if (magnitude > totalmean + (stddev * 1.2)) {

      text_layer_set_text(text_layer, "Press a button");
      text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);


      /*
      thisTime = time(NULL);

      if(lastTime != 0){
        time_t tempTime = thisTime - lastTime;
        add_to_queue(periodQueue, tempTime, PERIOD_QUEUE_SIZE);

      }
      lastTime = thisTime;

      */
    }
    else{
      text_layer_set_text(text_layer, "");
      text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);

    }
  }
  
  // if (i % 1000 == 0){
  //   canChange = 1;
  //   sort()
  // }


  if ((i % 200) == 0) {
    stddev = sample();
    // if (canChange) {
    //     sort()
    //     double fuckyou = [[sortedArray objectAtIndex:(int)(sortedArray.count/2)] doubleValue];
    //     double shit = (1.0 / fuckyou) * 60;
        
    //     [self playURLWithRate:[self pickCorrectTempo:tempoOfSong feetBPM:shit]];
    // }
  }



  i = (i + 1) % 10000;

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

   qu = malloc(sizeof(queue));

   qu->head = NULL;
   qu->size = 0;
   qu->rear = NULL;

   periodQueue->head = NULL;
   periodQueue->size = 0;
   periodQueue->rear = NULL;

   tot = 0;

   // lastData = malloc(sizeof (AccelData));
   // thisData = malloc(sizeof (thisData));
   thisData = (AccelData) { .x = 0, .y = 0, .z = 0 };
   lastData = (AccelData) { .x = 0, .y = 0, .z = 0 };

   first = 1;
   canChange = 0;

   lastTime = 0;
   thisTime = 0;

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
