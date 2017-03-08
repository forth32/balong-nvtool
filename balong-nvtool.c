#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "nvfile.h"
#include "nvio.h"
#include "nvid.h"

// Файл с открытым образом nvram
FILE* nvf=0;

int mflag=-1;
int moff=0;
int mdata[128];
int midx;

int aflag=-1;
int aoff=0;
char adata[128];


//****************************************************************
//*   Разбор параметров ключа -m
//****************************************************************
void parse_mflag(char* arg) {
  
char buf[16384];
char* sptr, *offptr;
int endflag=0;

strcpy(buf,arg);

// проверяем на наличие байта +
sptr=strchr(buf,'+');

if (sptr != 0) {
  // есть смещение
  offptr=sptr+1;
  *sptr=0;
  sscanf(buf,"%i",&mflag);
  // ищем первое двоеточие
  sptr=strchr(sptr+1,':');
  if (sptr == 0) {
    printf("\n - в ключе -m не указано ни одного замещающего байта\n");
    exit(1);
  }
  *sptr=0;
  sscanf(offptr,"%x",&moff);
}

else {
  // нет смещения
  // ищем первое двоеточие
  sptr=strchr(buf,':');
  if (sptr == 0) {
    printf("\n - в ключе -m не указано ни одного замещающего байта\n");
    exit(1);
  }
  *sptr=0;
  sscanf(buf,"%i",&mflag);
}
sptr++; // на первый байт;
do {
  offptr=sptr;
  sptr=strchr(sptr,':'); // ищем очередное двоеточие
  if (sptr != 0) *sptr=0;
  else endflag=1;
  sscanf(offptr,"%x",&mdata[midx]);
  if (mdata[midx] > 0xff) {
    printf("\n Ошибочный байт 0х%x в ключе -m",mdata[midx]);
    exit(1);
  }
  midx++;
  sptr++;
} while (!endflag);
}


//****************************************************************
//*   Разбор параметров ключа -a
//****************************************************************
void parse_aflag(char* arg) {
  
char buf[16384];
char* sptr, *offptr;

strcpy(buf,arg);

// проверяем на наличие байта +
sptr=strchr(buf,'+');

if (sptr != 0) {
  // есть смещение
  offptr=sptr+1;
  *sptr=0;
  sscanf(buf,"%i",&aflag);
  // ищем первое двоеточие
  sptr=strchr(sptr+1,':');
  if (sptr == 0) {
    printf("\n - в ключе -a не указано ни одного замещающего байта\n");
    exit(1);
  }
  *sptr=0;
  sscanf(offptr,"%x",&aoff);
}

else {
  // нет смещения
  // ищем первое двоеточие
  sptr=strchr(buf,':');
  if (sptr == 0) {
    printf("\n - в ключе -a не указано ни одного замещающего байта\n");
    exit(1);
  }
  *sptr=0;
  sscanf(buf,"%i",&aflag);
}
sptr++; // на первый байт;
strncpy(adata,sptr,128);
}

//****************************************************************
//*  Печать заголовка утилиты
//****************************************************************
void utilheader() {
  
printf("\n Утилита для редактирования образов NVRAM устройств на чипсете Hisilicon Balong, V1.0.%i, (c) forth32, 2016, GNU GPLv3",BUILDNO);
printf("\n-------------------------------------------------------------------------------------------------------------------------\n");

}


