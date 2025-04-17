#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "audio_pthread.h"

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define BUFFER_SIZE (SAMPLE_RATE * CHANNELS)
#define LOW_PASS_ALPHA 0.05f
#define HIGH_PASS_ALPHA 0.5f

typedef enum FilterMode {
    FILTER_NONE,
    FILTER_LOW_PASS,
    FILTER_HIGH_PASS
} FilterMode;

static FilterMode current_filter = FILTER_NONE;
static bool debug_enabled = false;

static pthread_mutex_t filter_mutex;

float low_pass_filter(float input, float* prev_output) {
    const float output = LOW_PASS_ALPHA * input + (1.0f - LOW_PASS_ALPHA) * (*prev_output);
    *prev_output = output;
    return output;
}

float high_pass_filter(float input, float* prev_input, float* prev_output) {
    const float output = HIGH_PASS_ALPHA * (*prev_output + input - *prev_input);
    *prev_output = output;
    *prev_input = input;
    return output;
}

void audio_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    if (input == NULL || output == NULL) return;

    float* sample_in = (float*)input;
    float* sample_out = (float*)output;

    static float lp_prev_output = 0.0f;
    static float hp_prev_input = 0.0f;
    static float hp_prev_output = 0.0f;

    pthread_mutex_lock(&filter_mutex);
    const FilterMode filter = current_filter;
    const bool is_debug_enabled = debug_enabled;
    pthread_mutex_unlock(&filter_mutex);

    for (ma_uint32 i = 0; i < frame_count; ++i) {
        const float x = sample_in[i];
        float y;

        switch (filter) {
            case FILTER_LOW_PASS:
                y = low_pass_filter(x, &lp_prev_output);
                break;
            case FILTER_HIGH_PASS:
                y = high_pass_filter(x, &hp_prev_input, &hp_prev_output);
                break;
            case FILTER_NONE:
            default:
                y = x;
                break;
        }

        sample_out[i] = y;

        if (is_debug_enabled && fabsf(x) > 0.1f) {
            switch (filter) {
                case FILTER_LOW_PASS:
                    printf("[LPF] raw: %.3f, filtered: %.3f, prev: %.3f\n", x, y, lp_prev_output);
                    break;
                case FILTER_HIGH_PASS:
                    printf("[HPF] raw: %.3f, filtered: %.3f, prev_in: %.3f, prev_out: %.3f\n",
                           x, y, hp_prev_input, hp_prev_output);
                    break;
                case FILTER_NONE:
                    printf("[NONE] raw passthrough: %.3f\n", y);
                    break;
            }
        }
    }
}

void* input_thread(void* arg) {
    (void)arg;
    printf("\nControls:\n");
    printf("  L - Low-Pass Filter\n");
    printf("  H - High-Pass Filter\n");
    printf("  N - No Filter\n");
    printf("  D - Toggle Debug Mode\n");
    printf("  Q - Quit\n\n");

    while (true) {
        int ch = getchar();
        if (ch == EOF) continue;

        pthread_mutex_lock(&filter_mutex);
        switch (ch) {
            case 'l': case 'L':
                current_filter = FILTER_LOW_PASS;
                printf("Switched to Low-Pass Filter\n");
                break;
            case 'h': case 'H':
                current_filter = FILTER_HIGH_PASS;
                printf("Switched to High-Pass Filter\n");
                break;
            case 'n': case 'N':
                current_filter = FILTER_NONE;
                printf("Switched to No Filter\n");
                break;
            case 'd': case 'D':
                debug_enabled = !debug_enabled;
                printf("Debug mode %s\n", debug_enabled ? "enabled" : "disabled");
                break;
            case 'q': case 'Q':
                pthread_mutex_unlock(&filter_mutex);
                return NULL;
        }
        pthread_mutex_unlock(&filter_mutex);
    }
    return NULL;
}

int main(int argc, char** argv) {
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_duplex);
    ma_device device;

    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.capture.channels = CHANNELS;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = CHANNELS;
    deviceConfig.sampleRate = SAMPLE_RATE;
    deviceConfig.dataCallback = audio_data_callback;
    deviceConfig.pUserData = NULL;

    pthread_mutex_init(&filter_mutex, NULL);

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to initialize audio device.\n");
        return -1;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start audio device.\n");
        ma_device_uninit(&device);
        return -1;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, input_thread, NULL);
    pthread_join(thread, NULL);

    ma_device_uninit(&device);
    pthread_mutex_destroy(&filter_mutex);
    return 0;
}
