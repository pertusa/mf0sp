// 
// Copyright 1997-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu> (from 18Dec1997) 
// Creation Date: 26 December 1997 
// Last Modified: Tue Apr 18 11:38:28 PDT 2000 (put CH_X defines here)
// Filename:      ...sig/include/sigInfo/gminstruments.h 
// Web Address:   http://sig.sapp.org/include/sigInfo/gminstruments.h
// Syntax:        C 
// 
// Description:   Defines names for instruments as arranged in General MIDI. 
// 

// MODIFIED AND CHANGED TO HEXADECIMAL

#ifndef _GMINSTRUMENTS_H_INCLUDED
#define _GMINSTRUMENTS_H_INCLUDED

#define  CH_1                             0x00
#define  CH_2                             0x01
#define  CH_3                             0x02
#define  CH_4                             0x03
#define  CH_5                             0x04
#define  CH_6                             0x05
#define  CH_7                             0x06
#define  CH_8                             0x07
#define  CH_9                             0x08
#define  CH_10                            0x09
#define  CH_11                            0x0A
#define  CH_12                            0x0B
#define  CH_13                            0x0C
#define  CH_14                            0x0D
#define  CH_15                            0x0E
#define  CH_16                            0x0F

#define  GM_ACOUSTIC_GRAND_PIANO          (0x00)
#define  GM_BRIGHT_ACOUSTIC_PIANO         (0x01)
#define  GM_ELECTRIC_GRAND_PIANO          (0x02)
#define  GM_HONKYTONK_PIANO               (0x03)
#define  GM_ELECTRIC_PIANO_1              (0x04)
#define  GM_ELECTRIC_PIANO_2              (0x05)
#define  GM_HARPSICHORD                   (0x06)
#define  GM_CLAVICHORD                    (0x07)

#define  GM_CELESTA                       (0x08)
#define  GM_GLOCKENSPIEL                  (0x09)
#define  GM_MUSIC_BOX                     (0x0A)
#define  GM_VIBRAPHONE                    (0x0B)
#define  GM_MARIMBA                       (0x0C)
#define  GM_XYLOPHONE                     (0x0D)
#define  GM_TUBULAR_BELLS                 (0x0E)
#define  GM_DULCIMER                      (0x0F)
                                 
#define  GM_DRAWBAR_ORGAN                 (0x10)
#define  GM_PERCUSSIVE_ORGAN              (0x11)
#define  GM_ROCK_ORGAN                    (0x12)
#define  GM_CHURCH_ORGAN                  (0x13)
#define  GM_REED_ORGAN                    (0x14)
#define  GM_ACCORDION                     (0x15)
#define  GM_HARMONICA                     (0x16)
#define  GM_TANGO_ACCORDION               (0x17)

#define  GM_ACOUSTIC_GUITAR_NYLON         (0x18)
#define  GM_ACOUSTIC_GUITAR_STEEL         (0x19)
#define  GM_ELECTRIC_GUITAR_JAZZ          (0x1A)
#define  GM_ELECTRIC_GUITAR_CLEAN         (0x1B)
#define  GM_ELECTRIC_GUITAR_MUTED         (0x1C)
#define  GM_OVERDRIVEN_GUITAR             (0x1D)
#define  GM_DISTORTION_GUITAR             (0x1E)
#define  GM_GUITAR_HARMONICS              (0x1F)
   
#define  GM_ACOUSTIC_BASS                 (0x20)
#define  GM_ELECTRIC_BASS_FINGER          (0x21)
#define  GM_ELECTRIC_BASS_PICK            (0x22)
#define  GM_FRETLESS_BASS                 (0x23)
#define  GM_SLAP_BASS_1                   (0x24)
#define  GM_SLAP_BASS_2                   (0x25)
#define  GM_SYNTH_BASS_1                  (0x26)
#define  GM_SYNTH_BASS_2                  (0x27)
                      
