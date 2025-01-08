/*------------------------------------------------------------------------------
* MMSJ300API.H - Arquivo de Header do MMSJ300
* Author: Moacir Silveira Junior (moacir.silveira@gmail.com)
* Date: 04/07/2013
*------------------------------------------------------------------------------*/
// Alternate definitions
//typedef void                    VOID;
//typedef char                    CHAR;
//typedef unsigned char           BYTE;                           /* 8-bit unsigned  */
//typedef unsigned short          WORD;                           /* 16-bit unsigned */
//typedef unsigned long           DWORD;                          /* 32-bit unsigned */

// Pointers to I/O devices
unsigned char *vdsk  = 0x00200000; // DISK Arduino Uno, r/w
unsigned char *vdskc = 0x00200001; // DISK Arduino Uno, r/w Command
unsigned char *vdskp = 0x00200005; // DISK Arduino Uno, r/w Command Param
unsigned char *vdskd = 0x00200003; // DISK Arduino Uno, r/w Data
unsigned char *vmfp  = 0x00400020; // MFP MC68901p       A8 to A12 => RS1 to RS5
unsigned char *vvdgd = 0x00400041; // VDP TMS9118 Data Mode
unsigned char *vvdgc = 0x00400043; // VDP TMS9118 Registers/Address Mode
unsigned char *vdest = 0x00000000; // Endereco destino + endre�o base

// Pointer to Variables
// Usando memoria ram de sistema (0x00200000 - 0x0060FFFF)
unsigned char *fgcolor = 0x00610000;    // Buffer da VRAM 6KB onde o computador vai trabalhar e a cada interrupcao, sera enviado a VRAM
unsigned char *bgcolor = 0x00611800; // QTD de BYTES no eixo X (COLS) a srem enviados pra VRAM
unsigned char *videoBufferQtdY = 0x00611802; // QTD de BYTES no eixo Y (Rows) a serem enviados pra VRAM
unsigned int *color_table = 0x00611806;
unsigned int *sprite_attribute_table = 0x0061180E; // Contador da quantidade ja enviada acumulada. A cada 2KB, ele esera uma nova interrupcao
unsigned long *videoFontes = 0x00611816; // Ponteir para a posicao onde estão as Fontes para vga
unsigned short *videoCursorPosCol = 0x0061181E;  // Posicao atual caracter do cursor na coluna (0 a 31)
unsigned short *videoCursorPosRow = 0x00611820;  // Posical atual caracter do cursor na linha (0 a 23) 
unsigned short *videoCursorPosColX = 0x00611822;  // Posicao atual do cursor na coluna (0 a 255)
unsigned short *videoCursorPosRowY = 0x00611824;  // Posical atual do cursor na linha (0 a 191)
unsigned char *videoCursorBlink = 0x00611826; // Cursor piscante (1 = sim, 0 = nao)
unsigned char *videoCursorShow = 0x00611828;  // Mostrar Cursor  (1 = sim, 0 = nao)
unsigned int *name_table = 0x0061182A;
unsigned char *vdp_mode = 0x00611832; // Modo de video 0 = caracter (32 x 24), 1 = grafico (256 x 192)
unsigned char *videoScroll = 0x00611834;    // Define se quando a linha passar de 23 (0-23), a tela será rolada (0-nao, 1-sim)
unsigned char *videoScrollDir = 0x00611836;    // Define a direcao do scroll (1-up, 2-down, 3-left, 4-right)
unsigned int *pattern_table = 0x00611838;
unsigned char *sprite_size_sel = 0x00611840;
unsigned char *vdpMaxCols = 0x00611842; // max col number
unsigned char *vdpMaxRows = 0x00611846;
unsigned char *fgcolorAnt = 0x00611848; // Cor Anterior de Frente
unsigned char *bgcolorAnt          = 0x0061184A; // Cor Anterior de Fundo
unsigned int *sprite_pattern_table = 0x00611850;
unsigned int *color_table_size     = 0x00611854;

unsigned char *kbdKeyPntr = 0x00609B1B; // Contador do ponteiro das teclas colocadas no buffer
unsigned char *kbdKeyBuffer = 0x00609B1C;   // 16 buffer char
unsigned char *kbdvprim = 0x00609B3C;
unsigned char *kbdvmove = 0x00609B3E;
unsigned char *kbdvshift = 0x00609B40;
unsigned char *kbdvctrl = 0x00609B42;
unsigned char *kbdvalt = 0x00609B44;
unsigned char *kbdvcaps = 0x00609B46;
unsigned char *kbdvnum = 0x00609B48;
unsigned char *kbdvscr = 0x00609B4A;
unsigned char *kbdvreleased = 0x00609B4C;
unsigned char *kbdve0 = 0x00609B4E;
unsigned long *SysClockms = 0x00609B50;
unsigned char *kbdvreleasedTemp = 0x00609B60;
unsigned char *kbdClockCount   = 0x00609B64;  // Countador do clock para recebimento de tecla PS/2
unsigned char *scanCode       = 0x00609B66;

