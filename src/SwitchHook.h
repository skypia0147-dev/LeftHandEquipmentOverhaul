#pragma once

// DualWieldShieldBone_BoneSwitchGen (BSBoneSwitchGenerator): with a one-handed weapon in the left
// hand it puts DW_ShieldAdjustment.hkx over the Shield bone. After the game's Generate, for an actor
// whose mode is not Vanilla, the Shield bone of the output is rewritten:
//   Free      the default child's Shield (the animation as is)
//   Additive  the animation's Shield moved and turned from the reference pose, applied in the parent
//             frame on top of the adjustment: T = adj.T + (anim.T - ref.T), R = anim.R * ref.R^-1 * adj.R
// The switch node is recognized by name once per node pointer, never by a per-frame string compare.
namespace LHEO::SwitchHook {

void Install();

}  // namespace LHEO::SwitchHook
