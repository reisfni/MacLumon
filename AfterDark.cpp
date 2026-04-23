// After Dark module entry point for the Lumon Industries MDR screen saver.
// Delegates all drawing to WorkMode.cpp / Logo.cpp via the shared gScreenBounds global.

#include <Types.h>
#include <Quickdraw.h>
#include <Memory.h>
#include <LowMem.h>
extern "C" {
#include <Retro68Runtime.h>
}

#include "Lumon.h"
#include "Logo.h"

// ---------------------------------------------------------------------------
// After Dark API types
// ---------------------------------------------------------------------------

struct MonitorData {
    Rect    bounds;
    Boolean sync;
    short   pixelDepth;
};

struct MonitorsInfo {
    short       monitorCount;
    MonitorData monitorList[1];
};
typedef MonitorsInfo *MonitorsInfoPtr;

struct GMParamBlock {
    short           controlValues[4];
    MonitorsInfoPtr monitors;
    Boolean         colorQDAvail;
    short           systemConfig;
    void           *qdGlobalsCopy;
    short           brightness;
    Rect            demoRect;
    StringPtr       errorMessage;
    void           *sndChannel;
    short           adVersion;
    void           *extensions;
};
typedef GMParamBlock *GMParamBlockPtr;

static const Pattern kBlackPat = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

// ---------------------------------------------------------------------------
// Module entry point
// ---------------------------------------------------------------------------

extern "C" pascal OSErr ADgmEntry(Handle * /*storage*/, RgnHandle /*blankRgn*/,
                                   short message, GMParamBlockPtr params)
{
    RETRO68_RELOCATE();

    switch (message) {

        case 0: /* Initialize */ {
            // Establish screen bounds from the first monitor; fall back to port.
            if (params->monitors && params->monitors->monitorCount > 0) {
                gScreenBounds = params->monitors->monitorList[0].bounds;
            } else {
                GrafPtr port; GetPort(&port);
                gScreenBounds = port->portRect;
            }
            SetupFonts();
            SetupLogos();
            StartWorkMode();
            break;
        }

        case 2: /* Blank */ {
            // Re-read bounds in case the port has been set up now.
            GrafPtr port; GetPort(&port);
            gScreenBounds = port->portRect;

            BackPat(&kBlackPat);
            PenSize(gPenSize, gPenSize);
            PenMode(patBic);
            DrawWorkMode(true);
            break;
        }

        case 3: /* DrawFrame */ {
            AnimateWorkMode();
            break;
        }

        case 1: /* Close */ {
            DisposeLogos();
            break;
        }
    }

    return noErr;
}
