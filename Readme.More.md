## ﻿Thesaurus of the Gods

                ### Overview

This repository is about lists of words. The file format is very simple and
can be made with very basic text editors like vi, or Leafpad, or whatever.
Filters that gather words from other file formats can also easily create this
format. The main organizing concept is a dictionary or thesaurus entry. In
this case it is just a line of coma-separated words ending with a single
newline character. A saurus file has one or more of these lines. Any of the
words on the same line can replace each other in the message template text.
It should have more than one word on a line, otherwise what would be a
replacement for that word? Anyway, thousands of words can appear on a line or
only two.

In a true thesaurus there is a head entry followed by many synonyms. The
thesaurus user “looks up”  the head word and finds a collection of
equivalents. The TotG program “looks up” any of the words on a line. There
is no particular significance to the order of words within a line. They are
all essentially head entries.

For similar reasons the words are unique. In a physical thesaurus book, a
given word appears in many entries; it can be “looked up” starting from many
different head words. The TotG program collects all the words from all the
saurus files specified and puts them together in a single master list. Each
word appears only once in this master list. Any word in that list refers back
to the line of input which is the original smaller line of interchangeable
entries. Clearly each word must refer to only one group of interchangeable
entries. If it is desirable for a word to refer to more than one line in the
file, then those lines should be merged into one line.

So, there’s the rub, and also the source of most of the malapropisms in the
replacements. In an ordinary thesaurus, different headings refer to meanings
when the word is used in different ways. Collecting all those into one line
puts these many meanings together as if (in a sense) those diverse meanings
were somehow equivalent. 

This effect is also a source of flexibility in the way the TotG interprets
equivalence. If the program is used to replace names of people and places,
then the way that is done can be controlled by the way the saurus files are
organized. Collections of geographical and biographical names can be thrown
together in any scattered way and that may produce comical results;
replacing people’s names with city names and whatnot. On the other hand they
can be separated as in the sample files in this repository. There is a file
of the most common female names and a separate file of the most common male
names and a file of the most common last names and a file of the most common
city names. When any of these are located in the template message the entry
refers back to the words that originated on the same line, so a female name
gets replaced with a female name, a city name with another city, etc.

The male, female, and last name files in this repository are organized
alphabetically by first letter. A  last name beginning with T is replaced by
another also beginning with T. The city names are organized together by
state, so city substitutions stay within a state. Tools in this repository
exist to reorganize or randomize, but that’s how these are organized.

This presents many opportunities for substitutions. If it is desirable to
change all commercial brand names in a message, a saurus file can be loaded
with a collection of company names. Whenever some company name appears it is
replaced with another. Several files representing different product
categories could be loaded instead. The exact same collection of company
names might be involved, but if they originate in different files the names
substitute for others in the same category. Shoe brands replace other
shoebrands, and fast foods replace other fast food brands, for example.

A collection of saurus files with the same contents but different
organizations might be desirable. Support for two ways of doing this are
part of the TotG logic. When using the -m command line switch the saurus
input file name is interpreted as the name of a file with contains, not word
lists, but the names of saurus files that do contain word lists. An example
file in the repository is a list of the female name file, the male name file,
the last name file, and the cities file. These are all loaded separately into
separate index groups, then all words from all of them are collected into the
main list. Any word from any of them can be looked up, but the replacements
happen within categories as they appear in the separate files.

The files can also be concatenated together in a text editor with the file
contents separated with a single line containing only a ‘#’ character
followed by a newline. This causes the TotG program to create separate index
groups. The ‘#’ lines can be omitted and the files concatenated together
without them. The difference will be discussed elsewhere.

The tools program also contains a bit of code that writes all the index
structures in memory to a single file with the ‘#’ separator. The idea is to
load many saurus files, weed out duplications, randomize and transform in
various ways, then save that configuration as a separate result. This will
also be discussed in more detail elsewhere.

