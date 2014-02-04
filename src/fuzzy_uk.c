#include "pebble.h"
#include "num2words.h"

#define BUFFER_SIZE 86

static struct CommonWordsData {
  TextLayer *text_time_layer;
  TextLayer *text_date_layer;
  Layer *line_layer;
  Window *window;
  char buffer[BUFFER_SIZE];
  
} s_data;

static void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void update_time(struct tm* t) {
  static char date_text[] = "Xxxxxxxxx 00";

  fuzzy_time_to_words(t->tm_hour, t->tm_min, s_data.buffer, BUFFER_SIZE);
  text_layer_set_text(s_data.text_time_layer, s_data.buffer);
  
  strftime(date_text, sizeof(date_text), "%a %e %b", t);
  text_layer_set_text(s_data.text_date_layer, date_text);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void do_init(void) {
  s_data.window = window_create();
  const bool animated = true;
  window_stack_push(s_data.window, animated);

  window_set_background_color(s_data.window, GColorBlack);
  GFont timefont = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  GFont datefont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  Layer *window_layer = window_get_root_layer(s_data.window);
  GRect frame = layer_get_frame(window_layer);

  s_data.text_date_layer = text_layer_create(GRect(0, 0, frame.size.w, 32));
  text_layer_set_text_color(s_data.text_date_layer, GColorWhite);
  text_layer_set_background_color(s_data.text_date_layer, GColorClear);
  text_layer_set_font(s_data.text_date_layer, datefont);
  layer_add_child(window_layer, text_layer_get_layer(s_data.text_date_layer));

  s_data.text_time_layer = text_layer_create(GRect(0, 35, frame.size.w, frame.size.h - 20));
  text_layer_set_text_color(s_data.text_time_layer, GColorWhite);
  text_layer_set_background_color(s_data.text_time_layer, GColorClear);
  text_layer_set_font(s_data.text_time_layer, timefont);
  layer_add_child(window_layer, text_layer_get_layer(s_data.text_time_layer));

  GRect line_frame = GRect(0, 33, 144, 2);
  s_data.line_layer = layer_create(line_frame);
  layer_set_update_proc(s_data.line_layer, line_layer_update_callback);
  layer_add_child(window_layer, s_data.line_layer);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  
}

static void do_deinit(void) {
  tick_timer_service_unsubscribe();
  text_layer_destroy(s_data.text_date_layer);
  text_layer_destroy(s_data.text_time_layer);
  layer_destroy(s_data.line_layer);
  window_destroy(s_data.window);
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}