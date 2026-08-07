#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <espelho.h>
#include <font5x7.h>
#include <health.h>
#include <position.h>

using namespace std::chrono_literals;

namespace
{

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
constexpr int centreX = windowWidth / 2;
constexpr int centreY = windowHeight / 2;
constexpr int dotSize = 3;

// Enough entities to force packet-splitting under the 1200-byte MTU budget,
// as a stand-in for a real game world ahead of phase two's optimisation work.
constexpr int objectCount = 1000;

// Golden angle (radians): placing point i at angle i * goldenAngle with
// radius proportional to sqrt(i) tiles a disc evenly (a phyllotaxis spiral),
// so the swarm reads as a filled cloud instead of overlapping rings.
constexpr double goldenAngle = 2.399963229728653;

SDL_Point pixelCentre(const Position& position)
{
  return SDL_Point {centreX + position.x(), centreY + position.y()};
}

void drawGrid(SDL_Renderer* renderer)
{
  constexpr int spacing = 40;
  SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
  for (int x = 0; x < windowWidth; x += spacing) {
    SDL_RenderDrawLine(renderer, x, 0, x, windowHeight);
  }
  for (int y = 0; y < windowHeight; y += spacing) {
    SDL_RenderDrawLine(renderer, 0, y, windowWidth, y);
  }
}

// Batched into a single SDL_RenderFillRects call rather than per-point
// SetColor+FillRect, since this runs on up to objectCount points a frame.
void drawSwarm(SDL_Renderer* renderer,
               const std::vector<SDL_Point>& points,
               SDL_Color colour)
{
  std::vector<SDL_Rect> rects;
  rects.reserve(points.size());
  for (const auto& p : points) {
    rects.push_back(
        SDL_Rect {p.x - dotSize / 2, p.y - dotSize / 2, dotSize, dotSize});
  }
  SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
  SDL_RenderFillRects(renderer, rects.data(), static_cast<int>(rects.size()));
}

void drawText(SDL_Renderer* renderer,
              int x,
              int y,
              std::string_view text,
              SDL_Color colour,
              int scale)
{
  SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
  int cursorX = x;
  for (char c : text) {
    const font5x7::Glyph& glyph = font5x7::forChar(c);
    for (int row = 0; row < font5x7::glyphHeight; ++row) {
      for (int col = 0; col < font5x7::glyphWidth; ++col) {
        if (glyph[row][col] == '#') {
          SDL_Rect pixel {cursorX + col * scale, y + row * scale, scale, scale};
          SDL_RenderFillRect(renderer, &pixel);
        }
      }
    }
    cursorX += (font5x7::glyphWidth + 1) * scale;
  }
}

std::string formatHud(std::string_view role, int port, size_t count)
{
  return std::string(role) + " #" + std::to_string(port)
      + " objects:" + std::to_string(count);
}

std::vector<SDL_Point> collectPositions(
    const std::vector<std::unique_ptr<Replicable>>& objects)
{
  std::vector<SDL_Point> points;
  points.reserve(objects.size());
  for (const auto& obj : objects) {
    if (obj->typeID() == TypeID::Position) {
      points.push_back(pixelCentre(static_cast<const Position&>(*obj)));
    }
  }
  return points;
}

void renderFrame(SDL_Renderer* renderer,
                 int ownPort,
                 int peerPort,
                 const std::vector<SDL_Point>& own,
                 const std::vector<SDL_Point>& mirrored)
{
  constexpr SDL_Color ownColour {0, 200, 0, 255};
  constexpr SDL_Color mirroredColour {255, 140, 0, 255};
  constexpr SDL_Color hudColour {230, 230, 230, 255};
  constexpr int hudScale = 2;
  constexpr int hudLineHeight = font5x7::glyphHeight * hudScale + 6;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  drawGrid(renderer);
  drawSwarm(renderer, own, ownColour);
  drawSwarm(renderer, mirrored, mirroredColour);

  drawText(renderer,
           10,
           10,
           formatHud("OWN", ownPort, own.size()),
           hudColour,
           hudScale);
  drawText(renderer,
           10,
           10 + hudLineHeight,
           formatHud("PEER", peerPort, mirrored.size()),
           hudColour,
           hudScale);

  SDL_RenderPresent(renderer);
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

  std::vector<std::unique_ptr<Replicable>> objects;
  objects.reserve(objectCount * 2);
  std::vector<Position*> positions;
  positions.reserve(objectCount);
  for (int i = 0; i < objectCount; ++i) {
    const int id = ownPort * 10000 + i;  // unique per entity, tagged by origin

    auto position = std::make_unique<Position>();
    position->id = id;
    positions.push_back(position.get());
    objects.push_back(std::move(position));

    auto health = std::make_unique<Health>();
    health->id = id;
    objects.push_back(std::move(health));
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
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // Per-port offsets so two peers running in the same process space produce
  // visibly distinct swarms instead of identical overlapping ones.
  constexpr double angularSpeed = 0.02;
  constexpr double degToRad = 3.14159265358979 / 180.0;
  const double angleOffset = static_cast<double>(ownPort % 360) * degToRad;
  const double radiusScale = 6.0 + static_cast<double>(ownPort % 5) * 0.5;

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
      for (int i = 0; i < objectCount; ++i) {
        const double radius = radiusScale * std::sqrt(static_cast<double>(i));
        const double angle =
            i * goldenAngle + angleOffset + tick * angularSpeed;
        positions[i]->set(static_cast<int>(radius * std::cos(angle)),
                          static_cast<int>(radius * std::sin(angle)));
      }
      ++tick;

      espelho.SendObjects(objects);
      espelho.Update();
      lastTick = now;
    }

    renderFrame(renderer,
                ownPort,
                peerPort,
                collectPositions(objects),
                collectPositions(espelho.Objects()));
    SDL_Delay(16);  // ~60fps render cadence, independent of the 100ms tick
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
