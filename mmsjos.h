typedef void                    VOID;
typedef char                    CHAR;
typedef unsigned char           BYTE;                           /* 8-bit unsigned  */
typedef unsigned short          WORD;                           /* 16-bit unsigned */
typedef unsigned long           DWORD;                          /* 32-bit unsigned */

#define FAT32       3

#define S_OK         0  // no error
#define S_NOTINIT    1  // ArduinoFDC.begin() was not called
#define S_NOTREADY   2  // Drive is not ready (no disk or power)
#define S_NOSYNC     3  // No sync marks found
#define S_NOHEADER   4  // Sector header not found
#define S_INVALIDID  5  // Sector data record has invalid id
#define S_CRC        6  // Sector data checksum error
#define S_NOTRACK0   7  // No track0 signal
#define S_VERIFY     8  // Verify after write failed
#define S_READONLY   9  // Attempt to write to a write-protected disk

typedef struct
{
    unsigned short       firsts;         // Logical block address of the first sector of the FAT partition on the device
    unsigned short       fat;            // Logical block address of the FAT
    unsigned short       root;           // Logical block address of the root directory
    unsigned short       data;           // Logical block address of the data section of the device.
    unsigned short        maxroot;        // The maximum number of entries in the root directory.
    unsigned short       maxcls;         // The maximum number of clusters in the partition.
    unsigned short     RootEntiesCount;  // Num Root Entries
    unsigned short      numheads;       // Number of Heads
    unsigned short       sectorSize;     // The size of a sector in bytes
    unsigned short       secperfat;        // Sector per Fat
    unsigned short       secpertrack;        // Sector per Fat
    unsigned short       fatsize;        // The number of sectors in the FAT
    unsigned char     NumberOfFATs;     // Number of Fat's
    unsigned short        reserv;         // The number of copies of the FAT in the partition
    unsigned char        SecPerClus;     // The number of sectors per cluster in the data region
    unsigned char        type;           // The file system type of the partition (FAT12, FAT16 or FAT16)
    unsigned char        mount;          // Device mount flag (TRUE if disk was mounted successfully, FALSE otherwise)
} DISK12;

typedef struct
{ 
    DWORD       firsts;         // Logical block address of the first sector of the FAT partition on the device
    DWORD       fat;            // Logical block address of the FAT
    DWORD       root;           // Logical block address of the root directory
    DWORD       data;           // Logical block address of the data section of the device.
    WORD        maxroot;        // The maximum number of entries in the root directory.
    DWORD       maxcls;         // The maximum number of clusters in the partition.
    WORD        sectorSize;     // The size of a sector in bytes
    DWORD       fatsize;        // The number of sectors in the FAT
    WORD        reserv;         // The number of copies of the FAT in the partition
    BYTE        SecPerClus;     // The number of sectors per cluster in the data region
    BYTE        type;           // The file system type of the partition (FAT12, FAT16 or FAT32)
    BYTE        mount;          // Device mount flag (TRUE if disk was mounted successfully, FALSE otherwise)
} DISK;

typedef struct
{
    BYTE        Name[8];
    BYTE        Ext[3];
	BYTE		Attr;
	WORD 		CreateDate;
	WORD 		CreateTime;
	WORD 		LastAccessDate;
	WORD 		UpdateDate;
	WORD 		UpdateTime;
	DWORD 		FirstCluster;
	DWORD       Size;
	DWORD       DirClusSec;	// Sector in Cluster of the directory (Position calculated)
	WORD		DirEntry;	// Entry in directory (step 32)
	BYTE        Updated;
} FAT32_DIR;

// Estrutura para Nome do Arquivo
typedef struct
{
    unsigned char        Name[8];
    unsigned char        Ext[3];
} FILE_NAME;

typedef struct
{
    char                 Name[13];
    DWORD                ClusterDir;
    DWORD                ClusterDirAtu;
} RET_PATH;

typedef struct 
{
    unsigned long *prev;         // previous allocation
    char name[11];          // file name alloccated here
    unsigned long address;  // address of the file
    unsigned long size;     // size of allocation
    char status;            // status used (1) of free (0)
    unsigned long *next;         // next allocation
} MEM_ALOC;

