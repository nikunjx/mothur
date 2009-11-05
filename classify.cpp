/*
 *  classify.cpp
 *  Mothur
 *
 *  Created by westcott on 11/3/09.
 *  Copyright 2009 Schloss Lab. All rights reserved.
 *
 */

#include "classify.h"
#include "sequence.hpp"
#include "kmerdb.hpp"
#include "suffixdb.hpp"
#include "blastdb.hpp"

/**************************************************************************************************/

Classify::Classify(string tfile, string tempFile, string method, int kmerSize, int gapOpen, int gapExtend, int match, int misMatch) : taxFile(tfile), templateFile(tempFile) {		
	try {											
		
		readTaxonomy(taxFile);
		
		int numSeqs = 0;
		//need to know number of template seqs for suffixdb
		if (method == "suffix") {
			ifstream inFASTA;
			openInputFile(tempFile, inFASTA);
			numSeqs = count(istreambuf_iterator<char>(inFASTA),istreambuf_iterator<char>(), '>');
			inFASTA.close();
		}

		mothurOut("Generating search database...    "); cout.flush();
				
		bool needToGenerate = true;
		string kmerDBName;
		if(method == "kmer")			{	
			database = new KmerDB(tempFile, kmerSize);			
			
			kmerDBName = tempFile.substr(0,tempFile.find_last_of(".")+1) + char('0'+ kmerSize) + "mer";
			ifstream kmerFileTest(kmerDBName.c_str());
			if(kmerFileTest){	needToGenerate = false;		}
		}
		else if(method == "suffix")		{	database = new SuffixDB(numSeqs);								}
		else if(method == "blast")		{	database = new BlastDB(gapOpen, gapExtend, match, misMatch);	}
		else {
			mothurOut(method + " is not a valid search option. I will run the command using kmer, ksize=8.");
			mothurOutEndLine();
			database = new KmerDB(tempFile, 8);
		}
		
		if (needToGenerate) {
			ifstream fastaFile;
			openInputFile(tempFile, fastaFile);
		
			while (!fastaFile.eof()) {
				Sequence temp(fastaFile);
				gobble(fastaFile);
			
				names.push_back(temp.getName());
				database->addSequence(temp);	
			}
			fastaFile.close();

			database->generateDB();
			
		}else if ((method == "kmer") && (!needToGenerate)) {	
			ifstream kmerFileTest(kmerDBName.c_str());
			database->readKmerDB(kmerFileTest);	
			
			ifstream fastaFile;
			openInputFile(tempFile, fastaFile);
		
			while (!fastaFile.eof()) {
				Sequence temp(fastaFile);
				gobble(fastaFile);
			
				names.push_back(temp.getName());
			}
			fastaFile.close();
		}
		
		database->setNumSeqs(names.size());
		
		mothurOut("DONE."); mothurOutEndLine();

	}
	catch(exception& e) {
		errorOut(e, "Classify", "Classify");
		exit(1);
	}
}
/**************************************************************************************************/

void Classify::readTaxonomy(string file) {
	try {
	
		ifstream inTax;
		openInputFile(file, inTax);
	
		mothurOutEndLine();
		mothurOut("Reading in the " + file + " taxonomy...\t");	cout.flush();
		
		string name, taxInfo;
		//read template seqs and save
		while (!inTax.eof()) {
			inTax >> name >> taxInfo;
			
			taxonomy[name] = taxInfo;
		
			gobble(inTax);
		}
		inTax.close();
	
		mothurOut("DONE.");
		mothurOutEndLine();	cout.flush();
	
	}
	catch(exception& e) {
		errorOut(e, "Classify", "readTaxonomy");
		exit(1);
	}
}
/**************************************************************************************************/
