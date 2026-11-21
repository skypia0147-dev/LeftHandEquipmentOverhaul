#pragma once

// State of the left-hand Shield adjustment (DW_ShieldAdjustment.hkx), per behavior graph.
// The player has a third- and a first-person graph and both generate every frame; their clips differ
// (first-person clips carry the adjustment already), so a mode set by one graph's annotation applies
// to that graph only. An entry also holds its actor's FormID: a graph address reused by another actor
// does not inherit a mode. Written on the main thread (annotation events, equip/death, load), read by
// the bone switch generate hook on animation threads.
namespace LHEO::ShieldState {

enum class Mode : std::uint8_t {
    Vanilla,   // DW_ShieldAdjustment overrides the Shield bone (game default)
    Free,      // the animation's Shield as is (Dual Wielding Unlocker / LHEO 1.x meaning: the flip is baked in)
    Additive   // the animation's Shield authored from the reference pose; the adjustment's flip is added
};

void Set(const RE::BShkbAnimationGraph* a_graph, RE::FormID a_actor, Mode a_mode);
Mode Get(const RE::BShkbAnimationGraph* a_graph, RE::FormID a_actor);
void ResetActor(RE::FormID a_actor);
void ResetAll();
// True while at least one graph is not Vanilla: lets the hook return before any lookup.
bool AnyActive();

const char* Name(Mode a_mode);

}  // namespace LHEO::ShieldState
