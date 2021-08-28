#include <Arduino.h>

#include "driver/i2s.h"
#include "flite.h"

#define I2S_BCK_PIN 23
#define I2S_WS_PIN 22
#define I2S_DATA_PIN 21

i2s_config_t i2s_config;

char txt[] = "The quick brown fox jumps over the lazy dog.";

void init_i2s(void) {
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    i2s_config.sample_rate = 44100;                          // will change later
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;  // left 16 bits and right 16 bits
    i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
    i2s_config.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.intr_alloc_flags = 0;  // default interrupt priority
    i2s_config.dma_buf_count = 8;
    i2s_config.dma_buf_len = 128;
    i2s_config.use_apll = false;

    static const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE};

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

/**
 * @brief This is a callback called by Flite each time a chunk of the WAV is
 * available. It send this chunk over I2S to the PCM chip.
 */
int i2s_stream_chunk(const cst_wave *w, int start, int size,
                     int last, cst_audio_streaming_info *asi) {
    /* Called with new samples from start for size samples */
    /* last is true if this is the last segment. */
    /* This is really just and example that you can copy for you streaming */
    /* function */
    /* This particular example is *not* thread safe */

    if (start == 0) {
        i2s_set_sample_rates(I2S_NUM_0, w->sample_rate);  //ad = audio_open(w->sample_rate,w->num_channels,CST_AUDIO_LINEAR16);
    }

    size_t bytes_written = 0;
    i2s_write(I2S_NUM_0, &(w->samples[start]), size * sizeof(uint16_t) /*sizeof(uint16_t) * wav->num_samples*/, &bytes_written, 100);

    //ESP_LOGI(TAG, "Wrote %d bytes to I2S", bytes_written);

    if (last == 1) {
        // Nothing
    }

    /* if you want to stop return CST_AUDIO_STREAM_STOP */
    return CST_AUDIO_STREAM_CONT;
}

void setup() {
    init_i2s();
    flite_init();
    cst_voice *v = register_cmu_us_kal(NULL);
    cst_audio_streaming_info *asi = cst_alloc(struct cst_audio_streaming_info_struct, 1);

    asi->min_buffsize = 256;
    asi->asc = i2s_stream_chunk;
    asi->userdata = NULL;

    feat_set(v->features, "streaming_info", audio_streaming_info_val(asi));

    cst_wave *wav = flite_text_to_wave(txt, v);
    delete_wave(wav);
}

void loop() {
}
