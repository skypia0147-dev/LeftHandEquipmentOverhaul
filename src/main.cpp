#include "PCH.h"
#include "AnnotationEvents.h"
#include "Resets.h"
#include "ShieldState.h"
#include "SwitchHook.h"

namespace {

void InitializeLog() {
    auto path = logger::log_directory();
    if (!path) return;
    *path /= "LeftHandEquipmentOverhaul.log";
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log = std::make_shared<spdlog::logger>("global", std::move(sink));
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        LHEO::Resets::Install();
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        LHEO::ShieldState::ResetAll();
        break;
    default:
        break;
    }
}

}  // namespace

SKSEPluginInfo(.Version = {2, 0, 0, 0}, .Name = "LeftHandEquipmentOverhaul", .Author = "Smooth",
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    InitializeLog();
    SKSE::Init(a_skse);
    logger::info("Left Hand Equipment Overhaul 2.0.0, runtime {}", REL::Module::get().version().string());
    SKSE::AllocTrampoline(64);
    // Before any behavior project loads: the main menu character loads graphs before kDataLoaded.
    LHEO::AnnotationEvents::InstallRegistration();
    LHEO::AnnotationEvents::InstallReceiver();
    LHEO::SwitchHook::Install();
    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
    return true;
}
