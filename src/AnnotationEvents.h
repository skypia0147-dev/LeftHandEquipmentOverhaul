#pragma once

// Annotation events that switch the left-hand Shield adjustment:
//   Free     "Adjustment_On", "DisableShieldAdjustment"
//   Additive "Adjustment_Additive"
//   Vanilla  "Adjustment_Off", "EnableShieldAdjustment", "weaponSheathe"
// The names are added to every character behavior project when its symbol map is built
// (what Behavior Data Injector did for LHEO 1.x), and received through the Character /
// PlayerCharacter animation event sink of every actor.
namespace LHEO::AnnotationEvents {

void InstallRegistration();
void InstallReceiver();

}  // namespace LHEO::AnnotationEvents