#define  GM_VIOLIN                        (0x28)
#define  GM_VIOLA                         (0x29)
#define  GM_CELLO                         (0x2A)
#define  GM_CONTRABASS                    (0x2B)
#define  GM_TREMOLO_STRINGS               (0x2C)
#define  GM_PIZZACATO_STRINGS             (0x2D)
#define  GM_ORCHESTRAL_HARP               (0x2E)
#define  GM_TIMPANI                       (0x2F)
                         
#define  GM_STRING_ENSEMBLE_1             (0x30)
#define  GM_STRING_ENSEMBLE_2             (0x31)
#define  GM_SYNTHSTRINGS_1                (0x32)
#define  GM_SYNTHSTRINGS_2                (0x33)
#define  GM_CHOIR_AAHS                    (0x34)
#define  GM_VOICE_OOHS                    (0x35)
#define  GM_SYNTH_VOICE                   (0x36)
#define  GM_ORCHESTRA_HIT                 (0x37)
                          
#define  GM_TRUMPET                       (0x38)
#define  GM_TROMBONE                      (0x39)
#define  GM_TUBA                          (0x3A)
#define  GM_MUTED_TRUMPED                 (0x3B)
#define  GM_FRENCH_HORN                   (0x3C)
#define  GM_BRASS_SECTION                 (0x3D)
#define  GM_SYNTHBRASS_1                  (0x3E)
#define  GM_SYNTHBRASS_2                  (0x3F)
                           
#define  GM_SOPRANO_SAX                   (0x40)
#define  GM_ALTO_SAX                      (0x41)
#define  GM_TENOR_SAX                     (0x42)
#define  GM_BARITONE_SAX                  (0x43)
#define  GM_OBOE                          (0x44)
#define  GM_ENGLISH_HORN                  (0x45)
#define  GM_BASSOON                       (0x46)
#define  GM_CLARINET                      (0x47)
                            
#define  GM_PICCOLO                       (0x48)
#define  GM_FLUTE                         (0x49)
#define  GM_RECORDER                      (0x4A)
#define  GM_PAN_FLUTE                     (0x4B)
#define  GM_BLOWN_BOTTLE                  (0x4C)
#define  GM_SHAKUHACHI                    (0x4D)
#define  GM_WHISTLE                       (0x4E)
#define  GM_OCARINA                       (0x4F)
                             
#define  GM_LEAD_SQUARE                   (0x50)
#define  GM_LEAD_SAWTOOTH                 (0x51)
#define  GM_LEAD_CALLIOPE                 (0x52)
#define  GM_LEAD_CHIFF                    (0x53)
#define  GM_LEAD_CHARANG                  (0x54)
#define  GM_LEAD_VOICE                    (0x55)
#define  GM_LEAD_FIFTHS                   (0x56)
#define  GM_LEAD_BASS                     (0x57)
                              
#define  GM_PAD_NEW_AGE                   (0x58)
#define  GM_PAD_WARM                      (0x59)
#define  GM_PAD_POLYSYNTH                 (0x5A)
#define  GM_PAD_CHOIR                     (0x5B)
#define  GM_PAD_BOWED                     (0x5C)
#define  GM_PAD_METALLIC                  (0x5D)
#define  GM_PAD_HALO                      (0x5E)
#define  GM_PAD_SWEEP                     (0x5F)
                               
#define  GM_FX_TRAIN                      (0x60)
#define  GM_FX_SOUNDTRACK                 (0x61)
#define  GM_FX_CRYSTAL                    (0x62)
#define  GM_FX_ATMOSPHERE                 (0x63)
#define  GM_FX_BRIGHTNESS                 (0x64)
#define  GM_FX_GOBLINS                    (0x65)
#define  GM_FX_ECHOES                     (0x66)
#define  GM_FX_SCI_FI                     (0x67)
                                
#define  GM_SITAR                         (0x68)
#define  GM_BANJO                         (0x69)
#define  GM_SHAMISEN                      (0x6A)
#define  GM_KOTO                          (0x6B)
#define  GM_KALIMBA                       (0x6C)
#define  GM_BAGPIPE                       (0x6D)
#define  GM_FIDDLE                        (0x6E)
#define  GM_SHANAI                        (0x6F)
                                 
