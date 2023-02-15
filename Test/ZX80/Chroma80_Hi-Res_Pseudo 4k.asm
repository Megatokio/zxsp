; *****************************************************
; * ZX80 Chroma 80 - 4K ROM - Pseudo Hi-Res Mode Demo *
; *****************************************************
; (c)2023 Paul Farrow, www.fruitcake.plus.com
;
; V1.00  07 JAN 2023  Initial version.
;
; The source code in this file may be freely copied and used without restriction.
;
; ----------------------------------------------------------------------------------------------------

; =====
; Notes
; =====
; ZX80 CPU runs at 3.25MHz.

; ======================================================================================================================================================

; ====================
; Assembler Directives
; ====================

; For use with the TASM assembler.

#define DEFB .BYTE
#define DEFW .WORD
#define DEFM .TEXT
#define DEFS .FILL
#define EQU  .EQU
#define ORG  .ORG
#define END  .END

; Assemble using command line arguments: -80 -fFF -g3 -i -s -a

; ======================================================================================================================================================

; ============
; ROM Routines
; ============

DISP_2                          EQU     $01AD
UNSTACK_Z                       EQU     $06E0
PRINT_CR_2                      EQU     $0720

; ======================================================================================================================================================

; =======================
; Character Set Constants
; =======================

_SPACE                          EQU     $00     ; 0
_QUOTE                          EQU     $01     ; 1
_LEFTBLACK                      EQU     $02     ; 2
_BOTTOMBLACK                    EQU     $03     ; 3
_TOPLEFTBLACK                   EQU     $04     ; 4
_TOPRIGHTBLACK                  EQU     $05     ; 5
_BOTTOMLEFTBLACK                EQU     $06     ; 6
_BOTTOMRIGHTBLACK               EQU     $07     ; 7
_BOTTOMLEFTTOPRIGHT             EQU     $08     ; 8
_CHEQUER                        EQU     $09     ; 9
_TOPWHITEBOTTOMCHEQUER          EQU     $0A     ; 10
_TOPCHEQUERBOTTOMWHITE          EQU     $0B     ; 11
_POUND                          EQU     $0C     ; 12
_DOLLAR                         EQU     $0D     ; 13
_COLON                          EQU     $0E     ; 14
_QUESTIONMARK                   EQU     $0F     ; 15
_OPENBRACKET2                   EQU     $10     ; 16
_CLOSEBRACKET2                  EQU     $11     ; 17
_MINUS2                         EQU     $12     ; 18
_PLUS2                          EQU     $13     ; 19
_ASTERISK2                      EQU     $14     ; 20
_DIVIDE2                        EQU     $15     ; 21
_EQUALS2                        EQU     $16     ; 22
_GREATERTHAN2                   EQU     $17     ; 23
_LESSTHAN2                      EQU     $18     ; 24
_SEMICOLON2                     EQU     $19     ; 25
_COMMA2                         EQU     $1A     ; 26
_FULLSTOP                       EQU     $1B     ; 27
_0                              EQU     $1C     ; 28
_1                              EQU     $1D     ; 29
_2                              EQU     $1E     ; 30
_3                              EQU     $1F     ; 31
_4                              EQU     $20     ; 32
_5                              EQU     $21     ; 33
_6                              EQU     $22     ; 34
_7                              EQU     $23     ; 35
_8                              EQU     $24     ; 36
_9                              EQU     $25     ; 37
_A                              EQU     $26     ; 38
_B                              EQU     $27     ; 39
_C                              EQU     $28     ; 40
_D                              EQU     $29     ; 41
_E                              EQU     $2A     ; 42
_F                              EQU     $2B     ; 43
_G                              EQU     $2C     ; 44
_H                              EQU     $2D     ; 45
_I                              EQU     $2E     ; 46
_J                              EQU     $2F     ; 47
_K                              EQU     $30     ; 48
_L                              EQU     $31     ; 49
_M                              EQU     $32     ; 50
_N                              EQU     $33     ; 51
_O                              EQU     $34     ; 52
_P                              EQU     $35     ; 53
_Q                              EQU     $36     ; 54
_R                              EQU     $37     ; 55
_S                              EQU     $38     ; 56
_T                              EQU     $39     ; 57
_U                              EQU     $3A     ; 58
_V                              EQU     $3B     ; 59
_W                              EQU     $3C     ; 60
_X                              EQU     $3D     ; 61
_Y                              EQU     $3E     ; 62
_Z                              EQU     $3F     ; 63
_NEWLINE                        EQU     $76     ; 118
_BLACK                          EQU     $80     ; 128
_INVQUOTE                       EQU     $81     ; 129
_RIGHTBLACK                     EQU     $82     ; 130
_TOPBLACK                       EQU     $83     ; 131
_TOPLEFTWHITE                   EQU     $84     ; 132
_TOPRIGHTWHITE                  EQU     $85     ; 133
_BOTTOMLEFTWHITE                EQU     $86     ; 134
_BOTTOMRIGHTWHITE               EQU     $87     ; 135
_TOPLEFTBOTTOMRIGHT             EQU     $88     ; 136
_INVCHEQUER                     EQU     $89     ; 137
_TOPBLACKBOTTOMCHEQUER          EQU     $8A     ; 138
_TOPCHEQUERBOTTOMBLACK          EQU     $8B     ; 139
_INVPOUND                       EQU     $8C     ; 140
_INVDOLLAR                      EQU     $8D     ; 141
_INVCOLON                       EQU     $8E     ; 142
_INVQUESTIONMARK                EQU     $8F     ; 143
_INVOPENBRACKET                 EQU     $90     ; 144
_INVCLOSEBRACKET                EQU     $91     ; 145
_INVMINUS                       EQU     $92     ; 146
_INVPLUS                        EQU     $93     ; 147
_INVASTERISK                    EQU     $94     ; 148
_INVDIVIDE                      EQU     $95     ; 149
_INVEQUALS                      EQU     $96     ; 150
_INVGREATERTHAN                 EQU     $97     ; 151
_INVLESSTHAN                    EQU     $98     ; 152
_INVSEMICOLON                   EQU     $99     ; 153
_INVCOMMA                       EQU     $9A     ; 154
_INVFULLSTOP                    EQU     $9B     ; 155
_INV0                           EQU     $9C     ; 156
_INV1                           EQU     $9D     ; 157
_INV2                           EQU     $9E     ; 158
_INV3                           EQU     $9F     ; 159
_INV4                           EQU     $A0     ; 160
_INV5                           EQU     $A1     ; 161
_INV6                           EQU     $A2     ; 162
_INV7                           EQU     $A3     ; 163
_INV8                           EQU     $A4     ; 164
_INV9                           EQU     $A5     ; 165
_INVA                           EQU     $A6     ; 166
_INVB                           EQU     $A7     ; 167
_INVC                           EQU     $A8     ; 168
_INVD                           EQU     $A9     ; 169
_INVE                           EQU     $AA     ; 170
_INVF                           EQU     $AB     ; 171
_INVG                           EQU     $AC     ; 172
_INVH                           EQU     $AD     ; 173
_INVI                           EQU     $AE     ; 174
_INVJ                           EQU     $AF     ; 175
_INVK                           EQU     $B0     ; 176
_INVL                           EQU     $B1     ; 177
_INVM                           EQU     $B2     ; 178
_INVN                           EQU     $B3     ; 179
_INVO                           EQU     $B4     ; 180
_INVP                           EQU     $B5     ; 181
_INVQ                           EQU     $B6     ; 182
_INVR                           EQU     $B7     ; 183
_INVS                           EQU     $B8     ; 184
_INVT                           EQU     $B9     ; 185
_INVU                           EQU     $BA     ; 186
_INVV                           EQU     $BB     ; 187
_INVW                           EQU     $BC     ; 188
_INVX                           EQU     $BD     ; 189
_INVY                           EQU     $BE     ; 190
_INVZ                           EQU     $BF     ; 191
_QUOTE2                         EQU     $D4     ; 212
_THEN                           EQU     $D5     ; 213
_TO                             EQU     $D6     ; 214
_SEMICOLON                      EQU     $D7     ; 215
_COMMA                          EQU     $D8     ; 216
_CLOSEBRACKET                   EQU     $D9     ; 217
_OPENBRACKET                    EQU     $DA     ; 218
_NOT                            EQU     $DB     ; 219
_MINUS                          EQU     $DC     ; 220
_PLUS                           EQU     $DD     ; 221
_ASTERISK                       EQU     $DE     ; 222
_DIVIDE                         EQU     $DF     ; 223
_AND                            EQU     $E0     ; 224
_OR                             EQU     $E1     ; 225
_DOUBLEASTERISK                 EQU     $E2     ; 226
_EQUALS                         EQU     $E3     ; 227
_GREATERTHAN                    EQU     $E4     ; 228
_LESSTHAN                       EQU     $E5     ; 229
_LIST                           EQU     $E6     ; 230
_RETURN                         EQU     $E7     ; 231
_CLS                            EQU     $E8     ; 232
_DIM                            EQU     $E9     ; 233
_SAVE                           EQU     $EA     ; 234
_FOR                            EQU     $EB     ; 235
_GOTO                           EQU     $EC     ; 236
_POKE                           EQU     $ED     ; 237
_INPUT                          EQU     $EE     ; 238
_RANDOMISE                      EQU     $EF     ; 239
_LET                            EQU     $F0     ; 240
_NEXT                           EQU     $F3     ; 243
_PRINT                          EQU     $F4     ; 244
_NEW                            EQU     $F6     ; 246
_RUN                            EQU     $F7     ; 247
_STOP                           EQU     $F8     ; 248
_CONTINUE                       EQU     $F9     ; 249
_IF                             EQU     $FA     ; 250
_GOSUB                          EQU     $FB     ; 251
_LOAD                           EQU     $FC     ; 252
_CLEAR                          EQU     $FD     ; 253
_REM                            EQU     $FE     ; 254

