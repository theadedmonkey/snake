#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <numeric>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Screen dimension constants
const int SCREEN_WIDTH = 1024;
const int SCREEN_WIDTH_HALF = SCREEN_WIDTH / 2;

const int SCREEN_HEIGHT = 768;
const int SCREEN_HEIGHT_HALF = SCREEN_HEIGHT / 2;

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

// window and renderer
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// game object constants
enum class DIRECTIONS { UP, DOWN, LEFT, RIGHT };
DIRECTIONS direction = DIRECTIONS::DOWN;

const int SNAKE_SEGMENT_WIDTH = 32;
const int SNAKE_SEGMENT_WIDTH_HALF = SNAKE_SEGMENT_WIDTH / 2;
const int SNAKE_SEGMENT_HEIGHT = 32;
const int SNAKE_SEGMENT_HEIGHT_HALF = SNAKE_SEGMENT_HEIGHT / 2;

// game textures
SDL_Texture* snakeSegmentTexture = nullptr;
SDL_Texture* foodTexture = nullptr;

// game rects
std::deque<SDL_Rect> snakeSegmentRects;
SDL_Rect foodRect;

bool initSDL();
SDL_Texture* loadTexture(const std::string& path);
bool loadMedia();
bool initGame();
bool rectsIntersects(const SDL_Rect &a, const SDL_Rect &b);
void drawGame();
void drawSnake();
void updateGame();
void updateSnake();
void generateFood();
void resetGame();

bool initSDL() {
	if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
	  std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
		return false;
	}

  window = SDL_CreateWindow(
		"Snake",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);
  if(window == NULL) {
    std::cout << "Failed to create window with error: " << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(renderer == NULL) {
    std::cout << "Failed to create renderer with error: " << SDL_GetError() << std::endl;
    return false;
  }

	return true;
}

SDL_Texture* loadTexture(const std::string &path) {
  SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
  if (texture == nullptr) {
    std::cout << "Failed to load texture " << path << " error : " << SDL_GetError() << std::endl;
    return nullptr;
  }
  return texture;
}

bool loadMedia() {
  snakeSegmentTexture = loadTexture("assets/snake-segment.png");
  if(!snakeSegmentTexture) {
    return false;
  }

  foodTexture = loadTexture("assets/food.png");
  if(!foodTexture) {
    return false;
  }

  return true;
}

bool initGame() {
  if(!initSDL()) {
    return false;
  }

  if(!loadMedia()) {
    return false;
  }

  resetGame();

  return true;
}

bool rectsIntersects(const SDL_Rect &a, const SDL_Rect &b) {
  if(a.x >= b.x + b.w) {
    return false;
  }

  if(b.x >= a.x + a.w) {
    return false;
  }

  if(a.y >= b.y + b.h) {
    return false;
  }

  if(b.y >= a.y + a.h) {
    return false;
  }

  return true;
}

void drawGame() {
  SDL_RenderClear(renderer);
  drawSnake();
  SDL_RenderCopy(renderer, foodTexture, nullptr, &foodRect);
  SDL_RenderPresent(renderer);
}

void drawSnake() {
	for (SDL_Rect &snakeSegment : snakeSegmentRects) {
    SDL_RenderCopy(renderer, snakeSegmentTexture, nullptr, &snakeSegment);
	}
}

void updateGame() {
  updateSnake();
}

void updateSnake() {
  Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);

  if(keys[SDL_SCANCODE_UP]) {
    direction = DIRECTIONS::UP;
  }

  if(keys[SDL_SCANCODE_DOWN]) {
    direction = DIRECTIONS::DOWN;
  }

  if(keys[SDL_SCANCODE_LEFT]) {
    direction = DIRECTIONS::LEFT;
  }

  if(keys[SDL_SCANCODE_RIGHT]) {
    direction = DIRECTIONS::RIGHT;
  }

  SDL_Rect headRect = snakeSegmentRects.front();

  if(direction == DIRECTIONS::UP) {
    headRect.y -= SNAKE_SEGMENT_HEIGHT;
  }

  if(direction == DIRECTIONS::DOWN) {
    headRect.y += SNAKE_SEGMENT_HEIGHT;
  }

  if(direction == DIRECTIONS::LEFT) {
    headRect.x -= SNAKE_SEGMENT_WIDTH;
  }

  if(direction == DIRECTIONS::RIGHT) {
    headRect.x += SNAKE_SEGMENT_WIDTH;
  }

  snakeSegmentRects.push_front(headRect);

	if(rectsIntersects(headRect, foodRect)) {
		// foodRect.x = -32;
		// foodRect.y = -32;
		generateFood();
	}
	else {
		snakeSegmentRects.pop_back();
	}

  SDL_Delay(150);
}

int random(int min, int max) //range : [min, max)
{
   static bool first = true;
   if (first)
   {
      srand(time(NULL)); //seeding for the first time only!
      first = false;
   }
   return min + rand() % (max - min);
}

void generateFood() {
  foodRect.x = random(0, 32) * 32;
	foodRect.y = random(0, 24) * 32;
}

void resetGame() {

  snakeSegmentRects.push_back({
    SNAKE_SEGMENT_WIDTH * 16,
		SNAKE_SEGMENT_HEIGHT * 12,
		SNAKE_SEGMENT_WIDTH,
		SNAKE_SEGMENT_HEIGHT
  });

  snakeSegmentRects.push_back({
    SNAKE_SEGMENT_WIDTH * 16,
		SNAKE_SEGMENT_HEIGHT * 12 - SNAKE_SEGMENT_HEIGHT,
		SNAKE_SEGMENT_WIDTH,
		SNAKE_SEGMENT_HEIGHT
  });

  snakeSegmentRects.push_back({
    SNAKE_SEGMENT_WIDTH * 16,
		SNAKE_SEGMENT_HEIGHT * 12 - SNAKE_SEGMENT_HEIGHT * 2,
		SNAKE_SEGMENT_WIDTH,
		SNAKE_SEGMENT_HEIGHT
  });

  foodRect = { 32, 64, 32, 32 };
}

int main( int argc, char* args[] ) {
  if(!initGame()) {
    return 0;
  }

	SDL_Event e;
  bool isRunning = true;

  while(isRunning) {
    long int oldTime = SDL_GetTicks();

    while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT) {
				isRunning = false;
			}
    }
    updateGame();
    drawGame();

    int frameTime = SDL_GetTicks() - oldTime;
    if(frameTime < DELAY_TIME) {
      SDL_Delay(DELAY_TIME - frameTime);
    }
  }

}
