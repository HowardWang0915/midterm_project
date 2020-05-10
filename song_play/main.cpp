#include "mbed.h"
#include "uLCD_4DGL.h"
#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <cmath>
#include "DA7212.h"

#define FOWARD 0
#define BACKWARD 1
#define CHANGESONG 2
#define SONG1 1
#define SONG2 2
#define SONG3 3
#define RING 3
#define SLOPE 4

DA7212 audio;
uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn sw2(SW2);
DigitalIn sw3(SW3);


int16_t waveform[kAudioTxBufferSize];
int output = 0;         // indicate the output patern
bool Play_The_Music = false;

void Play_Song(void);
void Mode_Select(void);
void Song_Select(void);
int PredictGesture(float* output);
void DNN(void);
void playNote(int freq);
void PlayMusic(void);

Thread thread1 (osPriorityNormal, 120 * 1024);      // for DNN
Thread thread2;                                     // for mode selection interrupt
Thread thread3;                                     // for sound playing
// Thread thread4;                                     // to make the sound playing module play out a note
EventQueue queue1(100 * EVENTS_EVENT_SIZE);         // for mode selection interrupt
EventQueue queue2(32 * EVENTS_EVENT_SIZE);

int main(void) {
    thread1.start(DNN);
    while(true) {
        Play_Song();
    }
}

void Mode_Select(void) {
    // thread1.start(callback(&queue1, &EventQueue::dispatch_forever));
    Play_The_Music = false;                 // stop the music playing
    int current_mode = FOWARD;
    static int current_song = SONG1;        // remember the current song
    
    uLCD.cls();
    uLCD.color(BLUE);
    uLCD.printf("\nFuck you Mode Select\n"); //Default Green on black text
    uLCD.color(GREEN);
    uLCD.printf("Foward\n");
    uLCD.color(RED);
    uLCD.printf("Backward\n");
    uLCD.printf("Change song\n");
    while(1) {
        // returning to play song
        if (!sw3) {
            uLCD.cls();
            uLCD.color(GREEN);
            if (current_mode == FOWARD) {
                Play_The_Music = true;
                uLCD.printf("\nPlaying the fucking song\n"); //Default Green on black text
                if (current_song == SONG1) {
                    uLCD.printf("\nPlaying song 2\n");
                    current_song = SONG2;
                    return;
                }
                else if (current_song == SONG2) {
                    uLCD.printf("\nPlaying song 3\n");
                    current_song = SONG3;
                    return;
                }
                else if (current_song == SONG3) {
                    uLCD.printf("\nPlaying song 1\n");
                    current_song = SONG1;
                    return;
                }
            }
            else if (current_mode == BACKWARD) {
                Play_The_Music = true;
                uLCD.printf("\nPlaying the fucking song\n"); //Default Green on black text
                if (current_song == SONG1) {
                    uLCD.printf("\nPlaying song 3\n");
                    current_song = SONG3;
                    return;
                }
                else if (current_song == SONG2) {
                    uLCD.printf("\nPlaying song 1\n");
                    current_song = SONG1;
                    return;
                }
                else if (current_song == SONG3) {
                    uLCD.printf("\nPlaying song 2\n");
                    current_song = SONG2;
                    return;
                }
            }
            else if (current_mode == CHANGESONG) {
                Song_Select();
                return;
            }
            // return;
        }
        else if(output == RING) {       // previous mode
            uLCD.cls();
            uLCD.color(BLUE);
            uLCD.printf("\nFuck you Mode Select\n"); //Default Green on black text
            if (current_mode == FOWARD) {
                uLCD.color(RED);
                uLCD.printf("Foward\n");
                uLCD.printf("Backward\n");
                uLCD.color(GREEN);
                uLCD.printf("Change song\n");
                current_mode = CHANGESONG;
            }
            else if (current_mode == BACKWARD) {
                uLCD.color(GREEN);
                uLCD.printf("Foward\n");
                uLCD.color(RED);
                uLCD.printf("Backward\n");
                uLCD.printf("Change song\n");
                current_mode = FOWARD;
            }
            else if (current_mode == CHANGESONG) {
                uLCD.color(RED);
                uLCD.printf("Foward\n");
                uLCD.color(GREEN);
                uLCD.printf("Backward\n");
                uLCD.color(RED);
                uLCD.printf("Change song\n");
                current_mode = BACKWARD;
            }
            wait(0.1);
        }
        else if(output == SLOPE) {      // next mode
            uLCD.cls();
            uLCD.color(BLUE);
            uLCD.printf("\nFuck you Mode Select\n"); //Default Green on black text
            if (current_mode == BACKWARD) {
                uLCD.color(RED);
                uLCD.printf("Foward\n");
                uLCD.printf("Backward\n");
                uLCD.color(GREEN);
                uLCD.printf("Change song\n");
                current_mode = CHANGESONG;
            }
            else if (current_mode == CHANGESONG) {
                uLCD.color(GREEN);
                uLCD.printf("Foward\n");
                uLCD.color(RED);
                uLCD.printf("Backward\n");
                uLCD.printf("Change song\n");
                current_mode = FOWARD;
            }
            else if (current_mode == FOWARD) {
                uLCD.color(RED);
                uLCD.printf("Foward\n");
                uLCD.color(GREEN);
                uLCD.printf("Backward\n");
                uLCD.color(RED);
                uLCD.printf("Change song\n");
                current_mode = BACKWARD;
            }
            wait(0.1);
        }
    }
}
void Play_Song(void) {
    // sw2.rise(&Mode_Select);
    
    thread2.start(callback(&queue1, &EventQueue::dispatch_forever));
    sw2.rise(queue1.event(Mode_Select));
    Play_The_Music = true;
    uLCD.cls();
    uLCD.color(GREEN);
    uLCD.printf("\nPlaying the fucking song\n"); //Default Green on black text
    uLCD.printf("\nPlaying song 1");
    
    while(1) {
        PlayMusic();
    }
}

