#include <Ramure_V3.h>

unsigned int boot __attribute__((__section__(".noinit")));

Worker worker;

File file;

EthernetClient client[CLIENT_COUNT];

PubSubClient mqtt(client[CLIENT_MQTT]);
SMS sms(client[CLIENT_SMS]);

DS18B20* ds18b20[DS18B20_COUNT];

#define SerialPSU Serial6

PSU psu;

EthernetUDP udp;
NTPClient ntp(udp);

STM32RTC& rtc = STM32RTC::getInstance();

SDRAM sdram;
DisplayController tft;
FT5336 ts;

#if RAMURE_LVGL

#define LV_HOR_RES (480)
#define LV_VER_RES (272)

static uint8_t lvgl_buffer[LV_HOR_RES * LV_VER_RES / 10];

void lvgl_display_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
#if LV_USE_GPU_STM32_DMA2D
    display->draw_ctx->buffer_copy(NULL, tft.getBuffer(), LV_HOR_RES, area, px_map, area->x2 - area->x1 + 1, area);
#else
    tft.drawBuffer(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t*) px_map);
#endif
    lv_display_flush_ready(display);
}
// Input
#define TS_MIN 10
#define TS_MAX 1000

void lvgl_input_read(lv_indev_t* indev, lv_indev_data_t* data) {
#if 0
    UNUSED(indev);
    static lv_indev_data_t last = { { 0, 0 }, 0, 0, 0, LV_INDEV_STATE_RELEASED };
    TSPoint point = ts.getPoint();
    if ((point.z > TS_MIN) && (point.z < TS_MAX)) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->point = last.point;
        data->state = LV_INDEV_STATE_RELEASED;
    }
    last.point = data->point;
#else
    UNUSED(indev);
    TSPoint point = ts.getPoint();
    if ((point.z > TS_MIN) && (point.z < TS_MAX)) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
#endif
}

// Initialize
void lvgl_init(void) {
#if LV_USE_LOG != 0
    lv_log_register_print_cb(lvgl_print);
#endif
    // Initialize the library
    lv_init();
    lv_display_t *display = lv_display_create(LV_HOR_RES, LV_VER_RES);
    lv_display_set_buffers(display, lvgl_buffer, NULL, sizeof(lvgl_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, lvgl_display_flush);
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lvgl_input_read);
}

lv_obj_t* lvgl_probe[DS18B20_COUNT];
lv_obj_t* lvgl_power[POWER_COUNT];

void lvgl_setup() {
    lv_obj_t *object;

    // Probe 0
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set a background color and a radius
        lv_style_set_radius(&style, 5);
        lv_style_set_bg_opa(&style, LV_OPA_COVER);
        lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 4));
        // Box format
        lv_style_set_x(&style, 20);
        lv_style_set_y(&style, 20);
        lv_style_set_width(&style, 440);
        lv_style_set_height(&style, 90);
        // Add outline
        lv_style_set_outline_width(&style, 2);
        lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_outline_pad(&style, 10);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_48);
        lv_style_set_text_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 4));
        // Padding
        lv_style_set_pad_ver(&style, 10);
        lv_style_set_pad_left(&style, 40);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_probe[0] = lv_label_create(object);
        lv_label_set_text(lvgl_probe[0], "-");
    }
    int y = 130;
    int w = 100;
    int h = 60;
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(1));
        lv_style_set_y(&style, y);
        lv_style_set_width(&style, w);
        lv_style_set_height(&style, h);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_probe[1] = lv_label_create(object);
        lv_label_set_text(lvgl_probe[1], "-");
    }
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(25));
        lv_style_set_y(&style, y);
        lv_style_set_width(&style, w);
        lv_style_set_height(&style, h);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_probe[2] = lv_label_create(object);
        lv_label_set_text(lvgl_probe[2], "-");
    }
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(50));
        lv_style_set_y(&style, y);
        lv_style_set_width(&style, w);
        lv_style_set_height(&style, h);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_probe[3] = lv_label_create(object);
        lv_label_set_text(lvgl_probe[3], "-");
    }
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(75));
        lv_style_set_y(&style, y);
        lv_style_set_width(&style, w);
        lv_style_set_height(&style, h);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_probe[4] = lv_label_create(object);
        lv_label_set_text(lvgl_probe[4], "-");
    }
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(1));
        lv_style_set_y(&style, 200);
        lv_style_set_width(&style, 220);
        lv_style_set_height(&style, LV_SIZE_CONTENT);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_power[0] = lv_label_create(object);
        lv_label_set_text(lvgl_power[0], "-");
    }
    {
        static lv_style_t style;
        lv_style_init(&style);
        // Set radius
        lv_style_set_radius(&style, 5);
        // Box format
        lv_style_set_x(&style, lv_pct(50));
        lv_style_set_y(&style, 200);
        lv_style_set_width(&style, 220);
        lv_style_set_height(&style, LV_SIZE_CONTENT);
        // Police
        lv_style_set_text_font(&style, &lv_font_montserrat_20);
        lv_style_set_text_color(&style, lv_color_black());
        // Padding
        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);
        // Create an object with the new style
        object = lv_obj_create(lv_screen_active());
        lv_obj_add_style(object, &style, 0);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
        lvgl_power[1] = lv_label_create(object);
        lv_label_set_text(lvgl_power[1], "-");
    }
}

