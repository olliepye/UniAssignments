/*
CAB202 Assignment 1

Aurthor: Oliver Pye n9703977
         o.pye@connect.qut.edu.au
         Queensland University of Technology

August 2018

The following code produces a game where a player has to navigate through a
series of obsticals while gaining points and retaining lives.

*/


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

// Configuration
#define DELAY (10) /* Millisecond delay between game updates */
#define MAXSAFEBLOCKS (160)
#define MAXFORBIDDENBLOCKS (40)
#define ROWS (4)

// Game state.
bool game_over = false; /* Set this to true when game is over */
bool update_screen = true; /* Set to false to prevent screen update. */

char * player_image =
/**/	" O "
/**/	"/|\\"
/**/	"/ \\";

char * player_fall_im =
/**/	"\\O/"
/**/	" | "
/**/	"/ \\";

char * player_left_im =
/**/	" O "
/**/	" | "
/**/	"\\\\\\";

char * player_right_im =
/**/	" O "
/**/	" | "
/**/	"///";

char * treasure_image =
/**/	"$$"
/**/	"$$";

char * treasure_image_2 =
/**/	"##"
/**/	"##";

char * student_id_image =
/**/	"n9703977";

char * block_image_5 =
/**/	"====="
/**/	"=====";

char * forbidden_block_image_5 =
/**/	"xxxxx"
/**/	"xxxxx";

char * block_image_6 =
/**/	"======"
/**/	"======";

char * forbidden_block_image_6 =
/**/	"xxxxxx"
/**/	"xxxxxx";

char * block_image_7 =
/**/	"======="
/**/	"=======";

char * forbidden_block_image_7 =
/**/	"xxxxxxx"
/**/	"xxxxxxx";
char * block_image_8 =
/**/	"========"
/**/	"========";

char * forbidden_block_image_8 =
/**/	"xxxxxxxx"
/**/	"xxxxxxxx";
char * block_image_9 =
/**/	"========="
/**/	"=========";

char * forbidden_block_image_9 =
/**/	"xxxxxxxxx"
/**/	"xxxxxxxxx";
char * block_image_10 =
/**/	"=========="
/**/	"==========";

char * forbidden_block_image_10 =
/**/	"xxxxxxxxxx"
/**/	"xxxxxxxxxx";


char * lives_text_image = "Lives: ";
char * time_text_image = "Time: ";
char * score_text_image = "Score: ";


// declare the sprite_id for the sprites in this game
sprite_id player;
sprite_id treasure;
sprite_id student_id;
sprite_id blocks[MAXSAFEBLOCKS] = {0};
sprite_id forbidden_blocks[MAXFORBIDDENBLOCKS];
int total_blocks;

// declare lives, score and time
int lives = 10;
int score = 0;
int minutes;
int seconds;
double start_time;
int y_max;
int y_min;
int current_block;
bool treasure_stop = false;
bool player_moving_block = false;
int block_width = 8;
int block_width_space = 2;


void sprite_select(int x, int y, int size, bool type, sprite_id s){
	if (size == 5 && type == true){
		sprite_init(s, x,y,size,2, block_image_5);
	}
	else if (size == 6 && type == true){
		sprite_init(s,x,y,size,2,block_image_6);
	}
	else if (size == 7 && type == true){
		sprite_init(s,x,y,size,2,block_image_7);
	}
	else if (size == 8 && type == true){
		sprite_init(s,x,y,size,2,block_image_8);
	}
	else if (size == 9 && type == true){
		sprite_init(s,x,y,size,2,block_image_9);
	}
	else if (size == 10 && type == true){
		sprite_init(s,x,y,size,2,block_image_10);
	}
	else if (size == 5 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_5);
	}
	else if (size == 6 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_6);
	}
	else if (size == 7 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_7);
	}
	else if (size == 8 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_8);
	}
	else if (size == 9 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_9);
	}
	else if (size == 10 && type == false){
		sprite_init(s,x,y,size,2,forbidden_block_image_10);
	}
}


