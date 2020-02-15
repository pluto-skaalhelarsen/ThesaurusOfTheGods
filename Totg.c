/*
 *	Totg.c - Thesaurus Of The Gods
 *
 *	D.Merrell - 2/9/20
 *
 *	gcc -m64 -c ListLoader.c
 *	gcc -m64 -o Totg Totg.c ListLoader.o
 *	  Or, possibly:
 *	gcc -m64 -D DEBUG_VER -o Totg Totg.c ListLoader.o
 */
// ./Totg -pmr LoaderTestFile.txt BunchOfNamesMsg.txt
// ./Totg -pr TestAllFile.txt BunchOfNamesMsg.txt > poopoo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <alloca.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdbool.h>

#include "Saurus.h"

#define FILE_NAME_SIZE 512
#define STAGE_SIZE 1024
#define MAX_TEMPLATE_FILE_BUFFER_SIZE 2000000000	// somewhat arbitrary limit on the template file size

#define DEFAULT_SAURUS_FILE_NAME "GoofyThesaurus.txt"
#define DEFAULT_SAURUS_FILE_LIST_NAME "GoofyMultiThesaurus.txt"
#define DEFAULT_TEMPLATE_FILE_NAME "BlandMessage.txt"
#define DEFAULT_CHAIN_FILE_NAME "BlandChain.txt"

#define DFLT_PSUEDORANDOM_SEED 42

	// how to invoke this program
char *Usage[] = 
{
  "Totg (Thesaurus of the Gods) - A text-processing program that reads a text file and re-writes it with thesaurus-like word selections",
  "",
  "Totg [-[<flags>]] [<thesaurus file name> [<template file name> [<chain file name>]]]",
  "    Where:",
  "      <flags> is one of the following:",
  "        ?    Prints this message.",
  "        c    Do not do the usual translation to all lower case for the search.",
  "        m    Indicates that <thesaurus file name> contains a list of file names of saurus data, and not the data itself.",
  "        p    Enables printing of presumably informative messages.",
  "        r <seed num> Uses <seed num> for random selection.",
#if DEBUG_VER
  "        w    Write a chain file instead of the usual substitution output.",
#endif
  "",
  "      <thesaurus file name> is the name of a text file of comma-separated and newline terminated word lists.",
  "",
  "      <template file name> is the name of a text file (ordinary human-readable) with some of the words from the thesaurus file.",
  "",
  "      <chain file name> is the name of a text file containing a numeric sequence that can (optionally) direct the thesaurus word selection.",
  "",
  "    Output is written to stdout.",
  "",
  "    Example: Totg -p GoofyThesaurus.txt BlandMessage.txt > JokeMessage.txt",
  NULL
};

	// the various files
char SaurusFileName[FILE_NAME_SIZE] = DEFAULT_SAURUS_FILE_NAME;
int SaurusFile;
char TemplateFileName[FILE_NAME_SIZE] = DEFAULT_TEMPLATE_FILE_NAME;
int TemplateFile;
char ChainFileName[FILE_NAME_SIZE] = "";

	// text buffers for file content
char *SaurusBuf;
int SaurusBufSize = 0;
char *TemplateBuf;
int TemplateBufSize = 0;
char *ChainBuf;
int ChainBufSize = 0;

char Stage[STAGE_SIZE];
int StageCnt = 0;

	// word vectors (think argv[])
char *TemplateVector;
int TemplateVectorCnt = 0;
char *OutVector;
int OutVectorCnt = 0;

	// seed for random mode of operation
int PsuedoRandomSeed = DFLT_PSUEDORANDOM_SEED;

	// flags
bool f_chatty = false;				// enable printing of informative messages as execution progresses
bool f_random = false;				// enable psuedorandom selection with given seed
bool f_saurus_multiple_file_list = false;	// file named on command line contains a list of file names of saurus files to load
bool f_preserve_case = false;			// do not do the usual translation to all lower case for the search

bool f_saurus_assigned = false;
bool f_template_assigned = false;
bool f_chain_assigned = false;
bool f_seed_assigned = false;
#if DEBUG_VER
bool f_debug_write = false;
#endif

pListItem FileArray;		// array of preliminary items for each of the word files loaded
int FileArrayCnt = 0;		// count of the number of indcies in the preliminary configuration

pListItem SaurusArray;		// array of structured index collections for each of the word files loaded
int SaurusArrayCnt = 0;		// count of the number of indcies in the final configuration

