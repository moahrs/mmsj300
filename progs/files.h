typedef struct FILES_DIR
{
    BYTE        Name[9];
    BYTE        Ext[4];
    BYTE        Modify[12];
    BYTE        Size[8];
	BYTE		Attr[5];
    BYTE        posy;
} FILES_DIR;

typedef struct LIST_DIR 
{
  FILES_DIR dir[150];
  int pos;
} LIST_DIR;

LIST_DIR *dfile = 0x0083E000;  // Lista de arquivos carregados da pasta atual
BYTE *clinha    = 0x0083DFE0;
WORD *vpos      = 0x0083DFF0;
WORD *vposold   = 0x0083DFF4;
BYTE *dFileCursor = 0x0083DFF8;

// DEFINE FUNCOES
void linhastatus(BYTE vtipomsgs, BYTE * vmsgs);
void SearchFile(void);
void carregaDir(void);
void listaDir(void);
