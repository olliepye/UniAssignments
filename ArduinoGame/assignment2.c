/*
CAB202 Assignment 2

Aurthor: Oliver Pye n9703977
         o.pye@connect.qut.edu.au
         Queensland University of Technology

October 2018

The following code produces a game where a player has to navigate through a
series of obsticals while gaining points and retaining lives. This program
is developed to work on the QUT EESS TeensyPewPew.

*/

#include <graphics.h>
#include <lcd.h>
#include <lcd_model.h>
#include <macros.h>
#include <ram_utils.h>
#include <sprite.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <cpu_speed.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include "cab202_adc.h"
#include <stdlib.h>

#include "usb_serial.h"
#include <avr/pgmspace.h>


bool game_over = false;
bool game_start;
bool game_pause = false;
bool update_screen = true;
// initialise player and block sprites
int total_blocks = 0;
Sprite player;
Sprite blocks[40];
Sprite start_block;
Sprite treasure;
bool treasure_stop = false;
int lives = 10;
int score = 0;
double block_speed = .3;
int current_block;
int total_zombies = 5;
const int total_zombies1 PROGMEM = 5;
Sprite zombies[5];
int total_food = 5;
Sprite food[5];
int food_ctr = 5;
bool on_block = false;
int zombie_ctr = 0;
bool timer_reset = false;
bool zombie_timer = false;
int total_zombies_fed = 0;




int player_size = 5;
// const uint8_t player_bitmap[] PROGMEM = {
// 		0b00100000,
// 		0b01110000,
// 		0b00100000,
// 		0b01010000,
// 		0b10001000,
// };
uint8_t player_bitmap[] = {
		0b00100000,
		0b01110000,
		0b00100000,
		0b01010000,
		0b10001000,
};

uint8_t food_bitmap[] = {
		0b00110000,
		0b11001100,
		0b11001100,
		0b00110000
};


void timer_setup( void ){
    // game timer
    TCCR3A = 0;
    TCCR3B = 1;
    TIMSK3 = 1;
    timer_reset = true;
    // debounce timer
    TCCR0A = 0;
    TCCR0B = 4;
    TIMSK0 = 1;
    // zombie timer
    TCCR1A = 0;
    TCCR1B = 1;
    TIMSK1 = 1;
}

// function from lecture notes
char buffer[20];
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour){
    snprintf(buffer, sizeof(buffer), "%d", value);
    draw_string(x, y, buffer, colour);
}

void draw_00int(uint8_t x, uint8_t y, int value, colour_t colour){
    snprintf(buffer, sizeof(buffer), "%02d", value);
    draw_string(x, y, buffer, colour);
}

#define FREQ (8000000.0)
#define PRESCALE (1.0)


//debounce timer
volatile uint8_t js_count = 0;
volatile uint8_t down_count = 0;
volatile uint8_t left_count = 0;
volatile uint8_t sw2_count = 0;
volatile uint8_t sw3_count = 0;
volatile uint8_t right_count = 0;
volatile uint8_t up_count = 0;

volatile uint8_t js_pressed;
volatile uint8_t down_pressed;
volatile uint8_t left_pressed;
volatile uint8_t sw2_pressed;
volatile uint8_t sw3_pressed;
volatile uint8_t right_pressed;
volatile uint8_t up_pressed;

void up_press(void){
    uint8_t up_mask = 0b00000111;
    up_count = (((up_count << 1) & up_mask) | BIT_VALUE(PIND,1));
    if (up_count == up_mask){
        up_pressed = 1;
    }
    if (up_count == 0){
        up_pressed = 0;
    }
}

void right_press(void){
    uint8_t right_mask = 0b00000111;
    right_count = (((right_count << 1) & right_mask) | BIT_VALUE(PIND,0));
    if (right_count == right_mask){
        right_pressed = 1;
    }
    if (right_count == 0){
        right_pressed = 0;
    }
}

void sw3_press(void){
    uint8_t sw3_mask = 0b00000111;
    sw3_count = (((sw3_count << 1) & sw3_mask) | BIT_VALUE(PINF,5));
    if (sw3_count == sw3_mask){
        sw3_pressed = 1;
    }
    if (sw3_count == 0){
        sw3_pressed = 0;
    }
}