pWordItem WordArray;		// the main sorted and searchable array of all words
int WordArrayCnt = 0;



void EmitConsoleText(char **InText)
{
  char **tptr;

  for(tptr = InText; *tptr; tptr++)
    puts(*tptr);
}

bool IsAnInteger(char *InStr)
{
  char *wptr;

  wptr = InStr;
  while(*wptr)
    if(!isdigit(*wptr++))
      return(false);
  return(true);
}

#define MAX_CHAIN_SIZE 1000000000	// a somewhat arbitrary limit on the chain file size

char *ChainIn;
int ChainInSize = 0;	// size of chain text buffer
int CurChainIn = 0;	// index of the current element in use

bool LoadChainInput(char *InName)
{
  struct stat ChainFileStat;	// file statistics including file size
  int ChainFile;

  if(stat(InName, &ChainFileStat) < 0)
  {
    printf("Cannot stat chain input file %s\n", InName);
    return(false);
  } // if

  ChainInSize = (int) ChainFileStat.st_size;
  if((ChainInSize < 0) || (ChainInSize > MAX_CHAIN_SIZE))
  {
    printf("Bad size of chain input file %s\n", InName);
    return(false);
  } // if

  if((ChainIn = (char *) malloc(ChainInSize +1)) == NULL)
  {
    printf("Cannot allocate memory for input chain\n");
    return(false);
  } // if

  if((ChainFile = open(InName, O_RDONLY)) < 0)
  {
    printf("Cannot open chain file %s\n", InName);
    return(false);
  } // if
  if(read(ChainFile, ChainIn, ChainInSize) < ChainInSize)
  {
    printf("Error reading chain file\n");
    return(false);
  } // if
  close(ChainFile);

  ChainIn[ChainInSize = 0;	// make sure final string is terminated
  CurChainIn = 0;	// point to beginning

  return(true);
}

	// get the next chain selection after verifying match with ent
int NextChainIn(int RequiredEn)
{
  int Sel, En;
  int Start, End;

  if(CurChainIn >= ChainInSize)		// unexpected end of chain
    return(-1);

  for(Start=CurChainIn; (Start < CurChainIn+10) && (!isdigit(ChainIn[Start])); Start++);	// find the start of a number
  for(End=Start; (End < CurChainIn+10) && (ChainIn[End] != ','); End++);	// find the end of a number
  ChainIn[End] = 0;
  Sel = atoi(&ChainIn[Start]);

  for(Start=++End; (Start < CurChainIn+20) && (!isdigit(ChainIn[Start])); Start++);	// find the start of a number
  for(End=Start; (End < CurChainIn+20) && (ChainIn[End] != ';'); End++);	// find the end of a number
  ChainIn[End++] = 0;
  En = atoi(&ChainIn[Start]);

  if(En != RequiredEn)	// mismatch
    return(-1);

  CurChainIn = End;
  return(Sel);
}



	// step through a string and set all characters to lower case
#define STRING_LENGTH_LIMIT 512		// don't loop into seg fault land, please
void ToLowerCase(char *InWord)
{
  int i;
  char *wptr;

  if(InWord == NULL)
    return;

  for(i=0, wptr=InWord; *wptr && (i < STRING_LENGTH_LIMIT); i++, wptr++)
    *wptr = tolower(*wptr);

  if(i >= STRING_LENGTH_LIMIT)
  {
    InWord[20] = 0;
    printf("Word length exceeded in string starting with %s\n", InWord);
  } // if
}


bool EmitTransformation(char *InBuf, int InBufCnt)
{
  int i, j, Len;
  char *wptr, *Start, *End;
  WordItem DummyKey;	// for bsearch comparisons
  pWordItem RetPtr;
  int InitialUpperCase;

  if((InBuf == NULL) || (InBufCnt == 0))
    return(false);
  for(Start=End=wptr=InBuf; InBufCnt > (int) ((unsigned long) wptr - (unsigned long) InBuf); wptr++)
  {
    while((*End) && (isalnum(*End) || (*End == '-')))	// find end of word
      End++;
    if((Len = (int) ((unsigned long) End - (unsigned long) Start)) > 0)		// got a word
    {
      strncpy(Stage, Start, Len);
      Stage[Len] = 0;
      if(!f_preserve_case)	// do search in lower case
      {
        InitialUpperCase = isupper(Stage[0]);	// when the initial is uppercase, the case will be restored (all caps will not be restored)
        ToLowerCase(Stage);
      } // if
//printf("\"%s\" ", Stage);	// all right... I debug without a debugger sometimes
//fflush(stdout);
      DummyKey.Word = Stage;	// set up dummy key struct for search
      if((RetPtr = bsearch(&DummyKey, WordArray, WordArrayCnt, sizeof(WordItem), CmpWordString)) != NULL)	// match in the word list
      {
#if DEBUG_VER
        if(f_debug_write)
        {
          fprintf(stdout, "%d,%d;", RetPtr->Entry, SaurusArray[RetPtr->List].HeadIndex[RetPtr->Head].SynCnt);
        } // if
#else
        if(f_random)	// select a replacement word at random from the compatible list
        {
          if((i = rand() % SaurusArray[RetPtr->List].HeadIndex[RetPtr->Head].SynCnt) == RetPtr->Entry)	// random index should not be same as original
            i = rand() % SaurusArray[RetPtr->List].HeadIndex[RetPtr->Head].SynCnt;	// so do it again
        } // if
        else	// not random - can we match it to the input chain?
          if((i = NextChainIn(SaurusArray[RetPtr->List].HeadIndex[RetPtr->Head].SynCnt)) < 0)
          {
            printf("Mismatch with input chain. Cannot continue.\n");
            return(false);
          } // if
        strcpy(Stage, SaurusArray[RetPtr->List].HeadIndex[RetPtr->Head].SynTbl[i]);
        if(!f_preserve_case)	// do search in lower case
        {
          if(InitialUpperCase)
            Stage[0] = (char) toupper(Stage[0]);
        } // if
        fputs(Stage, stdout);
      } // if
      else	// no match - this is just a filler word between
      {
        if(!f_preserve_case)	// restore case
        {
          if(InitialUpperCase)
            Stage[0] = (char) toupper(Stage[0]);
        } // if
        fputs(Stage, stdout);
#endif
      } // else
    } // if
    for(Start = End; (*Start) && !((isalnum(*Start)) || (*Start == '-')); Start++)	// find end of separator
#if DEBUG_VER
      ;
#else
      fputc(*Start, stdout);
#endif
    wptr = End = Start;
  } // for

  return(true);
}

int main(int argc, char *argv[])
{
  int ArgNdx;
  char *aptr, *wptr;
  struct stat TemplateFileStat;
  int SeedSource;

  int i, j, k;
  int CurFileNdx;	// index in the name file array of the current file being processed

  if((SeedSource = (int) clock()) < 0)
    SeedSource = -SeedSource;

  if(argc < 2)
  {
    EmitConsoleText(Usage);
    return(-1);
  } // if

  if(argc > 1)
  {
    for(ArgNdx=1; ArgNdx < argc; ArgNdx++)
    {
      aptr = argv[ArgNdx];
      if(*aptr == '-')	// flag
      {
        while(*(++aptr))
          switch(*aptr)
          {
          case '?':
          {
            EmitConsoleText(Usage);
            return(-1);
          }
          case 'c':
            f_preserve_case = true;
            break;
          case 'm':
            f_saurus_multiple_file_list = true;
            break;
          case 'p':
            f_chatty = true;
            break;
          case 'r':
            f_random = true;
            break;
#if DEBUG_VER
          case 'w':
            f_debug_write = true;
            break;
#endif
          default:
            printf("Unrecognized command line switch %c\n", *aptr);
            return(-1);
          } // switch
      } // if
      else	// paramter
      {
        if(f_random && IsAnInteger(aptr))
        {
            PsuedoRandomSeed = atoi(aptr);
            f_seed_assigned = true;
        } // if
        else if(!f_saurus_assigned)
        {
          strcpy(SaurusFileName, aptr);
          f_saurus_assigned = true;
        } // else
        else if(!f_template_assigned)
        {
          strcpy(TemplateFileName, aptr);
          f_template_assigned = true;
        } // else
        else if(!f_chain_assigned)
        {
          if(!f_random)
          {
            strcpy(ChainFileName, aptr);
            f_chain_assigned = true;
          } // if
        } // else
      } // else
    } // for
  } // if

  if(f_random)
  {
    if(f_seed_assigned)		// a specific seed was specified on the command line
      srand(PsuedoRandomSeed);
    else
      srand(SeedSource);
  } // if
  else
    if(!f_chain_assigned)
      strcpy(ChainFileName, DEFAULT_CHAIN_FILE_NAME);

  if(f_chatty)
  {
    if(f_saurus_multiple_file_list)
      printf("Multi-saurus file: %s\n", SaurusFileName);
    else
      printf("Saurus file: %s\n", SaurusFileName);

    printf("Template file: %s\n", TemplateFileName);

    if(f_random == true)		// a specific seed was specified on the command line
        printf("Generati random text with seed %d\n", (f_seed_assigned)?PsuedoRandomSeed:SeedSource);
    else
      printf("Chain file: %s\n", ChainFileName);

    if(f_preserve_case)
      printf("The word search will be done without the usual translation to all lower case.\n");
  } // if

	// =========== load the 'saurus ============

  if(f_saurus_multiple_file_list)	// saurus file on command line is a list of saurus files
  {
    if(!LoadFileNameList(SaurusFileName, &FileArray, &FileArrayCnt))
    { 
      printf("Error loading text file of file names to process\n");
      return(-1);
    } // if
  } // if
  else	// saurus file on command line is the name of the only file which contains the thesaurus entries themselves
  {
    if(!LoadSingleFileName(SaurusFileName, &FileArray, &FileArrayCnt))
    { 
      printf("Error loading single text file of saurus entries\n");
      return(-1);
    } // if
  } // else

  if(f_chatty)
    printf("File array count %d\n", FileArrayCnt);

	// loop through the files named in the file list file, loading each of them and building an index
  for(CurFileNdx=0; CurFileNdx < FileArrayCnt; CurFileNdx++)
  {
    if(f_chatty)
      printf("---- Processing word list file %s\n", FileArray[CurFileNdx].OriginFileName);

    if(!LoadWordList(&FileArray[CurFileNdx]))
    {
      printf("Error loading file list at index %d (%s)\n", CurFileNdx, FileArray[CurFileNdx].OriginFileName);
      return(-1);
    } // if
  } // for

  if(!LoadIndexList(FileArray, FileArrayCnt, &SaurusArray, &SaurusArrayCnt))
  { 
    printf("Error loading index structures from loaded files\n");
    return(-1);
  } // if

	// create main word array
  if(!CreateMainWordArray(SaurusArray, SaurusArrayCnt, &WordArray, &WordArrayCnt))
  {
    printf("Error encountered while creating main word array\n");
    return(-1);
  } // if

  if(f_chatty)
    printf("Total word count is %d\n", WordArrayCnt);

	// ================ load the template ==================

  if(stat(TemplateFileName, &TemplateFileStat) < 0)
  {
    printf("Cannot stat name list file %s\n", TemplateFileName);
    return(false);
  } // if

  if((TemplateFileStat.st_size < (size_t) 0) || (TemplateFileStat.st_size > (size_t) MAX_TEMPLATE_FILE_BUFFER_SIZE))
  {
    printf("Cannot process name list file of size %d\n", (int) TemplateFileStat.st_size);
    return(false);
  } // if
  TemplateBufSize = (int) TemplateFileStat.st_size;
  if((TemplateBuf = (char *) malloc(TemplateBufSize+1)) == NULL)
  {
    printf("Cannot allocate template buffer\n");
    return(false);
  } // if
  TemplateBuf[TemplateBufSize] = 0;	// make sure string at end is null terminated

  if((TemplateFile = open(TemplateFileName, O_RDONLY)) < 0)
  {
    printf("Cannot open template file %s\n", TemplateFileName);
    return(false);
  } // if
  if(read(TemplateFile, TemplateBuf, TemplateBufSize+1) != TemplateBufSize)
  {
    printf("Error reading template file\n");
    exit(-1);
  } // if
  close(TemplateFile);

	// ================== optionally load the chain input ====================
  if(!f_random)
  {
    if(!f_chain_assigned)
      strcpy(ChainFileName, DEFAULT_CHAIN_FILE_NAME);
    if(!LoadChainInput(ChainFileName))
    {
      printf("Error loading chain input file %s\n", ChainFileName);
      return(-1);
    } // if
  } // if

	// ================== transform the input message text ===================
  if(!EmitTransformation(TemplateBuf, TemplateBufSize))
  {
    printf("Error encountered writing transformed output\n");
    return(-1);
  } // if

  return(0);
}