void initialise_blocks(){
	int num_cols = (screen_width()-1)/(block_width + block_width_space);
	int y[ROWS] = {0};
	int x[100] = {0}; 
	y_max = 12 + (ROWS-1)*screen_height()/5;
	y_min = 12;
	for (int i = 0; i < ROWS; i++){
		y[i] = 12 + i*screen_height()/5;
	}
	for (int i = 0; i < num_cols; i++){
		x[i] = 4 + i*(block_width + block_width_space);
	}

	total_blocks = 0;
	for (int i = 0; i < ROWS; i++){
		for (int j = 0; j < num_cols; j++){
			double u = (double)rand() / (double)RAND_MAX;
			int block_length = rand()%6+5;
			if ( u < .8){
				blocks[total_blocks]=sprite_create(1,1,1,1,"");
				sprite_select(x[j],y[i],block_length, true, blocks[total_blocks]);
				blocks[total_blocks]->cookie = (void *)false;
				total_blocks++;
			}
			else if ( u < .9 ){
				blocks[total_blocks]=sprite_create(1,1,1,1,"");
				sprite_select(x[j],y[i],block_length, false, blocks[total_blocks]);
				blocks[total_blocks]->cookie = (void *)true;
				total_blocks++;
			}
		}
	}
}

void draw_blocks(){
	for (int i = 0; i < total_blocks ; i++){
			sprite_draw(blocks[i]);
	}
}



void draw_display(){
	draw_line(0,0,screen_width()-1,0,'*');
	draw_line(0,0,0,4,'*');
	draw_line(0,4, screen_width()-1, 4, '*');
	draw_line(screen_width()-1, 0, screen_width()-1, 4, '*');


	student_id = sprite_create(screen_width() - 10, 2, 8,1, student_id_image);
	sprite_draw(student_id);

	// Draw the Lives, timer and score titles
	draw_string(3, 2, lives_text_image);
	draw_string(15, 2, time_text_image);
	draw_string(37, 2, score_text_image);

	draw_int(10,2,lives);
	draw_int(44,2,score);
	
	minutes = (int)(get_current_time()-start_time)/60;
	seconds = (int)(get_current_time()-start_time)%60;
	draw_formatted(22, 2, "%02d:%02d", minutes, seconds);

}




void blocks_move(){
	for (int i = 0; i < total_blocks; i++){
		if (blocks[i]->y > y_min && blocks[i]->y < (y_max)){
		       if (blocks[i]->y == 12 + 1*screen_height()/5){
				blocks[i]->dx = -0.15;
		       }
		       else blocks[i]->dx = 0.08;
		}
 		sprite_step(blocks[i]);
		if (blocks[i]->x < -block_width){
			blocks[i]->x = screen_width()-1;
		}
		if (blocks[i]->x > screen_width()-1){
			blocks[i]->x = -block_width;
		}	
	}
}

void treasure_move(int key){
	int tx = round(sprite_x(treasure));
	if (key == 't'){
		treasure_stop = !treasure_stop;
	}
	if (tx == 0 || tx == screen_width() - 1){
		treasure->dx *= -1;
	}
}




// Returns true if two sprites collide.
bool sprites_collide ( sprite_id s1, sprite_id s2 ){
	int l1 = round(sprite_x(s1))-1;
	int l2 = round(sprite_x(s2))-1;
	int t1 = round(sprite_y(s1))+2;
	int t2 = round(sprite_y(s2))+1;
	int r1 = l1 + sprite_width(s1)-1;
	int r2 = l2 + sprite_width(s2)-1;
	int b1 = t1 + sprite_height(s1)-1;
	int b2 = t2 + sprite_height(s2)-1;

	if ( l1 > r2 ) return false;
	if ( l2 > r1 ) return false;
	if ( t1 > b2 ) return false;
	if ( t2 > b1 ) return false;

	return true;
}


void draw_all(void){
	clear_screen();
	sprite_draw(player);
	sprite_draw(treasure);
	draw_blocks();

	draw_display();	
}



void player_jump(){
	double speed[7] = {3, 2, 1, 0, -1, -2, -3};
	for (int i = 0; i < 7; i++){
		player->dy = speed[i];
		sprite_step(player);
	}
}

void player_animation(){
	if (player->dy == 0){
		player->bitmap = player_image;
	}
	else if (player->dy != 0){
		player->bitmap = player_fall_im;
	}
	if (player->dx < 0){
		player->bitmap = player_left_im;
	}
	else if (player->dx > 0){
		player->bitmap = player_right_im;
	}
}


void player_horz_movement(bool direction){
	if (direction == true ){
		player->dx += 0.05;
	}
	else if (direction == false ){
		player->dx -= 0.05;
	}
	
}