; ======================================================================================================================================================

; ================
; Chroma Constants
; ================

; -----------------
; Attribute Colours
; -----------------

INK_BLACK                       EQU     $00
INK_BLUE                        EQU     $01
INK_RED                         EQU     $02
INK_GREEN                       EQU     $04
INK_BRIGHT                      EQU     $08
INK_WHITE                       EQU     INK_GREEN + INK_RED + INK_BLUE
INK_YELLOW                      EQU     INK_GREEN + INK_RED
INK_CYAN                        EQU     INK_GREEN + INK_BLUE
INK_MAGENTA                     EQU     INK_RED + INK_BLUE

PAPER_BLACK                     EQU     $00
PAPER_BLUE                      EQU     $10
PAPER_RED                       EQU     $20
PAPER_GREEN                     EQU     $40
PAPER_BRIGHT                    EQU     $80
PAPER_WHITE                     EQU     PAPER_GREEN + PAPER_RED + PAPER_BLUE
PAPER_YELLOW                    EQU     PAPER_GREEN + PAPER_RED
PAPER_CYAN                      EQU     PAPER_GREEN + PAPER_BLUE
PAPER_MAGENTA                   EQU     PAPER_RED + PAPER_BLUE

; --------------
; Border Colours
; --------------

BORDER_BLACK                    EQU     $00
BORDER_BLUE                     EQU     $01
BORDER_RED                      EQU     $02
BORDER_GREEN                    EQU     $04
BORDER_BRIGHT                   EQU     $08
BORDER_WHITE                    EQU     BORDER_GREEN + BORDER_RED + BORDER_BLUE
BORDER_YELLOW                   EQU     BORDER_GREEN + BORDER_RED
BORDER_CYAN                     EQU     BORDER_GREEN + BORDER_BLUE
BORDER_MAGENTA                  EQU     BORDER_RED + BORDER_BLUE

; ---------------------------
; Colour I/O Port Definitions
; ---------------------------

COLOUR_PORT                     EQU     $7FEF
COLOUR80_ENABLED_MASK           EQU     $42
COLOUR80_ENABLED_VALUE          EQU     $02
COLOUR_ENABLE                   EQU     $20
COLOUR_MODE1                    EQU     $10

; ======================================================================================================================================================

; =================
; Program Constants
; =================

MSG_TERMINATOR                  EQU     $40

; ======================================================================================================================================================

; ================
; System Variables
; ================

        ORG  $4000

ERR_NR: DEFB $FF                ;16384          $4000
FLAGS:  DEFB $80                ;16385          $4001
PPC:    DEFW $FFFE              ;16386          $4002
P_PTR:  DEFW DFILE_AREA+$0001   ;16388          $4004
E_PPC:  DEFW $0001              ;16390          $4006
VARS:   DEFW VARS_AREA          ;16392          $4008
E_LINE: DEFW ELINE_AREA         ;16394          $400A
D_FILE: DEFW DFILE_AREA         ;16396          $400C
DF_EA:  DEFW DFILE_AREA+$0001   ;16398          $400E
DF_END: DEFW DFILE_AREA+$0019   ;16400          $4010
DF_SZ:  DEFB $02                ;16402          $4012
S_TOP:  DEFW $0000              ;16403          $4013
X_PTR:  DEFW $0000              ;16405          $4015
OLDPPC: DEFW $0000              ;16407          $4017
FLAGX:  DEFB $00                ;16409          $4019
T_ADDR: DEFW $07B6              ;16410          $401A
SEED:   DEFW $0000              ;16412          $401C
FRAMES: DEFW $0000              ;16414          $401E
DEST:   DEFW DFILE_AREA+$0001   ;16416          $4020
RESULT: DEFW $0000              ;16418          $4022
S_POSN: DEFB $21                ;16420          $4024
        DEFB $17                ;16421          $4025
CH_ADD: DEFW $FFFF              ;16422          $4026

; ======================================================================================================================================================

; =============
; BASIC Program
; =============

        DEFB 10 >> 8
        DEFB 10 & $FF
        DEFB _REM, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _INVC, _INVH, _INVR, _INVO, _INVM, _INVA, _BLACK, _INV8, _INV0, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK
        DEFB _NEWLINE

        DEFB 20 >> 8
        DEFB 20 & $FF
        DEFB _REM, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK, _INVP, _INVS, _INVE, _INVU, _INVD, _INVO, _BLACK, _INVH, _INVI, _INVMINUS, _INVR, _INVE, _INVS, _BLACK, _BLACK, _BLACK, _BLACK, _BLACK
        DEFB _NEWLINE

        DEFB 30 >> 8
        DEFB 30 & $FF
        DEFB _REM
        DEFB _NEWLINE

        DEFB 40 >> 8
        DEFB 40 & $FF
        DEFB _REM, _SPACE, _SPACE, _OPENBRACKET, _C, _CLOSEBRACKET, _2, _0, _2, _3, _SPACE, _P, _A, _U, _L, _SPACE, _F, _A, _R, _R, _O, _W
        DEFB _NEWLINE

        DEFB 50 >> 8
        DEFB 50 & $FF
        DEFB _REM, _SPACE, _W, _W, _W, _FULLSTOP, _F, _R, _U, _I, _T, _C, _A, _K, _E, _FULLSTOP, _P, _L, _U, _S, _FULLSTOP, _C, _O, _M
        DEFB _NEWLINE

        DEFB 60 >> 8
        DEFB 60 & $FF
        DEFB _REM
        DEFB _NEWLINE

        DEFB 70 >> 8
        DEFB 70 & $FF
        DEFB _REM
        DEFB _NEWLINE

        DEFB 80 >> 8
        DEFB 80 & $FF
        DEFB _LET, _A, _EQUALS, _P, _E, _E, _K, _OPENBRACKET, _1, _6, _3, _9, _2, _CLOSEBRACKET
        DEFB _PLUS, _2, _5, _6, _ASTERISK, _P, _E, _E, _K, _OPENBRACKET, _1, _6, _3, _9, _3, _CLOSEBRACKET
        DEFB _MINUS, _3
        DEFB _NEWLINE

        DEFB 90 >> 8
        DEFB 90 & $FF
        DEFB _RANDOMISE, _U, _S, _R, _OPENBRACKET, _A, _CLOSEBRACKET
        DEFB _NEWLINE

        DEFB 100 >> 8
        DEFB 100 & $FF
        DEFB _REM
        DEFB _NEWLINE

        DEFB 110 >> 8
        DEFB 110 & $FF
        DEFB _REM
        DEFB _NEWLINE

        DEFB 120 >> 8
        DEFB 120 & $FF
        DEFB _LIST
        DEFB _NEWLINE
        
