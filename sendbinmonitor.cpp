/********************************************************************************
*    Programa    : sendbinmonitor.cpp
*    Objetivo    : Comunicação serial com o Modulo para gravação via PIC
*    Criado em   : 01/10/2022
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Responsavel  Motivo
* 01/10/2022  Moacir Jr.   criação
********************************************************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <conio.h> 
#include <dos.h> 
#include <iostream>
#include <string>
#include <windows.h>  //Necessário para: LoadLibrary(), GetProcAddress() e HINSTANCE.
using namespace std;

unsigned char endlsb, endlsb_bkp;
unsigned char endmsb32, endmsb24, endmsb16, endmsb_bkp;
unsigned char RetByte;       // Para armazenar o valor recebido da Porta Paralela.
unsigned char RetInput[2], RetErro; 
HANDLE hSerial = NULL;

void delay(unsigned int ttempo);
int IniciaSerial();
void TerminaSerial();
void EnviaDado(unsigned char sbyte);
DWORD EnviaComando(unsigned char sbyte);
DWORD EnviaCtrl(unsigned char sbyte); 
DWORD RecebeComando();
DWORD RecebeByte();
void AcompanhaGravacao();

// Programa Principal
int main(int argc, char *argv[])
{
  FILE *fp;
	char xfilebin[100] = "d:\\projetos\\mmsj300\\";
	int vnumFF = 0, verro, vnumerros;
	unsigned char dados, dadosf;
	unsigned char dadosrec;
  string argum;
  
	if (argc < 2)
	{
  	if (argc < 1) {
  		printf("Erro. Nao foi passado o programa a ser enviado.\n");
  		printf("Sintaxe: sendbinmonitor <nome do arquivo>\n");
  		printf("         <nome do arquivo> - nome do arquivo a importar com .bin\n");
  		return 0;
  	}
	}
	
	// Variaveis de Saida

	clrscr();
	
	// Inicio da Rotina
	printf(">Iniciando Envio de Dados via porta Serial. Usando %s.\n", argv[1]);

	printf(">Inicializando Porta Serial (COM4, 19200, 8, N, 1).\n");
	IniciaSerial();
	
	printf(">Abrindo Arquivo.\n");
	strcat(xfilebin,argv[1]);
	fp = fopen(xfilebin,"rb");

	if (fp == NULL) {
		printf(">Erro ao Abrir Arquivo.\n");
		vnumFF = 255;
	}

	// Enviando Dados (Adress LSB -> MSB -> Dados)
  if (vnumFF != 255) {
    // Envia tamanho
		printf(">Enviando Tamanho.\n");

		EnviaDado(0x00); // Envia Byte
		EnviaDado(0x00); // Envia Byte
		EnviaDado(0x55); // Envia Byte
		EnviaDado(0x3A); // Envia Byte
    
    // Envia dados
		printf(">Enviando Dados.\n");
    
  	while (!feof(fp) && vnumFF <= 20)
  	{
  		dadosf = getc(fp);

			// Grava Dados
			printf(">%02X",endmsb32);
			printf("%02X",endmsb24);
			printf("%02X",endmsb16);
			printf("%02X",endlsb);
			printf(" : %02X",dadosf);

			// Grava Dados
      printf(".");
			EnviaDado(dadosf); // Envia Byte
      printf(".\r");
  	}

  	printf(">\nFechando Arquivo.\n");

  	fclose(fp);
  
  	printf(">Finalizando Comunicação.\n");
  
  	TerminaSerial();
  
  	printf(">Dados do Arquivo Enviados.\n");
	  printf(">Bom Teste.\n");
  }
  
	return(0);
}

// adiciona atraso (delay))
void delay(unsigned int ttempo)
{
	for (unsigned int i = 0; i <= ttempo; i++);
}

// inicializa porta serial
int IniciaSerial(){ 
	char *NomePorta = "COM4"; //COM1, COM2... 

	hSerial = CreateFile(NomePorta,	 //Nome da porta. 
 						 GENERIC_READ|GENERIC_WRITE, //Para leitura e escrita. 
						 0,	 //(Zero) Nenhuma outra abertura será permitida. 
						 NULL,	 //Atributos de segurança. (NULL) padrão. 
						 OPEN_EXISTING,	 //Criação ou abertura. 
						 0,	 //Entrada e saída sem overlapped. 
						 NULL	 //Atributos e Flags. Deve ser NULL para COM. 
						 ); 

	if(hSerial == INVALID_HANDLE_VALUE) 
		return false; //Erro ao tentar abrir a porta 

	DCB dcb; //Estrutura DCB é utilizada para definir todos os 
		 	 // parâmetros da comunicação. 

	if( !GetCommState(hSerial, &dcb)) 
		return false; //// Erro na leitura de DCB. 

	dcb.BaudRate = CBR_19200; 
	dcb.ByteSize = 8; 
	dcb.Parity = NOPARITY; 
	dcb.StopBits = ONESTOPBIT; 

	//Define novo estado. 
	if( SetCommState(hSerial, &dcb) == 0 ) 
		return false; //Erro. 

	return true;
} 

// fecha porta serial 
void TerminaSerial(){ 
	CloseHandle( hSerial );	 //Fecha a porta 
} 

//envia dados 
void EnviaDado(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[1]; 

	cmd[0] = sbyte; 

	WriteFile( hSerial, cmd, 1, &BytesEscritos, NULL ); 
}

//envia dados começando com 0xDDh 
DWORD EnviaComando(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xDD;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

// envia controle começando com 0xEEh
DWORD EnviaCtrl(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xEE;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

// aguarda recebimento de 2 bytes
DWORD RecebeComando(){ 
	DWORD BytesLidos = 0; 

	ReadFile( hSerial, RetInput, 2, &BytesLidos, NULL ); 

	return BytesLidos;
} 

DWORD RecebeByte(){ 
	DWORD BytesLidos = 0; 

	ReadFile( hSerial, RetInput, 1, &BytesLidos, NULL ); 

	return BytesLidos;
}

// acompanha gravação dos 64 bytes enviados
void AcompanhaGravacao(){
  unsigned char vstatus;
  
  vstatus = 1;
  
	printf("\n>PIC: Iniciando Gravação 512 Bytes / 256 Words.\n");

  while (vstatus) {
  	RecebeComando(); // Recebe Controle: 0xEE + 71, 72 ou 73
    
    if (RetInput[0] == 0xEE && RetInput[1] == 0x72) {
  		printf(">PIC: Ocorreu(ram) Erro(s) de Gravação(ões). Tentando Novamente.\n");
    	RecebeComando(); // Recebe Controle: 0xEE + endereco lsb com problema
  		printf(">PIC: LSB com erro %02X\n", RetInput[1]);
    	RecebeComando(); // Recebe Controle: 0xEE + dado que deveria ter sido  gravado
  		printf(">PIC: Gravado..: %02X\n", RetInput[1]);
    	RecebeComando(); // Recebe Controle: 0xEE + dado que foi lido
  		printf(">PIC: Lido.....: %02X\n", RetInput[1]);
    }
    else if (RetInput[0] == 0xEE && RetInput[1] == 0x71)
  		printf(">PIC: Gravação Executada com Sucesso.\n");
    else if (RetInput[0] == 0xEE && RetInput[1] == 0x73){
  		printf(">PIC: Fim de Gravação.\n");
      vstatus = 0;
    }
  }
}

