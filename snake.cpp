#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <numeric>
#include <algorithm>
#include <utility>
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
std::vector<std::pair<int, int>> tileCoords;

enum class DIRECTIONS { UP, DOWN, LEFT, RIGHT };
DIRECTIONS direction;

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
int random(int min, int max);
std::vector<std::pair<int, int>> difference(
	const std::vector<std::pair<int, int>> &a,
	const std::vector<std::pair<int, int>> &b
);
void generateTileCoords();
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

  generateTileCoords();

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
	SDL_SetRenderDrawColor(renderer, 210, 180, 140, 255);
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
	// snake direction
  Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);

  if(keys[SDL_SCANCODE_UP] && direction != DIRECTIONS::DOWN) {
    direction = DIRECTIONS::UP;
  }

  if(keys[SDL_SCANCODE_DOWN] && direction != DIRECTIONS::UP) {
    direction = DIRECTIONS::DOWN;
  }

  if(keys[SDL_SCANCODE_LEFT] && direction != DIRECTIONS::RIGHT) {
    direction = DIRECTIONS::LEFT;
  }

  if(keys[SDL_SCANCODE_RIGHT] && direction != DIRECTIONS::LEFT) {
    direction = DIRECTIONS::RIGHT;
  }

  // snake movement
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

  // check collisions of snake with food
	if(rectsIntersects(headRect, foodRect)) {
		generateFood();
	}
	else {
		snakeSegmentRects.pop_back();
	}

  SDL_Delay(150);

  // check collisions of snake with itself
	for (auto i = 1; i < snakeSegmentRects.size(); i++) {
	  if (rectsIntersects(headRect, snakeSegmentRects[i])) {
			SDL_Delay(500);
			resetGame();
			break;
		}
	}

	// check collisions of snake with screen
	if(
	  headRect.x < 0 ||
		headRect.x + SNAKE_SEGMENT_WIDTH > SCREEN_WIDTH ||
		headRect.y < 0 ||
		headRect.y + SNAKE_SEGMENT_HEIGHT > SCREEN_HEIGHT
	 ) {
	  SDL_Delay(500);
    resetGame();
  }

}

// range : [min, max) max no inclusive
int random(int min, int max) {
  static bool first = true;
  if (first) {
	  //seeding for the first time only!
    srand(time(NULL));
    first = false;
  }
  return min + rand() % (max - min);
}

// Finds the set (i.e. no duplicates) of all elements in the
// first vector not contained in the second vector.
std::vector<std::pair<int, int>> difference(
	const std::vector<std::pair<int, int>> &a,
  const std::vector<std::pair<int, int>> &b
) {
  std::vector<std::pair<int, int>> out;
	int idx = 0;
	unsigned int aSize = a.size();
	while (idx < aSize) {
		if (
		  std::find(b.begin(), b.end(), a[idx]) == b.end() &&
	    std::find(out.begin(), out.end(), a[idx]) == out.end()
		) {
			out.push_back(a[idx]);
		}
		idx++;
	}
  return out;
}

void generateTileCoords() {
  for (auto x = 0; x < 32; x++) {
	  for (auto y = 0; y < 24; y++) {
		  tileCoords.push_back(std::pair<int, int>(x * 32, y * 32));
	  }
  }
}

void generateFood() {
	std::pair<int, int> position;
  std::vector<std::pair<int, int>> excludedTileCoords;
	std::vector<std::pair<int, int>> availableTileCoords;
  // take the x y coords of all snake segments
	for (auto &snakeSegmentRect : snakeSegmentRects) {
		excludedTileCoords.push_back(std::pair<int, int>(
			snakeSegmentRect.x, snakeSegmentRect.y
		));
	}
  // take all x y free coords
  availableTileCoords = difference(tileCoords, excludedTileCoords);
	// take one free coord randomly
	position = availableTileCoords[random(0, availableTileCoords.size())];
  // update food position
	foodRect.x = position.first;
	foodRect.y = position.second;
}

void resetGame() {

  direction = DIRECTIONS::DOWN;
  snakeSegmentRects.clear();

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

  foodRect = { -32, -32, 32, 32 };
	generateFood();
}

int main( int argc, char* args[] ) {

	std::vector<std::pair<int, int>> a;
	a.push_back(std::pair<int, int>(1, 2));
	a.push_back(std::pair<int, int>(3, 4));

	std::vector<std::pair<int, int>> b;
	b.push_back(std::pair<int, int>(1, 2));

	std::vector<std::pair<int, int>> out;
	out = difference(a, b);

	for (auto i = 0; i < out.size(); i++) {
		std::cout << out[i].first << " " << out[i].second << std::endl;
	}

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