; -----------------------------------
; Pretend End of BASIC Listing Marker
; -----------------------------------

        DEFB $80

; ======================================================================================================================================================

; ================
; Border Character
; ================

BORDER: DEFB _NEWLINE                           ; A display file byte used when generating the top and bottom borders.

; ===================
; Hi-Res Display File
; ===================
; The display file consists of 192 lines of 33 bytes, with the last byte being an end-of-line terminator [instruction JP (HL)]. The pixel resolution is 192 x 256.
; There are no constraints on where in the 16K RAM the display file must reside.
; The following encodes an image of the Spectrum game "Alien 8" by Ultimate Play The Game.

HR_DFILE:
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B1, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $01, $A0, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3A, $AC, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $BF, $BC, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $04, $3A, $AC, $07, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $90, $0E, $25, $93, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A8, $9C, $39, $AD, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $8D, $19, $39, $0A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $8D, $19, $39, $0A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $25, $19, $39, $0E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $8E, $1A, $25, $9C, $39, $0E, $1A, $A7, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B1, $1A, $39, $9C, $39, $9C, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B1, $1A, $39, $88, $27, $9C, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $01, $3B, $39, $88, $27, $19, $1A, $A0, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3A, $3B, $39, $31, $13, $19, $1A, $AC, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $BF, $1C, $2E, $31, $13, $2E, $B9, $BC, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $04, $3A, $95, $2E, $07, $10, $2E, $A6, $AC, $07, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $90, $0E, $3F, $39, $3F, $2B, $19, $A4, $25, $93, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A8, $9C, $26, $06, $88, $27, $02, $15, $39, $AD, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $8D, $19, $25, $2C, $9C, $39, $BA, $1F, $39, $0A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $8D, $19, $2E, $13, $19, $39, $31, $2E, $39, $0A, $1A, $1A, $87, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $90, $1A, $1A, $25, $19, $2E, $13, $19, $39, $31, $2E, $39, $0E, $1A, $1A, $00, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $97, $1A, $1A, $25, $9C, $2E, $27, $19, $39, $88, $2E, $39, $0E, $1A, $1C, $15, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $26, $B9, $1A, $39, $9C, $2E, $27, $9C, $39, $88, $2E, $39, $9C, $1A, $9F, $1F, $A5, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $17, $02, $B9, $1A, $39, $88, $2E, $39, $9C, $39, $9C, $2E, $27, $9C, $1A, $00, $25, $87, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $90, $1F, $06, $3B, $39, $88, $2E, $39, $88, $27, $9C, $2E, $27, $19, $1A, $09, $13, $05, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $1A, $15, $13, $0D, $3B, $39, $31, $2E, $39, $88, $27, $19, $2E, $13, $19, $1A, $3F, $1F, $15, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $3B, $26, $25, $2B, $1C, $2E, $31, $2E, $39, $31, $13, $19, $2E, $13, $2E, $B9, $8D, $19, $1F, $A5, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $17, $02, $39, $0A, $95, $2E, $07, $19, $2E, $31, $13, $2E, $39, $10, $2E, $A6, $25, $2E, $25, $87, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $90, $1F, $2E, $0E, $3F, $39, $3F, $1F, $2E, $26, $15, $2E, $25, $2B, $19, $A4, $25, $2E, $13, $A4, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $3F, $13, $2E, $1F, $26, $06, $88, $15, $39, $3F, $2B, $19, $26, $27, $02, $15, $39, $2E, $1F, $15, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $26, $25, $2E, $19, $25, $2C, $9C, $26, $06, $88, $27, $02, $15, $39, $BA, $1F, $39, $2E, $19, $81, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $1A, $12, $39, $2E, $19, $2E, $13, $19, $25, $2C, $9C, $39, $BA, $1F, $39, $31, $2E, $84, $39, $2E, $90, $1A, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $3B, $06, $2E, $19, $84, $2E, $13, $19, $2E, $13, $19, $39, $31, $2E, $39, $31, $2E, $9C, $25, $2E, $16, $B9, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $3B, $88, $2E, $97, $84, $2E, $27, $19, $2E, $13, $19, $39, $31, $2E, $39, $88, $2E, $0E, $13, $2E, $3C, $B9, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $3B, $31, $2E, $13, $02, $2E, $27, $9C, $2E, $27, $19, $39, $88, $2E, $39, $88, $2E, $80, $1F, $2E, $31, $B9, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $9F, $13, $2E, $25, $AC, $2E, $39, $9C, $2E, $27, $9C, $39, $88, $2E, $39, $9C, $2E, $81, $97, $39, $31, $06, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $B1, $13, $19, $25, $12, $2E, $39, $88, $2E, $39, $9C, $39, $9C, $2E, $27, $9C, $2E, $81, $97, $25, $88, $93, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $8E, $27, $97, $25, $12, $2E, $39, $88, $2E, $39, $88, $27, $9C, $2E, $27, $19, $2E, $81, $97, $17, $9C, $A7, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $B1, $39, $A5, $25, $12, $2E, $39, $31, $2E, $39, $88, $27, $19, $2E, $13, $19, $2E, $81, $2B, $3B, $9C, $B9, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $B1, $39, $B9, $AB, $12, $19, $2E, $31, $2E, $39, $31, $13, $19, $2E, $13, $2E, $39, $81, $B1, $3B, $19, $B9, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $01, $39, $B9, $BC, $12, $1F, $2E, $26, $19, $2E, $0A, $8D, $2E, $39, $15, $2E, $25, $81, $A0, $3B, $19, $A0, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $85, $39, $1A, $01, $12, $15, $39, $3F, $1F, $2E, $A5, $17, $2E, $25, $2B, $19, $26, $81, $36, $3B, $19, $AC, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $1A, $BA, $39, $1A, $01, $12, $26, $06, $88, $15, $39, $B9, $3B, $19, $26, $27, $02, $15, $81, $A0, $1A, $19, $2C, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $04, $85, $39, $1A, $01, $12, $25, $2C, $9C, $26, $39, $1A, $1A, $9C, $15, $84, $36, $1F, $81, $36, $1A, $84, $AC, $07, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $0E, $84, $1A, $01, $12, $2E, $9E, $02, $25, $38, $1A, $1A, $09, $1F, $1F, $AB, $2E, $81, $93, $1A, $84, $25, $93, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $A8, $9C, $84, $1A, $93, $12, $2E, $13, $0B, $2E, $A5, $1A, $1A, $17, $2E, $13, $13, $2E, $81, $97, $1A, $84, $39, $AD, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $8D, $19, $84, $1A, $25, $12, $2E, $25, $97, $2E, $B9, $1A, $1A, $3B, $2E, $25, $1F, $2E, $81, $97, $1A, $84, $39, $0A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $8D, $19, $84, $1A, $25, $12, $19, $39, $19, $39, $1A, $1A, $1A, $1A, $19, $39, $19, $39, $81, $97, $1A, $84, $39, $0A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $25, $19, $84, $1A, $25, $12, $1F, $2E, $2E, $90, $1A, $1A, $1A, $1A, $09, $2E, $2E, $25, $81, $97, $1A, $84, $39, $0E, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $8E, $1A, $25, $9C, $84, $1A, $25, $12, $13, $2E, $2E, $A5, $1A, $1A, $1A, $1A, $17, $2E, $2E, $13, $81, $97, $1A, $84, $39, $0E, $1A, $A7, $1A, $1A, $E9
        DEFB $1A, $1A, $B1, $1A, $39, $9C, $84, $1A, $25, $12, $25, $2E, $2E, $97, $1A, $1A, $1A, $1A, $25, $2E, $2E, $1F, $81, $97, $1A, $84, $39, $9C, $1A, $B9, $1A, $1A, $E9
        DEFB $1A, $1A, $B1, $1A, $39, $88, $84, $1A, $25, $12, $39, $2E, $2E, $19, $1A, $1A, $1A, $1A, $39, $2E, $2E, $19, $81, $97, $1A, $84, $27, $9C, $1A, $B9, $1A, $1A, $E9
        DEFB $1A, $1A, $01, $3B, $39, $88, $84, $1A, $25, $12, $25, $2E, $2E, $1F, $1A, $1A, $1A, $3B, $25, $2E, $2E, $97, $81, $97, $1A, $84, $27, $19, $1A, $A0, $1A, $1A, $E9
        DEFB $1A, $1A, $3A, $3B, $39, $31, $84, $1A, $25, $12, $13, $2E, $2E, $13, $A5, $1A, $1A, $17, $13, $2E, $2E, $10, $81, $97, $1A, $84, $13, $19, $1A, $AC, $1A, $1A, $E9
        DEFB $1A, $1A, $BF, $1C, $2E, $31, $84, $1A, $25, $12, $06, $2E, $2E, $25, $97, $1A, $1A, $25, $1F, $2E, $2E, $AC, $81, $97, $1A, $84, $13, $2E, $B9, $BC, $1A, $1A, $E9
        DEFB $1A, $04, $3A, $95, $2E, $07, $84, $1A, $25, $12, $84, $39, $19, $39, $19, $1A, $1A, $39, $19, $39, $19, $92, $81, $97, $1A, $84, $10, $2E, $A6, $AC, $07, $1A, $E9
        DEFB $1A, $90, $0E, $3F, $39, $3F, $84, $1A, $25, $12, $25, $25, $1F, $2E, $2E, $1A, $3B, $2E, $2E, $25, $1F, $B9, $81, $97, $1A, $84, $2B, $19, $A4, $25, $93, $1A, $E9
        DEFB $1A, $A8, $9C, $26, $06, $88, $84, $1A, $25, $12, $27, $13, $13, $2E, $2E, $A5, $17, $2E, $2E, $13, $10, $04, $81, $97, $1A, $84, $27, $02, $15, $39, $AD, $1A, $E9
        DEFB $1A, $8D, $19, $25, $2C, $9C, $84, $1A, $25, $12, $13, $1F, $25, $2E, $2E, $97, $25, $2E, $2E, $1F, $AC, $8C, $81, $97, $1A, $84, $39, $BA, $1F, $39, $0A, $1A, $E9
        DEFB $1A, $8D, $19, $2E, $13, $19, $84, $1A, $25, $12, $27, $1F, $39, $2E, $2E, $19, $39, $2E, $2E, $19, $98, $18, $81, $97, $1A, $84, $39, $31, $2E, $39, $0A, $1A, $E9
        DEFB $1A, $25, $19, $2E, $13, $19, $84, $1A, $25, $12, $27, $9C, $25, $2E, $2E, $1F, $25, $2E, $2E, $97, $BC, $BC, $81, $97, $1A, $84, $39, $31, $2E, $39, $0E, $1A, $E9
        DEFB $1A, $25, $9C, $2E, $27, $19, $84, $1A, $25, $12, $39, $88, $13, $2E, $2E, $13, $13, $2E, $2E, $10, $98, $18, $81, $A5, $1A, $84, $39, $88, $2E, $39, $0E, $1A, $E9
        DEFB $1A, $39, $9C, $2E, $27, $9C, $84, $1A, $17, $12, $39, $88, $06, $2E, $2E, $25, $1F, $2E, $2E, $AC, $AC, $BC, $81, $1A, $1A, $84, $39, $88, $2E, $39, $9C, $1A, $E9
        DEFB $1A, $39, $88, $2E, $39, $9C, $84, $1A, $3B, $12, $39, $31, $84, $39, $19, $39, $19, $39, $19, $92, $05, $98, $2B, $1A, $1A, $84, $39, $9C, $2E, $27, $9C, $1A, $E9
        DEFB $3B, $39, $88, $2E, $39, $88, $84, $1A, $1A, $98, $25, $97, $25, $25, $1F, $2E, $2E, $25, $1F, $B9, $1E, $B9, $A0, $1A, $1A, $84, $27, $9C, $2E, $27, $19, $1A, $E9
        DEFB $3B, $39, $31, $2E, $39, $88, $84, $1A, $1A, $01, $17, $31, $27, $13, $13, $2E, $2E, $13, $10, $04, $05, $A7, $1A, $1A, $1A, $84, $27, $19, $2E, $13, $19, $1A, $E9
        DEFB $1C, $2E, $31, $2E, $39, $31, $84, $1A, $1A, $1A, $3B, $88, $17, $1F, $25, $2E, $2E, $1F, $AC, $BC, $AC, $1A, $1A, $1A, $1A, $84, $13, $19, $2E, $13, $2E, $B9, $E9
        DEFB $95, $2E, $07, $19, $2E, $0A, $84, $1A, $1A, $1A, $1A, $88, $25, $1F, $39, $2E, $2E, $19, $98, $18, $98, $1A, $1A, $1A, $1A, $84, $13, $2E, $39, $10, $2E, $A6, $E9
        DEFB $3F, $39, $3F, $1F, $2E, $8F, $02, $1A, $1A, $1A, $1A, $90, $25, $9C, $25, $2E, $2E, $97, $BC, $BC, $B9, $1A, $1A, $1A, $1A, $1F, $BC, $2E, $25, $2B, $19, $A4, $E9
        DEFB $26, $06, $88, $15, $39, $B8, $0B, $1A, $1A, $1A, $1A, $8E, $25, $88, $13, $2E, $2E, $10, $98, $18, $A7, $1A, $1A, $1A, $1A, $13, $A5, $19, $26, $27, $02, $15, $E9
        DEFB $25, $2C, $9C, $26, $06, $25, $97, $1A, $1A, $1A, $1A, $3B, $39, $88, $06, $2E, $2E, $AC, $AC, $BC, $1A, $1A, $1A, $1A, $1A, $25, $1F, $02, $15, $39, $BA, $1F, $E9
        DEFB $2E, $13, $19, $25, $2C, $39, $19, $1A, $1A, $1A, $1A, $1A, $39, $31, $84, $39, $19, $92, $05, $98, $1A, $1A, $1A, $1A, $1A, $39, $19, $36, $1F, $39, $31, $2E, $E9
        DEFB $2E, $13, $19, $2E, $9E, $2E, $2E, $1A, $1A, $1A, $1A, $1A, $25, $97, $25, $25, $1F, $B9, $1E, $B9, $1A, $1A, $1A, $1A, $3B, $2E, $2E, $AB, $2E, $39, $31, $2E, $E9
        DEFB $2E, $27, $19, $2E, $13, $2E, $2E, $A5, $1A, $1A, $1A, $1A, $17, $31, $27, $13, $10, $04, $05, $A7, $1A, $1A, $1A, $1A, $17, $2E, $2E, $13, $2E, $39, $88, $2E, $E9
        DEFB $2E, $27, $9C, $2E, $25, $2E, $2E, $97, $1A, $1A, $1A, $1A, $3B, $88, $17, $1F, $AC, $BC, $AC, $1A, $1A, $1A, $1A, $1A, $25, $2E, $2E, $1F, $2E, $39, $88, $2E, $E9
        DEFB $2E, $39, $9C, $2E, $39, $2E, $2E, $19, $1A, $1A, $1A, $1A, $1A, $88, $25, $1F, $98, $18, $98, $1A, $1A, $1A, $1A, $1A, $39, $2E, $2E, $19, $2E, $39, $9C, $2E, $E9
        DEFB $2E, $39, $88, $2E, $25, $2E, $2E, $1F, $1A, $1A, $1A, $1A, $1A, $90, $25, $9C, $BC, $BC, $B9, $1A, $1A, $1A, $1A, $3B, $25, $2E, $2E, $97, $2E, $27, $9C, $2E, $E9
        DEFB $2E, $39, $88, $2E, $13, $2E, $2E, $13, $A5, $1A, $1A, $1A, $1A, $8E, $25, $88, $98, $18, $A7, $1A, $1A, $1A, $1A, $17, $13, $2E, $2E, $10, $2E, $27, $19, $2E, $E9
        DEFB $2E, $39, $31, $2E, $09, $2E, $2E, $25, $97, $1A, $1A, $1A, $1A, $3B, $39, $88, $AC, $BC, $1A, $1A, $1A, $1A, $1A, $25, $1F, $2E, $2E, $01, $2E, $13, $19, $2E, $E9
        DEFB $19, $2E, $31, $2E, $02, $39, $19, $39, $19, $1A, $1A, $1A, $1A, $1A, $39, $31, $05, $98, $1A, $1A, $1A, $1A, $1A, $39, $19, $39, $19, $A0, $2E, $13, $2E, $39, $E9
        DEFB $1F, $2E, $26, $19, $27, $25, $1F, $2E, $2E, $1A, $1A, $1A, $1A, $1A, $25, $97, $1E, $B9, $1A, $1A, $1A, $1A, $3B, $2E, $2E, $25, $1F, $BC, $39, $15, $2E, $25, $E9
        DEFB $15, $39, $3F, $1F, $27, $13, $13, $2E, $2E, $A5, $1A, $1A, $1A, $1A, $17, $31, $05, $A7, $1A, $1A, $1A, $1A, $17, $2E, $2E, $13, $10, $04, $25, $2B, $19, $26, $E9
        DEFB $26, $06, $88, $15, $13, $1F, $25, $2E, $2E, $97, $1A, $1A, $1A, $1A, $3B, $88, $AC, $1A, $1A, $1A, $1A, $1A, $25, $2E, $2E, $1F, $AC, $98, $26, $27, $02, $15, $E9
        DEFB $25, $2C, $9C, $26, $25, $1F, $39, $2E, $2E, $19, $1A, $1A, $1A, $1A, $1A, $88, $98, $1A, $1A, $1A, $1A, $1A, $39, $2E, $2E, $19, $98, $18, $15, $39, $BA, $1F, $E9
        DEFB $2E, $13, $19, $25, $27, $9C, $25, $2E, $2E, $1F, $1A, $1A, $1A, $1A, $1A, $90, $B9, $1A, $1A, $1A, $1A, $3B, $25, $2E, $2E, $97, $BC, $BC, $1F, $39, $31, $2E, $E9
        DEFB $2E, $13, $19, $2E, $39, $88, $13, $2E, $2E, $13, $A5, $1A, $1A, $1A, $1A, $8E, $A7, $1A, $1A, $1A, $1A, $17, $13, $2E, $2E, $10, $98, $18, $2E, $39, $31, $2E, $E9
        DEFB $2E, $27, $19, $2E, $39, $88, $06, $2E, $2E, $25, $97, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $25, $1F, $2E, $2E, $AC, $AC, $BC, $2E, $39, $88, $2E, $E9
        DEFB $2E, $27, $9C, $2E, $39, $31, $25, $39, $19, $39, $19, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $39, $19, $39, $19, $92, $05, $98, $2E, $39, $88, $2E, $E9
        DEFB $2E, $39, $9C, $1F, $25, $97, $13, $25, $1F, $2E, $2E, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $97, $1A, $3B, $2E, $2E, $25, $1F, $B9, $1E, $B9, $25, $39, $9C, $2E, $E9
        DEFB $2E, $39, $88, $97, $17, $80, $09, $13, $13, $2E, $2E, $A5, $1A, $1A, $1A, $1A, $1A, $17, $97, $1A, $17, $2E, $2E, $13, $10, $8D, $B1, $A7, $17, $27, $9C, $2E, $E9
        DEFB $2E, $39, $88, $B9, $3B, $81, $05, $1F, $25, $2E, $2E, $97, $1A, $1A, $1A, $1A, $1A, $25, $26, $1A, $25, $2E, $2E, $1F, $AC, $90, $8E, $1A, $3B, $27, $19, $2E, $E9
        DEFB $2E, $39, $31, $1A, $1A, $8C, $15, $90, $39, $2E, $2E, $19, $1A, $1A, $1A, $1A, $1A, $B8, $A6, $1A, $39, $2E, $2E, $19, $A0, $15, $2C, $1A, $1A, $13, $19, $2E, $E9
        DEFB $19, $2E, $0A, $1A, $1A, $0F, $1F, $A5, $25, $2E, $2E, $97, $1A, $1A, $1A, $1A, $1A, $0E, $A4, $B9, $25, $2E, $2E, $1F, $BC, $26, $30, $1A, $1A, $8D, $2E, $39, $E9
        DEFB $1F, $2E, $A5, $1A, $1A, $13, $25, $09, $13, $2E, $2E, $10, $1A, $1A, $1A, $1A, $1A, $9C, $06, $A5, $13, $2E, $2E, $13, $13, $02, $2B, $1A, $1A, $17, $2E, $25, $E9
        DEFB $15, $39, $B9, $1A, $1A, $27, $13, $05, $1F, $2E, $2E, $01, $1A, $1A, $1A, $1A, $3B, $9C, $8D, $1A, $09, $2E, $2E, $25, $90, $1F, $0A, $1A, $1A, $3B, $19, $26, $E9
        DEFB $26, $39, $1A, $1A, $1A, $39, $1F, $15, $02, $39, $19, $A0, $1A, $1A, $1A, $1A, $1A, $19, $05, $1A, $02, $39, $19, $84, $15, $13, $88, $1A, $1A, $1A, $9C, $15, $E9
        DEFB $25, $38, $1A, $1A, $3B, $39, $19, $1F, $13, $25, $1F, $B9, $1A, $1A, $1A, $1A, $1A, $02, $1F, $B9, $25, $25, $1F, $1F, $26, $25, $9C, $1A, $1A, $1A, $09, $1F, $E9
        DEFB $2E, $A5, $1A, $1A, $1A, $2E, $2E, $25, $3F, $13, $10, $A7, $1A, $1A, $1A, $1A, $3B, $05, $13, $1A, $25, $13, $13, $13, $02, $39, $19, $1A, $1A, $1A, $17, $2E, $E9
        DEFB $2E, $B9, $1A, $1A, $3B, $2E, $2E, $13, $A4, $1F, $AC, $B9, $1A, $1A, $1A, $1A, $1A, $15, $02, $B9, $17, $1F, $25, $A4, $1F, $2E, $2E, $1A, $1A, $1A, $3B, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $3B, $2E, $2E, $1F, $AC, $19, $98, $A0, $1A, $1A, $1A, $1A, $3B, $02, $1F, $A5, $25, $1F, $39, $24, $13, $2E, $2E, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $1F, $1A, $1A, $1A, $3B, $1F, $2E, $19, $0A, $97, $BC, $B9, $1A, $1A, $1A, $1A, $3B, $26, $13, $A5, $25, $9C, $25, $B6, $25, $2E, $2E, $1A, $1A, $1A, $1A, $25, $E9
        DEFB $97, $1A, $1A, $1A, $3B, $0A, $39, $2E, $88, $A5, $98, $A0, $1A, $1A, $1A, $1A, $3B, $15, $90, $A5, $25, $88, $13, $3C, $39, $2E, $25, $1A, $1A, $1A, $1A, $17, $E9
        DEFB $B9, $1A, $1A, $1A, $3B, $81, $25, $2E, $2D, $1E, $AC, $98, $1A, $1A, $1A, $1A, $3B, $90, $97, $97, $25, $88, $06, $16, $2E, $19, $8D, $1A, $1A, $1A, $1A, $3B, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $20, $13, $2E, $28, $18, $05, $98, $1A, $1A, $1A, $1A, $3B, $1E, $2B, $97, $39, $31, $84, $28, $2E, $1F, $20, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $9E, $2E, $16, $0D, $1E, $B9, $1A, $1A, $1A, $1A, $3B, $1E, $11, $97, $25, $97, $90, $2D, $2E, $13, $81, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $8E, $8D, $3C, $39, $3C, $11, $05, $A7, $1A, $1A, $1A, $1A, $3B, $05, $07, $97, $17, $31, $03, $88, $2E, $AB, $80, $A7, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $8D, $13, $25, $31, $3B, $AC, $1A, $1A, $1A, $1A, $1A, $3B, $00, $A4, $97, $3B, $88, $8A, $31, $19, $31, $80, $06, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $9C, $8D, $8D, $13, $0A, $81, $98, $1A, $1A, $1A, $1A, $1A, $3B, $1E, $8F, $B9, $1A, $88, $BC, $13, $1F, $31, $80, $84, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $AC, $8D, $13, $1F, $A5, $B9, $1A, $1A, $1A, $1A, $3B, $04, $1E, $07, $87, $1A, $90, $98, $13, $31, $80, $80, $06, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $B0, $AC, $8D, $13, $88, $9E, $A7, $1A, $1A, $1A, $1A, $17, $A5, $0B, $1F, $1F, $1A, $8E, $0F, $25, $31, $0A, $80, $0C, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $09, $AC, $8D, $AB, $88, $B0, $1A, $1A, $1A, $1A, $1A, $25, $97, $02, $97, $84, $1A, $3B, $B6, $27, $9E, $80, $85, $01, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $02, $AC, $AC, $92, $88, $2B, $1A, $1A, $1A, $1A, $1A, $39, $19, $00, $07, $06, $1A, $1A, $0A, $27, $3C, $0A, $B6, $A0, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $AC, $92, $BC, $88, $B9, $1A, $1A, $1A, $1A, $3B, $2E, $2E, $BC, $A6, $B9, $1A, $1A, $1E, $27, $27, $AC, $85, $BC, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $AC, $A5, $04, $88, $B9, $1A, $1A, $1A, $1A, $17, $2E, $2E, $A5, $97, $1A, $1A, $1A, $9F, $27, $25, $A5, $B6, $A7, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $00, $AC, $21, $BC, $88, $B9, $1A, $1A, $1A, $1A, $25, $2E, $2E, $1F, $1F, $1A, $1A, $1A, $9F, $27, $17, $0A, $85, $BC, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $8D, $2B, $18, $88, $B9, $1A, $1A, $1A, $1A, $39, $2E, $2E, $19, $97, $1A, $1A, $1A, $9F, $27, $25, $31, $B6, $A0, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $8D, $21, $BC, $88, $B9, $1A, $1A, $1A, $3B, $25, $2E, $2E, $1F, $1A, $1A, $1A, $1A, $9F, $27, $25, $31, $80, $BC, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $02, $8D, $2B, $18, $88, $B9, $1A, $1A, $1A, $17, $13, $2E, $2E, $13, $A5, $1A, $1A, $1A, $9F, $27, $39, $31, $80, $A0, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $02, $8D, $21, $BC, $88, $B9, $1A, $1A, $1A, $25, $1F, $2E, $2E, $25, $97, $1A, $1A, $1A, $9F, $27, $39, $31, $80, $01, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $9C, $8D, $2B, $18, $88, $B9, $1A, $1A, $1A, $39, $19, $39, $19, $39, $19, $1A, $1A, $1A, $9F, $27, $39, $31, $80, $98, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $90, $8D, $21, $BC, $88, $B9, $1A, $1A, $3B, $2E, $2E, $25, $1F, $2E, $2E, $1A, $1A, $1A, $9F, $27, $25, $31, $80, $BC, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $8E, $8D, $2B, $04, $88, $B9, $1A, $1A, $17, $2E, $2E, $13, $13, $2E, $2E, $A5, $1A, $1A, $9F, $27, $07, $31, $80, $A7, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $21, $3B, $88, $B9, $1A, $1A, $25, $2E, $2E, $1F, $25, $2E, $2E, $97, $1A, $1A, $9F, $27, $04, $31, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $2B, $3B, $88, $B9, $1A, $1A, $39, $2E, $2E, $19, $39, $2E, $2E, $19, $1A, $1A, $9F, $27, $1A, $31, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $1A, $3B, $25, $2E, $2E, $97, $25, $2E, $2E, $1F, $1A, $1A, $9F, $27, $1A, $AB, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $1A, $17, $13, $2E, $2E, $10, $13, $2E, $2E, $13, $A5, $1A, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $1A, $25, $1F, $2E, $2E, $AC, $06, $2E, $2E, $25, $97, $1A, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $1A, $39, $19, $39, $19, $92, $84, $39, $19, $39, $19, $1A, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $3B, $2E, $2E, $25, $1F, $B9, $25, $25, $1F, $2E, $2E, $1A, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $A5, $3B, $88, $B9, $17, $2E, $2E, $13, $10, $04, $27, $13, $13, $2E, $2E, $A5, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $1A, $1A, $1A, $3B, $8D, $1A, $3B, $88, $B9, $25, $2E, $2E, $1F, $AC, $BC, $17, $1F, $25, $2E, $2E, $97, $9F, $27, $1A, $17, $80, $1A, $1A, $1A, $1A, $1A, $E9
        DEFB $1A, $B9, $1A, $1A, $3B, $AC, $1A, $3B, $88, $B9, $39, $2E, $2E, $19, $98, $18, $25, $1F, $39, $2E, $2E, $19, $9F, $27, $1A, $3B, $80, $1A, $1A, $1A, $1C, $1A, $E9
        DEFB $1A, $97, $1A, $1A, $3B, $A0, $1A, $3B, $88, $B9, $25, $2E, $2E, $97, $BC, $BC, $25, $9C, $25, $2E, $2E, $97, $9F, $27, $1A, $1A, $80, $1A, $1A, $1A, $17, $1A, $E9
        DEFB $3B, $1F, $1A, $1A, $1A, $A7, $1A, $3B, $88, $B9, $13, $2E, $2E, $10, $98, $18, $25, $88, $13, $2E, $2E, $10, $9F, $27, $1A, $1A, $1E, $1A, $1A, $1A, $25, $1A, $E9
        DEFB $3B, $2E, $1A, $1A, $1A, $1A, $1A, $3B, $88, $B9, $09, $2E, $2E, $AC, $AC, $BC, $39, $88, $06, $2E, $2E, $01, $9F, $27, $1A, $1A, $8E, $1A, $1A, $1A, $2E, $B9, $E9
        DEFB $17, $2E, $B9, $1A, $1A, $1A, $1A, $3B, $88, $B9, $02, $39, $19, $92, $05, $98, $39, $31, $84, $39, $19, $A0, $9F, $27, $1A, $1A, $1A, $1A, $1A, $3B, $2E, $A5, $E9
        DEFB $17, $19, $A6, $1A, $1A, $1A, $1A, $3B, $88, $B9, $25, $25, $1F, $B9, $1E, $B9, $25, $97, $25, $25, $1F, $B9, $9F, $27, $1A, $1A, $1A, $1A, $1A, $95, $39, $97, $E9
        DEFB $90, $1F, $95, $1A, $1A, $1A, $1A, $3B, $88, $B9, $25, $13, $10, $04, $05, $A7, $17, $31, $27, $13, $10, $A7, $9F, $27, $1A, $1A, $1A, $1A, $1A, $A6, $25, $06, $E9
        DEFB $87, $97, $3B, $1A, $1A, $1A, $1A, $3B, $88, $B9, $17, $1F, $AC, $BC, $AC, $1A, $3B, $88, $17, $1F, $AC, $B9, $9F, $27, $1A, $1A, $1A, $1A, $1A, $B9, $25, $90, $E9
        DEFB $97, $97, $1A, $B9, $1A, $1A, $1A, $3B, $88, $B9, $25, $1F, $98, $18, $98, $1A, $1A, $88, $25, $1F, $98, $A0, $9F, $27, $1A, $1A, $1A, $1A, $3B, $1A, $00, $00, $E9
        DEFB $A5, $A5, $1A, $A6, $1A, $1A, $1A, $3B, $88, $B9, $25, $9C, $BC, $BC, $B9, $1A, $1A, $90, $25, $9C, $BC, $B9, $9F, $27, $1A, $1A, $1A, $1A, $95, $1A, $17, $17, $E9
        DEFB $80, $B9, $1A, $95, $1A, $1A, $1A, $3B, $88, $B9, $25, $88, $98, $18, $A7, $1A, $1A, $8E, $25, $88, $98, $A0, $9F, $27, $1A, $1A, $1A, $1A, $A6, $1A, $3B, $17, $E9
        DEFB $88, $1A, $1A, $3B, $1A, $1A, $1A, $3B, $88, $B9, $25, $88, $AC, $BC, $1A, $1A, $1A, $3B, $39, $88, $AC, $98, $9F, $27, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $27, $E9
        DEFB $2E, $1A, $1A, $1A, $B9, $1A, $1A, $3B, $88, $B9, $39, $31, $05, $98, $1A, $1A, $1A, $1A, $39, $31, $05, $98, $9F, $27, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $A6, $1A, $1A, $3B, $88, $B9, $25, $97, $1E, $B9, $1A, $1A, $1A, $1A, $25, $97, $1E, $B9, $9F, $27, $1A, $1A, $1A, $95, $1A, $1A, $1A, $2E, $E9
        DEFB $11, $1A, $1A, $1A, $95, $1A, $1A, $3B, $88, $B9, $17, $31, $05, $A7, $1A, $1A, $1A, $1A, $17, $31, $05, $A7, $9F, $27, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $88, $B9, $3B, $88, $AC, $1A, $1A, $1A, $1A, $1A, $3B, $88, $AC, $1A, $9F, $27, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $02, $1A, $1A, $B9, $1A, $1A, $09, $B9, $1A, $88, $98, $1A, $1A, $1A, $1A, $1A, $1A, $88, $98, $1A, $9F, $16, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $13, $1A, $1A, $A6, $1A, $1A, $3B, $B9, $1A, $90, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $90, $B9, $1A, $9F, $0D, $1A, $1A, $95, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $2E, $3B, $13, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $8E, $A7, $1A, $1A, $1A, $1A, $1A, $1A, $8E, $A7, $1A, $1C, $B9, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $3B, $02, $B9, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $3B, $39, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $1C, $2E, $B9, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $11, $1C, $02, $B9, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1C, $11, $B9, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1C, $2E, $B9, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1C, $2E, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $2E, $08, $2E, $B9, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $17, $02, $97, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $90, $11, $97, $84, $84, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $09, $2E, $1F, $B9, $B9, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $3B, $2E, $2E, $2E, $2E, $B9, $2E, $E9
        DEFB $11, $BC, $2E, $97, $B9, $B9, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $17, $2E, $2E, $2E, $2E, $A5, $11, $E9
        DEFB $11, $A8, $2E, $0A, $AD, $AD, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $17, $2E, $2E, $2E, $2E, $97, $11, $E9
        DEFB $11, $BC, $2E, $BF, $AD, $AD, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $25, $1A, $1A, $1A, $1A, $97, $11, $E9
        DEFB $11, $95, $84, $A6, $AD, $AD, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $02, $2E, $2E, $2E, $2E, $02, $11, $E9
        DEFB $2E, $3B, $04, $B9, $84, $84, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1F, $2E, $2E, $2E, $2E, $02, $2E, $E9
        DEFB $2E, $1A, $84, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $15, $1A, $1A, $1A, $1A, $05, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $0A, $84, $84, $84, $04, $3C, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $0A, $B9, $B9, $1C, $1F, $3C, $2E, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $0A, $B9, $B9, $1C, $06, $3C, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $0A, $84, $84, $9F, $04, $3C, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $0A, $9F, $9F, $9F, $2E, $3C, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $0A, $AD, $9F, $9F, $04, $3C, $11, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $0A, $84, $9F, $9F, $39, $3C, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $0A, $1A, $1A, $1A, $39, $3C, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $1A, $3B, $A5, $1A, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $15, $1A, $1A, $1A, $1A, $05, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $A6, $1A, $1C, $A7, $1A, $1A, $1A, $95, $1A, $1A, $1A, $1A, $1A, $1F, $2E, $2E, $2E, $2E, $02, $2E, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $95, $1A, $17, $97, $1A, $1A, $1A, $A6, $1A, $1A, $1A, $1A, $1A, $02, $2E, $2E, $2E, $2E, $02, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1A, $17, $97, $1A, $1A, $1A, $B9, $1A, $1A, $1A, $1A, $1A, $25, $1A, $1A, $1A, $1A, $97, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $B9, $3B, $B9, $1A, $1A, $3B, $1A, $1A, $1A, $1A, $1A, $1A, $17, $2E, $2E, $2E, $2E, $97, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $97, $00, $87, $1A, $1A, $17, $1A, $1A, $1A, $1A, $1A, $1A, $17, $2E, $2E, $2E, $2E, $A5, $11, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $1F, $00, $97, $1A, $1A, $25, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $2E, $2E, $2E, $2E, $B9, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $2E, $00, $97, $1A, $1A, $2E, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $3B, $02, $00, $97, $84, $97, $1F, $B9, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $17, $00, $00, $97, $B9, $36, $97, $A5, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $17, $90, $3B, $A5, $B9, $36, $87, $A5, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $00, $90, $3B, $B9, $AD, $36, $87, $97, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $00, $87, $8E, $97, $AD, $84, $90, $97, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $11, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $25, $87, $09, $97, $AD, $93, $90, $97, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $11, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $25, $97, $00, $87, $84, $93, $17, $97, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9
        DEFB $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $97, $1A, $1A, $1A, $1A, $17, $2E, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $1A, $2E, $E9

