#include "PCH.h"
#include "SwitchHook.h"
#include "PoseTrack.h"
#include "ShieldState.h"

namespace LHEO::SwitchHook {
namespace {

using PoseTrack::Output;

constexpr const char* kSwitchName = "DualWieldShieldBone_BoneSwitchGen";

// The context's character is the graph's characterInstance member; step back to the graph.
static_assert(offsetof(RE::BShkbAnimationGraph, characterInstance) == 0xC0);
RE::BShkbAnimationGraph* GraphOf(const RE::hkbContext& a_ctx) {
    if (!a_ctx.character) return nullptr;
    return reinterpret_cast<RE::BShkbAnimationGraph*>(
        reinterpret_cast<std::byte*>(a_ctx.character) - offsetof(RE::BShkbAnimationGraph, characterInstance));
}

// Node pointer -> is it the DW switch, decided once by name. The name's string pointer is kept with it:
// a node freed with its project and another allocated at the same address has another name string,
// so a pointer comparison (no string work per frame) sends it back to the name check.
struct NodeInfo {
    const char* name;
    bool shieldSwitch;
};
std::shared_mutex g_nodeLock;
std::unordered_map<const void*, NodeInfo> g_nodes;

bool IsShieldSwitch(const RE::hkbGenerator* a_node) {
    const char* name = a_node->name.c_str();
    {
        std::shared_lock guard(g_nodeLock);
        if (const auto it = g_nodes.find(a_node); it != g_nodes.end() && it->second.name == name)
            return it->second.shieldSwitch;
    }
    const bool match = name && _stricmp(name, kSwitchName) == 0;
    std::unique_lock guard(g_nodeLock);
    g_nodes.insert_or_assign(a_node, NodeInfo{name, match});
    return match;
}

// Skeleton -> Shield bone index and its reference transform. Kept with the bone array's address and
// size, checked on every use like the node names above.
struct ShieldBone {
    int index = -1;
    PoseTrack::Qs ref{};
    const void* bones = nullptr;
    int count = 0;
};
std::shared_mutex g_skeletonLock;
std::unordered_map<const void*, ShieldBone> g_skeletons;

ShieldBone ShieldOf(const RE::hkbContext& a_ctx) {
    const auto* setup = a_ctx.character ? a_ctx.character->setup.get() : nullptr;
    const auto* skeleton = setup ? setup->animationSkeleton.get() : nullptr;
    if (!skeleton) return {};
    {
        std::shared_lock guard(g_skeletonLock);
        if (const auto it = g_skeletons.find(skeleton); it != g_skeletons.end() &&
                                                        it->second.bones == skeleton->bones._data &&
                                                        it->second.count == skeleton->bones._size)
            return it->second;
    }
    ShieldBone bone{-1, {}, skeleton->bones._data, skeleton->bones._size};
    for (int i = 0; i < skeleton->bones._size && i < skeleton->referencePose._size; ++i) {
        const char* name = skeleton->bones._data[i].name.c_str();
        if (name && _stricmp(name, "Shield") == 0) {
            bone.index = i;
            std::memcpy(bone.ref.data(), &skeleton->referencePose._data[i], sizeof(PoseTrack::Qs));
            break;
        }
    }
    std::unique_lock guard(g_skeletonLock);
    g_skeletons.insert_or_assign(skeleton, bone);
    return bone;
}

// Quaternions x, y, z, w.
std::array<float, 4> Mul(const float* a, const float* b) {
    return {a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1], a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
        a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3], a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2]};
}
std::array<float, 4> Conj(const float* q) { return {-q[0], -q[1], -q[2], q[3]}; }

struct Generate {
    static void thunk(RE::hkbGenerator* a_node, const RE::hkbContext& a_ctx, const Output** a_children, Output& a_out,
        float a_offset) {
        func(a_node, a_ctx, a_children, a_out, a_offset);
        if (!ShieldState::AnyActive() || !IsShieldSwitch(a_node) || !a_children || !a_children[0]) return;
        // Child 0 is the switch's default generator (the animation), child 1 the adjustment clip;
        // the output holds the adjustment's Shield (investigated in game 2026-11-21).
        const auto shield = ShieldOf(a_ctx);
        if (shield.index < 0) return;
        auto* graph = GraphOf(a_ctx);
        auto* actor = graph ? graph->holder : nullptr;
        if (!actor) return;
        const auto mode = ShieldState::Get(graph, actor->GetFormID());
        if (mode == ShieldState::Mode::Vanilla) return;
        const float* anim = PoseTrack::Bone(*a_children[0], shield.index);
        float* out = PoseTrack::Bone(a_out, shield.index);
        if (!anim || !out) return;
        if (mode == ShieldState::Mode::Free) {
            std::memcpy(out, anim, sizeof(PoseTrack::Qs));
            return;
        }
        // Additive, in the parent frame on top of the adjustment now in the output.
        const auto& ref = shield.ref;
        PoseTrack::Qs result;
        std::memcpy(result.data(), anim, sizeof(result));
        for (int i = 0; i < 3; ++i) result[i] = out[i] + (anim[i] - ref[i]);
        const auto delta = Mul(anim + 4, Conj(ref.data() + 4).data());
        auto q = Mul(delta.data(), out + 4);
        const float len = std::sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
        if (!(len > 1e-6f)) return;
        for (int i = 0; i < 4; ++i) result[4 + i] = q[i] / len;
        std::memcpy(out, result.data(), sizeof(result));
    }
    static inline REL::Relocation<decltype(thunk)> func;
};

}  // namespace

void Install() {
    REL::Relocation<std::uintptr_t> vtable{RE::VTABLE_BSBoneSwitchGenerator[0]};
    Generate::func = vtable.write_vfunc(0x17, Generate::thunk);
    logger::info("bone switch generate hook installed");
}

}  // namespace LHEO::SwitchHook
