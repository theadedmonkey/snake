#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// screen dimension constants
const int SCREEN_WIDTH = 1024;
const int SCREEN_WIDTH_HALF = SCREEN_WIDTH / 2;

const int SCREEN_HEIGHT = 768;
const int SCREEN_HEIGHT_HALF = SCREEN_HEIGHT / 2;

// FPS constants
const int FPS = 15;
const int DELAY_TIME = 1000.0f / FPS;

// window and renderer
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// score
int score;

// score font
TTF_Font* scoreFont = nullptr;
const int SCORE_FONT_SIZE = 24;
SDL_Color SCORE_FONT_COLOR = { 000, 000, 000, 255 };

// game scenes
enum class GAME_SCENES { SPLASH, PLAY, PAUSED, GAME_OVER };
GAME_SCENES gameScene;

// tiles coordinates
std::vector<std::pair<int, int>> tileCoords;

// snake
enum class SNAKE_DIRECTIONS { UP, DOWN, LEFT, RIGHT };
SNAKE_DIRECTIONS snakeDirection;

const int SNAKE_SEGMENT_WIDTH = 32;
const int SNAKE_SEGMENT_WIDTH_HALF = SNAKE_SEGMENT_WIDTH / 2;
const int SNAKE_SEGMENT_HEIGHT = 32;
const int SNAKE_SEGMENT_HEIGHT_HALF = SNAKE_SEGMENT_HEIGHT / 2;

// game textures
SDL_Texture* scoreTexture = nullptr;
SDL_Texture* splashTexture = nullptr;
SDL_Texture* pausedTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* snakeSegmentTexture = nullptr;
SDL_Texture* foodTexture = nullptr;

// game rects
SDL_Rect scoreRect;
SDL_Rect splashRect;
SDL_Rect pausedRect;
SDL_Rect gameOverRect;
std::deque<SDL_Rect> snakeSegmentRects;
SDL_Rect foodRect;

bool initSDL();
SDL_Texture* loadTexture(const std::string& path);
SDL_Texture* loadTextTexture(std::string text, SDL_Color color);
bool loadMedia();
bool initGame();
bool rectsIntersects(const SDL_Rect &a, const SDL_Rect &b);
void drawScene();
void drawPlay();
void drawSplash();
void drawPaused();
void drawGameOver();
void drawScore();
void drawSnake();
void updateScene();
void updatePlay();
void updateSplash();
void updateGameOver();
void updateSnake();
int random(int min, int max);
std::vector<std::pair<int, int>> difference(
	const std::vector<std::pair<int, int>> &a,
	const std::vector<std::pair<int, int>> &b
);
void generateTileCoords();
void generateFood();
void resetPlay();