unsigned short *vxmaxold    = 0x00609B68;
unsigned short *vymaxold    = 0x00609B6A;
unsigned short *voverx      = 0x00609B6C; //  Overlay video texto para janelas
unsigned short *vovery      = 0x00609B6E; //  Overlay video texto para janelas
unsigned char *vparamstr   = 0x00609B70; //  255 Char Param string
unsigned short *vparam      = 0x00609C70; //  29 Parameters
unsigned char *vbbutton    = 0x00609CAA;
unsigned char *vkeyopen    = 0x00609CAC;
unsigned char *vbytetec    = 0x00609CAE;
unsigned short *pposx       = 0x00609CB0;
unsigned short *pposy       = 0x00609CB2;
unsigned short *vbuttonwiny = 0x00609CB6;
unsigned char *vbuttonwin  = 0x00609CB8;
unsigned short *vpostx      = 0x00609CC0;
unsigned short *vposty      = 0x00609CC2;
unsigned char  *next_pos   = 0x00609CCE;
//unsigned char  *mcfgfile   = 0x00609FF8; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
unsigned short  *viconef    = 0x00609FF8; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
unsigned short  *vcorf      = 0x0060CFFC; // cor padrao de frente
unsigned short  *vcorb      = 0x0060CFFE; // cor padrao de fundo
unsigned short  *vcol       = 0x0060D000;
unsigned short  *vlin       = 0x0060D002;
unsigned short  *voutput    = 0x0060D004; // 0 - LCD (16x2), 1 - LCD Grafico (320 x 240), 2 - VGA (somente modo texto)
//unsigned char  *vbuf       = 0x0060D006; // Buffer Linha Digitavel OS 32 bytes
unsigned short  *vxmax      = 0x0060D026;
unsigned short  *vymax      = 0x0060D028;
unsigned short  *xpos       = 0x0060D02A;
unsigned short  *ypos       = 0x0060D02C;
unsigned char  *vinip      = 0x0060D0C0; // Inicio da digitacao do prompt
unsigned long *vbufk      = 0x0060D0C2; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
unsigned char  *vbufkptr   = 0x0060D0C2; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
unsigned char  *vbufkmove  = 0x0060D0C2;
unsigned char  *vbufkatu   = 0x0060D0C2;
unsigned long *vbufkbios  = 0x0060D0E2; // Interface ponteiros entre BIOS e OS para o teclado
unsigned char  *inten      = 0x0060D0F0; // Usar interrupcoes para comunicacao com o PIC
unsigned short  *vxgmax     = 0x0060D0F2;
unsigned short  *vygmax     = 0x0060D0F6;
unsigned long *vmtaskatu  = 0x0060DFA4; // Posicao atual no vmtask do prog sendo executado atualmente:
unsigned char  *vmtask     = 0x0060DFA4; // Lista de 8 posicoes com 6 Bytes cada posicao:
                                //   - Byte 0 a 1 - ordem de execu��o dos programas (2 Bytes)
                                //       - 0xAA: programa sendo usado atualmente, 10mS de tempo de execucao
                                //       - 0xFF: programas que estao em 2o.Plano, 1mS de tempo de execucao
                                //   - Bytes 2 a 5 - ordem dos SP's (A7) dos programas (4 Bytes)
unsigned char  *vmtaskup   = 0x0060DFA4; // Lista de 8 posicoes com 6 Bytes cada posicao (TOPO)
unsigned long *intpos     = 0x0060DFF8; // Posicao da rotina de controle de multitask - OS
unsigned short  *vtotmem    = 0x0060DFFC; // Quantidade de memoria total, vindo da bios
unsigned short  *v10ms      = 0x0060DFFE; // contador 10mS para programas principais na multitarefa
unsigned char  *vPS2                 = 0x0060E000; // Define se tem teclado PS2 (1) ou Touch (0)
unsigned char *vBufXmitEmpty         = 0x0060E002; // Mostra buffer envio serial vazio (1) ou nao (0)
unsigned char *vBufReceived          = 0x0060E004; // Byte recebido pelo MFP
unsigned char *vbuf                  = 0x0060E006; // Buffer Linha Digitavel, maximo de 512 caracteres - 
                                 // to 0x0060E206
unsigned short *errorBufferAddrBus   = 0x0060E208; // 64 words
                                 // to 0x0060E288
unsigned short *traceData            = 0x0060E28A; // 512 words
                                 // to 0x0060E68A
unsigned long *tracePointer          = 0x0060E68C;
unsigned long *traceA7               = 0x0060E692;
unsigned short *regA7                = 0x0060E696;
unsigned short *startBasic           = 0x0060E69A;

unsigned char *vBufDataDisk = 0x0060E6A0; // 512 bytes
                                 // to 0x0060E89F

unsigned long *kbdtimeout    = 0x0060E8A2;
unsigned long *Msetimeout    = 0x0060E8AA;
unsigned char *MseClockCount = 0x0060E8B2;  // Countador do clock para recebimento de tecla PS/2
unsigned char *scanCodeMse   = 0x0060E8B4;
unsigned char *MseMovPntr    = 0x0060E8B6; // Contador do ponteiro das dados do mouse recebidos
unsigned char *MseMovBuffer  = 0x0060E8B8;   // 64 buffer mouse movimentos
unsigned char *Ps2PairChk    = 0x0060E904;
unsigned char *vUseMouse     = 0x0060E906;
unsigned char *vStatusMse    = 0x0060E908;
unsigned char *TypeMse       = 0x0060E90A;
unsigned long *vSizeTotalRec = 0x0060E90C;