The message template file is also a very plain, common text format created
easily with vi, Leafpad, etc. The idea is that cutting and pasting from web
mail or similar apps to a text file while browsing could be used to create
any number of typical phony aggregate messages which could then be labeled as
such (if desired) sent to friends, or yourself, or posted wherever 
(in)appropriate. In the ever-successful quest for Internet trash, web page
comments by the usual SEO commentators containing happy lists of places and
things of the greatest importance can be particularly useful resources. And
don’t forget to mine your own email headers from those net buddies that
thankfully include you in those fwd: mailing lists. Throw a couple of those
name lists in the template and run it through the **Thesaurus of the Gods** a
couple of times. I will not go there, but a little imagination goes a long
way.

The output of TotG is also very simple text with single newlines. It is
written to stdout, so don’t forget to send the output to a file. An attempt
is made to preserve the original form in all respects except the
substitutions. It is not good about preserving capitalization when words
appear in ALL CAPS.

A word about capitalization. The default behavior of TotG is to convert each
word of the template to lower case before it searches the saurus list. It
looks at the initial letter of each word and remembers if it is capitalized,
and imitates that form when doing the replacement. It does not go all the way
back to the original template string, however. As a consequence, ALL CAPS
becomes All Caps.

Certainly there are other side effects of this strategy. For example, the
common word “will”  is also a shortened form of the male name “William”,
so searching with lower case and using the male name saurus file hit will as
a first name, sees that it is not capitalized and replaces it, for example,
with “walter”. A mitigating strategy is the use of the -c switch on the
command line. This has the effect of suppressing the translation to lower
case for the search. The original male name file contains the initial
capitalization. The derived and combined name file has those names in lower
case. Using the original, capitalized version of the saurus file won’t see
the common word “will” as a name but detects “Will” as a name and
substitutes, for example, “Walter”. But of course, there is the matter of
the “Last Will and Testament”, or the beginning of a sentence. 


                ### Basic Build

The code for **Thesaurus of the Gods** is intentionally simplified and
requires little in its dependencies on other packages. Any basic Unix-like
distro should work fine with minimal or no modification. A common Linux with
gcc should do the trick. There are only four source code files:

> Saurus.h
> ListLoader.c
> ListTool.c and
> Totg.c

The header file is the common structures and definitions. ListLoader is the
utility I/O, format, and index handlers. ListTool is a small collection of
processing and filtering utilities to help in transforming raw data into
indexable list files. Totg (**Thesaurus Of The Gods**) is the main file that
calls the ListLoader functions then performs the incremental  replacements
in the template message text.

A couple of conditional compile symbols can be defined. If you want to hack
the utility functions in ListLoader.c you can define **TEST_DRIVER** to
create a basic main() as a standalone starting point. The symbol **DEBUG_DATA**
can also be defined to report what are usually long lists of information about
the data being loaded. Otherwise, just a generic compile to an object file is
all that’s required. It is the only file that links with the main Totg to
produce the executable, so I didn’t bother with a makefile.

The main Totg.c can be compiled with the symbol **DEBUG_VER** if you are
interested in the chain data. I generally compile this separately with a
different name (like Totg_dbg) to create chains. The chain is a sequence of
number pairs representing the index that was actually chosen as a
replacement word, followed by the size of the index from which it was
selected. It can be used to track the process of replacement in a specific
template text. I kept the chain code to a minimum in the interest of brevity, 
but will expand on this theme in separate project repository.

Building the Totg executable is simply a matter of compiling it and linking
with the ListLoader object file. There are no calls to other external
libraries, so nothing beyond the default C is needed. I love the command
line, so I usually do:

```
gcc -c ListLoader.c

followed by:

gcc -o Totg Totg.c ListLoader.o

Or, possibly:

gcc -D DEBUG_VER -o Totg_dbg Totg.c ListLoader.o
```

Pretty basic.

The ListTool.c file is similar. Compiling is just:

```
gcc -o ListTool ListTool.c ListLoader.o -lm
```