bool initSDL() {
	if(SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
	  std::cout << " Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return false;
	}

  if(TTF_Init() == -1) {
    std::cout << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
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

SDL_Texture* loadTextTexture(std::string text, SDL_Color color) {
  //Get rid of preexisting texture
	// free();

  //Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(scoreFont, text.c_str(), color);
	if(!textSurface) {
	  std::cout << "Failed to render text surface error: " << TTF_GetError() << std::endl;
		return nullptr;
	}
  //Create texture from surface pixels
  SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if(!textTexture) {
	 std::cout << "Failed to create text texture from text surface error: " << SDL_GetError() << std::endl;
	 return nullptr;
  }

  SDL_FreeSurface(textSurface);
	return textTexture;
}

bool loadMedia() {
  scoreFont = TTF_OpenFont("assets/Roboto-Black.ttf", SCORE_FONT_SIZE);
	if(!scoreFont) {
	  return false;
	}

  splashTexture = loadTexture("assets/splash.png");
	if(!splashTexture) {
		return false;
	}

  pausedTexture = loadTexture("assets/paused.jpg");
	if(!pausedTexture) {
		return false;
	}

	gameOverTexture = loadTexture("assets/game-over.jpg");
	if(!gameOverTexture) {
		return false;
	}

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

  splashRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
  // w = 500
	// h = 100
	pausedRect = { SCREEN_WIDTH_HALF - 500 / 2, SCREEN_HEIGHT_HALF - 100 / 2, 500, 100 };
  // w = 500
	// h = 200
	gameOverRect = { SCREEN_WIDTH_HALF - 500 / 2, SCREEN_HEIGHT_HALF - 200 / 2, 500, 200 };

  gameScene = GAME_SCENES::SPLASH;

  generateTileCoords();

  resetPlay();

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

void drawScene() {
	SDL_RenderClear(renderer);

	if (gameScene == GAME_SCENES::SPLASH) {
		drawSplash();
	}

	if (gameScene == GAME_SCENES::PLAY) {
		drawPlay();
	}

	if (gameScene == GAME_SCENES::PAUSED) {
		drawPaused();
	}

	if (gameScene == GAME_SCENES::GAME_OVER) {
		drawGameOver();
	}

	SDL_RenderPresent(renderer);
}

void drawPlay() {
  SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	drawScore();
  drawSnake();
  SDL_RenderCopy(renderer, foodTexture, nullptr, &foodRect);
  SDL_RenderPresent(renderer);
}

void drawSplash() {
	SDL_RenderCopy(renderer, splashTexture, nullptr, &splashRect);
}

void drawPaused() {
	drawPlay();
	SDL_RenderCopy(renderer, pausedTexture, nullptr, &pausedRect);
}

void drawGameOver() {
	drawPlay();
	SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
}

void drawScore() {
  if(scoreTexture) {
		SDL_DestroyTexture(scoreTexture);
	}

	scoreTexture = loadTextTexture(std::to_string(score), SCORE_FONT_COLOR);
	int scoreRectW, scoreRectH;
	SDL_QueryTexture(scoreTexture, nullptr, nullptr, &scoreRectW, &scoreRectH);

	scoreRect = { SCREEN_WIDTH - scoreRectW, 0, scoreRectW, scoreRectH };
	SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
}

void drawSnake() {
	for (SDL_Rect &snakeSegment : snakeSegmentRects) {
    SDL_RenderCopy(renderer, snakeSegmentTexture, nullptr, &snakeSegment);
	}
}

void updateScene() {
	if (gameScene == GAME_SCENES::SPLASH) {
		updateSplash();
	}

	if (gameScene == GAME_SCENES::PLAY) {
		updatePlay();
	}

	if (gameScene == GAME_SCENES::GAME_OVER) {
		updateGameOver();
	}

}

void updatePlay() {
  updateSnake();
}

void updateSplash() {
	Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);
	if(keys[SDL_SCANCODE_RETURN]) {
	  resetPlay();
		gameScene = GAME_SCENES::PLAY;
	}
}

void updateGameOver() {
	Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);
	if(keys[SDL_SCANCODE_SPACE]) {
	  gameScene = GAME_SCENES::SPLASH;
	}
}

void updateSnake() {
	// snake snakeDirection
  Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);

  if(keys[SDL_SCANCODE_UP] && snakeDirection != SNAKE_DIRECTIONS::DOWN) {
    snakeDirection = SNAKE_DIRECTIONS::UP;
  }

  if(keys[SDL_SCANCODE_DOWN] && snakeDirection != SNAKE_DIRECTIONS::UP) {
    snakeDirection = SNAKE_DIRECTIONS::DOWN;
  }

  if(keys[SDL_SCANCODE_LEFT] && snakeDirection != SNAKE_DIRECTIONS::RIGHT) {
    snakeDirection = SNAKE_DIRECTIONS::LEFT;
  }

  if(keys[SDL_SCANCODE_RIGHT] && snakeDirection != SNAKE_DIRECTIONS::LEFT) {
    snakeDirection = SNAKE_DIRECTIONS::RIGHT;
  }

  // snake movement
  SDL_Rect headRect = snakeSegmentRects.front();

  if(snakeDirection == SNAKE_DIRECTIONS::UP) {
    headRect.y -= SNAKE_SEGMENT_HEIGHT;
  }

  if(snakeDirection == SNAKE_DIRECTIONS::DOWN) {
    headRect.y += SNAKE_SEGMENT_HEIGHT;
  }

  if(snakeDirection == SNAKE_DIRECTIONS::LEFT) {
    headRect.x -= SNAKE_SEGMENT_WIDTH;
  }

  if(snakeDirection == SNAKE_DIRECTIONS::RIGHT) {
    headRect.x += SNAKE_SEGMENT_WIDTH;
  }

  snakeSegmentRects.push_front(headRect);

  // check collisions of snake with food
	if(rectsIntersects(headRect, foodRect)) {
		score += 50;
		generateFood();
	}
	else {
		snakeSegmentRects.pop_back();
	}

  // check collisions of snake with itself
	for (auto i = 1; i < snakeSegmentRects.size(); i++) {
	  if (rectsIntersects(headRect, snakeSegmentRects[i])) {
			gameScene = GAME_SCENES::GAME_OVER;
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
		gameScene = GAME_SCENES::GAME_OVER;
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

void resetPlay() {

  score = 0;

  snakeDirection = SNAKE_DIRECTIONS::DOWN;
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
  if(!initGame()) {
    return 0;
  }

	SDL_Event event;
  bool isRunning = true;

  while(isRunning) {
    long int oldTime = SDL_GetTicks();

    while(SDL_PollEvent(&event) != 0) {
			// close the game
			if(event.type == SDL_QUIT) {
				isRunning = false;
			}
			// game pause logic
			if(event.type == SDL_KEYDOWN) {
			  if(event.key.keysym.sym == SDLK_ESCAPE) {
          if(gameScene == GAME_SCENES::PLAY) {
						gameScene = GAME_SCENES::PAUSED;
					}
					else if(gameScene == GAME_SCENES::PAUSED) {
						gameScene = GAME_SCENES::PLAY;
					}
				}
			}
    }

		updateScene();
		drawScene();

    int frameTime = SDL_GetTicks() - oldTime;
    if(frameTime < DELAY_TIME) {
      SDL_Delay(DELAY_TIME - frameTime);
    }
  }

}
