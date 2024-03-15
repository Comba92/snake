#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

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
  int size;
  int capacity;
} Snake;

Snake initSnake() {
  Snake s;

  s.body = malloc(32 * sizeof(Block));
  s.body[0] = (Block) {0, 0, dRIGHT};
  s.size = 1;
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

  s->body[0].x += DIRECTIONS[s->body[0].direction][0] * BLOCK_SIZE;
  s->body[0].y += DIRECTIONS[s->body[0].direction][1] * BLOCK_SIZE;
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

void moveLeft(State* s)   { if (s->snake.body[0].direction != dRIGHT) s->snake.body[0].direction = dLEFT; }
void moveRight(State* s)  { if (s->snake.body[0].direction != dLEFT) s->snake.body[0].direction = dRIGHT; }
void moveUp(State* s)     { if (s->snake.body[0].direction != dDOWN) s->snake.body[0].direction = dUP; }
void moveDown(State* s)   { if (s->snake.body[0].direction != dUP) s->snake.body[0].direction = dDOWN; }
void growInPlace(State *s)   { growSnake(&s->snake); }

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
 
  do {
    int x = (int)( rand() % 800 ) / BLOCK_SIZE * BLOCK_SIZE;
    int y = (int)( rand() % 600 ) / BLOCK_SIZE * BLOCK_SIZE;

    for (int i=0; i<s.size; i++) {
      if (s.body[i].x != x && s.body[i].y != y) {
        return (Block) {x, y, dNONE};
      }
    }
  } while (true);

  return (Block) {0};
}

bool isSnakeCollidingWithItself(Snake s) {
  for (int i=1; i<s.size; i++) {
    if (s.body[0].x == s.body[i].x && s.body[0].y == s.body[i].y) return true;
  }

  return false;
}

bool isSnakeCollidingWithApple(State s) {
  return s.snake.body[0].x == s.apple.x && s.snake.body[0].y == s.apple.y;
}

int main() {
  InitWindow(800, 600, "Snake");
  SetTargetFPS(30);

  static const float GAME_DELAY = 0.075;

  Snake snake = initSnake();
  Block apple = spawnApple(snake);
  State gameState = {snake, apple};
  float timer = 0;

  while( !WindowShouldClose() ) {
    BeginDrawing();

    if (timer >= GAME_DELAY) {
      timer -= GAME_DELAY;
      updateSnake(&gameState.snake);
      if (isSnakeCollidingWithItself(gameState.snake)) return 1;
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