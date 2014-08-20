#include <pebble.h>

static Window *window;
static TextLayer *hello_layer;

typedef enum {
  ACCEL_AXIS_X = 0,
  ACCEL_AXIS_Y = 1,
  ACCEL_AXIS_Z = 2
} AccelAxisType;

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Process tap on ACCEL_AXIS_X, ACCEL_AXIS_Y or ACCEL_AXIS_Z
  // Direction is 1 or -1
}

void handle_init(void) {
  accel_tap_service_subscribe(accel_tap_handler);
}

void handle_deinit(void) {
  accel_tap_service_unsubscribe();
}

void accel_data_handler(AccelData *data, uint32_t num_samples) {
  val = data.x;
  char message[40];
  snprintf(message, 40, "%d", (int) (val * 1000));
  text_layer_set_text(text_layer, message);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(hello_layer, "Hello world");
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));

  accel_data_service_subscribe(25, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);


}

static void window_unload(Window *window) {
  text_layer_destroy(hello_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed watchface helloworld: %p", window);

  app_event_loop();
  deinit();
}