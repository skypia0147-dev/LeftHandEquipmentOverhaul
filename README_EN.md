# Left Hand Equipment Overhaul 2.0

[한국어](README.md)

With a one-handed weapon in the left hand, vanilla Skyrim overrides the Shield bone with `DW_ShieldAdjustment.hkx`, so an animation that moves the Shield bone never moves the left-hand weapon. This mod turns that override off and on with animation annotations.

Since 2.0 it is a single SKSE plugin. The ESP, Dynamic Animation Casting, Behavior Data Injector and the 0-byte `DW_ShieldAdjustment.hkx` are no longer needed. Nemesis and Pandora are not needed either.

## Requirements

- SKSE
- Address Library for SKSE Plugins
- Skyrim SE 1.5.97 / AE 1.6.x / 1.7.x

## Annotations for animators

| Annotation | Effect |
|---|---|
| `Adjustment_On` | Turns the adjustment off: the animation's Shield bone is shown as is. Bake the flip into the Shield bone yourself so the left-hand weapon does not look upside down (same meaning as 1.x) |
| `Adjustment_Additive` | Keeps the adjustment and **adds** the Shield bone's movement and rotation on top. Animate from the reference pose; no flip needed |
| `Adjustment_Off` | Back to the vanilla adjustment |
| `DisableShieldAdjustment` / `EnableShieldAdjustment` | Dual Wielding Unlocker's annotations, same as `Adjustment_On` / `Adjustment_Off` |

Example:

```
0.000000 Adjustment_On
0.666666 Adjustment_Off
```

- An annotation applies to the graph that sent it (third or first person).
- Back to the vanilla adjustment on: `Adjustment_Off` / `EnableShieldAdjustment`, sheathing (`weaponSheathe`), ragdoll, death, load and new game.

## Updating from 1.x

1. Remove 1.x and install 2.0.
2. `Left Hand Equipment Overhaul.esp` is gone. A magic effect a 1.x save still holds stays as an orphan effect and does nothing.

## License

GPL-3.0 (includes CommonLibSSE-NG). See `LICENSE.txt` and `THIRD_PARTY_NOTICES.txt`.
