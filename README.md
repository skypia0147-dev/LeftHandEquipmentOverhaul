# Left Hand Equipment Overhaul 2.0

[English](README_EN.md)

왼손에 한손 무기를 들면 바닐라는 `DW_ShieldAdjustment.hkx`로 Shield 뼈를 덮어써서, 애니메이션이 Shield 뼈를 움직여도 왼손 무기에는 반영되지 않습니다. 이 모드는 애니메이션 주석으로 그 덮어쓰기를 끄고 켭니다.

2.0부터 SKSE 플러그인 하나로 동작합니다. ESP, Dynamic Animation Casting, Behavior Data Injector, 0바이트 `DW_ShieldAdjustment.hkx`가 더는 필요 없습니다. Nemesis·Pandora도 필요 없습니다.

## 요구 사항

- SKSE
- Address Library for SKSE Plugins
- Skyrim SE 1.5.97 / AE 1.6.x / 1.7.x
- 왼손 장비 교체 애니메이션: Open Animation Replacer, Open Animation Replacer - IED Conditions, Immersive Equipment Displays

## 애니메이터용 주석

| 주석 | 하는 일 |
|---|---|
| `Adjustment_On` | 보정을 끔. 애니메이션의 Shield 뼈가 그대로 보입니다. 왼손 무기가 뒤집혀 보이지 않게 Shield 뼈의 뒤집힘을 애니메이션에 직접 넣어야 합니다(1.x와 같은 의미) |
| `Adjustment_Additive` | 보정을 유지한 채 Shield 뼈의 이동·회전을 **더합니다**. 기본 자세 기준으로 만들면 되고 뒤집힘을 넣을 필요가 없습니다 |
| `Adjustment_Off` | 바닐라 보정으로 돌아감 |
| `DisableShieldAdjustment` / `EnableShieldAdjustment` | Dual Wielding Unlocker 주석. `Adjustment_On` / `Adjustment_Off`와 같습니다 |

예시:

```
0.000000 Adjustment_On
0.666666 Adjustment_Off
```

- 주석은 그 주석을 보낸 그래프(3인칭 또는 1인칭)에만 적용됩니다.
- 다음 때 바닐라 보정으로 돌아갑니다: `Adjustment_Off`·`EnableShieldAdjustment`, 무기를 넣을 때(`weaponSheathe`), 래그돌, 죽음, 불러오기·새 게임.

## 1.x에서 올리는 법

1. 1.x를 지우고 2.0을 설치합니다.
2. `Left Hand Equipment Overhaul.esp`는 없어졌습니다. 1.x 세이브에 남은 마법 효과는 주인이 없는 효과로 남지만 동작에는 영향이 없습니다.

## 라이선스

GPL-3.0 (CommonLibSSE-NG 포함). `LICENSE.txt`, `THIRD_PARTY_NOTICES.txt`를 보세요.