#if 0
static void event_handler(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event); // origin
}
#endif

void lvgl_loop() {
    watchdog_loop();
    for (auto index = 0; index < DS18B20_COUNT; ++index) {
        if (worker.probes >> index & 1) {
            lv_label_set_text_fmt(lvgl_probe[index], "%.2f °C", worker.temperatures[index]);
        } else {
            lv_label_set_text(lvgl_probe[index], "N/A");
        }
    }
    lv_label_set_text_fmt(lvgl_power[0], "%.2f V", worker.voltage.out);
    lv_label_set_text_fmt(lvgl_power[1], "%.2f A", worker.current.out);
    lv_timer_handler();
}

void HAL_SYSTICK_Callback(void) {
    lv_tick_inc(1);
}

#endif




void path_affectation()
{
path_mqtt_publish = "network/" + worker.network.mqtt.topic + worker.network.mqtt.topic2 + "/data";
path_mqtt_subscribe = "network/" + worker.network.mqtt.topic + worker.network.mqtt.topic2 +"/control";
msg_alarm_int = "Internal_Alarm_"+ worker.network.mqtt.topic2 + "_"+ worker.temperatures[0];
msg_alarm_ext =  "External_Alarm_"+ worker.network.mqtt.topic2 + "_"+ worker.temperatures[3];

serial_printf("[Path] %s\n", path_mqtt_publish.c_str());
serial_printf("[Path] %s\n", path_mqtt_subscribe.c_str());
serial_printf("[Path] %s\n", msg_alarm_int.c_str());
serial_printf("[Path] %s\n", msg_alarm_int.c_str());
}





void worker_init() {
    worker.store = false;
    worker.last = millis() - 5 * 60 * 1000 - 1;
    worker.alarms[Alarm::Internal].active = false;
    worker.alarms[Alarm::Internal].sms = false;
    worker.alarms[Alarm::External].active = false;
    worker.alarms[Alarm::External].sms = false;
}

String worker_json() {
    JsonDocument document;
    document["relays"] = worker.relays;
#if 0
    JsonArray temperatures = document["temperature"].to<JsonArray>();
    for (auto index = 0; index < DS18B20_COUNT; ++index) {
        temperatures.add(worker.temperatures[index]);
    }
#else
    for (auto index = 0; index < DS18B20_COUNT; ++index) {
        String temperature = "temperature_" + String(index);
        document[temperature] = worker.temperatures[index];
    }
#endif
    document["light"] = worker.light;
    JsonObject psu = document["psu"].to<JsonObject>();
    psu["status"] = worker.status;
    JsonObject voltage = psu["voltage"].to<JsonObject>();
    voltage["set"] = worker.voltage.set;
    voltage["get"] = worker.voltage.get;
    voltage["out"] = worker.voltage.out;

    JsonObject current = psu["current"].to<JsonObject>();
    current["set"] = worker.current.set;
    current["get"] = worker.current.get;
    current["out"] = worker.current.out;

    document["id"] = worker.network.mqtt.topic2;
    document["UTC_Time"] = UTC_time.c_str() ; //"to be defined";

    String string;
    document.shrinkToFit();
    serializeJson(document, string);
    serial_printf("[JSON] %s\n", string.c_str());
    return string;
}

void serial_init() {
#if RAMURE_SERIAL
    Serial.begin(921600);
#endif
}

void serial_loop() {
#if RAMURE_SERIAL
    if (Serial.available()) {
        json_command(Serial.readString());
    }
#endif
}

