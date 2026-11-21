#include "PCH.h"
#include "Resets.h"
#include "ShieldState.h"

namespace LHEO::Resets {
namespace {

class Sink : public RE::BSTEventSink<RE::TESDeathEvent> {
public:
    static Sink* Get() {
        static Sink sink;
        return &sink;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override {
        if (a_event && a_event->actorDying && ShieldState::AnyActive())
            ShieldState::ResetActor(a_event->actorDying->GetFormID());
        return RE::BSEventNotifyControl::kContinue;
    }
};

}  // namespace

void Install() {
    auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
    if (!holder) {
        logger::error("no script event source holder; death reset not installed");
        return;
    }
    holder->AddEventSink<RE::TESDeathEvent>(Sink::Get());
    logger::info("death reset installed");
}

}  // namespace LHEO::Resets
