#include <TFT.h>
#include <SPI.h>
#include <EEPROM.h>

// TFT pins
#define CS    10
#define DC    9
#define RESET 8

// global variable for the screen
TFT screen = TFT(CS, DC, RESET);

// EEPROM STUFF
#define MAGIC_ADDR 0        
#define HIGHSCORE_ADDR 1    
#define MAGIC_VALUE 0x42   

// screens parameter
#define SCREEN_W 128
#define SCREEN_H 160
#define GAME_W 128
#define GAME_H 128

// used for both snake sizing and food sizing
#define CELL_SIZE 8
#define MAX_X (GAME_W / CELL_SIZE - 1)
#define MAX_Y (GAME_H / CELL_SIZE - 1)

#define JS_BTN_PIN 2
#define JS_X_PIN A0
#define JS_Y_PIN A1

#define EXTRA_BTN_PIN 3

#define MAX_X (GAME_W / CELL_SIZE)
#define MAX_Y (GAME_H / CELL_SIZE)

#define MAX_SNAKE_SIZE (GAME_W * GAME_H / (CELL_SIZE * CELL_SIZE))
#define STARTING_SNAKE_SIZE 3

#define BUTTON_DEBOUNCE_VALUE 100

// main colours
#define FOOD_COLOR screen.Color565(212, 175, 55)
#define SNAKE_COLOR screen.Color565(77, 153, 77)
#define BG_COLOR screen.Color565(0, 0, 0)
#define SCORE_COLOR screen.Color565(255, 255, 255)
#define GAMEOVER_COLOR screen.Color565(255, 0, 0)

// light mode
#define FOOD_COLOR_LM screen.Color565(0, 0, 0)
#define SNAKE_COLOR_LM screen.Color565(0, 0, 0)
#define BG_COLOR_LM screen.Color565(255, 255, 255)
#define SCORE_COLOR_LM screen.Color565(0, 0, 0)
#define GAMEOVER_COLOR_LM screen.Color565(0, 0, 0)

// variables for global colours
uint16_t food_color = FOOD_COLOR;
uint16_t snake_color = SNAKE_COLOR;
uint16_t background_color = BG_COLOR;
uint16_t score_color = SCORE_COLOR;
uint16_t gameover_color = GAMEOVER_COLOR;

// lower and higher threshold values for joystick
#define L_T_VALUE 300
#define H_T_VALUE 700

#define HEADER_HEIGHT 32
#define GAME_OFFSET_Y HEADER_HEIGHT
#define HEADER_COLOR screen.Color565(100, 100, 100)

#define HEADER_TEXT "SNAKEUINO"
#define GAMEOVER_TEXT "GAME OVER"

// global variables for interrupts
volatile bool button_pressed_flag = false;
volatile uint32_t last_interrupt_time = 0;
volatile bool tick = false;
volatile bool color_button_pressed_flag = false;

#define STATE_RUN 0
#define STATE_PAUSED 1
#define STATE_GAMEOVER 2
volatile uint8_t state = 0;

uint16_t highscore = 0;

// direction of the snake movement optimize later to use a single vvariable
int8_t dir_x = 0;
int8_t dir_y = 0;


// food coordinates x, y
uint8_t food_cords[2];
uint8_t food_cords_old[2];
bool food_consumed = false;

// used for printing score upon startup and color change
bool score_condition = true;

// store previous score to detect changes
uint8_t prev_score = 0;

// saves the current snake size
uint8_t snake_size = STARTING_SNAKE_SIZE;

// saves the coordinates of the snake cells
// [0], [1] = x and y of the first cell and etc
uint8_t snake[MAX_SNAKE_SIZE * 2] = {2, 0, 1, 0, 0, 0};

// save the tail to undraw it
uint8_t tail[2] = {snake[snake_size * 2 - 1], snake[snake_size * 2 - 2]};

void setup() {
  Serial.begin(9600);

  highscore = read_highscore();

  // initialize the screen
  screen.begin();
  screen.background(0,0,0); 
  screen.setRotation(2);

  // pin for button press
  pinMode(JS_BTN_PIN, INPUT_PULLUP);
  pinMode(EXTRA_BTN_PIN, INPUT_PULLUP);

  // add interrupt for button pressed
  attachInterrupt(digitalPinToInterrupt(JS_BTN_PIN), button_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(EXTRA_BTN_PIN), color_button_pressed, FALLING);

  // set a random seed for the food
  randomSeed(analogRead(A0));

  // set the food cords
  food_cords[0] = random(MAX_X);
  food_cords[1] = random(MAX_Y);

  // https://deepbluembedded.com/arduino-timer-interrupts/
  noInterrupts();           // disable interrupts during setup
  TCCR1A = 0;               // clear control register A
  TCCR1B = 0;               // clear control register B
  TCNT1  = 0;               // reset counter

  // (16 MHz) / (prescaler 64) = 250,000 ticks per second
  // 0.07 seconds * 250000 = 17500 ticks
  OCR1A = 17500;            // compare match register for 70ms

  TCCR1B |= (1 << WGM12);   // CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << CS11) | (1 << CS10);  // prescaler 64
  TIMSK1 |= (1 << OCIE1A);  // enable Timer1 compare interrupt
  interrupts();             // enable interrupts

}