void Song_Select(void) {
    int current_song = SONG1;
    uLCD.cls();
    uLCD.color(BLUE);
    uLCD.printf("\nSong Select\n");
    uLCD.color(GREEN);
    uLCD.printf("Song 1\n");
    uLCD.color(RED);
    uLCD.printf("Song 2\n");
    uLCD.printf("Song 3\n");
    while (true) {
        if (!sw3) {
            uLCD.cls();
            uLCD.color(GREEN);
            uLCD.printf("\nPlaying the fucking song\n");
            uLCD.printf("\nPlaying song %d\n", current_song);
            Play_The_Music = true;
            return;
        }
        else if (output == RING) {
            uLCD.cls();
            uLCD.color(BLUE);
            uLCD.printf("\nSong Select\n");
            if (current_song == SONG1) {
                uLCD.color(RED);
                uLCD.printf("Song 1\n");
                uLCD.printf("Song 2\n");
                uLCD.color(GREEN);
                uLCD.printf("Song 3\n");
                current_song = SONG3;
            }
            else if (current_song == SONG2) {
                uLCD.color(GREEN);
                uLCD.printf("Song 1\n");
                uLCD.color(RED);
                uLCD.printf("Song 2\n");
                uLCD.printf("Song 3\n");
                current_song = SONG1;
            }
            else if (current_song == SONG3) {
                uLCD.color(RED);
                uLCD.printf("Song 1\n");
                uLCD.color(GREEN);
                uLCD.printf("Song 2\n");
                uLCD.color(RED);
                uLCD.printf("Song 3\n");
                current_song = SONG2;
            }
            wait(0.1);
        }
        else if (output == SLOPE) {
            uLCD.cls();
            uLCD.color(BLUE);
            uLCD.printf("\nSong Select\n");
            if (current_song == SONG1) {
                uLCD.color(RED);
                uLCD.printf("Song 1\n");
                uLCD.color(GREEN);
                uLCD.printf("Song 2\n");
                uLCD.color(RED);
                uLCD.printf("Song 3\n");
                current_song = SONG2;
            }
            else if (current_song == SONG2) {
                uLCD.color(RED);
                uLCD.printf("Song 1\n");
                uLCD.printf("Song 2\n");
                uLCD.color(GREEN);
                uLCD.printf("Song 3\n");
                current_song = SONG3;
            }
            else if (current_song == SONG3) {
                uLCD.color(GREEN);
                uLCD.printf("Song 1\n");
                uLCD.color(RED);
                uLCD.printf("Song 2\n");
                uLCD.printf("Song 3\n");
                current_song = SONG1;
            }
            wait(0.1);
        }
    }
}
// ***************************************************DNN******************************************* //
// Return the result of the last prediction
int PredictGesture(float* output) {
    // How many times the most recent gesture has been matched in a row
    static int continuous_count = 0;
    // The result of the last prediction
    static int last_predict = -1;

    // Find whichever output has a probability > 0.8 (they sum to 1)
    int this_predict = -1;
    for (int i = 0; i < label_num; i++) {
        if (output[i] > 0.8) this_predict = i;
    }

    // No gesture was detected above the threshold
    if (this_predict == -1) {
        continuous_count = 0;
        last_predict = label_num;
        return label_num;
    }

    if (last_predict == this_predict) {
        continuous_count += 1;
    } 
    else {
        continuous_count = 0;
    }
    last_predict = this_predict;

    // If we haven't yet had enough consecutive matches for this gesture,
    // report a negative result
    if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
        return label_num;
    }
    // Otherwise, we've seen a positive result, so clear all our variables
    // and report it
    continuous_count = 0;
    last_predict = -1;

    return this_predict;
}
void DNN(void) {
    // Create an area of memory to use for input, output, and intermediate arrays.
    // The size of this will depend on the model you're using, and may need to be
    // determined by experimentation.
    constexpr int kTensorArenaSize = 60 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];

    // Whether we should clear the buffer next time we fetch data
    bool should_clear_buffer = false;
    bool got_data = false;

    // The gesture index of the prediction
    int gesture_index;

    // Set up logging.
    static tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report(
            "Model provided is schema version %d not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // Pull in only the operation implementations we need.
    // This relies on a complete list of all the ops needed by this graph.
    // An easier approach is to just use the AllOpsResolver, but this will
    // incur some penalty in code space for op implementations that are not
    // needed by this graph.
    static tflite::MicroOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddBuiltin(
        tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
        tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                             tflite::ops::micro::Register_RESHAPE(), 1);
    // Build an interpreter to run the model with
    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    tflite::MicroInterpreter* interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors
    interpreter->AllocateTensors();

    // Obtain pointer to the model's input tensor
    TfLiteTensor* model_input = interpreter->input(0);
    if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
        (model_input->dims->data[1] != config.seq_length) ||
        (model_input->dims->data[2] != kChannelNumber) ||
        (model_input->type != kTfLiteFloat32)) {
            error_reporter->Report("Bad input tensor parameters in model");
        return;
    }

    int input_length = model_input->bytes / sizeof(float);

    TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
    if (setup_status != kTfLiteOk) {
        error_reporter->Report("Set up failed\n");
        return;
    }

    error_reporter->Report("Set up successful...\n");

    while (true) {

        // Attempt to read new data from the accelerometer
        got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                 input_length, should_clear_buffer);

        // If there was no new data,
        // don't try to clear the buffer again and wait until next time
        if (!got_data) {
            should_clear_buffer = false;
            continue;
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            error_reporter->Report("Invoke failed on index: %d\n", begin_index);
            continue;
        }

        // Analyze the results to obtain a prediction
        gesture_index = PredictGesture(interpreter->output(0)->data.f);

        // Clear the buffer next time we read data
        should_clear_buffer = gesture_index < label_num;

        // Produce an output
        output = false;
        if (gesture_index < label_num) {
            if (gesture_index == 0)
                output = RING;
            else if (gesture_index == 1)
                output = SLOPE;
            // error_reporter->Report(config.output_message[gesture_index]);
            
        }
    }
}
// **************************************Note playing*******************************************//
int song[42] = {
    261, 261, 392, 392, 440, 440, 392,
    349, 349, 330, 330, 294, 294, 261,
    392, 392, 349, 349, 330, 330, 294,
    392, 392, 349, 349, 330, 330, 294,
    261, 261, 392, 392, 440, 440, 392,
    349, 349, 330, 330, 294, 294, 261
};

int noteLength[42] = {
    1, 1, 1, 1, 1, 1, 2,
    1, 1, 1, 1, 1, 1, 2,
    1, 1, 1, 1, 1, 1, 2,
    1, 1, 1, 1, 1, 1, 2,
    1, 1, 1, 1, 1, 1, 2,
    1, 1, 1, 1, 1, 1, 2
}; 

void playNote(int freq) {
    for(int i = 0; i < kAudioTxBufferSize; i++) {
        waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
    }
    audio.spk.play(waveform, kAudioTxBufferSize);
}
void PlayMusic(void) {
    thread3.start(callback(&queue2, &EventQueue::dispatch_forever));

    for(int i = 0; i < 42 && Play_The_Music; i++) {
        int length = noteLength[i];
        while(length-- && Play_The_Music) {
        // the loop below will play the note for the duration of 1s
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize  && Play_The_Music; ++j) {
                queue2.call(playNote, song[i]);
            }
            if(length < 1) wait(0.5);
        }
    }
    if (!Play_The_Music)
        queue2.call(playNote, 0);
}