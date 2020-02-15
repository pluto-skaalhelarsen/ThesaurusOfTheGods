/*
 *	ListLoader.c - load thesaurus-like lists
 *
 *	D.Merrell - 2/3/20
 *
 *	gcc -m64 -c ListLoader.c
 *
 *	Use gcc -D TEST_DRIVER -m64 -o ListLoader ListLoader.c
 *	  to build a standalone test version of the loader
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdbool.h>

#include "Saurus.h"

#define DEBUG_DATA 1

#define IN_BUF_SIZE 3000000	// size of input character buffer

	// ............................ load the list of file names to process ..........................

#define MAX_NAME_FILE_BUFFER_SIZE 100000	// limit the size of the name file list

static char *NameFileBuffer;		// this is where the actual name strings reside, although they are accessed through the vector
static int NameFileBufferSize;

	//.............. load the contents of a single saurus file - this file contains the actual index lists - it is not a list of file names ........................
bool LoadSingleFileName(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt)
{
  pListItem LocalSaurusList;
  int LocalSaurusCnt;

  LocalSaurusCnt = 1;
  if((LocalSaurusList = (pListItem) malloc(sizeof(ListItem))) == NULL)
  {
    printf("Cannot allocate single name file array\n");
    return(false);
  } // if
  LocalSaurusList->OriginFileName = NameFileName;

  *SaurusList = LocalSaurusList;
  *SaurusListCnt = LocalSaurusCnt;

  return(true);
}
	//........... load the contents of multiple files, each of which contain index lists - the file named here only contains a list of file names - those files contain the saurus data ...............

bool LoadFileNameList(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt)
{
  int i;
  int NameFile;
  struct stat NameFileStat;
  char *StartPtr, *EndPtr;
  pListItem LocalSaurusList;
  int LocalSaurusCnt;

  if(stat(NameFileName, &NameFileStat) < 0)
  {
    printf("Cannot stat name list file %s\n", NameFileName);
    return(false);
  } // if

  if((NameFileStat.st_size < (size_t) 0) || (NameFileStat.st_size > (size_t) MAX_NAME_FILE_BUFFER_SIZE))
  {
    printf("Cannot process name list file of size %d\n", (int) NameFileStat.st_size);
    return(false);
  } // if
  NameFileBufferSize = (int) NameFileStat.st_size;
  if((NameFileBuffer = (char *) malloc(NameFileBufferSize+1)) == NULL)
  {
    printf("Cannot allocate name file buffer\n");
    return(false);
  } // if
  if((NameFile = open(NameFileName, O_RDONLY)) < 0)
  {
    printf("Cannot open name list file %s\n", NameFileName);
    return(false);
  } // if
  if(read(NameFile, NameFileBuffer, NameFileBufferSize+1) != NameFileBufferSize)
  {
    printf("Error reading name file list\n");
    exit(-1);
  } // if
  close(NameFile);

  NameFileBuffer[NameFileBufferSize] = 0;	// make sure last string is terminated
  for(StartPtr=EndPtr=NameFileBuffer, LocalSaurusCnt=0; (*EndPtr > ' ') && ((EndPtr = strchr(StartPtr, '\n')) != NULL); StartPtr = ++EndPtr)
  {
    *EndPtr = 0;
    LocalSaurusCnt++;
  } // for
  if((LocalSaurusList = (pListItem) calloc(LocalSaurusCnt, sizeof(ListItem))) == NULL)
  {
    printf("Cannot allocate name file list array\n");
    return(false);
  } // if
  for(i=0, StartPtr=NameFileBuffer; i < LocalSaurusCnt; i++)
  {
    LocalSaurusList[i].OriginFileName = StartPtr;
    for(StartPtr++; *StartPtr; StartPtr++);
    StartPtr++;
  } // for

  *SaurusList = LocalSaurusList;
  *SaurusListCnt = LocalSaurusCnt;

  return(true);
}

	// ............................ word list file load and index creation ...................

#define MAX_WORD_FILE_BUFFER_SIZE 500000000	// somewhat arbitrary limit to half gig name file

	// ......................... starting with a list item containing a file name, open the file, read it into a buffer, and look for sub-buffer markers .................
bool LoadWordList(pListItem CurListItem)
{
  int i;
  char *StartPtr, *EndPtr;
  struct stat WordFileStat;	// file statistics including file size
  int InFile;
  int LocalIndexCnt;		// Local count of head items

  if(stat(CurListItem->OriginFileName, &WordFileStat) < 0)
  {
    printf("Cannot stat word list file %s\n", CurListItem->OriginFileName);
    return(false);
  } // if

  if((WordFileStat.st_size < (size_t) 0) || (WordFileStat.st_size > (size_t) MAX_WORD_FILE_BUFFER_SIZE))
  {
    printf("Cannot process word list file of size %d\n", (int) WordFileStat.st_size);
    return(false);
  } // if
  CurListItem->OriginFileSize = (int) WordFileStat.st_size;
  if((CurListItem->InBuf = (char *) malloc(CurListItem->OriginFileSize+1)) == NULL)
  {
    printf("Cannot allocate word file buffer\n");
    return(false);
  } // if
  if((InFile = open(CurListItem->OriginFileName, O_RDONLY)) < 0)
  {
    printf("Cannot open name list file %s\n", CurListItem->OriginFileName);
    return(false);
  } // if
  if(read(InFile, CurListItem->InBuf, CurListItem->OriginFileSize+1) != CurListItem->OriginFileSize)
  {
    printf("Error reading word file list\n");
    exit(-1);
  } // if
  close(InFile);

  CurListItem->InBuf[CurListItem->OriginFileSize] = 0;	// make sure last string is terminated

	// count the number of separate index aggregations in the current buffer
  for(StartPtr=EndPtr=CurListItem->InBuf, LocalIndexCnt=0; (*EndPtr > ' ') && ((EndPtr = strchr(StartPtr, '#')) != NULL); StartPtr = ++EndPtr)
  {
    *EndPtr = 0;
    LocalIndexCnt++;
  } // for

  CurListItem->SerialNum = LocalIndexCnt + 1;

  return(true);
}

	// .......................... starting with a list entry pointing to a raw buffer, create the index structure in that entry ....................
bool ParseIndex(pListItem CurListItem)
{
  int i;
  char *StartPtr, *EndPtr;
  int LocalIndexCnt;		// Local count of head items

	// count the number of entry lines in the current buffer
  for(StartPtr=EndPtr=CurListItem->InBuf, LocalIndexCnt=0; (*EndPtr > ' ') && ((EndPtr = strchr(StartPtr, '\n')) != NULL); StartPtr = ++EndPtr)
  {
    *EndPtr = 0;
    LocalIndexCnt++;
  } // for

	// make an index item for each entry line
  if((CurListItem->HeadIndex = (HeadItem *) calloc(LocalIndexCnt, sizeof(HeadItem))) == NULL)		// head entries
  {
    printf("Cannot allocate head index for %s\n", CurListItem->OriginFileName);
    return(false);
  } // if

	// fill in the index head items with a list of the entry words
  for(i=0, StartPtr=CurListItem->InBuf; i < LocalIndexCnt; i++)
  {
    CurListItem->HeadIndex[i].HeadBufPtr = StartPtr;
    StartPtr = strchr(StartPtr, 0);	// point to end of line
    StartPtr++;				// point to beginning of next line
    if(!LoadIndexEntryLine(&CurListItem->HeadIndex[i]))
    {
      printf("Error loading index entry\n");
      return(false);
    } // if
  } // for

  CurListItem->HeadIndexCnt = LocalIndexCnt;

  return(true);
}

	// ........................ allocate index space for the individual sub-buffers (if present) and initialize the index structures ..............................
bool LoadIndexList(pListItem InFileArray, int InFileArrayCnt, pListItem *SaurusList, int *SaurusListCnt)
{
  int i, j, Ndx;
  char *StartPtr, *EndPtr;
  pListItem LocalSaurusList;
  int LocalSaurusCnt;
  int LocalIndexCnt;		// Local count of head items

  for(i=0, *SaurusListCnt=0; i < InFileArrayCnt; i++)
     LocalSaurusCnt += InFileArray[i].SerialNum;

//printf("Total number of saurus index %d\n",  LocalSaurusCnt);

  if((LocalSaurusList = (pListItem) calloc(LocalSaurusCnt, sizeof(ListItem))) == NULL)
  {
    printf("Cannot allocate name file list array\n");
    return(false);
  } // if
  for(i=0, Ndx=0; i < InFileArrayCnt; i++)
  {
    for(j=0, StartPtr=InFileArray[i].InBuf; j < InFileArray[i].SerialNum; j++)
    {
      LocalSaurusList[Ndx].OriginFileName = InFileArray[i].OriginFileName;
      LocalSaurusList[Ndx].SerialNum = j;
      LocalSaurusList[Ndx].InBuf = StartPtr;
      EndPtr = strchr(&StartPtr[1], 0);
      LocalSaurusList[Ndx].OriginFileSize = (int) ((unsigned long) EndPtr - (unsigned long) StartPtr) + 1;
      if(!ParseIndex(&LocalSaurusList[Ndx]))
      {
        printf("Error parsing index from buffer for index %d\n", Ndx);
        return(false);
      } // if
      if(j < (InFileArray[i].SerialNum - 1))
      {
        if(*EndPtr <= ' ')	// look for 0x0d
          EndPtr++;
        if(*EndPtr <= ' ')	// look for 0x0a
          EndPtr++;
      } // if              
      StartPtr = EndPtr;
      Ndx++;
    } // for
  } // for

  *SaurusListCnt = LocalSaurusCnt;
  *SaurusList = LocalSaurusList;

  return(true);
}

	//............................. load a line of comma separated words as an entry in an index item .........................
bool LoadIndexEntryLine(pHeadItem CurIndexItem)
{
  int i, LocalWordCnt;
  char *StartPtr, *EndPtr;

	// count the number of entry lines in the current buffer
  for(StartPtr=EndPtr=CurIndexItem->HeadBufPtr, LocalWordCnt=0; (*EndPtr > ' ') && ((EndPtr = strchr(StartPtr, ',')) != NULL); StartPtr = ++EndPtr)
  {
    *EndPtr = 0;
    LocalWordCnt++;
  } // for

  if((CurIndexItem->SynTbl = (char **) calloc(LocalWordCnt, sizeof(char *))) == NULL)
  {
    printf("Cannot allocate syn table for %d items\n", LocalWordCnt);
    return(false);
  } // if

  for(i=0, StartPtr=CurIndexItem->HeadBufPtr; i < LocalWordCnt; i++)
  {
    CurIndexItem->SynTbl[i] = StartPtr;
    StartPtr += strlen(StartPtr);	// point to end of word
    StartPtr++;				// point to beginning of next word
  } // for

  CurIndexItem->SynCnt = LocalWordCnt;

  return(true);
}


	//............................. string comparison function for sort and search in the main word array ..............................
int CmpWordString(const void *p1, const void *p2)
{
  return(strcmp(((pWordItem) p1)->Word, ((pWordItem) p2)->Word));
}

	// ............................ create main word array ...............................
bool CreateMainWordArray(pListItem ListArray, int ListArrayCnt, pWordItem *WordList, int *WordListCnt)
{
  int i, j, k;
  int TotalWords;
  pWordItem LocalWordList;

	// count the total number of words in the loaded files
  for(i=0, TotalWords=0; i < ListArrayCnt; i++)
    for(j=0; j < ListArray[i].HeadIndexCnt; j++)
      TotalWords += ListArray[i].HeadIndex[j].SynCnt;

	// create the main sorted table of all words
  if((LocalWordList = (pWordItem) calloc(TotalWords, sizeof(WordItem))) == NULL)
  {
    printf("Cannot allocate word list for %d words\n", TotalWords);
    return(false);
  } // if

  for(i=0, TotalWords=0; i < ListArrayCnt; i++)
    for(j=0; j < ListArray[i].HeadIndexCnt; j++)
      for(k=0; k < ListArray[i].HeadIndex[j].SynCnt; k++)
      {
        LocalWordList[TotalWords].Word = ListArray[i].HeadIndex[j].SynTbl[k];
        LocalWordList[TotalWords].List = i;
        LocalWordList[TotalWords].Head = j;
        LocalWordList[TotalWords].Entry = k;
        TotalWords++;
      } // for
  
  qsort(LocalWordList, TotalWords, sizeof(WordItem), CmpWordString);

  *WordList = LocalWordList;
  *WordListCnt = TotalWords;

  return(true);
}

	// ............................ stand alone test driver ..............................
#if TEST_DRIVER

#define DFLT_IN_FILE_NAME "LoaderTestFile.txt"
#define NAME_SIZE 512

char InFileName[NAME_SIZE] = DFLT_IN_FILE_NAME;

pListItem FileArray;		// array of preliminary items for each of the word files loaded
int FileArrayCnt = 0;		// count of the number of indcies in the preliminary configuration

pListItem SaurusArray;		// array of structured index collections for each of the word files loaded
int SaurusArrayCnt = 0;		// count of the number of indcies in the final configuration

pWordItem WordArray;		// the main sorted and searchable array of all words
int WordArrayCnt = 0;

int main(int argc, char *argv[])
{
  int i, j, k, tmp;
  int CurFileNdx;	// index in the name file array of the current file being processed
  char *Start, *PrevPtr, *wptr;

  if(argc > 1)
    strcpy(InFileName, argv[1]);	// override the default list of file names to load

  if(!LoadFileNameList(InFileName, &FileArray, &FileArrayCnt))
  { 
    printf("Error loading text file of file names to process\n");
    return(-1);
  } // if

	// loop through the files named in the file list file, loading each of them and building an index
  for(CurFileNdx=0; CurFileNdx < FileArrayCnt; CurFileNdx++)
  {
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

#if DEBUG_DATA
	// .................. report repeated items ..........................
  for(i=j=k=0; i < WordArrayCnt; i++)
    if(strcmp(WordArray[i].Word, WordArray[j].Word) == 0)	// match
    {
      if(i != j)	// at different locations
      {
        printf("Duplicate %s (list %d, index %d, loc %d from %s)\n        = %s (list %d, index %d, loc %d from %s)\n",
          WordArray[i].Word, WordArray[i].List, WordArray[i].Head, WordArray[i].Entry, SaurusArray[WordArray[i].List].OriginFileName,
          WordArray[j].Word, WordArray[j].List, WordArray[j].Head, WordArray[j].Entry, SaurusArray[WordArray[j].List].OriginFileName);
        k++;
      } // if
    } // if
    else
    {
      j = i;
    } // else    

  printf("------ Found %d repeated items ---------\n", k);

	// ................................... report singleton index entries (has a head word but no other entries in its list) ..........................
  for(i=k=0; i < SaurusArrayCnt; i++)
    for(j=0; j < SaurusArray[i].HeadIndexCnt; j++)
      if(SaurusArray[i].HeadIndex[j].SynCnt == 1)
      {
        printf("Singleton %s (list %d, index %d, from %s)\n", SaurusArray[i].HeadIndex[j].SynTbl[0], i, j, SaurusArray[i].OriginFileName);
        k++;
      } // if

  printf("------ Found %d singleton entries ------\n", k);

#endif	// DEBUG_DATA



  return(0);
}

#endif	// TEST_DRIVER

