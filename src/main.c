#include <pebble.h>

#define ELAPSED_TOP 14
#define DATE_TOP 84
#define MARGIN_LEFT 9

#define text_layer_add_to_window(layer, window) layer_add_child(window_get_root_layer(window), text_layer_get_layer(layer))
#define bitmap_layer_add_to_window(layer, window) layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(layer))

static Window *s_main_window;
static TextLayer *s_elapsed_minutes_layer;
static TextLayer *s_elapsed_seconds_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_line_layer;
static TextLayer *s_time_layer;

static GBitmap *s_prime_bitmap;
static BitmapLayer *s_prime_layer;
static BitmapLayer *s_double_prime_a_layer;
static BitmapLayer *s_double_prime_b_layer;

static time_t s_start_time = 0;
static int s_minutes_elapsed = -1;
static int s_seconds_elapsed = -1;
static int s_current_hour = -1;
static int s_current_minute = -1;

static void update_elapsed() {
  double elapsed_time = time(NULL) - s_start_time;
  int seconds = (int)elapsed_time % 60;
  int minutes = (int)elapsed_time / 60 % 100;
  
  if (minutes != s_minutes_elapsed) {
    s_minutes_elapsed = minutes;
    static char elapsed_minutes_buffer[] = "00";
    snprintf(elapsed_minutes_buffer, sizeof(elapsed_minutes_buffer), "%02d", minutes);
    text_layer_set_text(s_elapsed_minutes_layer, elapsed_minutes_buffer);
  }
  
  if (seconds != s_seconds_elapsed) {
    s_seconds_elapsed = seconds;
    static char elapsed_seconds_buffer[] = "00";
    snprintf(elapsed_seconds_buffer, sizeof(elapsed_seconds_buffer), "%02d", seconds);
    text_layer_set_text(s_elapsed_seconds_layer, elapsed_seconds_buffer);
  }
}

static void update_time(struct tm *tick_time) {
  static char time_buffer[] = "00:00";
  if (clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
  } else {
    strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
    if (time_buffer[0] == '0') {
      memmove(time_buffer, &time_buffer[1], sizeof(time_buffer) - 1);
    }
  }
  text_layer_set_text(s_time_layer, time_buffer);
  s_current_hour = tick_time->tm_hour;
  s_current_minute = tick_time->tm_min;
}

static void update_date(struct tm *tick_time) {
  static char date_text[] = "Xxxxxxxxxxxxxxxxxxxxx 00";
  strftime(date_text, sizeof(date_text),  "%B %e", tick_time);
  text_layer_set_text(s_date_layer, date_text);
}

static void prep_for_screenshot() {
  text_layer_set_text(s_time_layer, "7:28");
  text_layer_set_text(s_elapsed_minutes_layer, "00");
  text_layer_set_text(s_elapsed_seconds_layer, "00");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_elapsed();
  if (s_current_hour != tick_time->tm_hour || s_current_minute != tick_time->tm_min) {
    update_time(tick_time);
    update_date(tick_time);
  }
  //prep_for_screenshot();
}

static void main_window_load(Window *window) {
  // stopwatch
  s_elapsed_minutes_layer = text_layer_create(GRect(MARGIN_LEFT, ELAPSED_TOP, 56, 49));
  text_layer_set_background_color(s_elapsed_minutes_layer, GColorClear);
  text_layer_set_text_color(s_elapsed_minutes_layer, GColorWhite);
  text_layer_set_font(s_elapsed_minutes_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_add_to_window(s_elapsed_minutes_layer, window);
  s_elapsed_seconds_layer = text_layer_create(GRect(MARGIN_LEFT+67, ELAPSED_TOP, 56, 49));
  text_layer_set_background_color(s_elapsed_seconds_layer, GColorClear);
  text_layer_set_text_color(s_elapsed_seconds_layer, GColorWhite);
  text_layer_set_font(s_elapsed_seconds_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_add_to_window(s_elapsed_seconds_layer, window);
  
  // primes
  s_prime_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PRIME);
  s_prime_layer = bitmap_layer_create(GRect(MARGIN_LEFT+57, ELAPSED_TOP+13, 4, 9));
  bitmap_layer_set_bitmap(s_prime_layer, s_prime_bitmap);
  bitmap_layer_add_to_window(s_prime_layer, window);
  s_double_prime_a_layer = bitmap_layer_create(GRect(MARGIN_LEFT+124, ELAPSED_TOP+13, 4, 9));
  bitmap_layer_set_bitmap(s_double_prime_a_layer, s_prime_bitmap);
  bitmap_layer_add_to_window(s_double_prime_a_layer, window);
  s_double_prime_b_layer = bitmap_layer_create(GRect(MARGIN_LEFT+130, ELAPSED_TOP+13, 4, 9));
  bitmap_layer_set_bitmap(s_double_prime_b_layer, s_prime_bitmap);
  bitmap_layer_add_to_window(s_double_prime_b_layer, window);
  
  // date
  s_date_layer = text_layer_create(GRect(MARGIN_LEFT, DATE_TOP, 144-MARGIN_LEFT, 21));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_add_to_window(s_date_layer, window);
  
  // line
  s_line_layer = bitmap_layer_create(GRect(MARGIN_LEFT-1, DATE_TOP+29, 144-(MARGIN_LEFT-1)*2, 2));
  bitmap_layer_set_background_color(s_line_layer, GColorWhite);
  bitmap_layer_add_to_window(s_line_layer, window);
  
  // time
  s_time_layer = text_layer_create(GRect(MARGIN_LEFT, DATE_TOP+24, 144-9, 49));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_add_to_window(s_time_layer, window);
  
  // simulate tick
  time_t current_time = time(NULL);
  tick_handler(localtime(&current_time), SECOND_UNIT);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_elapsed_minutes_layer);
  gbitmap_destroy(s_prime_bitmap);
  text_layer_destroy(s_elapsed_seconds_layer);
  bitmap_layer_destroy(s_double_prime_a_layer);
  bitmap_layer_destroy(s_double_prime_b_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_line_layer);
  text_layer_destroy(s_time_layer);
  bitmap_layer_destroy(s_prime_layer);
}
  
static void init() {
  time(&s_start_time);
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
