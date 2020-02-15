/*
 *	Saurus.h - defines, typedefs, and forward declarations for thesaurus-like list loader
 *
 *	D.Merrell - 2/6/20
 */
	
#ifndef _SAURUS_H_
#define _SAURUS_H_

	// descrition of the individual word and its position in the index
typedef struct _WordItem
{
  char *Word;		// pointer to the string
  int List;		// index of the list item in which it originated
  int Head;		// index of the head item the word belongs to
  int Entry;		// index of the word's location in the head entry
} WordItem, *pWordItem;

// description of each sub-list item within a list structure (think head word and entries in a thesaurus or dictionary)
typedef struct _HeadItem
{
  char *HeadBufPtr;		// buffer location of head
  char **SynTbl;		// list of single-word synonyms
  int SynCnt;			// number of single word synonyms
} HeadItem, *pHeadItem;

	// description of each separate list loaded into memory
typedef struct _ListItem
{
  char *OriginFileName;		// name of the file containing the list data
  int OriginFileSize;		// size of the file data
  char *InBuf;			// memory buffer containing the word data
  HeadItem *HeadIndex;		// head entries
  int HeadIndexCnt;		// number of head entries
  int SerialNum;		// non-zero when a file contains more than one index
} ListItem, *pListItem;

	// external references
bool CreateMainWordArray(pListItem ListArray, int ListArrayCnt, pWordItem *WordList, int *WordListCnt);
bool LoadSingleFileName(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt);
bool LoadFileNameList(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt);
bool LoadWordList(pListItem CurListItem);
bool ParseIndex(pListItem CurListItem);
bool LoadIndexList(pListItem InFileArray, int InFileArrayCnt, pListItem *SaurusList, int *SaurusListCnt);
bool LoadIndexEntryLine(pHeadItem CurIndexItem);

int CmpWordString(const void *p1, const void *p2);	// string comparison for sorting and searching the word list

#endif	// _SAURUS_H_
