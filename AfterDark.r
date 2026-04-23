// AfterDark.r — resources for the Lumon Industries After Dark module

#include "Types.r"

// The compiled code resource (flat binary produced by the linker).
data 'ADgm' (0, locked) {
    $$read("LumonAD")
};

// Cals: bit flags for which messages this module wants to receive.
//   bit 0 = Initialize, bit 1 = Blank, bit 2 = DrawFrame, bit 3 = Close
data 'Cals' (1) {
    $"0F"
};

// Credit line shown in the After Dark control panel.
resource 'STR ' (128) {
    "Lumon Industries"
};