void sw2_press(void){
    uint8_t sw2_mask = 0b00000111;
    sw2_count = (((sw2_count << 1) & sw2_mask) | BIT_VALUE(PINF,6));
    if (sw2_count == sw2_mask){
        sw2_pressed = 1;
    }
    if (sw2_count == 0){
        sw2_pressed = 0;
    }
}

void left_press(void){
    uint8_t l_mask = 0b00000111;
    left_count = (((left_count << 1) & l_mask) | BIT_VALUE(PINB,1));
    if (left_count == l_mask){
        left_pressed = 1;
    }
    if (left_count == 0){
        left_pressed = 0;
    }
}

void js_press(void){
    uint8_t js_mask = 0b00000111;
    js_count = (((js_count << 1) & js_mask) | BIT_VALUE(PINB,0));
    if (js_count == js_mask){
        js_pressed = 1;
    }
    if (js_count == 0){
        js_pressed = 0;
    }
}

void down_press(void){
    uint8_t down_mask = 0b00000111;
    down_count = (((down_count << 1) & down_mask) | BIT_VALUE(PINB,7));
    if (down_count == down_mask){
        down_pressed = 1;
    }
    if (down_count == 0){
        down_pressed = 0;
    }
}

ISR(TIMER0_OVF_vect){
    js_press();
	down_press();
	left_press();
	sw2_press();
	sw3_press();
	right_press();
	up_press();
}

//game timer setup
volatile uint32_t overflow_counter = 0;
int seed = 0;
ISR(TIMER3_OVF_vect){
    if (game_pause == false && game_over != true){
        overflow_counter++;
    }
    if (timer_reset == true){
        overflow_counter = 0;
        timer_reset = false;
    }
	seed++;
}

//seed timer
volatile uint8_t seed_overflow = 0;
ISR(TIMER4_OVF_vect){
	seed_overflow++;
}


double get_time( void ){
    double time;
    time = (overflow_counter * 65536.0 + TCNT3) * PRESCALE/FREQ;
    return time;
}

// function from lecture notes
void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}

// function from lecture notes
void usb_serial_send_int(int value) {
	static char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", value);
	usb_serial_send( buffer );
}

void game_start_usb(void){
	char *game_start = "\nGame has started";
	usb_serial_send(game_start);
	char *playerx = "\nPlayer (x) = ";
	usb_serial_send(playerx);
	usb_serial_send_int((int)player.x);
	char *playery = "\nPlayer (y) = ";
	usb_serial_send(playery);
	usb_serial_send_int((int)player.y);
}

void game_time_usb(void){
	double time = get_time();
    int min = (int)time/60;
    int sec = (int)time%60;

	char *game_time = "\n Time: ";
	usb_serial_send(game_time);
	usb_serial_send_int(min);
	char *colon = ":";
	usb_serial_send(colon);
	usb_serial_send_int(sec);
}

void player_death_usb(void){
	char *player_death = "\n Player Death";
	usb_serial_send(player_death);
	// reason for Death
	char *lives_left = "\n Lives Left: ";
	usb_serial_send(lives_left);
	usb_serial_send_int(lives);
	char *score = "\n Score: ";
	usb_serial_send(score);
	usb_serial_send_int(lives);
	game_time_usb();
}

void player_respawn_usb(void){
	usb_serial_send("\n Player respawn");
	char *playerx = "\nPlayer (x) = ";
	usb_serial_send(playerx);
	usb_serial_send_int((int)player.x);
	char *playery = "\nPlayer (y) = ";
	usb_serial_send(playery);
	usb_serial_send_int((int)player.y);
}

void zombies_appear_usb(void){
	usb_serial_send("\n Zombies Spawn");
	usb_serial_send("\n Number of zombies: ");
	usb_serial_send_int(zombie_ctr);
	game_time_usb();
	usb_serial_send("\n Lives: ");
	usb_serial_send_int(lives);
	usb_serial_send("\n Score: ");
	usb_serial_send_int(score);
}

void zom_col_food_usb(void){
	usb_serial_send("\n Zombie killed by food");
	usb_serial_send("\n Number of zombies: ");
	usb_serial_send_int(zombie_ctr);
	usb_serial_send("\n Food left: ");
	usb_serial_send_int(food_ctr);
	game_time_usb();
}

