#include <iostream>
#include <memory>

#include <espelho.h>
#include <position.h>

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Error: Incorrect number of args provided. You need to "
                 "provide a port number"
              << std::endl;
    return 1;
  }

  Espelho espelho(*argv[1]);
  std::vector<std::unique_ptr<Replicable>> objects = {
      std::make_unique<Replicable>(Position(1, 1))};

  while (true) {
    espelho.SendObjects(objects);
    espelho.Update();
  }
}