/**
 * @brief
 *
 * Example
 *
 * {"action":"mobile","mobile":"0678901234"}
 * {"action":"target","target":23}
 * {"action":"sms","message":"temperature alarm"}
 * {"action":"store"}
 * {"action":"dump"}
 * {"action":"reboot"}
 *
 * @param command
 */
void json_command(String command) {
    bool store = false;
    JsonDocument document;
    DeserializationError error = deserializeJson(document, command);
    if (error == DeserializationError::Ok) {
        String action = String(document["action"]);
        action.toLowerCase();
        if (action == "relays") {
            worker.relays = document["relays"];
            store = document["store"] | true;
        } else if (action == "probes") {
            worker.probes = document["probes"];
            store = document["store"] | true;
        } else if (action == "target") {
            worker.target = document["target"] | RAMURE_TARGET;
            store = document["store"] | true;
        } else if (action == "iset") {
            worker.current.set = document["iset"];
            psu.iset(worker.current.set);
            worker.current.get = psu.iset();
            serial_printf("[PSU] iset %.2f A\n", worker.current.get);
            store = document["store"] | true;
        } else if (action == "delay") {
            worker.delay = document["delay"];
            store = document["store"] | true;
        } else if (action == "mobile") {
            worker.mobile = String(document["mobile"]);
            store = document["store"] | true;
        } else if (action == "sms") {
            watchdog_loop();
            String message = String(document["message"]);
            sms_send(message);
        } else if (action == "publish") {
            probes_loop();
            light_loop();
            relays_loop();
            fans_loop();
            psu_loop();
            mqtt.publish(path_mqtt_publish.c_str(), worker_json().c_str());
        } else if (action == "check") {
            uint8_t relays = worker.relays;
            // Stop fans
            for (auto index = 0; index < RELAYS_COUNT; ++index) {
                digitalWrite(RELAYS_PINS[index], LOW);
            }
            // Check fans
            for (auto index = 0; index < RELAYS_COUNT; ++index) {
                watchdog_loop();
                worker.relays = 1 << index;
                digitalWrite(RELAYS_PINS[index], HIGH);
                for (auto index = 0; index < 5; ++index) {
                    watchdog_loop();
                    delay(1000);
                }
                probes_loop();
                light_loop();
                psu_loop();
                mqtt.publish(path_mqtt_publish.c_str(), worker_json().c_str());
                digitalWrite(RELAYS_PINS[index], LOW);
                for (auto index = 0; index < 5; ++index) {
                    watchdog_loop();
                    delay(1000);
                }
            }
            // Back to normal
            worker.relays = relays;
            for (auto index = 0; index < RELAYS_COUNT; ++index) {
                uint32_t value = worker.relays >> index & 1 ? HIGH : LOW;
                digitalWrite(RELAYS_PINS[index], LOW);
            }
        } else if (action == "regulate") {
            worker.regulate = document["regulate"];
        } else if (action == "store") {
            store = true;
        } else if (action == "dump") {
            sd_dump();
        } else if (action == "reboot") {
            HAL_NVIC_SystemReset();
        }
        worker.store |= store;
    }
}

void serial_printf(const char* format, ...) {
#if RAMURE_SERIAL
    if (Serial) {
        va_list ap, copy;
        va_start(ap, format);
        va_copy(copy, ap);
        int size = vsnprintf(nullptr, 0, format, ap) + 1;
        va_end(ap);
        char buffer[size];
        vsnprintf(buffer, size, format, copy);
        va_end(copy);
        Serial.print(buffer);
        Serial.flush();
    }
#endif
}

void watchdog_init() {
#if RAMURE_WATCHDOG
    // 10 seconds
    IWatchdog.begin(10 * 1000 * 1000);
#endif
}

void watchdog_loop() {
#if RAMURE_WATCHDOG
    IWatchdog.reload();
#endif
}

void watchdog_status() {
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        boot = 1;
        __HAL_RCC_CLEAR_RESET_FLAGS();
        serial_printf("\n\n[WATCHDOG] POR reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        boot = 1;
        __HAL_RCC_CLEAR_RESET_FLAGS();
        serial_printf("\n\n[WATCHDOG] pin reset\n");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        boot = 1;
        __HAL_RCC_CLEAR_RESET_FLAGS();
        serial_printf("\n\n[WATCHDOG] software reset\n");
    }
#if RAMURE_WATCHDOG
    if (IWatchdog.isReset(true)) {
        serial_printf("\n\n[WATCHDOG] reset %u\n", boot++);
    }
#endif
}

