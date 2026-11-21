#include "PCH.h"
#include "AnnotationEvents.h"
#include "ShieldState.h"

namespace LHEO::AnnotationEvents {
namespace {

constexpr const char* kNames[] = {
    "Adjustment_On", "Adjustment_Off", "Adjustment_Additive", "DisableShieldAdjustment", "EnableShieldAdjustment"};

// ---------------------------------------------------------------- registration

// hkbEventInfo is 4 bytes (flags); a new event needs flags 0 or clips never raise it.
static_assert(sizeof(RE::hkbEventInfo) == 4);

int FindName(const RE::hkArray<RE::hkStringPtr>& a_names, const char* a_name) {
    for (int i = 0; i < a_names._size; ++i) {
        const char* s = a_names._data[i].c_str();
        if (s && _stricmp(s, a_name) == 0) return i;
    }
    return -1;
}

void MakeString(RE::hkStringPtr& a_dst, const char* a_text) {
    using Ctor = void (*)(RE::hkStringPtr*, const char*);
    static REL::Relocation<Ctor> ctor{REL::RelocationID(56801, 57231)};
    std::memset(&a_dst, 0, sizeof(a_dst));
    ctor(&a_dst, a_text);
}

// eventNames and eventInfos are parallel arrays; both grow by one or neither does.
bool AddEvent(RE::hkbBehaviorGraph* a_graph, const char* a_name) {
    auto* data = a_graph ? a_graph->data.get() : nullptr;
    if (!data || !data->stringData.get()) return false;
    auto& names = data->stringData->eventNames;
    if (!names._data || FindName(names, a_name) >= 0) return false;
    auto& infos = data->eventInfos;
    if (infos._size != names._size) {
        logger::warn("eventInfos ({}) != eventNames ({}); '{}' not added", infos._size, names._size, a_name);
        return false;
    }
    if (names._size >= names.capacity()) names.reserve(names._size + 8);
    if (names._size >= names.capacity()) return false;
    infos.push_back(RE::hkbEventInfo{});
    MakeString(names._data[names._size], a_name);
    names._size++;
    return true;
}

struct SymbolIdMap;
struct SymbolLinker;
using CreateSymbolIdMap = SymbolIdMap* (*)(RE::hkbBehaviorGraph*, const RE::BSScrapArray<RE::hkbBehaviorGraph*>&,
    const char*, RE::hkbCharacter*, SymbolLinker*, SymbolLinker*);
CreateSymbolIdMap g_create = nullptr;

SymbolIdMap* OnCreateSymbolIdMap(RE::hkbBehaviorGraph* a_master, const RE::BSScrapArray<RE::hkbBehaviorGraph*>& a_graphs,
    const char* a_project, RE::hkbCharacter* a_character, SymbolLinker* a_events, SymbolLinker* a_variables) {
    // Once per project, before its maps exist; character projects only (3rd and 1st person).
    const bool fresh = a_master && a_master->data.get() && !a_master->eventIDMap.get() && !a_master->variableIDMap.get();
    if (fresh && a_project && _strnicmp(a_project, "Actors\\Character", 16) == 0) {
        int added = 0;
        for (std::uint32_t i = 0; i < a_graphs.size(); ++i)
            for (const char* name : kNames) added += AddEvent(a_graphs[i], name) ? 1 : 0;
        logger::info("events registered in '{}': {} added over {} graphs", a_project, added, a_graphs.size());
    }
    return g_create(a_master, a_graphs, a_project, a_character, a_events, a_variables);
}

// ---------------------------------------------------------------- receiver

// The graph that raised the event when it is one of the actor's graphs; nullptr otherwise
// (an event another plugin sent without a source).
RE::BShkbAnimationGraph* SourceGraph(RE::Actor* a_actor, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source) {
    if (!a_source) return nullptr;
    auto* graph = static_cast<RE::BShkbAnimationGraph*>(a_source);
    RE::BSAnimationGraphManagerPtr manager;
    if (!a_actor->GetAnimationGraphManager(manager) || !manager) return nullptr;
    for (auto& g : manager->graphs)
        if (g.get() == graph) return graph;
    return nullptr;
}

void SetFromEvent(RE::Actor* a_actor, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source, ShieldState::Mode a_mode,
    const RE::BSFixedString& a_tag) {
    const auto id = a_actor->GetFormID();
    if (auto* graph = SourceGraph(a_actor, a_source)) {
        if (ShieldState::Get(graph, id) == a_mode) return;
        ShieldState::Set(graph, id, a_mode);
        logger::info("{:08X} graph {:X} '{}': {}", id, reinterpret_cast<std::uintptr_t>(graph), a_tag.c_str(),
            ShieldState::Name(a_mode));
        return;
    }
    // No known source: every graph of the actor.
    RE::BSAnimationGraphManagerPtr manager;
    if (!a_actor->GetAnimationGraphManager(manager) || !manager) return;
    for (auto& g : manager->graphs) ShieldState::Set(g.get(), id, a_mode);
    logger::info("{:08X} all graphs '{}': {}", id, a_tag.c_str(), ShieldState::Name(a_mode));
}

void Handle(RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source) {
    if (!a_event || !a_event->holder) return;
    auto* actor = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
    if (!actor) return;
    static const RE::BSFixedString on("Adjustment_On"), off("Adjustment_Off"), additive("Adjustment_Additive"),
        disable("DisableShieldAdjustment"), enable("EnableShieldAdjustment"), sheathe("weaponSheathe");
    const auto& tag = a_event->tag;
    using ShieldState::Mode;
    if (tag == on || tag == disable) {
        SetFromEvent(actor, a_source, Mode::Free, tag);
    } else if (tag == additive) {
        SetFromEvent(actor, a_source, Mode::Additive, tag);
    } else if (tag == off || tag == enable || tag == sheathe) {
        // Back to the game's adjustment in every view of the actor.
        if (!ShieldState::AnyActive()) return;
        ShieldState::ResetActor(actor->GetFormID());
    } else if (ShieldState::AnyActive() && actor->IsInRagdollState()) {
        // Knocked down: getup animations do not carry our annotations.
        ShieldState::ResetActor(actor->GetFormID());
    }
}

template <int N>
struct Sink {
    static RE::BSEventNotifyControl thunk(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
        RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source) {
        Handle(a_event, a_source);
        return func(a_sink, a_event, a_source);
    }
    static inline REL::Relocation<decltype(thunk)> func;
};

}  // namespace

void InstallRegistration() {
    REL::Relocation<std::uintptr_t> function{REL::RelocationID(62640, 63585)};
    const std::uintptr_t site = function.address() + REL::Relocate(0x22A, 0x237);
    // A 1.7.x layout that moved the call: do not overwrite something that is not a call.
    if (*reinterpret_cast<const std::uint8_t*>(site) != 0xE8) {
        logger::error("CreateSymbolIdMap call site is not a call (byte {:02X}); events not registered",
            *reinterpret_cast<const std::uint8_t*>(site));
        return;
    }
    g_create = reinterpret_cast<CreateSymbolIdMap>(SKSE::GetTrampoline().write_call<5>(site, OnCreateSymbolIdMap));
    logger::info("event registration installed");
}

void InstallReceiver() {
    REL::Relocation<std::uintptr_t> npc{RE::VTABLE_Character[2]};
    REL::Relocation<std::uintptr_t> player{RE::VTABLE_PlayerCharacter[2]};
    Sink<0>::func = npc.write_vfunc(0x1, Sink<0>::thunk);
    Sink<1>::func = player.write_vfunc(0x1, Sink<1>::thunk);
    logger::info("animation event receiver installed");
}

}  // namespace LHEO::AnnotationEvents
