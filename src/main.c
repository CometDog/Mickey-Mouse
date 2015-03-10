#include "pebble.h"
#include "TransRotBmp.h"
  
static Window *window;
static Layer *s_simple_bg_layer;
static BitmapLayer *s_background_layer, *s_face_layer, *s_bg_layer;
static GBitmap *s_background_bitmap, *s_face_bitmap, *s_bg_bitmap;
TransRotBmp *minute_layer, *hour_layer;

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	if (units_changed == MINUTE_UNIT || tick_time->tm_sec == 0)
	{
		int32_t angle = (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10)) / (12 * 6));
		transrotbmp_set_angle(hour_layer, angle);
		angle = (TRIG_MAX_ANGLE * ((tick_time->tm_min * 6) + (tick_time->tm_sec / 10)) / (60 * 6));
		transrotbmp_set_angle(minute_layer, angle);
	}
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);
  
  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  s_bg_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  bitmap_layer_set_compositing_mode(s_bg_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bg_layer));
  
  s_face_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FACE);
  s_face_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_face_layer, s_face_bitmap);
  bitmap_layer_set_compositing_mode(s_face_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_face_layer));

	hour_layer = transrotbmp_create_with_resource_prefix(RESOURCE_ID_HOUR);
	transrotbmp_set_pos_centered(hour_layer, -28, -29);
	transrotbmp_set_src_ic(hour_layer, GPoint(17, 54));
	transrotbmp_add_to_layer(hour_layer, window_layer);
  
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MICKEY);
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  minute_layer = transrotbmp_create_with_resource_prefix(RESOURCE_ID_MINUTE);
	transrotbmp_set_pos_centered(minute_layer, -25, -26);
	transrotbmp_set_src_ic(minute_layer, GPoint(1,57));
	transrotbmp_add_to_layer(minute_layer, window_layer);
  
  time_t now = time(NULL);
	struct tm *t = localtime(&now);
	tick_handler(t, MINUTE_UNIT);
  
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler)tick_handler);
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  
  gbitmap_destroy(s_face_bitmap);
  bitmap_layer_destroy(s_face_layer);
  
  gbitmap_destroy(s_bg_bitmap);
  bitmap_layer_destroy(s_bg_layer);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit() {
  transrotbmp_destroy(minute_layer);
	transrotbmp_destroy(hour_layer);
  window_destroy(window);
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}