/*
 *	ListTool.c- diagnostic and repair tool for List loader data
 *
 *	D.Merrell - 2/6/20
 *
 *	gcc -m64 -c ListLoader.c
 *	gcc -m64 -o ListTool ListTool.c ListLoader.o -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <alloca.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdbool.h>

#include "Saurus.h"

#define PSUEDO_SEED 42		// rng seed

void ToLowerCase(char *InWord);
bool SetAllWordsToLowerCase(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int WordListCnt);
bool IsSingleWord(char *InString);
bool RemoveAllMultiwordEntries(pWordItem WordList, int WordListCnt);
int FindWord(char *InSeeking, char **InArray, int InArrayCnt);
bool ReindexSaurus(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int *WordListCnt);
bool RepeatedItemResolve(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int *WordListCnt);
bool RemoveIndexSingletonEntries(pHeadItem InArray, int *InArrayCnt);
bool RemoveAllSingletonEntries(pListItem ListArray, int ListArrayCnt);
bool XformIndexToFullPartition(pListItem InItem);

bool RandomizeIndexEntries(pHeadItem InIndex, int InIndexCnt);
bool SaveSingleIndexToFile(pListItem InItem, char *OutFileName);
bool SaveAllToMultiIndexFile(pListItem ListArray, int ListArrayCnt, char *OutFileName);


	// ............... step through a string and set all characters to lower case ..................................
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

	// ............. set all words in the word list to lower case .................
bool SetAllWordsToLowerCase(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int WordListCnt)
{
  int i;

  if((ListArray == NULL) || (WordList == NULL))
    return(false);

  if((ListArrayCnt == 0) || (WordListCnt == 0))	// nothing to do
    return(true);

  for(i=0; i < WordListCnt; i++)
  {
    if(WordList[i].Word == NULL)
      continue;
    ToLowerCase(WordList[i].Word);
  } // for

  return(true);
}

	// ................... examine a string for embedded blanks .....................
bool IsSingleWord(char *InString)
{
  if(InString == NULL)
    return(false);
  if(strchr(InString, ' ') != NULL)
    return(false);
  if(strlen(InString) == 0)
    return(false);
  return(true);
}

	// ..................... eliminate all multi-word phrases .......................
bool RemoveAllMultiwordEntries(pWordItem WordList, int WordListCnt)
{
  int i;

  for(i=0; i < WordListCnt; i++)
    if(!IsSingleWord(WordList[i].Word))
      WordList[i].Word = NULL;

  return(true);
}

	// ....................... find a matching word in a buffer and return its index, or return -1 if not found .........................
int FindWord(char *InSeeking, char **InArray, int InArrayCnt)
{
  int i;

  if(InSeeking == NULL)		// don't index a null pointer
    return(-1);

  for(i=0; i < InArrayCnt; i++)
    if(InArray[i] == InSeeking)
      return(i);

  return(-1);
}

	// ............. re-index the system, moving null entries to the end of buffers, updating syn counts, and revising the word list to reflct new index locations .....................
bool ReindexSaurus(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int *WordListCnt)
{
  int i, j, k, m;

  if((ListArray == NULL) || (WordList == NULL))
    return(false);

  if((ListArrayCnt == 0) || (*WordListCnt == 0))	// nothing to do
    return(true);

	// clean up the word array
  for(i=0; i < *WordListCnt; i++)
  {
    if(WordList[i].Word == NULL)	// remove item from the index table
      ListArray[WordList[i].List].HeadIndex[WordList[i].Head].SynTbl[WordList[i].Entry] = NULL;
  } // for

  for(j=0; (j < *WordListCnt) && (WordList[j].Word != NULL); j++);	// find the first deleted slot

  if(j >= *WordListCnt)		// no deleted words
    return(true);

	// remove the deleted entries from the word list
  for(i=j+1; i < *WordListCnt; i++)
    if(WordList[i].Word != NULL)
      memcpy(&WordList[j++], &WordList[i], sizeof(WordItem));
  *WordListCnt = j;

  printf("New word list size is %d\n", *WordListCnt);

	// clean up the index entry lists
  for(i=0; i < ListArrayCnt; i++)
    for(j=0; j < ListArray[i].HeadIndexCnt; j++)
    {
      for(k=0; (k < ListArray[i].HeadIndex[j].SynCnt) && (ListArray[i].HeadIndex[j].SynTbl[k] != NULL); k++);	// find the first deleted slot
      if(k >= ListArray[i].HeadIndex[j].SynCnt)	// no deletions
          continue;
      for(m=k+1; m < ListArray[i].HeadIndex[j].SynCnt; m++)	// overwrite the deleted entries with remaining valid ones
        if(ListArray[i].HeadIndex[j].SynTbl[m] != NULL)
          ListArray[i].HeadIndex[j].SynTbl[k++] = ListArray[i].HeadIndex[j].SynTbl[m];
      ListArray[i].HeadIndex[j].SynCnt = k;	// don't count the crap at the end of the buffer
    } // for

	// update the entry locations in the main word table
  for(i=0; i < *WordListCnt; i++)
  {
    for(j=0; j < ListArray[WordList[i].List].HeadIndex[WordList[i].Head].SynCnt; j++)
      if((WordList[i].Entry = FindWord(WordList[i].Word, ListArray[WordList[i].List].HeadIndex[WordList[i].Head].SynTbl, ListArray[WordList[i].List].HeadIndex[WordList[i].Head].SynCnt)) < 0)	// not found
      {
        printf("Attempt to reindex %s failed\n", WordList[i].Word);
        return(false);
      } // for
  } // for

  return(true);
}

	// ............. delete repeated words until only one instance remains .....................
bool RepeatedItemResolve(pListItem ListArray, int ListArrayCnt, pWordItem WordList, int *WordListCnt)
{
  int i, j, k;

  if((ListArray == NULL) || (WordList == NULL))
    return(false);

  if((ListArrayCnt == 0) || (*WordListCnt == 0))	// nothing to do
    return(true);

  for(i=j=k=0; i < *WordListCnt; i++)
  {
    if(WordList[i].Word == NULL)
      continue;

    if(strcmp(WordList[i].Word, WordList[j].Word) == 0)	// match
    {
      if(i != j)	// at different locations
      {
        k++;
        WordList[i].Word = NULL;	// flag this instance for deletion
      } // if
    } // if
    else
    {
      j = i;
    } // else    
  } // for

  printf("------ Found %d repeated items ---------\n", k);

  if(k == 0)
    return(true);

	// clean up after the deletions
  ReindexSaurus(ListArray, ListArrayCnt, WordList, WordListCnt);

  return(true);
}

	// ................................... Loop through an index removing empty and singleton entries ..................................
bool RemoveIndexSingletonEntries(pHeadItem InArray, int *InArrayCnt)
{
  int i, j, k;

    for(i=0; (i < *InArrayCnt) && (InArray[i].SynCnt > 1); i++);
    if(i >= *InArrayCnt)	// no singletons in this index
      return(true);

    for(j=i+1; j < *InArrayCnt; j++)
      if(InArray[j].SynCnt > 1)
        InArray[i++] = InArray[j];

    *InArrayCnt = i;

  return(true);
}

	// ................................... report singleton index entries (has a head word but no other entries in its list) ..........................
bool RemoveAllSingletonEntries(pListItem ListArray, int ListArrayCnt)
{
  int i;

  for(i=0; i < ListArrayCnt; i++)
    if(!RemoveIndexSingletonEntries(ListArray[i].HeadIndex, &ListArray[i].HeadIndexCnt))
    {
      printf("Error encountered removing singleton entries from index for %s\n", ListArray[i].OriginFileName);
      return(false);
    } // if

  return(true);
}

	// ............. make a more fully balanced index for an aggregation (non-thesaurus structure) .............................
bool XformIndexToFullPartition(pListItem InItem)
{
  int i, j;
  int TotalWords, PartCnt, IdealSum;
  pHeadItem NewIndex;

	// context for NextIndexWord()
  int CurNdx, CurSyn;

  inline char *NextIndexWord(void)
  {
    if(CurSyn >= InItem->HeadIndex[CurNdx].SynCnt)
    {
      CurNdx++;
      CurSyn = 0;
    } // if
    return(InItem->HeadIndex[CurNdx].SynTbl[CurSyn++]);
  }

  for(i=0, TotalWords=0; i < InItem->HeadIndexCnt; i++)
    TotalWords += InItem->HeadIndex[i].SynCnt;

  PartCnt = (int) ((sqrt((double) (TotalWords * 8 + 1)) - 1.0) / 2.0);
	// (a+1)*(a+1) + a + 1 = (a * a + 2 * a + 1) + a + 1 = a * (a + 3) + 2
  IdealSum = (PartCnt * (PartCnt + 3) + 2) / 2 - 1;	// sum of consecutive integers starting with 2
  if((IdealSum - TotalWords) < 2)	// avoid creating a table with only one element
    PartCnt--;

  if((NewIndex = (pHeadItem) calloc(PartCnt, sizeof(HeadItem))) == NULL)
  {
    printf("Cannot allocate new index in XformIndexToFullPartition()\n");
    return(false);
  } // if
  for(i=j=0; i < (PartCnt-1); i++)
  {
    NewIndex[i].SynCnt = i+2;
    j += (i+2);
  } // for
  NewIndex[i].SynCnt = TotalWords - j;	// last bin catches what's left over

	// set up context for NextIndexWord() inline function
  CurNdx = CurSyn = 0;
	// move the words of the old index into the new index
  for(i=0; i < PartCnt; i++)
  {
    if((NewIndex[i].SynTbl = (char **) calloc(NewIndex[i].SynCnt, sizeof(char *))) == NULL)
    {
      printf("Cannot allocate new syn table in XformIndexToFullPartition\n");
      return(false);
    } // if
    for(j=0; j < NewIndex[i].SynCnt; j++)
      NewIndex[i].SynTbl[j] = NextIndexWord();
    NewIndex[i].HeadBufPtr = InItem->HeadIndex[i].HeadBufPtr;
  } // for

	// free the old index memory
  for(i=0; i < InItem->HeadIndexCnt; i++)
    free(InItem->HeadIndex[i].SynTbl);
  free(InItem->HeadIndex);
	// assign the new structure to its place in the saurus
  InItem->HeadIndex = NewIndex;
  InItem->HeadIndexCnt = PartCnt;
  
  return(true);
}

	// ............. rearrange all the entries in an index to a random arrangement - do not change the structure ..............................
bool RandomizeIndexEntries(pHeadItem InIndex, int InIndexCnt)
{
  int i, j, Ndx, TotalWords;
  char **WorkBuf, *Tmp;

  if(InIndex == NULL)
    return(false);

  for(i=0, TotalWords=0; i < InIndexCnt; i++)
    TotalWords += InIndex[i].SynCnt;

  if((WorkBuf = (char **) alloca(TotalWords * sizeof(char *))) == NULL)
  {
    printf("Cannot stack allocate working buffer for index randomization\n");
    return(false);
  } // if
  for(i=0, Ndx=0; i < InIndexCnt; i++)
    for(j=0; j < InIndex[i].SynCnt; j++)
      WorkBuf[Ndx++] = InIndex[i].SynTbl[j];
  for(Ndx = 0; Ndx < (11 * TotalWords); Ndx++)
  {
    i = rand() % TotalWords;
    j = rand() % TotalWords;
    Tmp = WorkBuf[i];
    WorkBuf[i] = WorkBuf[j];
    WorkBuf[j] = Tmp;
  } // for
  for(i=0, Ndx=0; i < InIndexCnt; i++)
    for(j=0; j < InIndex[i].SynCnt; j++)
      InIndex[i].SynTbl[j] = WorkBuf[Ndx++];

  return(true);
}

	// ............. save a single index to a file .................
#define FILE_NAME_SIZE 1024	// max file name length

bool SaveSingleIndexToFile(pListItem InItem, char *OutFileName)
{
  int i, j;
  char NewFileName[FILE_NAME_SIZE];
  FILE *NewFile;

  if(OutFileName == NULL)	// no name specified, so default to a variation on the origin file name
  {
    if(strlen(InItem->OriginFileName) > (FILE_NAME_SIZE - 10))
    {
      printf("File name to long\n");
      return(false);
    } // if
    strcpy(NewFileName, InItem->OriginFileName);
    if(InItem->SerialNum > 0)
      sprintf(&NewFileName[strlen(NewFileName)], "%03d", InItem->SerialNum);
    strcat(NewFileName, ".out");
  } // if
  else
    strcpy(NewFileName, OutFileName);

  if((NewFile = fopen(NewFileName, "w")) == NULL)
  {
    printf("Cannot open new output file %s for save single index\n", NewFileName);
    return(false);
  } // if

  for(i=0; i < InItem->HeadIndexCnt; i++)
  {
    if(InItem->HeadIndex[i].SynCnt <= 0)
      continue;
    fprintf(NewFile, "%s", InItem->HeadIndex[i].SynTbl[0]);
    for(j=1; j < InItem->HeadIndex[i].SynCnt; j++)
        fprintf(NewFile, ",%s", InItem->HeadIndex[i].SynTbl[j]);
    fprintf(NewFile, "\n");
  } // for

  fclose(NewFile);

  return(true);
}

	// ............................ save the entire hierarchy to a multi-index file (separated by '#' lines) if more than one index is present ..................
bool SaveAllToMultiIndexFile(pListItem ListArray, int ListArrayCnt, char *OutFileName)
{
  int i, j, ListNdx;
  char NewFileName[FILE_NAME_SIZE];
  FILE *NewFile;

  if(OutFileName == NULL)	// name must be specified
  {
    printf("No file name specified for save all to multi\n");
    return(false);
  } // if

  strcpy(NewFileName, OutFileName);

  if((NewFile = fopen(NewFileName, "w")) == NULL)
  {
    printf("Cannot open new output file %s for save single index\n", NewFileName);
    return(false);
  } // if

  for(ListNdx = 0; ListNdx < ListArrayCnt; ListNdx++)
  {
    for(i=0; i < ListArray[ListNdx].HeadIndexCnt; i++)
    {
      fprintf(NewFile, "%s", ListArray[ListNdx].HeadIndex[i].SynTbl[0]);
      for(j=1; j < ListArray[ListNdx].HeadIndex[i].SynCnt; j++)
          fprintf(NewFile, ",%s", ListArray[ListNdx].HeadIndex[i].SynTbl[j]);
      fprintf(NewFile, "\n");
    } // for
    if(ListNdx != (ListArrayCnt - 1))
      fprintf(NewFile, "#\n");
  } // for

  fclose(NewFile);

  return(true);
}

	// ........................ print a dump of the hierarchy - increasing level number prints increasing detail .........................

#define STAGE_SIZE 512
char Stage[STAGE_SIZE];

bool DumpSaurusIndexSummary(pListItem InItem, int Level)
{
  int i, j;

  if(InItem == NULL)
  {
    printf("Attempt to dump null item\n");
    return(false);
  } // if

  printf("  File origin: %s\n", InItem->OriginFileName);
  printf("  Serial number: %d\n", InItem->SerialNum);
  printf("  Buffer size: %d\n", InItem->OriginFileSize);
  printf("  Head index count: %d\n", InItem->HeadIndexCnt);

  if(Level < 1)
    return(true);

  j = (InItem->OriginFileSize > 16)?16:InItem->OriginFileSize;
  if(j > 0)
    strncpy(Stage, InItem->InBuf, j);
  else
    Stage[0] = 0;
  printf("  Buffer starts with: 0x%02x 0x%02x 0x%02x 0x%02x (%s)\n", InItem->InBuf[0], InItem->InBuf[1], InItem->InBuf[2], InItem->InBuf[3], Stage);

  if(Level < 2)
    return(true);

  printf("  The SynCnt for each of the %d head entries:\n", InItem->HeadIndexCnt);
  for(i=0; i < InItem->HeadIndexCnt; i++)
    printf("    %4d: %d\n", i, InItem->HeadIndex[i].SynCnt);

   if(Level < 3)
    return(true);

  printf("  The SynTbl head entry (at SynTbl[0]) for each of the %d head entries:\n", InItem->HeadIndexCnt);
  for(i=0; i < InItem->HeadIndexCnt; i++)
    if(InItem->HeadIndex[i].SynCnt <= 0)
      printf("    %4d: No entry\n", i);
    else if(InItem->HeadIndex[i].SynTbl[0] == NULL)
      printf("    %4d: NULL pointer\n");
    else
      printf("    %4d: %s\n", i, InItem->HeadIndex[i].SynTbl[0]);

 return(true);
}

	// ========================= the main() driver =============================

	// ............. declare the usual context ....................

//#define DFLT_IN_FILE_NAME "LoaderFileList.txt"
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

  srand(PSUEDO_SEED);

	// load the lists to work on

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

	// ========== start tools portion ====================

  for(i=0; i < SaurusArrayCnt; i++)
  {
    printf("Dump array[%d]:\n", i);
    DumpSaurusIndexSummary(&SaurusArray[i], 0);
  } // for

  if(!SetAllWordsToLowerCase(SaurusArray, SaurusArrayCnt, WordArray, WordArrayCnt))
  {
    printf("Encountered an error while setting words to lower case\n");
    return(-1);
  } // if

  if(!RemoveAllMultiwordEntries(WordArray, WordArrayCnt))
  {
    printf("Error encountered while removing multi-word entries\n");
    return(-1);
  } // if

	// update the word structures where entries were deleted
  ReindexSaurus(SaurusArray, SaurusArrayCnt, WordArray, &WordArrayCnt);

  if(!RepeatedItemResolve(SaurusArray, SaurusArrayCnt, WordArray, &WordArrayCnt))
  {
    printf("Error encountered while randomly deleting words to resolve repetitions\n");
    return(-1);
  } // if

  if(!RepeatedItemResolve(SaurusArray, SaurusArrayCnt, WordArray, &WordArrayCnt))
  {
    printf("repeat repeat Error encountered while randomly deleting words to resolve repetitions\n");
    return(-1);
  } // if

  if(!RemoveAllSingletonEntries(SaurusArray, SaurusArrayCnt))
  {
    printf("Error encountered while removing singleton index entries\n");
    return(-1);
  } // if

	// comment out the unconditional conditional compile flags if you want these - I left them as examples of what I often do  - this is really hack-to-suit code
#if 0
printf("------ Before:\n");
for(i=0; i < SaurusArray[0].HeadIndexCnt; i++)
{
  printf("%4d (%d): ", i, SaurusArray[0].HeadIndex[i].SynCnt);
  for(j=0; j < SaurusArray[0].HeadIndex[i].SynCnt; j++)
    printf(" %s", SaurusArray[0].HeadIndex[i].SynTbl[j]);
  printf("\n");
} // for
#endif

#if 0
  if(!XformIndexToFullPartition(&SaurusArray[0]))
  {
    printf("Attempt to transform index 0 failed\n");
    return(-1);
  } // if

	// update the word structures to reflect the new index locations
  ReindexSaurus(SaurusArray, SaurusArrayCnt, WordArray, &WordArrayCnt);

  if(!RandomizeIndexEntries(SaurusArray[0].HeadIndex, SaurusArray[0].HeadIndexCnt))
  {
    printf("Attempt to randomize index 0 failed\n");
    return(-1);
  } // if
#endif

#if 0
printf("------ After:\n");
for(i=0; i < SaurusArray[0].HeadIndexCnt; i++)
{
  printf("%4d (%d): ", i, SaurusArray[0].HeadIndex[i].SynCnt);
  for(j=0; j < SaurusArray[0].HeadIndex[i].SynCnt; j++)
    printf(" %s", SaurusArray[0].HeadIndex[i].SynTbl[j]);
  printf("\n");
} // for
#endif

#if 0
  if(!SaveSingleIndexToFile(&SaurusArray[1], "TestFile.txt"))
  {
    printf("Attempt to save single index 0 failed\n");
    return(-1);
  } // if
#endif

  if(!SaveAllToMultiIndexFile(SaurusArray, SaurusArrayCnt, "ListToolOutput.saurus.txt"))
  {
    printf("Attempt to save all to multi-index file failed\n");
    return(-1);
  } // if

  return(0);
}