void treasure_col_usb(void){
	usb_serial_send("\n Collect Treasure");
	usb_serial_send("\n Score: ");
	usb_serial_send_int(score);
	usb_serial_send("\n Lives: ");
	usb_serial_send_int(lives);
	game_time_usb();
	char *playerx = "\nPlayer (x) = ";
	usb_serial_send(playerx);
	usb_serial_send_int((int)player.x);
	char *playery = "\nPlayer (y) = ";
	usb_serial_send(playery);
	usb_serial_send_int((int)player.y);
}

void game_pause_usb(void){
	usb_serial_send("\n Game Paused");
	usb_serial_send("\n Lives: ");
	usb_serial_send_int(lives);
	usb_serial_send("\n Score: ");
	usb_serial_send_int(score);
	game_time_usb();
	usb_serial_send("\n Number of zombies: ");
	usb_serial_send_int(zombie_ctr);
	usb_serial_send("\n Food left: ");
	usb_serial_send_int(food_ctr);
}

void game_over_usb(void){
	usb_serial_send("\n GAME OVER!");
	usb_serial_send("\n Lives: ");
	usb_serial_send_int(lives);
	usb_serial_send("\n Score: ");
	usb_serial_send_int(score);
	game_time_usb();
	usb_serial_send("\n Number of zombies fed: ");
	usb_serial_send_int(total_zombies_fed);
}

// function from lecture notes
void setup_usb_serial(void) {
	// Set up LCD and display message
	draw_string(10, 10, "Connect USB...", FG_COLOUR);
	show_screen();

	usb_init();

	while ( !usb_configured() ) {
		// Block until USB is ready.
	}
	clear_screen();
}

// zombie timer
volatile uint8_t zombie_overflow_counter = 0;
ISR(TIMER1_OVF_vect){
    if (zombie_timer == true){
        zombie_overflow_counter++;
        double time1;
        time1 = (zombie_overflow_counter * 65536.0 + TCNT1) * 2/FREQ;
        int sec = (int)time1%60;
        if (sec>3){
            for (int i = 0; i < total_zombies1; i++){
                zombies[i].x = i*16+5;
                zombies[i].y = 2;
                zombies[i].dy = 1;
                zombies[i].dx = 0;
                zombies[i].is_visible = true;
            }
            zombie_ctr = 5;
            zombie_timer = false;
			zombies_appear_usb();
        }
    }
    else if (zombie_timer == false){
        zombie_overflow_counter = 0;
    }
}

uint8_t treasure_bitmap[] = {
    0b11111000,
    0b01110000,
    0b01110000,
    0b01110000,
};

uint8_t zombie_bitmap[] = {
    0b11110000,
    0b01100000,
    0b11110000,
};

uint8_t block_bitmap[] = {
    0b11111111, 0b11000000,
    0b11111111, 0b11000000,
};

uint8_t bad_block_bitmap[] = {
    0b01010101, 0b01000000,
    0b10101010, 0b10000000,
};

void block_setup ( void ){
    int cols = 4;
    int rows = 3;
	total_blocks = 0;
	srand(seed);
	int f1 = (int)rand%12;
	int f2 = (int)rand%11;
    int f3 = (int)rand%9;
	int ctr = 0;
	sprite_init(&start_block, 5, 6, 10,2,block_bitmap);
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
			if (ctr == f1 || ctr == f2 || ctr == f3){
				sprite_init(&blocks[total_blocks], j*21, i*11+16,10,2, bad_block_bitmap);
				total_blocks++;
				ctr++;
			}
            else {
                sprite_init(&blocks[total_blocks], j*21, i*11+16, 10, 2, block_bitmap);
                total_blocks++;
				ctr++;
            }}}}

void zombie_setup( void ){
    for (int i = 0; i < total_zombies; i++){
        sprite_init(&zombies[i], LCD_X, LCD_Y, 4, 3, zombie_bitmap);
        zombies[i].is_visible = false;
		zombies[i].dx = block_speed;
    }
}

