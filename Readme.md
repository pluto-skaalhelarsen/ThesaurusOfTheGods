
﻿## Thesaurus of the Gods


```
You have privacy if you retain the effective capacity to misrepresent yourself.
    - Dan Geer
```

Are you thinking about the many, many machines (not usually people) who are
reading your every social media post, email message, forum post, etc., fondly
making a database about YOU! How sweet! They are SO concerned! And when the
names of your friends and family appear in the text – they remember that, too!
And the names of movies, places, brand names of products (they love that) –
really any words that indicate what your interests are and who you associate
with. And they make big maps connecting you to everyone whose name ever
appears in one of your posts. You don’t care, right? Your not a terrorist.

But, how do they do that, actually? It’s just statistical aggregations of the
words that appear in your posts. The machine doesn’t actually read like a
human. The recent AIs are getting into more detail, but for the most part it
is just collecting statistics about what topical words appear in your
messages.

So, what can you do? Salt their machine diet with a little bullcrap. Did you
ever accidentally click on a YouTube video that turned out to be the model
name for the latest farm tractor or something, then for weeks you get that
“Suggested for you” with farm implements? Same principal. Let the **Thesaurus**
**of the Gods** help you keep the machines guessing about who you really are.
Throw out a message (or seven hundred) full of keywords about topics you have
no interest in, places you’d never even consider visiting, and the names of
people you don’t know, and who may not even exist.

Or, you might be writing a little amateur short story with many characters
and venues, and letting your friends see each working version in which the
list of minor characters always seems to be different.

### What is it?

This repository is a collection of thesaurus-like word lists with a little C
code that reads another text file (your email message, for example) and
performs thesaurus-like word replacements using the particular word lists
you’ve selected. Some of the word lists enable the usual selections of
similar meanings. Others are more random replacements; changes of people’s
first or last names, or city names, for example. This suggests several
different purposes that these ‘sauruses might serve.

### What is it used for?

An obvious application supports a long and proud tradition of humorous
malapropisms (is that the right word for it?) that even the Bard himself
participated in. The fact of two words having similar meanings in one context
doesn’t necessarily imply that this replacement holds up in another context.
A computer program such as TotG makes many such category mistakes when
selecting “synonyms” at random from a list.

Another application involves a growing segment of Internet users who are
concerned (or irritated) by AI and other robot readers of their personal
communications. Some curious people wonder if a cynical aside mentioning
Area 51 in an email message might be related to targeted advertising that
offers true confessions of alien abductees and package pleasure tours of the
Bermuda triangle. It could happen. Somewhat more nefariously, there seem to
be AI tools “out there” that construct maps of interrelationships among people
and places common to the email content and social media posts of diverse and
otherwise seemingly unrelated people.

To make messages that seem credible to the astute wisdom of an AI machine
readership, you could use an actual email or Facebook post as the base
template, but replace the names of your personal friends and acquaintances,
etc., with a feast of search-engine keywords that are totally unrelated to
your own acquaintances, activities and interests. Pastures of plenty!

### Who are the users?

I’ve gathered this little collection of lists from my own peculiar obsessions.
Many more are possible, and I’ve included here a few programming tools that I
use to manipulate the lists myself. If this somehow becomes enormously popular
I’ll build and HTML Web GUI, but not tonight. The main program that performs
the replacements is a CLI app that for that reason will appeal mostly to
programmers and other techies. It is pretty simple and I’ve got a couple of
command line switches that help with the usual variations. From a programming
standpoint I’ve tried to make it straightforward and hackable.

I read code and I expect other people do too, but I’ve written another more
detailed readme intended to help anyone who might have some particular itch.
That text also has more information about the organization of the lists
themselves, so you might look at it if you only want to create a custom list,
not necessarily a programming mod.