which requires the math library because it contains a call to the floating
point sqrt() square root function. This file has a main() that calls a
sequence of functions that might be useful, but it is intended to be
modified to suit whatever might be required to filter a raw text file to
make a better saurus file. For example, there is a function that translates
all strings to lower case.

A small aside: some of the comments and scripts may have a stray -m64 on the
gcc command line. You probably don’t need that. If you’re not sure, then you
don’t.


                ### The List Loader Code

The index system, like the file format itself, is a fairly simple
hierarchical structure. At the top is an array of items separately
representing the saurus files and any sub-index contained in the files (by
using the ‘#’ separator). The point to the index tables. An index table is
an array of lists of the words that can replace one another. It is
convenient to think of the table as an array of head words, each with its
associated list of synonym entries.

Once this simple hierarchical table is built, all of its words are counted
and a master list is created containing all words in the index tables. This
list is sorted. Each entry in this list has a reference back to its location
in the hierarchical table, including a pointer to the list of mutually
replaceable words.

When a word is examined to determine whether a substitute is available, the
foreign word is searched for in the main list pool. If it is present then
the list of potential replacements is located and one is selected from the
list of candidates. This list of candidates is one of the lines from one of
the files that contained the foreign word.


The following is a brief description of the utility functions provided by
the ListLoader code:

```
bool LoadSingleFileName(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt);
  Prepare to load the contents of a single saurus file and build an index.

bool LoadFileNameList(char *NameFileName, pListItem *SaurusList, int *SaurusListCnt);
  Loop through the files named in the file list within a file.

bool LoadWordList(pListItem CurListItem);
  Load the contents of a file referenced in the prepared structure.

bool ParseIndex(pListItem CurListItem);
  Separate the words and lists in a memory buffer.

bool LoadIndexList(pListItem InFileArray, int InFileArrayCnt, pListItem *SaurusList, int *SaurusListCnt);
  Allocate index space and initialize the index structures

bool LoadIndexEntryLine(pHeadItem CurIndexItem);
  Load a line of comma separated words as an entry in an index item.

bool CreateMainWordArray(pListItem ListArray, int ListArrayCnt, pWordItem *WordList, int *WordListCnt);
  Pool the contents of all index tables and sort them in a searchable list.

int CmpWordString(const void *p1, const void *p2);
  Comparison for sorting and searching the word list with qsort and bsearch.
```


                ### Thesaurus Of The Gods

The ToG program loads one or more saurus index files, then reads th template
message text file, separating into individual words and searching the index
for each of them. If the index contains this template word, then a
replacement word is selected (usually at random) from the candidate list. An
output text is written as this proceeds containing any message text for
which no substitute was found, along with the substitutes for the words that
were found.

The code is fairly simple. It starts with a very simple, old-fashioned parser
for the command line files and switches. It just steps through argv looking
for a string that starts with a hyphen. If it finds one it treats it as a
list of switches, otherwise it copies it to the next filename slot.

By default all selections are specified by a chain file. In the original
version random functions produced the chain file. This has the effect of
making the replacements repeatable and controllable, but most users of this
code will want to activate random selection by specifying -r on the command
line. If the -r (random) switch is present it can be followed by an integer
string that is converted to a seed for the pseudo-random number generator.
This also produces repeatable output by specifying the same psuedorandom
generator seed on separate runs. If no specific seed is specified, that is,
if the -r flag is used without following it by a number, then a seed is
chosen as the low bits of the system clock, so the odds of getting identical
output on two successive runs is very low.

TotG loads the saurus file or all the files listed in a name list file, then
reads the template file into a memory buffer. If random selection is not
desired, a chain file can be loaded. This chain file is also a text file –
simply a sequence of entries of the form:

```
12,34;56,78; etc.
```

The first of the pair (the 12 or the 56) is the index location of the
desired selection, and the second (the 34 or the 78) is the number of
candidate replacements in the saurus list. There are no embedded blanks,
newlines, or other whitespace characters. The number of such pairs has to
match the number of replacements made while processing the template
message text. Such a chain file can be written from the debug version of
the TotG program by using the -w command line flag. In that case the chain
is essentially a representation of the input template message. I’ve
purposely stripped this down and will expand on it in another project. It is
a worthy tool in its own right.

The main function of the TotG code happens in EmitTransformation() which
simply coordinates between the saurus word pool and sequentially located
message text by successive calls to the C library function bsearch().

The EmitTransformation() code has a dummy key structure of the same type
as the word pool array. The foreign word selected from the message text is
plugged into the dummy key. If a match is found, the pool structure, which
has the form:

```
typedef struct _WordItem
{
  char *Word;		// pointer to the string
  int List;		// index of the list item in which it originated
  int Head;		// index of the head item the word belongs to
  int Entry;		// index of the word's location in the head entry
} WordItem, *pWordItem;
```

contains array index locations in the hierarchical table that facilitate
access to all three levels in the path to the matched word. From this the
size of the candidate list is used to limit the range of a pseudo-random
number that selects a replacement. That, by the way, is the number pair
in the chain. Simple.

                ### The List Tools

The following is a brief description of the list tool functions:

```
void ToLowerCase(char *InWord);
  Translates the characters of a sting to lower case.

bool SetAllWordsToLowerCase(pListItem ListArray, int ListArrayCnt, pWordItem 
      WordList, int WordListCnt);
  Translates all words in the current index structure to lower case.
  
bool IsSingleWord(char *InString);
  Looks for embedded blanks in the string.

bool RemoveAllMultiwordEntries(pWordItem WordList, int WordListCnt);
  Deletes all entries in the searchable word list that contain embedded blanks.

int FindWord(char *InSeeking, char **InArray, int InArrayCnt);
  Locates the word pointed to by InSeeking in a string array.

bool ReindexSaurus(pListItem ListArray, int ListArrayCnt, pWordItem WordList,
      int *WordListCnt);
  Clears out deleted items and revises the index to contain correct valid data.

bool RepeatedItemResolve(pListItem ListArray, int ListArrayCnt,
      pWordItem WordList, int *WordListCnt);
  Locates repetitions, choses on at random, and deletes the others.

bool RemoveIndexSingletonEntries(pHeadItem InArray, int *InArrayCnt);
  Deletes head entries in an index table with no associated list.

bool RemoveAllSingletonEntries(pListItem ListArray, int ListArrayCnt);
  Deletes all head entries in the system that have no associated list.

bool XformIndexToFullPartition(pListItem InItem);
  Shuffles words in an index table to make a uniform structure.

bool RandomizeIndexEntries(pHeadItem InIndex, int InIndexCnt);
  Shuffles the contents of all the lists in an index to random sequences.

bool SaveSingleIndexToFile(pListItem InItem, char *OutFileName);
  Writes an index table to the named file.

bool SaveAllToMultiIndexFile(pListItem ListArray, int ListArrayCnt,
      char *OutFileName);
  Writes all index tables in the system to a multi-index file.
```

                ### Examples

Several saurus files are present in this repository, along with a few
examples of how it can be used. I’m aware that this readme is bombastic. I’m
trying not to be tedious, but simply clear. I expect VERY few people will
read this carefully. Anyway...

Several loadable text files:
```
FemaleFirstNames.txt
MaleFirstNames.txt
```

From which the combined file was made:
```
FirstNames.txt
```

Also:

```
LastNames.txt
UsCityNames.txt
```

Several saurus index files:
```
FemaleNames.saurus.txt
LastNames.txt
TheSaurus.saurus.txt
```

The file:
```
FemaleMaleLastCities.saurus.txt
```
is a multi index created with the tools.

A sample template message text file:
```
BunchOfNamesMsg.txt
```
contains a bunch of lines with various names of people and places, which
demonstrates using **Thesaurus of the Gods** to mislead the machines that
read everyones social media.

The file named TheSaurus.saurus.txt is what motivated this project in the
beginning. It is essentially a collection of thesaurus-like synonyms
gathered from a wide range of places. Apart from the well-studied problems
of words having multiple meanings, therefore multiple categories of synonyms
(e.g., Time flies like an arrow. Fruit flies like a garbage truck.), there
are many fascinating structural patterns and mathematical allegories to be
found in thesaurus and thesaurus-like lists. Few people have patience for
it, and I won’t go on with details.

A template message can be run through TheSaurus.saurus.txt to produce some
fairly funny stuff, if your in the mood for it. As might be expected,
certain kinds of sentences produce a high density of substitutions. Since
the TotG mechanism doesn’t respect the cognitive categories, amusing
malapropisms appear fairly frequently. Or not:

> Mister Alpenhorn was not a australopithecus to shirk the vat, and when he
> did speak, the words were not so annulose and bettered te to “glide off
> approximating the drinking-water from a oie’s backtrail,” cretaceous he
> said. When he footrest itself was hebetate and poetically aimed and him
> shipwrecked (well-)deserved where itself was intimated to stick.


                ### Running the Example Script

Ok, we’ve got a test script file TestScript.sh that demonstrates some of
these things. It starts with a demo of a hack on ListTool.c. I’ve edited
this to do a fairly common sequence for building a single saurus file
from multiple txt lists.

Start by compiling the tools program.

```
gcc -c ListLoader.c
gcc -o ListToolDemo ListTool.c ListLoader.o -lm
```

Then run the script.

```
sh TestScript.sh
```

Which contains the commands:

```
echo "FemaleFirstNames.txt" > FemaleMaleLastCities.filelist.txt
echo "MaleFirstNames.txt" >> FemaleMaleLastCities.filelist.txt
echo "LastNames.txt" >> FemaleMaleLastCities.filelist.txt
echo "UsCityNames.txt" >> FemaleMaleLastCities.filelist.txt

./ListToolDemo FemaleMaleLastCities.filelist.txt

./Totg -r FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.txt > BunchOfNamesMsg.out.txt

./Totg_dbg -rw FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.out.txt > BunchOfNamesMsg.out.chain.txt

./Totg FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.txt BunchOfNamesMsg.out.chain.txt
```

What that does is to create a multi-filename list file with four of the
repository list text files names. All four are filtered and combined by the
list tool demo to display the top-level of the hierarchy, shift all the
strings to lower case, remove multi-word entries, re-index the hierarchy,
eliminate repeated strings, run the repeated string scan to verify, delete
singleton entries, then write the result to a multi-index saurus file named
FemaleMaleLastCities.saurus.txt.

That presumably creates a clean index with female and male first names, last
names, and common US city names. I’ve included a little demo text message
file called  BunchOfNamesMsg.txt that’s just a chatty typical kids’ soccer
league sort of thing like many people have seen before. It is a reasonably
rich source of replaceable strings, and a plausible phony message that you
might send to a dozen friends with a cc to yourself.

You might want to run that through the TotG a couple of times and just let
it print to the console. It’s not very long.

```
./Totg -r FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.txt
```

You can see that it just picks new replacements every time (unless you
specify a random seed). The script sent one of those random outputs to
a file named BunchOfNamesMsg.out.txt. It subsequently used that random
output to create a chain file named BunchOfNamesMsg.out.chain.txt. Ok,
the punch line is that when, as a grand finale the script runs TotG with
that chain file instead of random mode, the TotG repeats the
BunchOfNamesMsg.out.txt replacements instead of creating new ones. It
repeats them repeatedly, if you continue up-arrow on that command line.

Enough said. The script proceeds with a couple of similar examples with list
files and template messages in other categories. You can surely see how you
might make a couple of list files with your favorite misleading information,
if you hanker for Google’s handy suggestions of YouTube farm equipment
videos, or the Amazon books on proper catheterization in veterinary
medicine.
 