void rtc_init() {
	// a mettre apres initialisatio sd et networks
	// RTC
	    rtc.setClockSource(STM32RTC::LSE_CLOCK);
	    rtc.begin();
	    serial_printf("[RTC] initialized\n");
	// NTP
	    ntp.setPoolServerName("134.158.120.1"); // a trouver solution pour une recuperation dans le fichier
	    ntp.begin();
    serial_printf("[NTP] initialized\n");
}


void rtc_read_update()
{

    static uint8_t day = 0;
    if (day == rtc.getDay()) {
       Serial.printf("[RTC_UTC_Now] ");
       UTC_time = "";
       UTC_time  = UTC_time +"20"+ rtc.getYear()+"-" + rtc.getMonth()+"-" +rtc.getDay()+ "T" + rtc.getHours()+":" + rtc.getMinutes()+":"+ rtc.getSeconds();
		Serial.printf(UTC_time.c_str()); Serial.printf("\n");

    } else {
        if (ntp.update()) {
            Serial.printf("[NTP] updated %s\n", ntp.getFormattedTime().c_str());
            rtc.setEpoch(ntp.getEpochTime());
            day = rtc.getDay();
         //   Serial.printf("[RTC] day %d\n", day);
         //   Serial.printf("[RTC] day_update_done");
         //   Serial.printf("[RTC] date %02d/%02d 20%02d\n", rtc.getDay(), rtc.getMonth(), rtc.getYear());
          //  Serial.printf("[RTC_UTC] date 20%02d-%02d-%02dT%02d:%02d:%02d\n", rtc.getYear(), rtc.getMonth(),rtc.getDay(),rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
        Serial.printf("[RTC_UTC_Update] ");
        UTC_time = "";
        UTC_time  = UTC_time +"20"+ rtc.getYear()+"-" + rtc.getMonth()+"-" +rtc.getDay()+ "T" + rtc.getHours()+":" + rtc.getMinutes()+":"+ rtc.getSeconds();
		Serial.printf(UTC_time.c_str()); Serial.printf("\n");
        }
    }
}
//"utc:":"2026-9-28T13:40:17"}


void sd_init() {
    if (SD.begin(SD_DETECT_PIN)) {
        serial_printf("[SD] initialized\n");
        file = SD.open(RAMURE_JSON);
        if (file) {
            serial_printf("[SD] read configuration file\n");
            JsonDocument document;
            DeserializationError error = deserializeJson(document, file);
            if (error == DeserializationError::Ok) {
                worker.network.dhcp = document["dhcp"] | false;
                if (worker.network.dhcp == false) {
                    worker.network.local.fromString(document["local"].as<String>());
                    worker.network.subnet.fromString(document["subnet"].as<String>());
                    worker.network.gateway.fromString(document["gateway"].as<String>());
                    worker.network.dns.fromString(document["dns"].as<String>());
                }
                worker.network.ntp.server.fromString(document["ntp"]["server"].as<String>());
                worker.network.ntp.port = document["ntp"]["port"] | 123;


                worker.network.mqtt.server.fromString(document["mqtt"]["server"].as<String>());
                worker.network.mqtt.port = document["mqtt"]["port"] | 1883;
                worker.network.mqtt.topic = String(document["mqtt"]["topic"]);
                worker.network.mqtt.topic2 = String(document["mqtt"]["topic2"]);
                worker.network.mqtt.authentication = document["mqtt"]["authentication"] | false;
                if (worker.network.mqtt.authentication) {
                    worker.network.mqtt.username = String(document["mqtt"]["username"]);
                    worker.network.mqtt.password = String(document["mqtt"]["password"]);
                }
                worker.mobile = String(document["mobile"]);
                JsonArray relays = document["relays"].as<JsonArray>();
    #if 0
                for (auto index = 0; index < relays.size(); ++index) {
                    worker.relays |= 1 << relays[index];
                }
    #else
                for (JsonVariant relay: relays) {
                    worker.relays |= 1 << relay.as<int>();
                }
    #endif
                worker.regulate = true;
                worker.delay = document["delay"] | RAMURE_MQTT_DELAY;
                worker.target = document["target"] | RAMURE_TARGET;
                worker.voltage.set = document["voltage"];
                worker.current.set = document["current"];
            }
        } else {
            serial_printf("[ERROR] unable to read configuration file\n");
    #if RAMURE_BENCH
            worker.network.dhcp = false;
            worker.network.local.fromString("134.158.124.65");
            worker.network.subnet.fromString("255.255.248.0");
            worker.network.gateway.fromString("134.158.120.1");
            worker.network.dns.fromString("134.158.120.3");
            worker.network.ntp.server.fromString("134.158.120.1");
            worker.network.ntp.port = 123;
            worker.network.mqtt.authentication = true;
            worker.network.mqtt.topic = "RAMURE/";
            worker.network.mqtt.topic2 = "LMGE";
            worker.network.mqtt.username = "RAMURE";
            worker.network.mqtt.password = "QgQDIKHe63JsRIPs";
            worker.network.mqtt.server.fromString("134.158.124.133");
            worker.network.mqtt.port = 1883;
            worker.mobile = "0684146899";
            worker.relays = 0x0F;
            worker.regulate = true;
            worker.delay = RAMURE_MQTT_DELAY;
            worker.target = RAMURE_TARGET;
            worker.voltage.set = 8.00;
            worker.current.set = 3.00;
    #endif
        }
        file.close();
    } else {
        serial_printf("[ERROR] unable to initialize SD\n");
    }
    SD.end();
}

