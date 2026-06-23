#include <iostream>
#include <world/world.hpp>

struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };

int main() {
    ecs::world world;
    world.register_component<Position>("Position");
    world.register_component<Velocity>("Velocity");
    world.register_component<Health>("Health", ecs::storage_type::sparse_set);

    auto e1 = world.spawn();
    auto e2 = world.spawn();

    // Table 组件：原型迁移
    world.add_component<Position>(e1, {1, 2});
    world.add_component<Velocity>(e1, {3, 4});
    world.add_component<Velocity>(e2, {5, 6});
    world.add_component<Position>(e2, {7, 8});  // 复用原型

    // SparseSet 组件：不改变原型
    world.set_sparse<Health>(e1, Health{100});

    auto print = [&](ecs::entity e, const char* name) {
        auto* p = world.get_component<Position>(e);
        auto* v = world.get_component<Velocity>(e);
        auto* h = world.get_sparse<Health>(e);
        std::cout << name << ":";
        if (p) std::cout << " Pos={" << p->x << "," << p->y << "}";
        if (v) std::cout << " Vel={" << v->dx << "," << v->dy << "}";
        if (h) std::cout << " HP=" << h->hp;
        std::cout << "\n";
    };

    print(e1, "e1");
    print(e2, "e2");

    // 原地更新
    world.add_component<Position>(e1, {10, 20});
    std::cout << "update: ";
    print(e1, "e1");

    // 移除组件（原型迁移回去）
    world.remove_component<Velocity>(e1);
    std::cout << "remove Vel: ";
    print(e1, "e1");

    // 删除
    world.despawn(e2);
    std::cout << "count: " << world.entity_count() << "\n";
    std::cout << "archetypes: " << world.archetype_count() << "\n";

    return 0;
}
