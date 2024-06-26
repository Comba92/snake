#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

static const int SCREEN_W = 800;
static const int SCREEN_H = 600;
static const int BLOCK_SIZE = 20; 

typedef enum {
  dLEFT, dRIGHT, dUP, dDOWN, dNONE
} Direction;
static const int DIRECTIONS[4][2] = {
  {-1, 0}, {1, 0}, {0, -1}, {0, 1}
};

typedef struct {
  int x; int y; Direction direction;
} Block;

typedef struct {
  Block* body;
  Block* head;
  int size;
  int capacity;
} Snake;

Snake initSnake() {
  Snake s;

  s.body = malloc(32 * sizeof(Block));
  s.head = &s.body[0];
  *s.head = (Block) {SCREEN_W / 2, SCREEN_H / 2, dRIGHT};
  s.body[1] = (Block) {s.head->x - 1, s.head->y, dRIGHT};
  s.body[2] = (Block) {s.head->x - 2, s.head->y, dRIGHT};
  s.size = 3;
  s.capacity = 32;

  return s;
}

void growSnake(Snake* s) {
  if (s->size == s->capacity) {
    s->body = realloc(s->body, s->capacity * 2);
    s->capacity *= 2;
  }

  s->body[s->size] = s->body[s->size-1];
  s->size++;
}

void updateSnake(Snake* s) {
  for (int i = s->size - 1; i > 0; i--) {
    s->body[i] = s->body[i-1];
  }

  if (s->size > 1) {
    s->body[s->size].x = DIRECTIONS[s->body[s->size-1].direction][0] * BLOCK_SIZE;
    s->body[s->size].y = DIRECTIONS[s->body[s->size-1].direction][1] * BLOCK_SIZE;
    s->body[s->size].direction = s->body[s->size-1].direction;
  }

  s->head->x += DIRECTIONS[s->head->direction][0] * BLOCK_SIZE;
  s->head->y += DIRECTIONS[s->head->direction][1] * BLOCK_SIZE;
}

typedef struct {
  Snake snake;
  Block apple;
} State;

typedef void (*EventCallback)(State*);
typedef bool (*EventHandler)(int);
typedef struct {
  int key;
  EventHandler handler;
  EventCallback callback;
} KeyEvent;

void moveLeft(State* s)   { if (s->snake.head->direction != dRIGHT) s->snake.head->direction = dLEFT; }
void moveRight(State* s)  { if (s->snake.head->direction != dLEFT)  s->snake.head->direction = dRIGHT; }
void moveUp(State* s)     { if (s->snake.head->direction != dDOWN)  s->snake.head->direction = dUP; }
void moveDown(State* s)   { if (s->snake.head->direction != dUP)    s->snake.head->direction = dDOWN; }
void growInPlace(State *s) { growSnake(&s->snake); }

typedef enum {
  kLEFT, kRIGHT, kUP, kDOWN, rSPACE, KEYS
} Keys;
static const KeyEvent KEY_EVENTS[KEYS] = {
  (KeyEvent) { .key = KEY_LEFT,   .handler = &IsKeyDown,    .callback = &moveLeft },
  (KeyEvent) { .key = KEY_RIGHT,  .handler = &IsKeyDown,    .callback = &moveRight },
  (KeyEvent) { .key = KEY_UP,     .handler = &IsKeyDown,    .callback = &moveUp },
  (KeyEvent) { .key = KEY_DOWN,   .handler = &IsKeyDown,    .callback = &moveDown },
  (KeyEvent) { .key = KEY_SPACE,  .handler = &IsKeyPressed, .callback = &growInPlace }
};

void handleInput(State* s) {
  for(int i=0; i<KEYS; i++) {
    KeyEvent k = KEY_EVENTS[i];
    
    if ( k.handler(k.key) ) {
      k.callback(s);
      break;
    }
  }
}

Block spawnApple(Snake s) {
  srand(time(NULL));

  bool valid = true; 
  int x, y;
  do {
    valid = true;
    x = ( rand() % SCREEN_W ) / BLOCK_SIZE * BLOCK_SIZE;
    y = ( rand() % SCREEN_H ) / BLOCK_SIZE * BLOCK_SIZE;

    for (int i=0; i<s.size; i++) {
      if (s.body[i].x == x && s.body[i].y == y) {
        valid = false;
      }
    }
  } while (!valid);

  return (Block) {x, y, dNONE};
}

bool isSnakeCollidingWithItself(State s) {
  for (int i=1; i<s.snake.size; i++) {
    if (s.snake.head->x == s.snake.body[i].x && s.snake.head->y == s.snake.body[i].y) 
      return true;
  }

  return false;
}

bool isSnakeCollidingWithApple(State s) {
  return s.snake.head->x == s.apple.x && s.snake.head->y == s.apple.y;
}

bool isSnakeCollidingWithWalls(State s) {
  return s.snake.head->x < 0 || s.snake.head->x >= SCREEN_W || 
    s.snake.head->y < 0 || s.snake.head->y >= SCREEN_H;
}

void initState(State* s) {
  free(s->snake.body);
  
  Snake snake = initSnake();
  Block apple = spawnApple(snake);
  s->snake = snake;
  s->apple = apple;
}

int main() {
  InitWindow(SCREEN_W, SCREEN_H, "Snake");
  SetTargetFPS(30);

  static const float GAME_DELAY = 0.075;

  State gameState = {0};
  initState(&gameState);
  float timer = 0;

  while( !WindowShouldClose() ) {
    BeginDrawing();

    if (timer >= GAME_DELAY) {
      timer -= GAME_DELAY;
      updateSnake(&gameState.snake);
      if (isSnakeCollidingWithItself(gameState) || isSnakeCollidingWithWalls(gameState))
        initState(&gameState);
    }

    ClearBackground(BLACK);

    handleInput(&gameState);
    if (isSnakeCollidingWithApple(gameState)) {
      growSnake(&gameState.snake);
      gameState.apple = spawnApple(gameState.snake);
    }

    DrawRectangle(gameState.apple.x, gameState.apple.y, BLOCK_SIZE, BLOCK_SIZE, RED);

    for (int i=0; i<gameState.snake.size; i++)
      DrawRectangle(
        gameState.snake.body[i].x,
        gameState.snake.body[i].y,
        BLOCK_SIZE, BLOCK_SIZE, WHITE
      );

    timer += GetFrameTime();
    EndDrawing();
  }

  free(gameState.snake.body);
  CloseWindow();
  return 0;
}