void sd_loop() {
    if (worker.store) {
        sd_remove();
        sd_store();
    }
}

void sd_remove() {
    if (SD.begin(SD_DETECT_PIN)) {
        serial_printf("[SD] initialized\n");
        if (SD.exists(RAMURE_JSON)) {
            if (SD.remove(RAMURE_JSON)) {
                serial_printf("[SD] configuration file removed\n");
            } else {
                serial_printf("[ERROR] unable to remove configuration file\n");
            }
        } else {
            serial_printf("[SD] configuration file not found\n");
        }
    } else {
        serial_printf("[ERROR] unable to initialize SD\n");
    }
    SD.end();
}

void sd_store() {
    if (SD.begin(SD_DETECT_PIN)) {
        serial_printf("[SD] initialized\n");
        file = SD.open(RAMURE_JSON, FILE_WRITE);
        if (file) {
            serial_printf("[SD] configuration file created\n");
            JsonDocument document;
            document["dhcp"] = worker.network.dhcp;
            document["local"] = worker.network.local.toString();
            document["subnet"] = worker.network.subnet.toString();
            document["gateway"] = worker.network.gateway.toString();
            document["dns"] = worker.network.dns.toString();
            JsonObject ntp = document["ntp"].to<JsonObject>();
            ntp["server"] = worker.network.ntp.server.toString();
            ntp["port"] = worker.network.ntp.port;
            JsonObject mqtt = document["mqtt"].to<JsonObject>();
            mqtt["server"] = worker.network.mqtt.server.toString();
            mqtt["port"] = worker.network.mqtt.port;
            mqtt["topic"] = worker.network.mqtt.topic;
            mqtt["topic2"] = worker.network.mqtt.topic2;
            mqtt["authentication"] = worker.network.mqtt.authentication;
            if (worker.network.mqtt.authentication) {
                mqtt["username"] = worker.network.mqtt.username;
                mqtt["password"] = worker.network.mqtt.password;
            }
            document["mobile"] = worker.mobile;
            JsonArray relays = document["relays"].to<JsonArray>();
            for (auto index = 0; index < RELAYS_COUNT; ++index) {
                if (worker.relays >> index & 1) {
                    relays.add(index);
                }
            }
            document["delay"] = worker.delay;
            document["target"] = worker.target;
            document["voltage"] = worker.voltage.set;
            document["current"] = worker.current.set;
            size_t size = serializeJsonPretty(document, file);
            serial_printf("[SD] configuration file written %d bytes\n", size);
            worker.store = false;
        } else {
            serial_printf("[ERROR] unable to create configuration file\n");
        }
        file.close();
    } else {
        serial_printf("[ERROR] unable to initialize SD\n");
    }
    SD.end();
}

void sd_dump() {
    if (SD.begin(SD_DETECT_PIN)) {
        serial_printf("[SD] initialized\n");
        file = SD.open(RAMURE_JSON);
        if (file) {
            serial_printf("[SD] dump configuration file\n");
            while (file.available()) {
                Serial.write(file.read());
            }
            serial_printf("\n");
        } else {
            serial_printf("[ERROR] unable to open configuration file\n");
        }
        file.close();
    } else {
        serial_printf("[ERROR] unable to initialize SD\n");
    }
    SD.end();
}