void zombie_lights(void){
	int t = overflow_counter%40;
    //if (zombie_ctr > 0){
		if (t < 20){
      //set flashing led 0 and 1
      SET_BIT(PORTB, 2);
      SET_BIT(PORTB, 3);
		}
		else if (t > 20){
		 	CLEAR_BIT(PORTB, 2);
			CLEAR_BIT(PORTB, 3);
		}
	//}
}

void food_setup ( void ){
    for (int i = 0; i < total_food; i++){
       sprite_init(&food[i], 100, 100, 6, 4, food_bitmap);
       food[i].is_visible = false;
    }
}

void draw_time(void){
    double time = get_time();
    int min = (int)time/60;
    int sec = (int)time%60;
    draw_string(17, 28, "Time:", FG_COLOUR);
    draw_00int(47, 28, min, FG_COLOUR);
    draw_string(57, 28, ":", FG_COLOUR);
    draw_00int(61, 28, sec, FG_COLOUR);
}

void pause_game ( void ){
	int c = usb_serial_getchar();
    static uint8_t prevState = 0;
    if ( (js_pressed && !prevState) || c == 'p'){
        if (game_pause == false){
            game_pause = true;
            clear_screen();
            draw_string(17, 12, "Lives:", FG_COLOUR);
            draw_int(47, 12, lives, FG_COLOUR);
            draw_string(17, 20, "Score:", FG_COLOUR);
            draw_int(47, 20, score, FG_COLOUR);
            draw_time();
            draw_string(17, 36, "Zombies:", FG_COLOUR);
            draw_int(56, 36, zombie_ctr, FG_COLOUR);
            draw_string(17, 4, "Food:", FG_COLOUR);
            draw_int(56, 4, food_ctr, FG_COLOUR);
			game_pause_usb();
        }
        else {
            game_pause = false;
        }
    }
    prevState = js_pressed;
}

void treasure_move( void ){
	static uint8_t prevState = 0;
	int c = usb_serial_getchar();
	if ((sw3_pressed && !prevState) || c == 't'){
		treasure_stop = !treasure_stop;
	}
	if (treasure.x == 0 || treasure.x == 79){
		treasure.dx *= -1;
	}
	prevState = sw3_pressed;
}

void player_reset( void ){
    //for (int i = 0; i < 4; i++){
        //if (blocks[i].x > 5 && blocks[i].x < 80){
            sprite_init(&player, 10, 1, 5, 5, player_bitmap);
			player_respawn_usb();
			current_block = 100;
        //    return;
        //}
    //}
}

void led_setup( void ){
	SET_BIT(DDRB,2);// set led0 for output
	SET_BIT(DDRB,3);// set led1 for output
}

// function from lecture notes
#define BIT(x) (1 << (x))
#define OVERFLOW_TOP (1023)
#define ADC_MAX (1023)
void pwm_setup(void){
	TC4H = OVERFLOW_TOP >> 8;
	OCR4C = OVERFLOW_TOP & 0xff;
	TCCR4A = BIT(COM4A1) | BIT(PWM4A);
	SET_BIT(DDRC, 7);
	TCCR4B = BIT(CS42)| BIT(CS41) | BIT(CS40);
	TCCR4D = 0;
}

// function from lecture notes
void set_duty_cycle(int duty_cycle) {
	// (a)	Set bits 8 and 9 of Output Compare Register 4A.
	TC4H = duty_cycle >> 8;
	// (b)	Set bits 0..7 of Output Compare Register 4A.
	OCR4A = duty_cycle & 0xff;
}

// function from lecture notes
void back_light(void){
	set_duty_cycle(ADC_MAX);
}

void light_fade(void){
	for (int i = 0; i < 20; i++){
		set_duty_cycle(ADC_MAX - 60*i);
		LCD_CMD(lcd_set_function, lcd_instr_extended);
		LCD_CMD( lcd_set_contrast, LCD_DEFAULT_CONTRAST-2*i );
		_delay_ms(100);
	}
	for (int j = 0; j < 20; j++){
		set_duty_cycle(60*j);
		LCD_CMD(lcd_set_function, lcd_instr_extended);
		LCD_CMD( lcd_set_contrast, 4*j);
		_delay_ms(100);
	}
	lcd_init(LCD_DEFAULT_CONTRAST);
	// LCD_CMD(lcd_set_function, lcd_instr_extended);
	// LCD_CMD( lcd_set_contrast, LCD_DEFAULT_CONTRAST );
}