#define  GM_TINKLE_BELL                   (0x70)
#define  GM_AGOGO                         (0x71)
#define  GM_STEEL_DRUMS                   (0x72)
#define  GM_WOODBLOCKS                    (0x73)
#define  GM_TAIKO_DRUM                    (0x74)
#define  GM_MELODIC_DRUM                  (0x75)
#define  GM_SYNTH_DRUM                    (0x76)
#define  GM_REVERSE_CYMBAL                (0x77)
                                  
#define  GM_GUITAR_FRET_NOISE             (0x78)
#define  GM_BREATH_NOISE                  (0x79)
#define  GM_SEASHORE                      (0x7A)
#define  GM_BIRD_TWEET                    (0x7B)
#define  GM_TELEPHONE_RING                (0x7C)
#define  GM_HELICOPTER                    (0x7D)
#define  GM_APPLAUSE                      (0x7E)
#define  GM_GUNSHOT                       (0x7F)

//
// Percussion instruments on channel 10
//

#define  GM_ACOUSTIC_BASS_DRUM            (0x23)
#define  GM_BASS_DRUM_1                   (0x24)
#define  GM_SIDE_STICK                    (0x25)
#define  GM_ACOUSTIC_SNARE                (0x26)
#define  GM_HAND_CLAP                     (0x27)
#define  GM_ELECTRIC_SNARE                (0x28)
#define  GM_LOW_FLOOR_TOM                 (0x29)
#define  GM_CLOSED_HI_HAT                 (0x2A)
#define  GM_HIGH_FLOOR_TOM                (0x2B)
#define  GM_PEDAL_HI_HAT                  (0x2C)
#define  GM_LOW_TOM                       (0x2D)
#define  GM_OPEN_HI_HAT                   (0x2E)
#define  GM_LOW_MID_TOM                   (0x2F)
#define  GM_HIGH_MID_TOM                  (0x30)
#define  GM_CRASH_CYMBAL_1                (0x31)
#define  GM_HIGH_TOM                      (0x32)
#define  GM_RIDE_CYMBAL_1                 (0x33)
#define  GM_CHINESE_CYMBAL                (0x34)
#define  GM_RIDE_BELL                     (0x35)
#define  GM_TAMBOURINE                    (0x36)
#define  GM_SPLASH_CYMBAL                 (0x37)
#define  GM_COWBELL                       (0x38)
#define  GM_CRASH_CYMBAL_2                (0x39)
#define  GM_VIBRASLAP                     (0x3A)
#define  GM_RIDE_CYMBAL_2                 (0x3B)
#define  GM_HI_BONGO                      (0x3C)
#define  GM_LOW_BONGO                     (0x3D)
#define  GM_MUTE_HI_CONGA                 (0x3E)
#define  GM_OPEN_HI_CONGA                 (0x3F)
#define  GM_LOW_CONGA                     (0x40)
#define  GM_HIGH_TIMBALE                  (0x41)
#define  GM_LOW_TIMBALE                   (0x42)
#define  GM_HIGH_AGOGO                    (0x43)
#define  GM_LOW_AGOGO                     (0x44)
#define  GM_CABASA                        (0x45)
#define  GM_MARACAS                       (0x46)
#define  GM_SHORT_WHISTLE                 (0x47)
#define  GM_LONG_WHISTLE                  (0x48)
#define  GM_SHORT_GUIRO                   (0x49)
#define  GM_LONG_GUIRO                    (0x4A)
#define  GM_CLAVES                        (0x4B)
#define  GM_HI_WOOD_BLOCK                 (0x4C)
#define  GM_LOW_WOOD_BLOCK                (0x4D)
#define  GM_MUTE_CUICA                    (0x4E)
#define  GM_OPEN_CUICA                    (0x4F)
#define  GM_MUTE_TRIANGLE                 (0x50)
#define  GM_OPEN_TRIANGLE                 (0x51)


#endif  /* _GMINSTRUMENTS_H_INCLUDED */

