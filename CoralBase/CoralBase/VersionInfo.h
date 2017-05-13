// $Id: VersionInfo.h,v 1.10.2.2 2012/01/25 10:59:20 avalassi Exp $
#ifndef CORALBASE_VERSIONINFO_H
#define CORALBASE_VERSIONINFO_H 1

// This switch is now hardcoded in the two branches of the code
// tagged as CORAL-preview and CORAL_2_3-patches (bug #89707).
//#define CORAL240 1 // CORAL 2.4.x
#undef CORAL240      // CORAL 2.3.x

#ifdef CORAL240
//---------------------------------------------------------------------------
// CORAL-preview (CORAL 2.4.x releases)
// Enable some extensions but do not undef the others (allow explicit -D)
//---------------------------------------------------------------------------
#define CORAL_VERSIONINFO_RELEASE_MAJOR 2
#define CORAL_VERSIONINFO_RELEASE_MINOR 4
#define CORAL_VERSIONINFO_RELEASE_PATCH 0
#define CORAL240PM 1 // API switch IPropertyManager struct/class (bug #63198)
#define CORAL240SQ 1 // API extensions for sequences (bug #61558)
//#define CORAL240CN // API extensions for change notifications (task #14700)
//#define CORAL240DC // API extensions for delete cascade (sr #115178)
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
// CORAL_2_3-patches (CORAL 2.3.x releases)
// Disable all extensions (do not allow explicit -D to enable them)
//---------------------------------------------------------------------------
#define CORAL_VERSIONINFO_RELEASE_MAJOR 2
#define CORAL_VERSIONINFO_RELEASE_MINOR 3
#define CORAL_VERSIONINFO_RELEASE_PATCH 21
#undef CORAL240PM // Do undef (do not leave the option to -D this explicitly)
#undef CORAL240SQ // Do undef (do not leave the option to -D this explicitly)
#undef CORAL240CN // Do undef (do not leave the option to -D this explicitly)
#undef CORAL240DC // Do undef (do not leave the option to -D this explicitly)
//---------------------------------------------------------------------------
#endif

// CORAL_VERSIONINFO_RELEASE[_x] are #defined as of CORAL 2.3.13 (task #17431)
#define CORAL_VERSIONINFO_RELEASE CORAL_VERSIONINFO_RELEASE_MAJOR.CORAL_VERSIONINFO_RELEASE_MINOR.CORAL_VERSIONINFO_RELEASE_PATCH

#endif // CORALBASE_VERSIONINFO_H
