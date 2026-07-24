#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include <SDL.h>
#include <espelho.h>
#include <health.h>
#include <position.h>

using namespace std::chrono_literals;

namespace
{

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
constexpr int boxSize = 20;
constexpr int worldWidth = windowWidth - boxSize;

void drawPosition(SDL_Renderer* renderer,
                  const Position& position,
                  SDL_Color colour,
                  int row)
{
  SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_Rect rect {position.x() % worldWidth, row, boxSize, boxSize};
  SDL_RenderFillRect(renderer, &rect);
}

void drawObjects(
    SDL_Renderer* renderer,
    const std::vector<std::unique_ptr<Replicable>>& ownObjects,
    const std::vector<std::unique_ptr<Replicable>>& mirroredObjects)
{
  constexpr SDL_Color ownColour {0, 200, 0, 255};
  constexpr SDL_Color mirroredColour {255, 140, 0, 255};
  constexpr int ownRow = 100;
  constexpr int mirroredRow = 300;

  for (const auto& obj : ownObjects) {
    switch (obj->typeID()) {
      case TypeID::Position:
        drawPosition(
            renderer, static_cast<const Position&>(*obj), ownColour, ownRow);
        break;
      case TypeID::Health:
        break;
    }
  }

  for (const auto& obj : mirroredObjects) {
    switch (obj->typeID()) {
      case TypeID::Position:
        drawPosition(renderer,
                     static_cast<const Position&>(*obj),
                     mirroredColour,
                     mirroredRow);
        break;
      case TypeID::Health:
        break;
    }
  }
}

}  // namespace

int main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <own-port> <peer-port>" << std::endl;
    return EXIT_FAILURE;
  }

  const int ownPort = std::stoi(argv[1]);
  const int peerPort = std::stoi(argv[2]);

  Espelho espelho(ownPort, 10ms);
  if (!espelho.AddPeer("127.0.0.1", peerPort)) {
    std::cerr << "Error: failed to add peer on port " << peerPort << std::endl;
    return EXIT_FAILURE;
  }

  auto ownedPosition = std::make_unique<Position>(0, 0);
  Position* position = ownedPosition.get();

  std::vector<std::unique_ptr<Replicable>> objects;
  objects.push_back(std::move(ownedPosition));
  objects.push_back(std::make_unique<Health>(100));
  for (auto& obj : objects) {
    obj->id = ownPort;  // tag records with their origin
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return EXIT_FAILURE;
  }

  SDL_Window* window = SDL_CreateWindow("espelho visualiser",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        windowWidth,
                                        windowHeight,
                                        SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer* renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  int tick = 0;
  auto lastTick = std::chrono::steady_clock::now();
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        running = false;
      }
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - lastTick >= 100ms) {
      position->set(tick++, 0);
      espelho.SendObjects(objects);
      espelho.Update();
      lastTick = now;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawObjects(renderer, objects, espelho.Objects());
    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60fps render cadence, independent of the 100ms tick
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
