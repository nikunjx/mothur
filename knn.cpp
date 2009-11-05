/*
 *  knn.cpp
 *  Mothur
 *
 *  Created by westcott on 11/4/09.
 *  Copyright 2009 Schloss Lab. All rights reserved.
 *
 */

#include "knn.h"

/**************************************************************************************************/
Knn::Knn(string tfile, string tempFile, string method, int kmerSize, int gapOpen, int gapExtend, int match, int misMatch, int n) 
: Classify(tfile, tempFile, method, kmerSize, gapOpen, gapExtend, match, misMatch), num(n)  {}
/**************************************************************************************************/
string Knn::getTaxonomy(Sequence* seq) {
	try {
		string tax;
		
		//use database to find closest seq
		vector<int> closest = database->findClosestSequences(seq, num);
		
		vector<string> closestNames;
		for (int i = 0; i < closest.size(); i++) {
			//find that sequences taxonomy in map
			it = taxonomy.find(names[closest[i]]);
		
			//is this sequence in the taxonomy file
			if (it == taxonomy.end()) { //error not in file
				mothurOut("Error: sequence " + names[closest[i]] + " is not in the taxonomy file.  It will be eliminated as a match to sequence " + seq->getName() + "."); mothurOutEndLine();
			}else{   closestNames.push_back(it->first);	}
		}
		
		if (closestNames.size() == 0) {
			mothurOut("Error: All the matches for sequence " + seq->getName() + " have been eliminated. " + seq->getName() + " will be disregarded."); mothurOutEndLine();
			tax = "bad seq";
		}else{
			tax = findCommonTaxonomy(closestNames);
			if (tax == "") { mothurOut("There are no common levels for sequence " + seq->getName() + ". " + seq->getName() + " will be disregarded."); mothurOutEndLine(); tax = "bad seq"; }
		}
		
		return tax;	
	}
	catch(exception& e) {
		errorOut(e, "Knn", "getTaxonomy");
		exit(1);
	}
}
/**************************************************************************************************/
string Knn::findCommonTaxonomy(vector<string> closest)  {
	try {
		vector< vector<string> > taxons;  //taxon[0] = vector of taxonomy info for closest[0].
										//so if closest[0] taxonomy is Bacteria;Alphaproteobacteria;Rhizobiales;Azorhizobium_et_rel.;Methylobacterium_et_rel.;Bosea;
										//taxon[0][0] = Bacteria, taxon[0][1] = Alphaproteobacteria....
										
		taxons.resize(closest.size());
		int smallest = 100;
		
		for (int i = 0; i < closest.size(); i++) {
		
			string tax = taxonomy[closest[i]];  //we know its there since we checked in getTaxonomy
		
			tax = tax.substr(0, tax.length()-1);  //get rid of last ';'
	
			//parse taxonomy
			string individual;
			while (tax.find_first_of(';') != -1) {
				individual = tax.substr(0,tax.find_first_of(';'));
				tax = tax.substr(tax.find_first_of(';')+1, tax.length());
				taxons[i].push_back(individual);
			
			}
			//get last one
			taxons[i].push_back(tax);
		
			//figure out who has the shortest taxonomy info. so you can start comparing there
			if (taxons[i].size() < smallest) {
				smallest = taxons[i].size();
			}
		}
	
		//start at the highest level all the closest seqs have
		string common = "";
		for (int i = (smallest-1); i >= 0; i--) {

			string thistax = taxons[0][i];
			int num = 0;
			for (int j = 1; j < taxons.size(); j++) {
				if (taxons[j][i] != thistax) { break; }
				num = j;
			}
		
			if (num == (taxons.size()-1)) { //they all match at this level
				for (int k = 0; k <= i; k++) {
					common += taxons[0][k] + ';';
				}
				break;
			}
		}
	
		return common;
	}
	catch(exception& e) {
		errorOut(e, "Knn", "findCommonTaxonomy");
		exit(1);
	}
}
/**************************************************************************************************/