; =====================
; Hi-Res Attribute File
; =====================
; The following are the Chroma equivalent colour attribute values for the image of the Spectrum game "Alien 8" by Ultimate Play The Game.
; These will be copied into the attribute file situated in the 48K-64K RAM at locations corresponding the hi-res display file.

HR_AFILE:
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $D0, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $70, $D0, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $D0, $D0, $D0, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $F0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $F0, $F0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $D0, $70, $70, $70, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $70, $70, $70, $D0, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $70, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $70, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $D0, $70, $70, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $D0, $70, $70, $D0, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $C0, $C0, $C0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $A0, $F0, $F0, $F0, $F0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $A0, $A0, $A0, $A0, $A0, $A0, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00
        DEFB $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $F0, $F0, $F0, $F0, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $10, $00

; ================
; Test Entry Point
; ================

RUN_TEST:
        CALL WAIT_FOR_NO_KEY                    ; Wait until all keys have been released.

        LD   A,$06                              ; Select the pixel pattern set to use.
        LD   I,A
        
        CALL HR_DRIVER                          ; Run the hi-res display driver.

        CALL WAIT_FOR_NO_KEY                    ; Wait until all keys have been released.

        CALL CHECK_COLOUR_AVAILABLE             ; Are the colour facilities are available?
        JR   NZ,COLOUR_UNAVAILABLE              ; Jump if not.

        CALL CONFIGURE_COLOUR                   ; Configure the attribute file and select attribute colour mode.
        
        CALL HR_DRIVER                          ; Run the hi-res display driver.

        XOR  A
        CALL SET_COLOUR_MODE                    ; Clear colour mode.