/**
 * @brief
 *
 * Static
 *
 * 09:30:19: [NETWORK] mac 00:80:E1:2E:00:33
 * 09:30:19: [NETWORK] local 134.158.124.66
 * 09:30:19: [NETWORK] subnet 255.255.248.0
 * 09:30:19: [NETWORK] gateway 134.158.120.1
 * 09:30:19: [NETWORK] dns 134.158.120.1
 *
 * DHCP
 *
 * 09:42:29: [NETWORK] mac 00:80:E1:2E:00:33
 * 09:42:29: [NETWORK] local 134.158.124.66
 * 09:42:29: [NETWORK] subnet 255.255.248.0
 * 09:42:29: [NETWORK] gateway 134.158.120.1
 * 09:42:29: [NETWORK] dns 134.158.120.3
 */
void network_init() {
  /*  serial_printf("[NETWORK] init\n");
    if (worker.network.dhcp) {
        serial_printf("[NETWORK] dhcp\n");
        Ethernet.begin();
    } else {
        serial_printf("[NETWORK] static\n");
        Ethernet.begin(worker.network.local, worker.network.subnet, worker.network.gateway, worker.network.dns);
    }
    uint8_t *mac = Ethernet.MACAddress();
    serial_printf("[NETWORK] mac %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    serial_printf("[NETWORK] local %s\n", Ethernet.localIP().toString().c_str());
    serial_printf("[NETWORK] subnet %s\n", Ethernet.subnetMask().toString().c_str());
    serial_printf("[NETWORK] gateway %s\n", Ethernet.gatewayIP().toString().c_str());
    serial_printf("[NETWORK] dns %s\n", Ethernet.dnsServerIP().toString().c_str());
    mqtt.setServer(worker.network.mqtt.server, worker.network.mqtt.port);
    mqtt.setBufferSize(384);*/

    uint8_t mac[6];
    mac[0] = 0x00;
    mac[1] = 0x80;
    mac[2] = 0xE1;
    mac[3] = ((uint8_t)(((*(uint32_t *)UID_BASE) & 0x00FF0000) >> 16));
    mac[4] = ((uint8_t)(((*(uint32_t *)UID_BASE) & 0x0000FF00) >> 8));
    mac[5] = ((uint8_t)((*(uint32_t *)UID_BASE) & 0x000000FF));

    Ethernet.setMACAddress(mac);

    serial_printf("[NETWORK] init\n");
    if (worker.network.dhcp) {
        serial_printf("[NETWORK] dhcp\n");
        Ethernet.begin();
    } else {
        serial_printf("[NETWORK] static\n");
        Ethernet.begin(worker.network.local, worker.network.subnet, worker.network.gateway, worker.network.dns);
    }
    serial_printf("[NETWORK] mac %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    serial_printf("[NETWORK] local %s\n", Ethernet.localIP().toString().c_str());
    serial_printf("[NETWORK] subnet %s\n", Ethernet.subnetMask().toString().c_str());
    serial_printf("[NETWORK] gateway %s\n", Ethernet.gatewayIP().toString().c_str());
    serial_printf("[NETWORK] dns %s\n", Ethernet.dnsServerIP().toString().c_str());
    mqtt.setServer(worker.network.mqtt.server, worker.network.mqtt.port);
    mqtt.setBufferSize(384);
}

void network_loop() {
    watchdog_loop();
    mqtt_init();
    mqtt_loop();
}

void mqtt_init() {
    const char *username = NULL;
    const char *password = NULL;
    if (mqtt.connected() == false) {
        if (worker.network.mqtt.authentication) {
            username = worker.network.mqtt.username.c_str();
            password = worker.network.mqtt.password.c_str();
        }
        if (mqtt.connect("ramure", username, password)) {
            serial_printf("[MQTT] connected\n");
            mqtt_subscribe();
        } else {
            serial_printf("[ERROR] unable to connect to broker\n");
        }
    }
}

void mqtt_loop() {
	serial_printf("[MQTT] Server %s\n", worker.network.mqtt.server.toString().c_str());
    watchdog_loop();
    mqtt.loop();
    mqtt_publish();
}

void mqtt_subscribe() {
    mqtt.setCallback(mqtt_callback);
    mqtt.subscribe(path_mqtt_subscribe.c_str());
}

void mqtt_publish() {
    uint32_t now = millis();
    if (worker.delay) {
        if (now - worker.last >= worker.delay * 60 * 1000) {
            worker.last = now;
            if (mqtt.publish(path_mqtt_publish.c_str(), worker_json().c_str())) {
                serial_printf("[MQTT] publish\n");
            } else {
                serial_printf("[ERROR] unable to publish\n");
            }
        }
    }
}

/**
 * @brief
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"relays","relays":0}'
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"relays","relays":1}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"probes","probes":0}'
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"probes","probes":1}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"target","target":24.0}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"iset","iset":2.5}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"delay","delay":0}'
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"delay","delay":2}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"mobile","mobile":"0678901234"}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"sms","message":"essai"}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"publish"}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"regulate","regulate":false}'
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"regulate","regulate":true}'
 *
 * mosquitto_pub -h 134.158.124.133 -u RAMURE -P QgQDIKHe63JsRIPs -t network/RAMURE/control -m '{"action":"reboot"}'
 *
 *
 * @param topic
 * @param payload
 * @param length
 */
void mqtt_callback(char* topic, uint8_t* payload, unsigned int length) {
    if (strcmp(topic, path_mqtt_subscribe.c_str()) == 0) {
        String command;
        for (auto index = 0; index < length; ++index) {
            command += static_cast<char>(payload[index]);
        }
        json_command(command);
    }
}

void sms_send(const String &message) {
    serial_printf("[SMS] init\n");
    watchdog_loop();
    if (sms.begin("ramure")) {
        watchdog_loop();
        if (sms.send(worker.mobile, message)) {
            serial_printf("[SMS] sms sent\n");
        } else {
            serial_printf("[ERROR] sms not sent\n");
        }
        watchdog_loop();
        sms.end();
    } else {
        watchdog_loop();
        serial_printf("[ERROR] unable to connect to sms server\n");
    }
}

void psu_init() {
    serial_printf("[PSU] init\n");
    psu.begin(SerialPSU);
    delay(500);
    serial_printf("[PSU] idn %s\n", psu.idn().c_str());
    psu.vset(worker.voltage.set);
    worker.voltage.get = psu.vset();
    serial_printf("[PSU] vset %.2f V\n", worker.voltage.get);
    psu.iset(worker.current.set);
    worker.current.get = psu.iset();
    serial_printf("[PSU] iset %.2f A\n", worker.current.get);
}

void psu_loop() {
    watchdog_loop();
    serial_printf("[PSU] loop\n");
    worker.voltage.out = psu.vout();
    serial_printf("[PSU] voltage out %.2f V\n", worker.voltage.out);
    worker.current.out = psu.iout();
    serial_printf("[PSU] current out %.2f A\n", worker.current.out);
    worker.status = psu.status();
    serial_printf("[PSU] status 0x%02X", worker.status);
    serial_printf(" %s", worker.status & PSU::Status::CV ? "CV" : "CC");
    serial_printf(" %s\n", worker.status & PSU::Status::Output ? "on" : "off");
}

void probes_init() {
    serial_printf("[PROBES] init\n");
    uint8_t probes = 0;
    for (auto index = 0; index < DS18B20_COUNT; ++index) {
        ds18b20[index] = new DS18B20(new OneWire(DS18B20_PINS[index]));
        if (ds18b20[index]->begin()) {
            // Found
            probes |= 1 << index;
            ds18b20[index]->setResolution(10);
            serial_printf("[PROBES] probe %u found\n", index);
        } else {
            // Not found
            serial_printf("[ERROR] probe %u not found\n", index);
        }
    }
    worker.probes = probes;
}

void probes_loop() {
    watchdog_loop();
    serial_printf("[PROBES] loop\n");
    for (auto index = 0; index < DS18B20_COUNT; ++index) {
        if (worker.probes >> index & 1) {
            ds18b20[index]->requestTemperatures();
            while (ds18b20[index]->isConversionComplete() == false) {
                delay(DS18B20_DELAY);
            }
            worker.temperatures[index] = ds18b20[index]->getTempC();
       //     serial_printf("[PROBES] probe %u temperature %.2f °C\n", index, worker.temperatures[index]);
            Serial.print("[PROBES] probe "); Serial.print(index); Serial.print(" temperature "); Serial.print( worker.temperatures[index]);Serial.println("  °C ");
        } else {
            worker.temperatures[index] = 0;
        }
    }
}

void light_init() {
    serial_printf("[LIGHT] init\n");
    pinMode(NSL19M51_PIN, INPUT_ANALOG);
}

void light_loop() {
    watchdog_loop();
    serial_printf("[LIGHT] loop\n");
    worker.light = analogRead(NSL19M51_PIN);
    serial_printf("[LIGHT] %u\n", worker.light);
}

void relays_init() {
    serial_printf("[RELAYS] init\n");
    for (auto index = 0; index < RELAYS_COUNT; ++index) {
        pinMode(RELAYS_PINS[index], OUTPUT);
    }
}

void relays_loop() {
    watchdog_loop();
    serial_printf("[RELAYS] loop\n");
    for (auto index = 0; index < RELAYS_COUNT; ++index) {
        uint32_t value = worker.relays >> index & 1 ? HIGH : LOW;
        digitalWrite(RELAYS_PINS[index], value);
    }
}

void fans_loop() {
    watchdog_loop();
    if (worker.regulate) {
        serial_printf("[FANS] loop\n");
        double weight = 0.0;
        for (auto index = 1; index < DS18B20_COUNT; ++index) {
            if (worker.probes >> index & 1) {
                weight = std::max<double>(weight, (worker.temperatures[index] - worker.target));
            }
        }
        worker.voltage.set = std::min<double>(6 + 0.5 * weight, 12);
        psu.vset(worker.voltage.set);
        worker.voltage.get = psu.vset();
        serial_printf("[FANS] vset %.2f V\n", worker.voltage.get);
    }
}

/**
 * @brief
 *
 */
void alarm_loop() {
    uint32_t now = rtc.getEpoch();
    // Internal
    if (worker.temperatures[0] > ALARM_INTERNAL) {
        // Alarm on
        worker.alarms[Alarm::Internal].active = true;
        serial_printf("[ALARM] internal on\n");
        // Send SMS if needed
        if (worker.alarms[Alarm::Internal].sms == false) {
        	sms_send(msg_alarm_int.c_str());
            // Mark as sent
            worker.alarms[Alarm::Internal].sms = true;
            worker.alarms[Alarm::Internal].last = now;
        }
    } else {
        // Alarm off
        if (worker.alarms[Alarm::Internal].active) {
            worker.alarms[Alarm::Internal].active = false;
            serial_printf("[ALARM] internal off\n");
        }
    }
    // Reset SMS
    if (worker.alarms[Alarm::Internal].sms) {
        if (now - worker.alarms[Alarm::Internal].last >= ALARM_DELAY) {
            worker.alarms[Alarm::Internal].sms = false;
            serial_printf("[ALARM] internal sms reset\n");
        }
    }
    // External
    bool external = false;
    for (auto index = 1; index < DS18B20_COUNT; ++index) {
        if (worker.probes >> index & 1) {
            if (worker.temperatures[index] > ALARM_EXTERNAL) {
                external = true;
            }
        }
    }
    if (external) {
        // Alarm on
        worker.alarms[Alarm::External].active = true;
        serial_printf("[ALARM] external on\n");
        // Send SMS if needed
        if (worker.alarms[Alarm::External].sms == false) {
        	sms_send(msg_alarm_ext.c_str());
            // Mark as sent
            worker.alarms[Alarm::External].sms = true;
            worker.alarms[Alarm::External].last = now;
        }
    } else {
        // Alarm off
        if (worker.alarms[Alarm::External].active) {
            worker.alarms[Alarm::External].active = false;
            serial_printf("[ALARM] external off\n");
        }
    }
    // Reset SMS
    if (worker.alarms[Alarm::External].sms) {
        if (now - worker.alarms[Alarm::External].last >= ALARM_DELAY) {
            worker.alarms[Alarm::External].sms = false;
            serial_printf("[ALARM] external sms reset\n");
        }
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    serial_init();
    worker_init();
    watchdog_status();
    sd_init();
    path_affectation();

    probes_init();
    probes_loop();

    light_init();
    relays_init();

    if (RAMURE_FAN == 1)  {psu_init();}

    network_init();
    rtc_init();
    rtc_read_update();

    sdram.begin();
    tft.begin();
    ts.begin();
//#if RAMURE_LVGL
    lvgl_init();
    lvgl_setup();
//#endif
    watchdog_init();
}

extern "C" char* sbrk(int incr);

int freemem() {
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
}

void loop() {
    digitalToggle(LED_BUILTIN);
    probes_loop();
    light_loop();
    relays_loop();
    rtc_read_update();

if (RAMURE_FAN == 1)
{   fans_loop();
    psu_loop();}

    network_loop();
    alarm_loop();

#if RAMURE_LVGL
    lvgl_loop();
#endif
    serial_loop();
    sd_loop();
    serial_printf("[RAM] free memory %d bytes\n", freemem());
}