void setup( void ) {
	set_clock_speed(CPU_8MHz);
	pwm_setup();
	adc_init();
	// enable input from toggle and switches
	CLEAR_BIT(DDRB, 7);//down
	CLEAR_BIT(DDRD, 1);//up
	CLEAR_BIT(DDRB, 1);//left
	CLEAR_BIT(DDRD, 0);//right
	CLEAR_BIT(DDRF, 5);//SW3
	CLEAR_BIT(DDRF, 6);//SW2
	CLEAR_BIT(DDRB, 0);//centre

	//	(b) Initialise the LCD display using the default contrast setting.
	lcd_init(LCD_DEFAULT_CONTRAST);
    sprite_init(&treasure, 3, 43, 5, 4, treasure_bitmap);
	treasure.dx = 1;
    // allow display to appear before game begins
	game_start = false;
    //timer_setup();
    food_setup();
}

// function from lecture notes
bool sprites_collide ( Sprite *s1, Sprite *s2 ){
    int l1 = round(s1->x);
    int l2 = round(s2->x);
    int t1 = round(s1->y);
    int t2 = round(s2->y);
    int r1 = l1 + s1->width - 1;
    int r2 = l2 + s2->width -1;
    int b1 = t1 + s1->height + 0.2;
    int b2 = t2 + s2->height - 1;
    if ( l1 > r2 ) return false;
    if ( l2 > r1 ) return false;
    if ( t1 > b2 ) return false;
    if ( t2 > b1 ) return false;
    return true;
}

void start_block_collide(void){
	if (sprites_collide(&player, &start_block)){
		player.dy = 0;
	}
}

void block_collide( void ){
    player.dy += .4;
	start_block_collide();
    for (int i = 0; i < total_blocks; i++){
        if (sprites_collide(&player, &blocks[i])){
            if (blocks[i].bitmap == block_bitmap){
                player.dy = 0;
                if (current_block != i){
                    player.dx = blocks[i].dx;
                    score++;
                    current_block = i;
                }
            }
            else if (blocks[i].bitmap == bad_block_bitmap){
                player_reset();
                lives--;
				if (lives > 0){light_fade();}
				player_death_usb();
				usb_serial_send("\n Death by Forbidden Block");
            }return;
        }
    }
}

void player_death( void ){
    if (round(player.x) < -1 || round(player.x)>LCD_X-3 || round(player.y) > LCD_Y-5
            || round(player.y) < -3){
            lives--;
			if (lives > 0){light_fade();}
			player_death_usb();
			usb_serial_send("\n Death by moving off screen");
            player_reset();
    }
}

void treasure_contact(){
	// occurs when player touches the treasure
	if (sprites_collide(&player, &treasure)){
		lives += 2;
	    treasure.is_visible = false;
		treasure.y = 100;
        player_reset();
		treasure_col_usb();

    }
}

bool sprite_step( sprite_id sprite){
    int x0 = round( sprite->x);
    int y0 = round( sprite->y);
    sprite->x += sprite->dx;
    sprite->y += sprite->dy;
    int x1 = round(sprite->x);
    int y1 = round(sprite->y);
    return (x1 != x0) || (y1 != y0);
}

void zombie_collide( void ){
	for (int i = 0; i < total_blocks; i++){
		for (int j = 0; j < total_zombies; j++){
			if (sprites_collide(&zombies[j], &blocks[i]) && blocks[i].bitmap == block_bitmap){
				//zombies[j].y -= zombies[j].dy;
				// zombies[j].x += zombies[j].dx;
				zombies[j].dy = 0;
				zombies[j].dx = blocks[i].dx;
				//zombies[j].x += blocks[i].dx;
				//zombies[j].x += zombies[j].dx
			}
			else if (!sprites_collide(&zombies[j], &blocks[i])){
				//zombies[j].y -= zombies[j].dy;
				// zombies[j].x -= zombies[j].dx;
				// zombies[j].dx = -zombies[j].dx;
			}
		}
	}
}