void player_move ( int key ){
	// (g)	Get the current screen coordinates of the hero in integer variables
	//		by rounding the actual coordinates.
	int px = round(sprite_x(player));
	int py = round(sprite_y(player));

	if ( key == 'a' && px > 1 ){
		//sprite_move(player,-1,0);
		player_horz_movement(false);
	}
	else if (key == 'd' && px < screen_width()-1-sprite_width(player)){
		//sprite_move(player,1,0);
		player_horz_movement(true);
	}
	else if (key == 'w' && py > 5){
		if (player->dy == 0){
			player_jump();
		}
	}
	if (player->dy == 0.1){
		player->dx = 0;
	}
	if (player->dy != 0){
		player_moving_block = false;
	}
	sprite_step(player);
}


void treasure_animate(int key){
	int time = (int)get_current_time()%60;
	if (time%2 == 0){
		treasure->bitmap = treasure_image_2;
	}
	else if (time%2 == 1){
		treasure->bitmap = treasure_image;
	}
	treasure_move(key);
	if (treasure_stop == false){
		sprite_step(treasure);
	}
}



void player_reset(){
	initialise_blocks();
	for (int i = 0; i < 10000; i++){
		for (int j = 0; j < total_blocks; j++){
			int u = rand()%(screen_width()-1);
			if (blocks[j]->x == u && blocks[j]->cookie == (void *)false
					&& blocks[j]->y == 12){
				player->x = u;
				player->y = 5;
				player->dx = 0;
				treasure->x = 6;
				current_block = 0;
				return;
			}
		}
	}
}


void setup(void){
	player = sprite_create(screen_width()-5,5,3,3,player_image);
	treasure = sprite_create(6, screen_height()-3, 2,2, treasure_image);
	// give the treasure sprite a speed
	treasure->dx =.1;
	// initialise player vertical speed for "Gravity"
	player->dy = .1;
	initialise_blocks();
	current_block = 0;
        start_time = get_current_time();
	srand((int)start_time);
	player_reset();
	draw_all();
}



void treasure_contact(){
	// occurs when player touches the treasure
	if (sprites_collide(treasure, player) == true){
		lives += 2;
		player_reset();
	}
}


void player_death(){
	// lose life if player falls to bottom of screen
	if (round(sprite_y(player)) == screen_height()-1 
			|| sprite_x(player) < 0 
			|| sprite_x(player) > screen_width()-1){
		lives--; 
		treasure_stop = false;
		player_reset();
	}
}


void block_collide(){     
for (int i = 0; i < total_blocks; i++){
    	if (sprites_collide(player, blocks[i])){
		    if (blocks[i]->dx != 0 && player->dx == 0 &&
				    player_moving_block == false){
			    player->dx = blocks[i]->dx;
			    player_moving_block = true;
		    }
		    player->dy = 0;
		    if (current_block != i && blocks[i]->cookie == (void *)false){
			    score++;
			    current_block = i;
		    }
		    if (blocks[i]->cookie == (void *)true){
		    	lives--; 
		    	player_reset();
		    } 
		    return;     
		}         
		else {
			player->dy = .1;
    	}	
    }
}


void reset_game(){
	lives = 10;
	score = 0;
	start_time = get_current_time();
	player_reset();
	game_over = false;
}


void quit_decison(){
	char key = wait_char();

	while(key != 'r' && key != 'q'){
		key = wait_char();
	}

	if (key == 'q'){
		return;
	}
	else if (key == 'r'){
		reset_game();
	}
}



void end_game(){
	game_over = true;
	clear_screen();
	draw_formatted(screen_width()/2-3, screen_height()/2, "GAME OVER!");
	draw_formatted(screen_width()/2-3, screen_height()/2+1, "Score: %d", score);
	draw_formatted(screen_width()/2-3, screen_height()/2+2, "Time: %02d:%02d", minutes, seconds);
	draw_formatted(screen_width()/2-14, screen_height()/2+4, "Press 'r' to restart or 'q' to quit");
		
	show_screen();
	quit_decison();
}

void process(void){
	int keyCode = get_char();

	player_move(keyCode);
	player_animation();
	blocks_move();
	block_collide();
	//treasure_move(keyCode);
	treasure_animate(keyCode);
	treasure_contact();
	player_death();
	
	// end game if lives get to 0
	if (lives == 0){
		end_game();
	}

	if (keyCode == 'q'){
		end_game();
	}
	draw_all();
}



int main(void){
	setup_screen();
	setup();
	show_screen();

	while( !game_over ){
		while ( !game_over ) {
			process();
			if ( update_screen ) {
				show_screen();
			}
			timer_pause(DELAY);
		}
	}
	return 0;
}