void loop() {
  switch(state) {
    case STATE_RUN:
      handle_input();
      if(tick) {
        update_snake();
        render();
        tick = false;
      }      
      break;
      case STATE_GAMEOVER:
        render_game_over();
        break;
    default:
      // if the game is paused do nothing
      break;   
  }

  if(button_pressed_flag) {
    if(state == STATE_GAMEOVER) {
      reset();
      state = STATE_RUN;
    }
    else if(state == STATE_RUN) {
      state = STATE_PAUSED;
    }
    else {
      state = STATE_RUN;
    }
    button_pressed_flag = false;
  }

  if(color_button_pressed_flag) {
    if(food_color == FOOD_COLOR) {
      food_color = FOOD_COLOR_LM;
      snake_color = SNAKE_COLOR_LM;
      background_color = BG_COLOR_LM;
      score_color = SCORE_COLOR_LM;
      gameover_color = GAMEOVER_COLOR_LM;
    }
    else {
      food_color = FOOD_COLOR;
      snake_color = SNAKE_COLOR;
      background_color = BG_COLOR;
      score_color = SCORE_COLOR;
      gameover_color = GAMEOVER_COLOR;
    }
    score_condition = true;
    screen.fillScreen(background_color);
    color_button_pressed_flag = false;
  }

}

// button interrupt function
void button_pressed() {
  uint32_t current_time = millis();
  if(current_time - last_interrupt_time > BUTTON_DEBOUNCE_VALUE) {
    button_pressed_flag = true;
  }
  
  last_interrupt_time = current_time;
}

void color_button_pressed() {
  uint32_t current_time = millis();
  if(current_time - last_interrupt_time > BUTTON_DEBOUNCE_VALUE) {
    color_button_pressed_flag = true;
  }
  
  last_interrupt_time = current_time; 
}

void handle_input() {
  int x = analogRead(JS_X_PIN); // 0-1023
  int y = analogRead(JS_Y_PIN); // 0-1023

  if (x < L_T_VALUE && dir_x != 1 && y > L_T_VALUE && y < H_T_VALUE) {          // left
    dir_x = -1;
    dir_y = 0;
  } 
  else if (x > H_T_VALUE && dir_x != -1 && y > L_T_VALUE && y < H_T_VALUE) {     // right
    dir_x = 1;
    dir_y = 0;
  } 
  else if (y < L_T_VALUE && dir_y != 1 && x > L_T_VALUE && x < H_T_VALUE) {     // up
    dir_x = 0;
    dir_y = -1;
  } 
  else if (y > H_T_VALUE && dir_y != -1 && x > L_T_VALUE && x < H_T_VALUE) {     // down
    dir_x = 0;
    dir_y = 1;
  }
}

// generates new coordinates for food
void eat_food() {
  food_cords_old[0] = food_cords[0];
  food_cords_old[1] = food_cords[1];

  food_consumed = true;

  food_cords[0] = random(MAX_X);
  food_cords[1] = random(MAX_Y);
}

void reset() {
  if(snake_size - STARTING_SNAKE_SIZE > highscore) {
    highscore = snake_size - STARTING_SNAKE_SIZE;
    write_highscore();
  }

  state = STATE_RUN;
  screen.background(background_color); 

  food_cords[0] = random(MAX_X);
  food_cords[1] = random(MAX_Y);

  // reset the directions
  dir_x = 0;
  dir_y = 0;

  // reset the snake
  snake_size = STARTING_SNAKE_SIZE;

  snake[0] = 2;
  snake[1] = 0;
  snake[2] = 1;
  snake[3] = 0;
  snake[4] = 0;
  snake[5] = 0;
} 

void draw_cell(uint8_t x, uint8_t y, uint16_t color) {
  screen.fillRect(x * CELL_SIZE, y * CELL_SIZE + GAME_OFFSET_Y, CELL_SIZE, CELL_SIZE, color);
}

void undraw_cell(uint8_t x, uint8_t y) {
  screen.fillRect(x * CELL_SIZE, y * CELL_SIZE + GAME_OFFSET_Y, CELL_SIZE, CELL_SIZE, background_color);
} 