EXIT_TEST:
        LD   A,$0E                              ; Reset back to the standard pixel pattern set.
        LD   I,A
        RET

; =====================
; Display Error Message
; =====================
; Colour facilities are unavailable.

COLOUR_UNAVAILABLE:
        LD   HL,MSG_COLOUR_UNAVAILABLE

DISPLAY_ERROR:
        LD   A,(HL)                             ; Fetch the next character of the message.
        CP   MSG_TERMINATOR                     ; Has the end of the message been reached?
        JR   Z,EXIT_TEST                        ; Return to BASIC if so.
        
        PUSH HL                                 ; Save the messaage pointer.
        EXX
        PUSH AF
        CALL UNSTACK_Z                          ; Fetch the print position.
        POP  AF
        CALL PRINT_CR_2                         ; Print a character.
        EXX
        POP  HL                                 ; Restore the message pointer.
        
        INC  HL                                 ; Advance to the next character of the message.
        JR   DISPLAY_ERROR                      ; Loop back to process the next character of the message.

; =============
; Error Message
; =============

MSG_COLOUR_UNAVAILABLE:
        DEFB _C, _O, _L, _O, _U, _R, _SPACE, _F, _A, _C, _I, _L, _I, _T, _I, _E, _S, _SPACE, _N, _O, _T, _SPACE, _E, _N, _A, _B, _L, _E, _D, _FULLSTOP
        DEFB MSG_TERMINATOR
        
