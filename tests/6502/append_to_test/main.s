
        .MEMORYMAP
           DEFAULTSLOT 0
           SLOTSIZE $2000
           SLOT 0 $0000
        .ENDME

        .ROMBANKMAP
           BANKSTOTAL 1
           BANKSIZE $2000
           BANKS 1
        .ENDRO

        .BANK 0 SLOT 0
        .ORG 0

        .DB "01>"
        
        .SECTION "SectionB" FREE APPENDTO "SectionA" PRIORITY 8
label_B:.DB 3, 4, 5, 6
        .ENDS

        .SECTION "SectionD" FREE APPENDTO "SectionA" PRIORITY 6
label_D:.DB 9, $A, $B, $C, $D
        .ENDS
        
        .SECTION "SectionA" FREE PRIORITY 9
label_A:.DB 0, 1, 2
        .ENDS

        .SECTION "SectionC" FREE APPENDTO "SectionA" PRIORITY 7
label_C .DB 7, 8
        .ENDS

        .DB "<01"
