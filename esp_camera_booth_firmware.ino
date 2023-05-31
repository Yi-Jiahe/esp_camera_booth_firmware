#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include "driver/ledc.h"

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

void initCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (psramFound())
    {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        Serial.println("PSRAM not found");
        // Limit the frame size when PSRAM is not available
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_quality(s, 4);
    s->set_brightness(s, 2);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);
    s->set_exposure_ctrl(s, 1);
}

int initSD()
{
    if (!SD_MMC.begin())
    {
        Serial.println("Failed to mount SD card"); 
        return -1;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card available"); 
        return -2;
    }

    return 0;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Hello");

    initCamera();
    Serial.println("Camera Initialized"); 

    if (initSD() != 0)
    {
        Serial.println("Failed to initialize SD"); 
        return;
    }
    Serial.println("SD Initialized");

    camera_fb_t *fb = NULL;

    // Grab a number of frames for automatic exposure control
    int i;
    for (i=0; i<3; i++){
        fb = esp_camera_fb_get();
        esp_camera_fb_return(fb);
    }
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed"); 
        return;
    }
    Serial.println("Picture taken"); 

    String path = "/picture.jpg";

    fs::FS &fs = SD_MMC;

    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file in writing mode"); 
        return;
    }

    file.write(fb->buf, fb->len); // payload (image), payload length
    file.close();
    Serial.println("Image saved"); 

    esp_camera_fb_return(fb);

    esp_deep_sleep_start();
}

void loop()
{
}
