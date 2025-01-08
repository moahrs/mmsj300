unsigned char *memPosConfig    = 0x0081A000; // Config file
unsigned char *imgsMenuSys     = 0x0081B000; // Images PBM 16x16 each icone in order (64 Bytes Each)
unsigned char *vFinalOS       = 0x00820000; // Atualizar sempre que a compilacao passar desse valor
unsigned char *vcorwf         = 0x00820002; //
unsigned char *vcorwb         = 0x00820004; //
unsigned char *vcorwb2        = 0x00820005; //
unsigned long *mousePointer   = 0x00820006;
unsigned int *spthdlmouse     = 0x0082000A;
unsigned int *mouseX         = 0x0082000E;
unsigned char *mouseY         = 0x00820012;
unsigned char *mouseBtnPres   = 0x00820014;
unsigned char *statusVdpSprite = 0x00820016;
unsigned long *mouseHourGlass   = 0x0082001A;
unsigned long *iconesMenuSys   = 0x0082001E;
unsigned char *saverPat        = 0x00821000;
unsigned char *saverCor        = 0x00825000;

unsigned int *tempDataBase     = 0x0082A000;
unsigned char *tempDataMgui2        = 0x0082A004;
unsigned int *tempDataMgui     = 0x0082F000;

#define BTNONE      0x00
#define BTOK        0x01
#define BTCANCEL    0x02
#define BTYES       0x04
#define BTNO        0x08
#define BTHELP      0x10
#define BTSTART     0x20
#define BTCLOSE     0x40

#define WINVERT     0x01
#define WINHORI     0x00

#define WINOPER     0x01
#define WINDISP     0x00

#define LINHAMENU      22
#define COLMENU       8
#define LINMENU       1

#define ICONSPERLINE   8  // Quantidade de Icones por linha
#define SPACEICONS     4  // Quantidade de Espaços entre os Icones Horizontal
#define COLINIICONS   40  // Linha Inicial dos Icones

#define MOUSE_POINTER 1
#define MOUSE_HOURGLASS 2

#define ICON_HOME  50
#define ICON_RUN  51
#define ICON_NEW  52
#define ICON_DEL  53
#define ICON_MMSJDOS  54
#define ICON_SETUP  55
#define ICON_EXIT  56
#define ICON_HOURGLASS  57

static unsigned char *listError[]= {
    /* 00 */ "reserved",
    /* 01 */ "Syntax Error"
};

// -------------------------------------------------------------------------------
// Funcoes Graficas
// -------------------------------------------------------------------------------
#define DrawHoriLine(x, y, length, color) FillRect(x, y, length, 1, color)
#define DrawVertLine(x, y, length, color) FillRect(x, y, 1, length, color)

void writesxy(unsigned short x, unsigned short y, unsigned char sizef, unsigned char *msgs, unsigned short pcolor, unsigned short pbcolor);
void writecxy(unsigned char sizef, unsigned char pbyte, unsigned short pcolor, unsigned short pbcolor);
void locatexy(unsigned short xx, unsigned short yy);
void SaveScreen(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight);
void RestoreScreen(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight);
void SetDot(unsigned short x, unsigned short y, unsigned short color);
void SetByte(unsigned short ix, unsigned short iy, unsigned char pByte, unsigned short pfcolor, unsigned short pbcolor);
void FillRect(unsigned char xi, unsigned char yi, unsigned short pwidth, unsigned char pheight, unsigned char pcor);
void DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color);
void DrawRect(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight, unsigned short color);
void DrawRoundRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned char radius, unsigned char color);
void DrawCircle(unsigned short x0, unsigned short y0, unsigned char r, unsigned char pfil, unsigned short pcor);
void PutIcone(unsigned int* vimage, unsigned short x, unsigned short y, unsigned char numSprite);
void InvertRect(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight);
void SelRect(unsigned short x, unsigned short y, unsigned short pwidth, unsigned short pheight);
void PutImage(unsigned char* cimage, unsigned short x, unsigned short y);
void LoadIconLib(unsigned char* cfile);
unsigned long readMousePs2 (void);
void VerifyMouse(void);
unsigned char waitButton(void);
unsigned char message(char* bstr, unsigned char bbutton, unsigned short btime);

// -------------------------------------------------------------------------------
// Funcoes Exclusivas do MGI
// -------------------------------------------------------------------------------
void startMGI(void);
void drawButtons(unsigned short xib, unsigned short yib);
void showWindow(unsigned char* bstr, unsigned char x1, unsigned char y1, unsigned short pwidth, unsigned char pheight, unsigned char bbutton);
void redrawMain(void);
void desenhaMenu(void);
unsigned char editortela(void);
unsigned char new_menu(void);
void TrocaSpriteMouse(unsigned char vicone);
void MostraIcone(unsigned short xi, unsigned short yi, unsigned char vicone, unsigned char colorfg, unsigned char colorbg);
void runFromMguiCmd(void);
void importFile(void);
void putImagePbmP4(unsigned long* memoria, unsigned short ix, unsigned short iy);

// -------------------------------------------------------------------------------
// Elementos
// -------------------------------------------------------------------------------
void fillin(unsigned char* vvar, unsigned short x, unsigned short y, unsigned short pwidth, unsigned char vtipo);