; ============================
; Wait Until No Key Is Pressed
; ============================

WAIT_FOR_NO_KEY:
        XOR  A                                  ; Select all keyboard half rows.
        IN   A,($FE)                            ; Read the keyboard port.
        AND  $1F                                ; Keep only the key bits.
        CP   $1F                                ; Were any keys being pressed?
        JR   NZ,WAIT_FOR_NO_KEY                 ; Jump back if so.

        RET

; ===============================
; Check Colour Facility Available
; ===============================
; Exit: Zero flag set if the colour facilities are available.

CHECK_COLOUR_AVAILABLE:
        LD   BC,COLOUR_PORT                     ; Chroma colour input port.
        IN   A,(C)                              ; Read in the Chroma settings.
        AND  COLOUR80_ENABLED_MASK              ; Keep only the Chroma 80 status bits.
        CP   COLOUR80_ENABLED_VALUE             ; Are Chroma 80 colour facilities available?
        RET

; ==================
; Set Up Colour Mode
; ==================

CONFIGURE_COLOUR:
        LD   HL,HR_AFILE                        ; Point to the attribute data.
        LD   DE,HR_DFILE + $8000                ; Point to the first entry in the attribute files
        LD   BC,$18C0                           ; There are 192 lines of 33 characters.
        LDIR                                    ; Copy the attribute values into the attribute file.
        
        LD   A,COLOUR_ENABLE + COLOUR_MODE1 + BORDER_BLACK