FAT32_DIR *vdir              = 0x00610CD0;
DISK  *vdisk                 = 0x00610D00;
DWORD *vclusterdir  = 0x00610DE0;
unsigned short *vclusteros   = 0x00610DE8;
unsigned char  *gDataBuffer  = 0x00610DF0; // The global data sector buffer to 0x00609FF7
unsigned short  *verroSo     = 0x0061102E;
unsigned char  *vdiratu      = 0x00611032; // Buffer de pasta atual 128 bytes
unsigned char  *vdiratup     = 0x00611032; // Pointer Buffer de pasta atual 128 bytes 
unsigned short  *vdiratuidx  = 0x006110B0; // Pointer Buffer de pasta atual 128 bytes (SO FUNCIONA NA RAM)
unsigned char *vFsLinhaCmd   = 0x006111B0;
unsigned int *vFsLinhaIx     = 0x006112B0;
unsigned int *vFsLinhaIy     = 0x006112B8;
unsigned char *vFsLinhaArg   = 0x006112C0;
unsigned char *vFsLinhaPar   = 0x006113C0;
unsigned char *vFsLinhaPar2  = 0x006114C0;
unsigned char *vFsLinhaPar3  = 0x006115C0;
unsigned char *vFsLinhaErro  = 0x006116C0;
unsigned char *vFatRead      = 0x00611900; // 1536 bytes fat lida (3 setores por vez) e a ser usada
unsigned char *vFatRead1     = 0x00611B00; // 1536 bytes fat lida (3 setores por vez) e a ser usada
unsigned char *vFatRead2     = 0x00611D00; // 1536 bytes fat lida (3 setores por vez) e a ser usada
unsigned char *vFirstFindFat = 0x00611F02; // Indica primeira leitura da fat, pra nao precisar mais ler enquanto estiver numa pesquisa sequencial
unsigned char  *mcfgfile     = 0x00611F04; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
                            // 0x00614F04
unsigned char *verro         = 0x00614F06;
RET_PATH *vretpath           = 0x00614F08;
RET_PATH *vretpath2          = 0x00614F20;
MEM_ALOC *vMemAloc           = 0x00614F40;

//FAT32_DIR *vdirsrc           = 0x00614F60;
//FAT32_DIR *vdirdst           = 0x00614F90;

unsigned long *tempData      = 0x00614FC0;
unsigned char *tempData2     = 0x00614FD0;
unsigned char *tempData3     = 0x006151D0;


#define FAT16       3

#define FS_CMD      0
#define FS_DATA     1
#define FS_PAR      2

#define TRUE        1
#define FALSE       0

#if !defined(NULL)
    #define NULL        '\0'
#endif

#define MEDIA_SECTOR_SIZE   512

// Demarca o Final do OS, constantes desse tipo nesse compilador vao pro final do codigo.
// Sempre verificar se esta no final mesmo
#define ATTR_READ_ONLY      0x01
#define ATTR_HIDDEN         0x02
#define ATTR_SYSTEM         0x04
#define ATTR_VOLUME         0x08
#define ATTR_LONG_NAME      0x0f
#define ATTR_DIRECTORY      0x10
#define ATTR_ARCHIVE        0x20
#define ATTR_MASK           0x3f

#define CLUSTER_EMPTY               0x0000
#define LAST_CLUSTER_FAT32      0x0FFFFFFF
#define END_CLUSTER_FAT32       0x0FFFFFF7
#define CLUSTER_FAIL_FAT32      0x0FFFFFFF

#define NUMBER_OF_BYTES_IN_DIR_ENTRY    32
#define DIR_DEL             0xE5
#define DIR_EMPTY           0
#define DIR_NAMESIZE        8
#define DIR_EXTENSION       3
#define DIR_NAMECOMP        (DIR_NAMESIZE+DIR_EXTENSION)

#define EOF             ((int)-1)

#define OPER_READ      0x01
#define OPER_WRITE     0x02
#define OPER_READWRITE 0x03

#define CONV_DATA    0x01
#define CONV_HORA    0x02

#define INFO_SIZE    0x01
#define INFO_CREATE  0x02
#define INFO_UPDATE  0x03
#define INFO_LAST    0x04

#define FIND_PATH_PART 0x00
#define FIND_PATH_LAST 0x01

#define FIND_PATH_RET_ERROR 0xFF
#define FIND_PATH_RET_FOLDER 0x01
#define FIND_PATH_RET_FILE 0x02

// Tipos para Cricao/Procura de Arquivos
#define TYPE_DIRECTORY   0x01
#define TYPE_FILE 		 0x02
#define TYPE_EMPTY_ENTRY 0x03
#define TYPE_CREATE_FILE 0x04
#define TYPE_CREATE_DIR  0x05
#define TYPE_DEL_FILE    0x06
#define TYPE_DEL_DIR     0x07
#define TYPE_FIRST_ENTRY 0x08
#define TYPE_NEXT_ENTRY  0x09
#define TYPE_ALL         0xFF

// Tipos para Procura de Clusters
#define FREE_FREE 0x01
#define FREE_USE  0x02
#define NEXT_FREE 0x03
#define NEXT_FULL 0x04
#define NEXT_FIND 0x05

// Codigo de Erros
#define ERRO_D_START	      0xFFFFFFF0
#define ERRO_D_FILE_NOT_FOUND 0xFFFFFFF0
#define ERRO_D_READ_DISK      0xFFFFFFF1
#define ERRO_D_WRITE_DISK     0xFFFFFFF2
#define ERRO_D_OPEN_DISK      0xFFFFFFF3
#define ERRO_D_DISK_FULL      0xFFFFFFF4
#define ERRO_D_INVALID_NAME   0xFFFFFFF5
#define ERRO_D_NOT_FOUND      0xFFFFFFFF

