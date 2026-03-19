#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile unsigned long t = 0;

ISR(TIMER0_COMPA_vect) {
    t++;
}

void timerStart() {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

unsigned long millis() {
    unsigned long x;
    cli();
    x = t;
    sei();
    return x;
}

void adcStart() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adcRead(int ch) {
    ADMUX = (1 << REFS0) | (ch & 0x07);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void ledsOff() {
    PORTB &= ~((1 << PB5) | (1 << PB4) | (1 << PB3) | (1 << PB2));
}

void ledsOn() {
    PORTB |= (1 << PB5) | (1 << PB4) | (1 << PB3) | (1 << PB2);
}

void rgbOff() {
    PORTB &= ~(1 << PB0);
    PORTD &= ~((1 << PD7) | (1 << PD6));
}

void rgbColor(int c) {
    rgbOff();

    if (c == 0) {
        PORTB |= (1 << PB0);
    }
    else if (c == 1) {
        PORTD |= (1 << PD7);
    }
    else if (c == 2) {
        PORTD |= (1 << PD6);
    }
    else if (c == 3) {
        PORTB |= (1 << PB0);
        PORTD |= (1 << PD7) | (1 << PD6);
    }
}

int main(void) {
    DDRB |= (1 << DDB5) | (1 << DDB4) | (1 << DDB3) | (1 << DDB2);
    DDRB |= (1 << DDB0);
    DDRD |= (1 << DDD7) | (1 << DDD6);

    DDRD &= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5));
    DDRB &= ~(1 << DDB1);

    PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5);
    PORTB |= (1 << PORTB1);

    adcStart();
    timerStart();
    sei();

    unsigned long blinkTime = 0;
    unsigned long encBtnTime = 0;
    unsigned long btn1Time = 0;
    unsigned long btn2Time = 0;

    int blink = 0;
    int mode = 0;
    int color = 0;
    int active = 0;

    int oldClk = (PIND & (1 << PD2));

    while (1) {
        unsigned long now = millis();
        int pot = adcRead(0);
        unsigned long speed = 100 + ((unsigned long)pot * 900) / 1023;

        if (now - blinkTime >= speed) {
            blinkTime = now;
            if (blink == 0) {
                blink = 1;
            } else {
                blink = 0;
            }
        }

        if (blink == 1) {
            ledsOn();
        } else {
            ledsOff();
        }

        int clk = (PIND & (1 << PD2));
        int dt = (PIND & (1 << PD3));

        if (clk != oldClk) {
            if (clk) {
                if (dt != clk) {
                    color++;
                    if (color > 4) {
                        color = 0;
                    }
                } else {
                    color--;
                    if (color < 0) {
                        color = 4;
                    }
                }
            }
        }
        oldClk = clk;

        if (mode == 0) {
            rgbOff();
        }
        else if (mode == 1) {
            if (blink == 1) {
                if (active != 4) {
                    rgbColor(active);
                } else {
                    rgbOff();
                }
            } else {
                rgbOff();
            }
        }
        else if (mode == 2) {
            if (active != 4) {
                rgbColor(active);
            } else {
                rgbOff();
            }
        }

        if (!(PIND & (1 << PD4))) {
            if (now - encBtnTime > 30) {
                active = color;
                mode = 1;
                encBtnTime = now;
            }
        }

        if (!(PIND & (1 << PD5))) {
            if (now - btn1Time > 30) {
                mode = 2;
                btn1Time = now;
            }
        }

        if (!(PINB & (1 << PB1))) {
            if (now - btn2Time > 30) {
                mode = 1;
                btn2Time = now;
            }
        }
    }
}