void update_snake() {
  if(dir_x == 0 && dir_y == 0) {
    return;
  }

  // calculate new head position
  int16_t new_x = snake[0] + dir_x;
  int16_t new_y = snake[1] + dir_y;

  // check for out of bounds
  if(new_x >= MAX_X) {
    new_x = 0;
  }
  else if(new_x < 0) {
    new_x = MAX_X - 1;
  }

  if(new_y >= MAX_Y) {
    new_y = 0;
  }
  else if(new_y < 0) {
    new_y = MAX_Y - 1;
  }

  if(new_y >= MAX_Y && new_y < 0xf0) {
    new_y = 0;    
  }

  else if(new_y > 0xf0) {
    new_y = MAX_Y;
  }

  // check if food was consumed
  if(new_x == food_cords[0] && new_y == food_cords[1]) {
    eat_food();
    ++snake_size;
  }

  // save the old tail to unrender it
  tail[0] = snake[(snake_size - 1) * 2];
  tail[1] = snake[(snake_size - 1) * 2 + 1];

  // move body (from tail to head)
  for (int i = snake_size - 1; i > 0; i--) {
    if(new_x == snake[i * 2] && new_y == snake[i * 2 + 1]) {
      state = STATE_GAMEOVER;
      break;
    }

    snake[i * 2] = snake[(i - 1) * 2];
    snake[i * 2 + 1] = snake[(i - 1) * 2 + 1];
  }

  // update head
  snake[0] = new_x;
  snake[1] = new_y;
}


void render() {
  // draw the header
  draw_header();

  // draw the food
  render_food();

  // render the snake
  render_snake();
}

void render_food() {
  // if food was consumed remove the old food
  if(food_consumed) {
    undraw_cell(food_cords_old[0], food_cords_old[1]);
    food_consumed = false;
  }

  draw_cell(food_cords[0], food_cords[1], food_color);
}

void render_snake() {
  // save the last cell and unrender it
  if(dir_x != 0 || dir_y != 0) {
    undraw_cell(tail[0], tail[1]);
  }

  // render only the head
  draw_cell(snake[0], snake[1], snake_color);
/*
  for(uint16_t i = 0, j = 0; i < snake_size; ++i, j += 2) {
    draw_cell(snake[j], snake[j + 1], snake_color);
  }
  */
}

void draw_header() { 
    // dont go here idk whats going on but it works (I hate doing calculating pixels)
    screen.stroke(snake_color); 
    screen.setTextSize(1); screen.text(HEADER_TEXT, 10, 5);  
    screen.fillRect(0, HEADER_HEIGHT - 2, SCREEN_W, 2, score_color);

    // Only update score if it changed or score condition was met (like starting the game or changing colors)
    if(snake_size - STARTING_SNAKE_SIZE != prev_score || score_condition) {
        char scoreText[16];
        sprintf(scoreText, "S:%d H:%d", snake_size - STARTING_SNAKE_SIZE, highscore);

        int textWidth = strlen(scoreText) * 6 * 1; 
        int xPos = SCREEN_W - textWidth - 4;       
        int yPos = 5;

        // Clear the area where score was previously drawn
        screen.fillRect(xPos, yPos, textWidth, 8 * 1, background_color);
        screen.stroke(score_color);
        screen.setTextSize(1);
        screen.text(scoreText, xPos, yPos);

        prev_score = snake_size - STARTING_SNAKE_SIZE;
        score_condition = false;
    }
}

ISR(TIMER1_COMPA_vect) {
  tick = true;
}

void render_game_over() {
  screen.stroke(gameover_color); 
  screen.setTextSize(2); 

  // compute the text width (approximate: each character is 6 pixels wide per size 1)
  const char* msg = GAMEOVER_TEXT;
  int charWidth = 6 * 2;
  int textWidth = strlen(msg) * charWidth;

  // center coordinates
  int x = (SCREEN_W - textWidth) / 2;
  int y = GAME_OFFSET_Y + (GAME_H - 8 * 2) / 2; // 8 pixels height per char * text size

  screen.text(msg, x, y);
}

uint8_t read_highscore() {
    uint8_t magic = EEPROM.read(MAGIC_ADDR);
    if (magic != MAGIC_VALUE) {
        EEPROM.update(MAGIC_ADDR, MAGIC_VALUE);
        EEPROM.update(HIGHSCORE_ADDR, 0); 
        return 0;
    }
    return EEPROM.read(HIGHSCORE_ADDR);
}

void write_highscore() {
    EEPROM.update(HIGHSCORE_ADDR, highscore);
    EEPROM.update(MAGIC_ADDR, MAGIC_VALUE); 
}