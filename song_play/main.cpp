#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn sw2(SW2);

void LCD(void);
void Mode_Select(void);
void Song_Select(void);

int main(void) {
    Thread Play_Song;
    Thread Mode_Select;
    EventQueue queue(100 * EVENTS_EVENT_SIZE);
    Play_Song.start(callback(&queue, &EventQueue::dispatch_forever));
    LCD();      // LCD display
    sw2.rise(queue.event(Mode_Select));
}

void LCD(void) {
    uLCD.background_color(BLACK);
    uLCD.cls();
    uLCD.background_color(DGREY);
    uLCD.filled_circle(60, 50, 30, 0xFF00FF);
    uLCD.triangle(120, 100, 40, 40, 10, 100, 0x0000FF);
    uLCD.line(0, 0, 80, 60, 0xFF0000);
    uLCD.filled_rectangle(50, 50, 100, 90, 0x00FF00);
    uLCD.pixel(60, 60, BLACK);
    uLCD.read_pixel(120, 70);
    uLCD.circle(120, 60, 10, BLACK);
    uLCD.set_font(FONT_7X8);
    uLCD.text_mode(TRANSPARENT);
    uLCD.text_bold(ON);
    uLCD.text_char('B', 9, 8, BLACK);
    uLCD.text_char('I',10, 8, BLACK);
    uLCD.text_char('G',11, 8, BLACK);
    uLCD.text_italic(ON);
    uLCD.text_string("This is a test of string", 1, 4, FONT_7X8, WHITE);
    wait(2);
}
void Mode_Select(void) {
    uLCD.cls();
    uLCD.printf("\nHello uLCD World\n"); //Default Green on black text
    uLCD.printf("\n  Starting Demo...");
}