SET_COLOUR_MODE:
        LD   BC,COLOUR_PORT
        OUT  (C),A                              ; Select attribute colour mode and set the border colour.
        RET

; =====================
; Hi-res Display Driver
; =====================
; Each scan line takes 207 T-cycles.
; For a 50Hz ZX80, each frame takes 64170 T-cycles and consists of 310 scan lines (6 VSync, 56 top border, 192 main picture, 56 bottom border).
; For a 60Hz ZX80, each frame takes 54234 T-cycles and consists of 262 scan lines (6 VSync, 32 top border, 192 main picture, 32 bottom border).

HR_DRIVER:

; Delay to complete the last line.
                                
        LD BC,$0B00
        
HR_VSYNC_LOOP1:        
        DJNZ HR_VSYNC_LOOP1

; Exit the hi-res display driver if a key is pressed

        XOR  A                                  ; Select all keyboard half rows.
        IN   A,($FE)                            ; Read the keyboard port.
        AND  $1F                                ; Keep only the key bits.
        CP   $1F                                ; Was a key being pressed?
        RET  NZ                                 ; Return if so.

; Set the number of border lines in RESULT_hi and starts the VSync pulse

        IN   A,($FE)                            ; Return the port that indicates a 50Hz or 60Hz configuraton, and start the VSync pulse.
        RLA
        RLA
        SBC  A,A                                ; $FF (50Hz) or $00 (60Hz).
        AND  $18
        ADD  A,$1F
        LD   (RESULT + $0001),A                 ; Number of border lines = 55 (50Hz) or 31 (60Hz).
        
