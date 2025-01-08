/********************************************************************************
*    Programa    : modulos.cpp
*    Objetivo    : Comunicação serial com o Modulo para gravação via PIC
*    Criado em   : 20/12/2011
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
*	  ------------------------------------------
*	  PROTOCOLO DE COMUNICACAO:
*	  ------------------------------------------
*	  SENTIDO  
*	   PC/PIC  COMANDO  FUNÇÃO
*	      ->   DDDD     INICIA COMUNICACAO
*	   <-      DDDD     COMUNICACAO ESTABELECIDA
*	   <- ->   DDdd     RECEBE OU ENVIA DADO (dd)
*	      ->   EE00     DADO VERIFICADO COM SUCESSO
*	      ->   EE01     ERRO NA VERIFICAÇÃO DO DADO
*	   <-      EE69     ENVIAR BYTE
*	   <-      EE70     INICIO DO PROCESSO DE GRAVAÇÃO
*	   <-      EE71     GRAVAÇÃO BYTE OK
*	   <-      EE72     GRAVAÇÃO BYTE COM ERRO
*	   <-      EE73     TERMINO DO PROCESSO DE GRAVAÇÃO
*	      ->   EEDD     ENCERRA COMUNICACAO
*	  ------------------------------------------
*--------------------------------------------------------------------------------
* Data        Responsavel  Motivo
* 20/12/2011  Moacir Jr.   criação
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
unsigned char endmsb, endmsb_bkp;
unsigned char RetByte;       // Para armazenar o valor recebido da Porta Paralela.
unsigned char RetInput[2], RetErro; 
HANDLE hSerial = NULL;

void delay(unsigned int ttempo);
int IniciaSerial();
void TerminaSerial();
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
	int vnumFF = 0, verro, vnumerros, vsendlsb, vsendmsb, vsenddados;
	unsigned char dados, dadosf;
	unsigned char dadosrec;
  string argum;
  
	if (argc < 4)
	{
  	if (argc < 3) {
  		printf("Erro. Nao foi passado o programa a ser enviado.\n");
  		printf("Sintaxe: sendrom <nome do arquivo> <msb/lsb> [endinic]\n");
  		printf("         <nome do arquivo> - nome do arquivo a importar com .bin\n");
  		printf("         <lsb/msb> - grava dado LSB (grava no chip com 0xFE)\n");
  		printf("                     ou MSB (grava no chip com 0xFF)\n");
  		printf("         [endinic] - em hex no formato 9999. Default 0000\n");
  		return 0;
  	}
  	else
  	   argum = "0000";
	}
	else 
	   argum = argv[3];
	
	// Variaveis de Saida

	clrscr();
	
	// Inicio da Rotina
	printf(">Iniciando Programação do Modulo via porta Serial. Usando %s.\n", argv[1]);

	printf(">Inicializando Porta Serial (COM3, 19200, 8, N, 1).\n");
	IniciaSerial();

	printf(">Abrindo Comunicação.\n");
	EnviaComando(0xDD);  // Envia Inicio de Comunicação: 0xDD + 0xDD
	
	printf(">Aguardando Confirmação.\n");
	RecebeComando(); // Recebe Confirmação de Inicio: 0xDD + 0xDD
	
	if (RetInput[0] != 0xDD || RetInput[1] != 0xDD)
		vnumFF = 255;

	// Gravar Dados
	if (vnumFF == 0) {
		printf(">Abrindo Arquivo.\n");
  	strcat(xfilebin,argv[1]);
  	fp = fopen(xfilebin,"rb");
    
    if (fp == NULL) {
  		printf(">Erro ao Abrir Arquivo.\n");
      vnumFF = 255;
    }
  }
	else
		printf(">Comunicação não Estabelecida. Recebido %02X:%02X. Verifique.\n", RetInput[0], RetInput[1]);

	// Enviando Dados (Adress LSB -> MSB -> Dados)
  if (vnumFF != 255) {
    sscanf(argum.substr(2,2).c_str(), "%x", &endlsb);
    sscanf(argum.substr(0,2).c_str(), "%x", &endmsb);
    
    vsendlsb = 0;
    vsendmsb = 0;
    vsenddados = 0;

		printf(">Enviando Dados.\n");
    
  	while (!feof(fp) && vnumFF <= 20)
  	{
  		dadosf = getc(fp);
  		
  		if (dados == 0xFF)
  			vnumFF += 1;
  
  		if (dados != 0xFF)
  			vnumFF = 0;

      if (!vsendlsb || !vsendmsb || (strcmp(argv[2],"lsb") == 0 && (endlsb & 0x01) == 0) || (strcmp(argv[2],"msb") == 0 && (endlsb & 0x01) == 1)) {        
        do {
      		verro = 1;
          RetErro = 0;
      		while (verro)
      		{
            if (vsendlsb && vsendmsb && (strcmp(argv[2],"msb") == 0 && (endlsb & 0x01) == 0)) {
              vsenddados = 1;            
              break;
            }
              
            dados = dadosf;
            
            if (!vsendlsb) 
              dados = endlsb;
            else if (!vsendmsb) 
              dados = endmsb;
               
      			// Grava Dados
      			printf(">%02X",endmsb);
      			printf("%02X",endlsb);
      			printf(" : %02X",dados);
      
      			// Grava Dados
            printf(".");
      			EnviaComando(dados); // Envia Byte: 0xDD + Byte
            printf(".");
      			RetByte = RecebeComando(); // Recebe Byte: 0xDD + Byte
            printf(".");
    
      			// Compara pra ver se Dado Enviado e Recebido estão Iguais 
      			if (RetInput[1] != dados) {
      				printf(" --> Erro Leitura. Leu %02X %02X\n", RetInput[1], RetByte);
              RetErro++;
              if (RetErro >= 0x03) {
      			    RecebeByte(); // Recebe 1 Byte para limpar buffer
                RetErro = 0;
              }
        			EnviaCtrl(0x01); // Comparação Byte Com Erro: 0xEE + 0x01
      			}
      			else {
      				printf(" --> OK.\r");
        			verro = 0;
        			EnviaCtrl(0x00); // Comparação Byte OK: 0xEE + 0x00
        			RecebeComando(); // Rece Controle Final: 0xEE + 69 ou 70
              
            	if (RetInput[0] == 0xEE && RetInput[1] == 0x70)
                AcompanhaGravacao();
        		}
  
            if (!vsendlsb)
              vsendlsb = 1;
            else if (!vsendmsb)
              vsendmsb = 1;
            else if (!vsenddados)
              vsenddados = 1;            
      		} 
        } while (!vsendlsb || !vsendmsb || !vsenddados); 
      }
      
  		if (endlsb == 0xFF)
  		{
  			endmsb += 1;
  			endlsb = 0x00;
  		}
  		else
  			endlsb += 1;
  	}

  	printf(">\nFechando Arquivo.\n");

  	fclose(fp);
  
  	printf(">Finalizando Comunicação.\n");
  
  	EnviaCtrl(0xDD); // Envia FIM: 0xEE + 0xDD

		RecebeComando(); // Recebe Controle Final: 0xEE + 69 ou 70

  	if (RetInput[0] == 0xEE && RetInput[1] == 0x70)
      AcompanhaGravacao();
    
  	TerminaSerial();
  
  	printf(">Dados Enviados.\n");
	  printf(">Programação Concluida. Bom Teste.\n");
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
	char *NomePorta = "COM3"; //COM1, COM2... 

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
  
	printf("\n>PIC: Iniciando Gravação 256 Bytes.\n");

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

