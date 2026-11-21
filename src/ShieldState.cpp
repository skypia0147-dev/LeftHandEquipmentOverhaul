#include "PCH.h"
#include "ShieldState.h"

namespace LHEO::ShieldState {
namespace {
struct Entry {
    RE::FormID actor;
    Mode mode;
};
std::shared_mutex g_lock;
// Graph address (identity only, never dereferenced) -> entry; only graphs that are not Vanilla.
std::unordered_map<const void*, Entry> g_graphs;
std::atomic<std::size_t> g_active{0};
}

void Set(const RE::BShkbAnimationGraph* a_graph, RE::FormID a_actor, Mode a_mode) {
    if (!a_graph || !a_actor) return;
    std::unique_lock guard(g_lock);
    if (a_mode == Mode::Vanilla)
        g_graphs.erase(a_graph);
    else
        g_graphs[a_graph] = {a_actor, a_mode};
    g_active.store(g_graphs.size(), std::memory_order_release);
}

Mode Get(const RE::BShkbAnimationGraph* a_graph, RE::FormID a_actor) {
    if (!AnyActive()) return Mode::Vanilla;
    std::shared_lock guard(g_lock);
    const auto it = g_graphs.find(a_graph);
    return it == g_graphs.end() || it->second.actor != a_actor ? Mode::Vanilla : it->second.mode;
}

void ResetActor(RE::FormID a_actor) {
    std::unique_lock guard(g_lock);
    const auto erased = std::erase_if(g_graphs, [&](const auto& e) { return e.second.actor == a_actor; });
    g_active.store(g_graphs.size(), std::memory_order_release);
    if (erased) logger::info("{:08X}: Vanilla ({} graphs)", a_actor, erased);
}

void ResetAll() {
    std::unique_lock guard(g_lock);
    g_graphs.clear();
    g_active.store(0, std::memory_order_release);
}

bool AnyActive() { return g_active.load(std::memory_order_acquire) != 0; }

const char* Name(Mode a_mode) {
    switch (a_mode) {
    case Mode::Free:
        return "Free";
    case Mode::Additive:
        return "Additive";
    default:
        return "Vanilla";
    }
}

}  // namespace LHEO::ShieldState
