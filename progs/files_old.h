typedef struct
{
    BYTE        Name[9];
    BYTE        Ext[4];
    BYTE        Modify[12];
    BYTE        Size[8];
	BYTE		Attr[5];
    BYTE        posy;
} FILES_DIR;

FILES_DIR *dfile = 0x0083E000;
BYTE *cfile     = 0x0083E000;   // Lista de arquivos carregados da pasta atual. 40 em 40. Max 100.
BYTE *clinha    = 0x0083DFE0;
WORD *vpos      = 0x0083DFF0;
WORD *vposold   = 0x0083DFF2;

// DEFINE FUNCOES
void linhastatus(BYTE vtipomsgs);
void SearchFile(void);
void carregaDir(void);
void listaDir(void);
BYTE * _strcat (BYTE * dst, BYTE * cp, BYTE * src);