#define ALL_OK                0x00
#define ERRO_B_START          0xE0
#define ERRO_B_FILE_NOT_FOUND 0xE0
#define ERRO_B_READ_DISK      0xE1
#define ERRO_B_WRITE_DISK     0xE2
#define ERRO_B_OPEN_DISK      0xE3
#define ERRO_B_INVALID_NAME   0xE4
#define ERRO_B_DIR_NOT_FOUND  0xE5
#define ERRO_B_CREATE_FILE    0xE6
#define ERRO_B_APAGAR_ARQUIVO 0xE7
#define ERRO_B_FILE_FOUND     0xE8
#define ERRO_B_UPDATE_DIR     0xE9
#define ERRO_B_OFFSET_READ    0xEA
#define ERRO_B_DISK_FULL      0xEB
#define ERRO_B_READ_FILE      0xEC
#define ERRO_B_WRITE_FILE     0xED
#define ERRO_B_DIR_FOUND      0xEE
#define ERRO_B_CREATE_DIR     0xEF
#define ERRO_B_NOT_FOUND      0xFF

#define RETURN_OK             0x00

const unsigned char strValidChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^&'@{}[],$=!-#()%.+~_";

const unsigned char vmesc[12][3] = {{'J','a','n'},{'F','e','b'},{'M','a','r'},
                                    {'A','p','r'},{'M','a','y'},{'J','u','n'},
                                    {'J','u','l'},{'A','u','g'},{'S','e','p'},
                                    {'O','c','t'},{'N','o','v'},{'D','e','c'}};

//--- FAT16 Functions
DWORD fsInit(void);
void fsVer(void);
void printDiskError(unsigned char pError);
unsigned char fsMountDisk(void);
char fsOsCommand(void);
unsigned char fsFormat (long int serialNumber, char * volumeID);
void fsSetClusterDir (DWORD vclusdiratu);
DWORD fsGetClusterDir (void);
BYTE fsSectorWrite(DWORD vsector, BYTE* vbuffer, BYTE vtipo);
BYTE fsSectorRead(DWORD vsector, BYTE* vbuffer);
int fsRecSerial(unsigned char* pByte, unsigned char pTimeOut);
int fsSendSerial(unsigned char pByte);
int fsSendByte(unsigned char vByte, unsigned char pType);
unsigned char fsRecByte(unsigned char pType);
int fsSendLongSerial(unsigned char *msg);
void fsConvClusterToTHS(unsigned short cluster, unsigned char* vtrack, unsigned char* vside, unsigned char* vsector);
void fsReadDir(unsigned short ix, unsigned short vdata);

// Funcoes de Manipulacao de Arquivos
unsigned char fsCreateFile(char * vfilename);
unsigned char fsOpenFile(char * vfilename);
unsigned char fsCloseFile(char * vfilename, unsigned char vupdated);
unsigned long fsInfoFile(char * vfilename, unsigned char vtype);
BYTE fsRWFile(DWORD vclusterini, DWORD voffset, BYTE *buffer, BYTE vtype);
WORD fsReadFile(char * vfilename, DWORD voffset, BYTE *buffer, WORD vsizebuffer);
unsigned char fsWriteFile(char * vfilename, unsigned long voffset, unsigned char *buffer, BYTE vsizebuffer);
unsigned char fsDelFile(char * vfilename);
unsigned char fsRenameFile(char * vfilename, char * vnewname);
void runFromOsCmd(void);
DWORD loadFile(BYTE *parquivo, unsigned short* xaddress);
void catFile(BYTE *parquivo);
BYTE fsLoadSerialToFile(char * vfilename, char * vPosMem);
BYTE fsFindDirPath(char * vpath, char vtype);

// Funcoes de Manipulacao de Diretorios
unsigned char fsMakeDir(char * vdirname);
unsigned char fsChangeDir(char * vdirname);
unsigned char fsRemoveDir(char * vdirname);
unsigned char fsPwdDir(unsigned char *vdirpath);

// Funcoes de Apoio
unsigned short fsLoadFat(unsigned short vclusteratual);
unsigned long fsFindInDir(char * vname, unsigned char vtype);
unsigned char fsUpdateDir(void);
DWORD fsFindNextCluster(DWORD vclusteratual, BYTE vtype);
DWORD fsFindClusterFree(BYTE vtype);
unsigned int bcd2dec(unsigned int bcd);
int getDateTimeAtu(void);
WORD datetimetodir(BYTE hr_day, BYTE min_month, BYTE sec_year, BYTE vtype);
unsigned long pow(int val, int pot);
int hex2int(char ch);
unsigned long hexToLong(char *pHex);
void strncpy2( char* _dst, const char* _src, int _n );
int isValidFilename(char *filename) ;
BYTE matches_wildcard(const char *pattern, const char *filename);
BYTE contains_wildcards(const char *pattern);

// Funcoes de Alocacao de Memoria
void memInit(void);

/************************************************;
; Convert LBA to CHS
; AX: LBA Address to convert
;
; absolute sector = (logical sector / sectors per track) + 1
; absolute head   = (logical sector / sectors per track) MOD number of heads
; absolute track  = logical sector / (sectors per track * number of heads)
;
;************************************************/