void zombie_move ( void ){
    for (int i = 0; i < total_zombies; i++){
        sprite_step(&zombies[i]);
        if (zombies[i].x > 90 || zombies[i].x < -5 || zombies[i].y > 50){
            if(zombies[i].is_visible == true){zombie_ctr--;}
            zombies[i].is_visible = false;
            zombies[i].x = LCD_X;
            zombies[i].y = LCD_Y;
        }
        if (sprites_collide(&zombies[i], &player)){
            player_reset();
            lives--;
			if(lives > 0 ){light_fade();}
			player_death_usb();
			usb_serial_send("\n Death by Zombie");
        }
    }
    if (zombie_ctr <= 0){
		zombie_timer= true;
    }
}

void block_movement(void){
    for (int i = 0; i < total_blocks; i++){
        if ((int)round(blocks[i].y)%2 == 1){
           blocks[i].dx = block_speed;
        }
        else {
           blocks[i].dx = -block_speed;
        }
        sprite_step(&blocks[i]);
        if (blocks[i].x < -10){
            blocks[i].x = LCD_X-1;
        }
        if (blocks[i].x > LCD_X-1){
            blocks[i].x = -10;
        }
    }
}

void player_move ( void ){
	int c = usb_serial_getchar();
	static uint8_t prevState1 = 0;
	static uint8_t prevState2 = 0;
	static uint8_t prevState3 = 0;
		// move left
	if (player.dy == 0){
        if (((left_pressed && !prevState1)  || c == 'a') && player.dx > 1.8*-block_speed){
		    player.dx -= block_speed;
	     }
	    // move right
	    if (((right_pressed && !prevState2) || c == 'd') && player.dx < 1.8*block_speed){
		    player.dx += block_speed;
	     }
	    // move up
	    if ((up_pressed && !prevState3) || c == 'w'){
		    player.dy = -3.3;
	     }
		 prevState1 = left_pressed;
		 prevState2 = right_pressed;
		 prevState3 = up_pressed;
    }
    sprite_step(&player);
}

void food_deploy ( void ){
	int c = usb_serial_getchar();
    if (player.dy == 0){
		static uint8_t prevState = 0;
        if ( ((down_pressed && !prevState) || c == 's') && food_ctr > 0){
			for (int i = 0; i < total_blocks; i++){
				if(food[i].is_visible == false){
					food[i].is_visible = true;
		            food[i].x = player.x;
		            food[i].y = player.y+1;
		            if (food[i].y  <10){food[i].dx = 0;}
                    else if (food[i].y > 15 && food[i].y < 25){food[i].dx = block_speed;}
                    else {food[i].dx = -block_speed;}
                    food_ctr--;
					return;
                }}		}
		prevState = down_pressed;
	}
    for (int j = 0; j < total_food; j++){
        sprite_step(&food[j]);
    }
}

void food_collision (void){
    for (int i = 0; i < total_zombies; i++){
        for (int j = 0; j < total_food; j++){
            if (sprites_collide(&zombies[i], &food[j])){
                zombies[i].is_visible = false;
                zombies[i].x = 89;
				zombie_ctr--;
				total_zombies_fed++;
                food[j].is_visible = false;
				food_ctr++;
                food[j].x = 100;
                score += 10;
				zom_col_food_usb();
				return;
            }
        }
    }
}

void start_screen( void ){
    //draw details
    draw_string(17,12, "Oliver Pye", FG_COLOUR);
    draw_string(17,20, "n9703977", FG_COLOUR);
    // start game when switch 2 is pressed
	int c = usb_serial_getchar();
	static uint8_t prevState = 0;
    if ((sw2_pressed && !prevState) || c == 's'){
        game_start = true;
		game_start_usb();
        block_setup();
        player_reset();
        zombie_setup();
        zombie_timer = true;
    }
	prevState = sw2_pressed;
}

void clear_zombies(void){
    for (int i = 0; i < 5; i++){
        zombies[i].is_visible = false;
        zombies[i].x = 89;
    }
}

