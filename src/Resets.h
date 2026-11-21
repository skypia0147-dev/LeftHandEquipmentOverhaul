#pragma once

// Death sends an actor back to the game's adjustment. Load and new game clear every actor (main.cpp);
// sheathing, Adjustment_Off / EnableShieldAdjustment and ragdoll are handled with the annotation events.
// An equipment change does not reset: LHEO's own left-hand swap animations turn the adjustment off at
// their start, and the equip event can arrive after that annotation.
namespace LHEO::Resets {

void Install();

}  // namespace LHEO::Resets