; Delay for the duration of the VSync pulse

        LD   B,$5A
        
HR_VSYNC_LOOP2:       
        DJNZ HR_VSYNC_LOOP2

        CP   (HL)                               ; Fine tune timing.
        
; End the VSync pulse

        OUT  ($FF),A                            ; Stop the VSync pulse.
        
; Generate the top border lines.

        INC  BC                                 ; Fine tune delay.

        LD   HL,BORDER + $8000                  ; Execution address of the terminator character used for generating the border lines.
        LD   A,$EA                              ; Delay constant that will be loaded into the R register.
        LD   B,$01                              ; There is one row in the top border.
        CALL DISP_2                             ; Output the top border.

; Generate the main picture area.

; Enter a loop to output all 192 lines of the hi-res display file. Each loop iteration takes 207 T-cycles. The position of the picture
; is horizontally aligned with that of the standard ZX80 picture. All HSync pulses are 20 T-cycles long (6.12us), as per the standard ZX80 picture.

        LD   B,$08                              ; Delay counter.

HR_DELAY:
        DJNZ HR_DELAY                           ; Delay until time to output the next scan line.

        NOP                                     ; Fine tune timing.
        INC  BC

        LD   BC,$C0FF                           ; B=Number of lines. C=Output port used to end a HSync pulse.
        LD   IY,HR_DFILE - $0021 + $8000        ; One line before the hi-res display file execution address (it will be advanced a line prior to use).
        LD   DE,$0021                           ; There are 32 characters + a terminator per line.
        LD   HL,HR_LINE_RET                     ; End of line return address.

        XOR  A                                  ; Clear the carry flag.
 
HR_LINE:
        IN   A,($FE)                            ; 11    Start a HSync pulse and reset the hardware line counter.
        RET  C                                  ; 5     Delay.
        OUT  (C),A                              ; 12    Stop the HSync pulse.

; The HSync pulse will have lasted 20 T-states (3 + 5 + 12), matching that of the top and bottom border areas.

        LD   A,$00                              ; 7     Fine tune timing.
        NOP                                     ; 4

        ADD  IY,DE                              ; 15    Point to the next line of the hi-res display file.
        JP   (IY)                               ; 8     Execute a line of the hi-res display file, returning back to the following instruction.

HR_LINE_RET:     
        DJNZ HR_LINE                            ; 13/8  Loop back if there are more lines to output.

        RET  C                                  ; 5     Delay.

; Generate the HSync pulse for the final main picture line.
        
        IN   A,($FE)                            ; 11    Start a HSync pulse.
        RET  C                                  ; 5     Delay.
        OUT  (C),A                              ; 12    Stop the HSync pulse.

; Generate the bottom border lines.

        LD   IY,$4000                           ; Reset the IY register pointing at the system variables.
        LD   HL,BORDER + $8000                  ; Execution address of the terminator character used for generating the border lines.
        LD   A,$EC                              ; Delay constant that will be loaded into the R register.
        LD   B,$01                              ; There is one row in the bottom border.
        CALL DISP_2                             ; Output the bottom border.
        
        JR   HR_DRIVER                          ; Jump back to start a VSync pulse.

; ======================
; Vector to Test Routine
; ======================

        JP   RUN_TEST

; ======================================================================================================================================================

; ==============
; Variables Area
; ==============

VARS_AREA:
        DEFB $80

; ======================================================================================================================================================

; =================
; Editing Workspace
; =================

ELINE_AREA:

; ======================================================================================================================================================

; ============
; Display File
; ============

DFILE_AREA:

; ======================================================================================================================================================

        END