void draw_all(void){
    clear_screen();
    sprite_draw(&player);
    sprite_draw(&treasure);
	sprite_draw(&start_block);
    for (int i = 0; i < total_blocks; i++){
        sprite_draw(&blocks[i]);
    }
    for (int j = 0; j < total_zombies; j++){
        sprite_draw(&zombies[j]);
    }
    for (int k = 0; k < total_food; k++){
        sprite_draw(&food[k]);
    }
}

void adc_speed(void){
	int right_adc = adc_read(0);
	block_speed = (double)right_adc/512;
}

void reset_game( void ){
    lives = 10;
    score = 0;
	food_ctr = 5;
    player_reset();
    game_over = false;
    overflow_counter = 0;
    timer_setup();
    clear_zombies();
	treasure.x = 3;
	treasure.y = 43;
	treasure.is_visible = true;
    zombie_timer = true;
	food_setup();
}

void quit_decision( void ){
	static uint8_t prevState1 = 0;
	static uint8_t prevState2 = 0;
	int c = usb_serial_getchar();
    if ((sw3_pressed && !prevState1) || c == 'r'){
        reset_game();
    }
    if ((sw2_pressed && !prevState2) || c == 'q'){
        while (1){
            clear_screen();
            draw_string(17,20, "n9703977", FG_COLOUR);
            show_screen();
        }
    }
	prevState1 = sw3_pressed;
	prevState2 = sw2_pressed;
}

uint8_t cross[8] = {
	0b10000001,
	0b01000010,
	0b00100100,
	0b00011000,
	0b00011000,
	0b00100100,
	0b01000010,
	0b10000001,
};
uint8_t cross_direct[8];
uint8_t x,y;
//function used from lecture notes
void cross_setup( void ){
	//use lcd write to animate end of game
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
			uint8_t bit_val = BIT_VALUE(cross[j], (7-i));
			WRITE_BIT(cross_direct[i], j, bit_val);
		}
	}
	x = (LCD_X-5)/2;
	y = (LCD_Y-5)/2;
	clear_screen();
}

void lcd_clear_screen(void){
	LCD_CMD(lcd_set_function, lcd_instr_basic | lcd_addr_horizontal);
	for (int i = 0; i < LCD_X; i++){
		for (int j = 0; j < LCD_Y; j++){
			LCD_CMD(lcd_set_x_addr, i);
			LCD_CMD(lcd_set_y_addr, j/8);
			for (int k = 0; k < 8; k++) {
				LCD_DATA(0);
			}
		}
	}

}
//function used from lecture notes
void end_animation(void){
	LCD_CMD(lcd_set_function, lcd_instr_basic | lcd_addr_horizontal);
	LCD_CMD(lcd_set_x_addr, x);
	LCD_CMD(lcd_set_y_addr, y/8);

	for (int i = 0; i < 8; i++){
		LCD_DATA(cross_direct[i]);
		_delay_ms(400);
	}
}

void end_game( void ){
    if (game_over == true){
        clear_screen();
        draw_string(17, 4, "GAME OVER!", FG_COLOUR);
        draw_string(17, 12, "Lives:", FG_COLOUR);
        draw_int(47, 12, lives, FG_COLOUR);
        draw_string(17, 20, "Score:", FG_COLOUR);
        draw_int(47, 20, score, FG_COLOUR);
        draw_time();
        show_screen();
        quit_decision();
    }
}

void game_death(void){
	if (lives < 1){
		lives = 0;
		game_over = true;
		cross_setup();
		lcd_clear_screen();
		end_animation();
		game_over_usb();
	}
}

void process( void ) {
	SET_BIT(PINC,7);
    pause_game();
    end_game();
	back_light();
    if (game_pause == false && game_over == false){
        player_move();
        treasure_move();
        treasure_contact();
        block_collide();
        zombie_move();
        zombie_collide();
        block_movement();
        player_death();
		zombie_lights();
        if (treasure_stop == false){
            sprite_step(&treasure);
        }
        food_deploy();
        food_collision();
		adc_speed();
        game_death();
		draw_all();
    }
}


int main(void) {
	timer_setup();
	setup_usb_serial();
    while ( !game_over) {
		setup();
		start_screen();
        while ( game_start ){
			process();
			show_screen();
			_delay_ms(100);
		}
		show_screen();
		_delay_ms(100);
	}
	return 0;
}
