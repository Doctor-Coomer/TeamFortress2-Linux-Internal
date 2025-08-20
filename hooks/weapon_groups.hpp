#ifndef WEAPON_GROUPS_HPP
#define WEAPON_GROUPS_HPP

#include <array>
#include "../classes/weapon.hpp"

namespace weapon_groups {

// this is stupid but i dont wanna find vtable for weapon groups xd

constexpr std::array<int, 14> melee_type_ids = {
  TF_WEAPON_BAT,
  TF_WEAPON_BAT_WOOD,
  TF_WEAPON_BOTTLE,
  TF_WEAPON_FIREAXE,
  TF_WEAPON_CLUB,
  TF_WEAPON_CROWBAR,
  TF_WEAPON_KNIFE,
  TF_WEAPON_FISTS,
  TF_WEAPON_SHOVEL,
  TF_WEAPON_WRENCH,
  TF_WEAPON_BONESAW,
  TF_WEAPON_SWORD,
  TF_WEAPON_BAT_FISH,
  TF_WEAPON_BAT_GIFTWRAP,
};

constexpr std::array<int, 11> projectile_type_ids = {
  TF_WEAPON_ROCKETLAUNCHER,
  TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
  TF_WEAPON_GRENADELAUNCHER,
  TF_WEAPON_PIPEBOMBLAUNCHER,
  TF_WEAPON_FLAREGUN,
  TF_WEAPON_COMPOUND_BOW,
  TF_WEAPON_CANNON,
  TF_WEAPON_CROSSBOW,
  TF_WEAPON_RAYGUN,
  TF_WEAPON_PARTICLE_CANNON,
  TF_WEAPON_DRG_POMSON,
};

constexpr std::array<int, 3> sniper_rifle_type_ids = {
  TF_WEAPON_SNIPERRIFLE,
  TF_WEAPON_SNIPERRIFLE_CLASSIC,
  TF_WEAPON_SNIPERRIFLE_DECAP,
};

constexpr std::array<int, 5> automatic_type_ids = {
  TF_WEAPON_PISTOL,
  TF_WEAPON_PISTOL_SCOUT,
  TF_WEAPON_SMG,
  TF_WEAPON_MINIGUN,
  TF_WEAPON_CHARGED_SMG,
};

constexpr std::array<int, 53> melee_def_ids_fallback = {
  Misc_t_FryingPan,                  // 264
  Misc_t_GoldFryingPan,              // 1071
  Misc_t_Saxxy,                      // 423
  Scout_t_TheConscientiousObjector,  // 474 (multi-class)
  Scout_t_TheFreedomStaff,           // 880 (multi-class)
  Scout_t_TheBatOuttaHell,           // 939 (multi-class)
  Scout_t_TheMemoryMaker,            // 954 (multi-class)
  Scout_t_TheHamShank,               // 1013 (multi-class)
  Scout_t_TheNecroSmasher,           // 1123 (multi-class)
  Scout_t_TheCrossingGuard,          // 1127 (multi-class)
  Scout_t_UnarmedCombat,             // 572
  Scout_t_Batsaber,                  // 30667
  Pyro_t_HotHand,                    // 1181
  // Scout melee
  Scout_t_Bat,                       // 0
  Scout_t_BatR,                      // 190
  Scout_t_TheSandman,                // 44
  Scout_t_TheHolyMackerel,           // 221
  Scout_t_TheCandyCane,              // 317
  Scout_t_TheBostonBasher,           // 325
  Scout_t_SunonaStick,               // 349
  Scout_t_TheFanOWar,                // 355
  Scout_t_TheAtomizer,               // 450
  Scout_t_ThreeRuneBlade,            // 452
  Scout_t_TheWrapAssassin,           // 648
  Scout_t_FestiveBat,                // 660
  Scout_t_FestiveHolyMackerel,       // 999
  // Soldier melee
  Soldier_t_Shovel,                  // 6
  Soldier_t_ShovelR,                 // 196
  Soldier_t_TheEqualizer,            // 128
  Soldier_t_ThePainTrain,            // 154
  Soldier_t_TheHalfZatoichi,         // 357
  Soldier_t_TheMarketGardener,       // 416
  Soldier_t_TheDisciplinaryAction,   // 447
  Soldier_t_TheEscapePlan,           // 775
  // Pyro melee
  Pyro_t_FireAxe,                    // 2
  Pyro_t_FireAxeR,                   // 192
  Pyro_t_TheAxtinguisher,            // 38
  Pyro_t_Homewrecker,                // 153
  Pyro_t_ThePowerjack,               // 214
  Pyro_t_TheBackScratcher,           // 326
  Pyro_t_SharpenedVolcanoFragment,   // 348
  Pyro_t_ThePostalPummeler,          // 457
  Pyro_t_TheMaul,                    // 466
  Pyro_t_TheThirdDegree,             // 593
  Pyro_t_TheLollichop,               // 739
  Pyro_t_NeonAnnihilator,            // 813
  Pyro_t_NeonAnnihilatorG,           // 834
  Pyro_t_TheFestiveAxtinguisher,     // 1000
  // Sniper melee (unique to Sniper; multi-class items already included via Scout_ enums)
  Sniper_t_Kukri,                    // 3
  Sniper_t_KukriR,                   // 193
  Sniper_t_TheTribalmansShiv,        // 171
  Sniper_t_TheBushwacka,             // 232
  Sniper_t_TheShahanshah,            // 401
};

constexpr std::array<int, 10> spy_knife_def_ids = {
  Spy_t_Knife,
  Spy_t_KnifeR,
  Spy_t_YourEternalReward,
  Spy_t_ConniversKunai,
  Spy_t_TheBigEarner,
  Spy_t_TheWangaPrick,
  Spy_t_TheSharpDresser,
  Spy_t_TheSpycicle,
  Spy_t_FestiveKnife,
  Spy_t_TheBlackRose,
};

template <typename T, size_t N>
inline bool contains(const std::array<T, N>& arr, const T& v) {
  for (const auto& e : arr) if (e == v) return true;
  return false;
}

inline bool is_melee_type_id(int t) { return contains(melee_type_ids, t); }

inline bool is_projectile_type_id(int t) { return contains(projectile_type_ids, t); }

inline bool is_sniper_rifle_type_id(int t) { return contains(sniper_rifle_type_ids, t); }

inline bool is_automatic_type_id(int t) { return contains(automatic_type_ids, t); }

inline bool is_melee_def_id(int d) { return contains(melee_def_ids_fallback, d); }

inline bool is_spy_knife_def_id(int d) { return contains(spy_knife_def_ids, d); }

inline bool is_knife_type_id(int t) { return t == TF_WEAPON_KNIFE; }

} // namespace weapon_groups

#endif // WEAPON_GROUPS_HPP
