#include <iostream>
#include <print>
#include <vector>

#include "core/component_mask.hpp"

struct Position {
	float x = 0, y = 0;
};
struct Velocity {
	float dx = 0, dy = 0;
};
struct Health {
	int hp = 100;
};

int main() {
	using namespace ecs;
	std::println("Position hash: {}", type_hash<Position>());

	std::println("Position hash: {}", type_hash<Position>());

	std::println("Position hash: {}", type_hash("Position"));





}