//****************************************************************
//* Печать help-блока
//****************************************************************
void utilhelp(char* utilname) {
 
utilheader();  
printf("\n Формат командной строки:\n\n\
%s [ключи] <имя файла-образа NVRAM>\n\n\
 Допустимы следующие ключи:\n\n\
-l       - вывести карту образа NVRAM\n\
-u       - печать уникальных идентификаторов и настроек\n\
-e       - извлечь все ячейки \n\
-x item  - извлечь в файл ячейку item\n\
-d item  - вывести дамп ячейки item\n\
-r item:file - заменить ячейку item на содержимое файла file\n\n\
-m item[+off]:nn[:nn...] - заменить байты в item на байты, указанные в команде\n\
-a item[+off]:text - записать текстовую строку в item\n\
        *  если +off не указан - замена начинается с нулевого байта. Смещение задается в hex\n\n\
-i imei    записать новый IMEI\n\
-s serial- записать новый серийный номер\n\
-c       - извлечь все компонентные файлы \n\
-b oem|simlock|all - произвести подбор OEM, SIMLOCK или обоих кодов\n\
\n",utilname);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
       
void main(int argc,char* argv[]) {

int i;
int opt;
int res;
uint32_t pos;
char buf[16384];
int len;
char rfilename[200];
char* sptr;
FILE* in;
char imei[16];
char serial[20];

int xflag=-1;
int lflag=0;
int cflag=0;
int eflag=0;
int dflag=-1;
int rflag=-1;
int bflag=0;
int uflag=0;
int iflag=0;
int sflag=0;

while ((opt = getopt(argc, argv, "hlucex:d:r:m:b:i:s:a:")) != -1) {
  switch (opt) {
   case 'h': 
    utilhelp(argv[0]);
    return;

   case 'b':
     switch (optarg[0]) {
       case 'o':
       case 'O':
	  bflag=1;
	  break;
	  
       case 's':
       case 'S':
	  bflag=2;
	  break;
	  
       case 'a':
       case 'A':
	  bflag=3;
	  break;

       default:  
	  printf("\n - неправильное значение ключа -b\n");
          return;
     }
     break;
    
   case 'l':
     lflag=1;
     break;
     
   case 'u':
     uflag=1;
     break;
     
   case 'e':
     eflag=1;
     break;
     
   case 'c':
     cflag=1;
     break;
     
   case 'i':
     iflag=1;
     strncpy(imei,optarg,15);
     break;
     
   case 's':
     sflag=1;
     memset(serial,0xff,16);
     strncpy(serial,optarg,20);
     break;
     
   case 'x':
     sscanf(optarg,"%i",&xflag);
     break;
     
   case 'd':
     sscanf(optarg,"%i",&dflag);
     break;
     
   case 'r':
     strcpy(buf,optarg);
     sptr=strchr(buf,':');
     if (sptr == 0) {
       printf("\n - Не указано имя файла в ключе -r\n");
       return;
     }
     *sptr=0;
     sscanf(buf,"%i",&rflag);
     strcpy(rfilename,sptr+1);
     break;
     
   case 'm':
     parse_mflag(optarg);
     break;
     
   case 'a':
     parse_aflag(optarg);
     break;
     
   case '?':
   case ':':  
     return;
  }
}  


if (optind>=argc) {
    printf("\n - Не файл образа nvram, для подсказки используйте ключ -h\n");
    return;
}

// открываем образ nvram
nvf=fopen(argv[optind],"r+");
if (nvf == 0) return;

// читаем заголовок образа
res=fread(&nvhd,1,sizeof(nvhd),nvf);
if (res != sizeof(nvhd)) {
  printf("\n - Ошибка чтения файла %s\n",argv[optind]);
  return;
}
if (nvhd.magicnum != FILE_MAGIC_NUM) {
  printf("\n - Файл %s не является оразом NVRAM\n",argv[optind]);
  return;
}

// Читаем каталог файлов
fseek(nvf,nvhd.file_offset,SEEK_SET);
pos=nvhd.ctrl_size;

for(i=0;i<nvhd.file_num;i++) {
 // вынимаем имеющуюся в файле информацию 
 fread(&flist[i],1,36,nvf);
 // вычисляем смещение до данных файла
 flist[i].offset=pos;
 pos+=flist[i].size;
} 

// Читаем каталог ячеек
fseek(nvf,nvhd.item_offset,SEEK_SET);
fread(&itemlist,1,nvhd.item_size,nvf);

// вывод карт и параметров
if (lflag) {
  utilheader();
  print_hd_info();
  print_filelist();
  print_itemlist();
  return;
}  

// Вывод утинкальных параметров
if (uflag) {
  print_data();
  return;
}  

// извлечение файлов и ячеек
if (cflag) extract_files();
if (eflag) extract_all_item();
if (xflag != -1) {
  printf("\n Извлекается ячейка %i\n",xflag);
  item_to_file(xflag,"");
}  

// Просмотр дампа ячеек
if (dflag != -1) {
  len=load_item(dflag,buf);
  if (len == -1) {
    printf("\n - Ячейка %i не найдена\n",dflag);
    return;
  }
  printf("\n ----- Ячейка: %i   Размер: %i байт ",dflag,len);
  sptr=find_desc(dflag);
  if (strlen(sptr) != 0) printf(" Имя: %s ",sptr);
  printf("-----\n");
  fdump(buf,len,0,stdout);
  printf("\n");
}

// Замена ячеек
if (rflag != -1) {
  len=itemlen(rflag);
  if (len == -1) {
    printf("\n - Ячейка %i не найдена\n",rflag);
    return;
  }  
  in=fopen(rfilename,"r");
  if (in == 0) {
    printf("\n - Ошибка открытия файла %s\n",rfilename);
    return;
  }
  fseek(in,0,SEEK_END);
  if (ftell(in) != len) {
    printf("\n - Размер файла (%u) не соответствует размеру ячейки (%i)\n",(uint32_t)ftell(in),len);
    return;
  }
  fseek(in,0,SEEK_SET);
  fread(buf,1,len,in);
  save_item(rflag,buf);
  printf("\n Ячейка %i успешно записана\n",rflag);
}

// прямое редактирование ячеек
if (mflag != -1) {
  len=load_item(mflag,buf);
  if (len == -1) {
    printf("\n - Ячейка %i не найдена\n",mflag);
    return;
  }  
  if ((midx+moff) > len) {
    printf("\n - превышена длина ячейки %i\n",mflag);
    return;
  } 
  
  for(i=0;i<midx;i++) buf[moff+i]=mdata[i];
  save_item(mflag,buf);
  printf("\n Ячейка %i отредактирована\n",mflag);
 
}

// Запись текстовых строк в ячейку
if (aflag != -1) {
  len=load_item(aflag,buf);
  if (len == -1) {
    printf("\n - Ячейка %i не найдена\n",mflag);
    return;
  }  
  if ((strlen(adata)+aoff) > len) {
    printf("\n - превышена длина ячейки %i\n",mflag);
    return;
  } 
  memcpy(buf+aoff,adata,strlen(adata));
  save_item(aflag,buf);
  printf("\n Ячейка %i отредактирована\n",aflag);
}  

// подбор кодов блокировкии
if (bflag) {
  utilheader();
  switch (bflag) {
    case 1:
      brute(1);
      return;
      
    case 2:
      brute(2);
      return;
      
    case 3:
      brute(1);
      brute(2);
      return;
  }   
}

// запись IMEI
if (iflag) write_imei(imei);

// запись серийника
if (sflag) write_serial(serial);
printf("\n");  
}