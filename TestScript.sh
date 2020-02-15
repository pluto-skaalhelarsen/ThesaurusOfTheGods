# Create a saurus from several text files, run a messagee template through it, then build a chain run it with that, too

echo "FemaleFirstNames.txt" > FemaleMaleLastCities.filelist.txt
echo "MaleFirstNames.txt" >> FemaleMaleLastCities.filelist.txt
echo "LastNames.txt" >> FemaleMaleLastCities.filelist.txt
echo "UsCityNames.txt" >> FemaleMaleLastCities.filelist.txt

./ListToolDemo FemaleMaleLastCities.filelist.txt

mv ListToolOutput.saurus.txt FemaleMaleLastCities.saurus.txt

./Totg -r FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.txt > BunchOfNamesMsg.out.txt

./Totg_dbg -rw FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.out.txt > BunchOfNamesMsg.out.chain.txt

./Totg FemaleMaleLastCities.saurus.txt BunchOfNamesMsg.txt BunchOfNamesMsg.out.chain.txt

# A different template and set of lists

echo "BrandNames.txt" > Products.filelist.txt
echo "GreatSnacks.txt" >> Products.filelist.txt
echo "FastFood.txt" >> Products.filelist.txt
echo "ShoeBrands.txt" >> Products.filelist.txt

./ListToolDemo Products.filelist.txt

mv ListToolOutput.saurus.txt Products.saurus.txt

./Totg -r Products.saurus.txt ProductsMsg.txt > ProductsMsg.out.txt

./Totg_dbg -rw Products.saurus.txt ProductsMsg.out.txt > ProductsMsg.out.chain.txt

./Totg Products.saurus.txt ProductsMsg.txt ProductsMsg.out.chain.txt

# Usage of the more-or-less thesaurus

./Totg -r TheSaurus.saurus.txt OddSentenceMsg.txt > OddSentenceMsg.out.txt