// Constantes
#define picSendMsg      0x00D0
#define picCarregaSO    0x00D1
#define picReadKbd      0x00D2
#define picDOScmd       0x00D3
#define picloadFile     0x00D4
#define picSectorRead   0x00D7
#define picSectorWrite  0x00D8

#define picIniIntr      0x00EA
#define picIniStatus    0x00EF

#define picErrorNoSO    0x00FD
#define picErrorIO      0x00FE
#define picOkSO         0x0060

#define picCommData     0x0030
#define picCommStop     0x0040

#define picDOSdir       0x00A0
#define picDOSdel       0x00A1
#define picDOSren       0x00A2
#define picDOSmd        0x00A3
#define picDOScd        0x00A4
#define picDOSrd        0x00A5
#define picDOSformat    0x00A6
#define picDOSdate      0x00A7
#define picDOStime      0x00A8
#define picDOSifconfig  0x00A9
#define picDOScopy      0x00AA

// VDP6847 VRAM ACCESS AND VARIABLES
#define vdpAddCol           0x000100   // A0 VDP = A8 CPU
#define vdpAddRow           0x002000   // 32 BYTES PER ROW
#define vdpCharWidth        5
#define vdpCharHeitgh       7
#define vdpPixelBetweenChar 3          // 8 x 8
#define vdpMaxX             256
#define vdpMaxY             192
#define kbdMaxCharBuff      8
#define kbdKeyBuffMax       15
#define MseMovBuffMax       63

#define NUMBER_OF_DIGITS 32   /* space for NUMBER_OF_DIGITS + '\0' */

#define NOREPOS_CURSOR             0
#define REPOS_CURSOR               1
#define REPOS_CURSOR_ON_CHANGE     2
#define ADD_POS_SCR                3
#define NOADD_POS_SCR              4

//--- Constantes de Tela
#define VDP_TRANSPARENT 0
#define VDP_BLACK 1
#define VDP_MED_GREEN 2
#define VDP_LIGHT_GREEN 3
#define VDP_DARK_BLUE 4
#define VDP_LIGHT_BLUE 5
#define VDP_DARK_RED 6
#define VDP_CYAN 7
#define VDP_MED_RED 8
#define VDP_LIGHT_RED 9
#define VDP_DARK_YELLOW 10
#define VDP_LIGHT_YELLOW 11
#define VDP_DARK_GREEN 12
#define VDP_MAGENTA 13
#define VDP_GRAY 14
#define VDP_WHITE 15

#define VDP_MODE_G1 0
#define VDP_MODE_G2 1
#define VDP_MODE_MULTICOLOR 2
#define VDP_MODE_TEXT 3

#define VDP_CSR_UP 0
#define VDP_CSR_DOWN 1
#define VDP_CSR_LEFT 2
#define VDP_CSR_RIGHT 3

/**
 * @brief VDP Status flags from status register
 */
#define VDP_FLAG_COIN 0x20 /*Coincidence flag, set when sprites overlap*/
#define VDP_FLAG_S5 0x40  /*5th sprite flag, set when more than 4 sprite per line */

/** Struct
 * @brief 4-Byte record defining sprite attributes
 */
typedef struct 
{
    unsigned char x; //Sprite X position
    unsigned char y; //Sprite Y position
    unsigned char name_ptr; //Sprite name in pattern table
    unsigned char ecclr; //Bit 7: Early clock bit, bit 3:0 color
} Sprite_attributes;

/**
 * @brief VDP status
 */
#define VDP_OK 0
#define VDP_ERROR 1

#define addline 0x01
#define noaddline 0x00

// ASCII character set
unsigned char ascii[]  = "abcdefghijklmnopqrstuvwxyz0123456789;=.,/'[]`- "; // Sem Caps sem Shift
unsigned char ascii2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ)!@#$%^&*(:+><?\"{}~_ "; // Sem Caps com Shift
unsigned char ascii3[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789;=.,/'[]`- "; // Com Caps sem Shift 
unsigned char ascii4[] = "abcdefghijklmnopqrstuvwxyz)!@#$%^&*(:+><?\"{}~_ "; // Com Caps com Shift

// KeyCode set

// Querty keycode set: uncomment to activate this code
unsigned char keyCode[]={0x1C,0x32,0x21,0x23,0x24,0x2B,0x34,0x33,0x43,0x3B,0x42,
                         0x4B,0x3A,0x31,0x44,0x4D,0x15,0x2D,0x1B,0x2C,0x3C,0x2A,
                         0x1D,0x22,0x35,0x1A,0x45,0x16,0x1E,0x26,0x25,0x2E,0x36,
                         0x3D,0x3E,0x46,0x4C,0x55,0x49,0x41,0x4A,0x52,0x54,0x5B,
                         0x0E,0x4E,0x29,0x00};
