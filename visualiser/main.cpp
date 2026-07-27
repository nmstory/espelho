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
constexpr int boxSize = 20;

// A replicated object naturally exposes at most one Position and one Health;
// pull both out in a single pass so drawing code doesn't repeat the switch.
struct ObjectView
{
  const Position* position = nullptr;
  const Health* health = nullptr;
};

ObjectView findObjectView(
    const std::vector<std::unique_ptr<Replicable>>& objects)
{
  ObjectView view;
  for (const auto& obj : objects) {
    switch (obj->typeID()) {
      case TypeID::Position:
        view.position = static_cast<const Position*>(obj.get());
        break;
      case TypeID::Health:
        view.health = static_cast<const Health*>(obj.get());
        break;
    }
  }
  return view;
}

SDL_Point pixelCentre(const Position& position)
{
  return SDL_Point {centreX + position.x(), centreY + position.y()};
}

// A short fading history of where an object has been, so motion reads as a
// path rather than a square jumping between positions.
struct Trail
{
  static constexpr size_t maxLength = 20;
  std::vector<SDL_Point> points;

  void push(SDL_Point p)
  {
    if (points.empty() || points.back().x != p.x || points.back().y != p.y) {
      points.push_back(p);
      if (points.size() > maxLength) {
        points.erase(points.begin());
      }
    }
  }
};

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

void drawTrail(SDL_Renderer* renderer, const Trail& trail, SDL_Color colour)
{
  const size_t count = trail.points.size();
  constexpr int trailSize = boxSize / 2;
  for (size_t i = 0; i < count; ++i) {
    const auto alpha = static_cast<Uint8>(255 * (i + 1) / (count + 1));
    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, alpha);
    SDL_Rect rect {trail.points[i].x - trailSize / 2,
                   trail.points[i].y - trailSize / 2,
                   trailSize,
                   trailSize};
    SDL_RenderFillRect(renderer, &rect);
  }
}

void drawSquare(SDL_Renderer* renderer, SDL_Point centre, SDL_Color colour)
{
  SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
  SDL_Rect rect {
      centre.x - boxSize / 2, centre.y - boxSize / 2, boxSize, boxSize};
  SDL_RenderFillRect(renderer, &rect);
}

// Health is a uint8_t (0-255), but the demo only ever constructs it with the
// default value of 100, so 100 is treated as a "full" bar rather than 255.
void drawHealthBar(SDL_Renderer* renderer,
                   SDL_Point above,
                   uint8_t value,
                   SDL_Color colour)
{
  constexpr int barWidth = 40;
  constexpr int barHeight = 6;
  constexpr int referenceMax = 100;
  const int clamped = value < referenceMax ? value : referenceMax;
  const int filled = clamped * barWidth / referenceMax;
  const int x = above.x - barWidth / 2;
  const int y = above.y - boxSize;

  SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
  SDL_Rect background {x, y, barWidth, barHeight};
  SDL_RenderFillRect(renderer, &background);

  SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
  SDL_Rect fill {x, y, filled, barHeight};
  SDL_RenderFillRect(renderer, &fill);
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

std::string formatHud(std::string_view role, int port, const ObjectView& view)
{
  std::string text = std::string(role) + " #" + std::to_string(port) + " X:";
  text += view.position != nullptr ? std::to_string(view.position->x()) : "--";
  text += " Y:";
  text += view.position != nullptr ? std::to_string(view.position->y()) : "--";
  if (view.health != nullptr) {
    text += " HP:" + std::to_string(view.health->value());
  }
  return text;
}

void renderFrame(SDL_Renderer* renderer,
                 int ownPort,
                 int peerPort,
                 const ObjectView& own,
                 const ObjectView& mirrored,
                 const Trail& ownTrail,
                 const Trail& mirroredTrail)
{
  constexpr SDL_Color ownColour {0, 200, 0, 255};
  constexpr SDL_Color mirroredColour {255, 140, 0, 255};
  constexpr SDL_Color hudColour {230, 230, 230, 255};
  constexpr int hudScale = 2;
  constexpr int hudLineHeight = font5x7::glyphHeight * hudScale + 6;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  drawGrid(renderer);
  drawTrail(renderer, ownTrail, ownColour);
  drawTrail(renderer, mirroredTrail, mirroredColour);

  if (own.position != nullptr) {
    const SDL_Point centre = pixelCentre(*own.position);
    drawSquare(renderer, centre, ownColour);
    if (own.health != nullptr) {
      drawHealthBar(renderer, centre, own.health->value(), ownColour);
    }
  }
  if (mirrored.position != nullptr) {
    const SDL_Point centre = pixelCentre(*mirrored.position);
    drawSquare(renderer, centre, mirroredColour);
    if (mirrored.health != nullptr) {
      drawHealthBar(renderer, centre, mirrored.health->value(), mirroredColour);
    }
  }

  drawText(
      renderer, 10, 10, formatHud("OWN", ownPort, own), hudColour, hudScale);
  drawText(renderer,
           10,
           10 + hudLineHeight,
           formatHud("PEER", peerPort, mirrored),
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
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // Distinct radii per port so two peers running in the same process space
  // trace visibly different orbits rather than overlapping circles.
  constexpr double angularSpeed = 0.04;
  const double radius = 80.0 + static_cast<double>(ownPort % 5) * 30.0;

  int tick = 0;
  Trail ownTrail;
  Trail mirroredTrail;
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
      const double angle = tick * angularSpeed;
      position->set(static_cast<int>(radius * std::cos(angle)),
                    static_cast<int>(radius * std::sin(angle)));
      ++tick;

      espelho.SendObjects(objects);
      espelho.Update();
      lastTick = now;

      ownTrail.push(pixelCentre(*position));
      const ObjectView mirroredView = findObjectView(espelho.Objects());
      if (mirroredView.position != nullptr) {
        mirroredTrail.push(pixelCentre(*mirroredView.position));
      }
    }

    const ObjectView ownView = findObjectView(objects);
    const ObjectView mirroredView = findObjectView(espelho.Objects());
    renderFrame(renderer,
                ownPort,
                peerPort,
                ownView,
                mirroredView,
                ownTrail,
                mirroredTrail);
    SDL_Delay(16);  // ~60fps render cadence, independent of the 100ms tick
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
