//============================================================================
// Name        : buildDL_Sada.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "SadaDocList64.h"
#include <ConfigFile.h>

bool TRACE = false;			// true: print all details for console
bool TEST = false;			// true: apply exhaustive test
uint N_REP = 5;
uint MAX_M = 10;

// Structure with all globals parameters program
typedef struct {
	uchar *seq;				// original sequence (1 byte for symbol)
	ulong n;				// Length of generalize Text = T1$T2$...TD$
	char cutDoc;			// new symbol to separate documents
	SadaDocList64 *index;

	ulong* EndDocs;			// this store the final phrase number for each document. This is no include in the final structure

	string configFile;		// properties file
	char inputFile[200];	// list of files
	bool filesInList;		// 1: list of files, 0: Unique file
	char boundSymbol;		// original symbol delimiter of documents when we read all documents in 1 file.
	char dirStore[200];		// directory to save/load the data structure (files *.dls)

	// The following data structure are only for test !!
	ulong* patterns;
} ParamProgram;

void testDLSad(ParamProgram *par);
uint searchPattInFMI(ParamProgram *par, uchar* pat, uint m, bool *V);
void createPatterns(ParamProgram *par, uint m);

int main(int argc, char *argv[]) {
	ParamProgram *par = new ParamProgram();
	char fileName[300];

	if(argc != 2){
		cout << "ERRORR !! " << endl;
		cout << "buildDL_LZ's usage requires the Properties File as parameter !! " << endl;
		cout << "Example for the file 'config.txt' in the same directory: ./buildDL_LZ config.txt" << endl;
		exit(1);
	}
	par->configFile = string(argv[1]);
	cout << "Congif file: " << par->configFile << endl;
	ConfigFile cf(par->configFile);

	TRACE = cf.Value("GLOBALS","TRACE");
	TEST = cf.Value("GLOBALS","TEST");
	N_REP = cf.Value("GLOBALS","N_REP");
	MAX_M = cf.Value("GLOBALS","MAX_M");

	strcpy(par->inputFile, ((string)(cf.Value("DL","inputFile"))).c_str());
	par->filesInList = cf.Value("DL","filesInList");
	par->boundSymbol = cf.Value("DL","boundSymbol");
	par->cutDoc = cf.Value("DL","cutDoc");
	strcpy(par->dirStore, ((string)(cf.Value("DL","dirStore"))).c_str());

	cout << "buildDL_LZ config parameters..." << endl;
	cout << "Input File            : " << par->inputFile << endl;
	cout << "Files in list         : " << par->filesInList << endl;
	cout << "Boundary Symbol(code) : " << (int)par->boundSymbol << endl;
	cout << "Cut Doc. Symbol(code) : " << (int)par->cutDoc << endl;
	cout << "Store Folder          : " << par->dirStore << endl;
	cout << "Sampling <SA, SA^-1>  : <" << SaS << ", " << SaI << ">" << endl;

	SadaDocList64::TRACE = TRACE;
	SadaDocList64::TEST = TEST;
	par->index = new SadaDocList64(par->inputFile, par->filesInList, par->boundSymbol, par->cutDoc, par->dirStore);
	par->n = par->index->n;
	par->EndDocs = par->index->EndDocs;
	cout << "____________________________________________________" << endl;
	cout << "***  Index size " << par->index->sizeDS << " bytes = " << (float)par->index->sizeDS*8.0/(float)par->n << " bpc" << endl;
	cout << "====================================================" << endl;

	par->index->saveDS(true);

	if (TEST){
		// load Sequence...
		strcpy(fileName, "");
		strcpy(fileName, par->dirStore);
		strcat(fileName, "sequence.test");
		ifstream is(fileName, ios::binary);
		par->seq = new uchar[par->n];
		is.read((char*)par->seq, par->n*sizeof(uchar));
		is.close();

		// load FMI...
		strcpy(fileName, "");
		strcpy(fileName, par->dirStore);
		strcat(fileName, "fmi.dls");
		load_from_file(par->index->fmi, fileName);

		cout << "Running test searchPattern.." << endl;
		testDLSad(par);
		cout << "Test searchPattern OK !!" << endl;
	}

	par->index->~SadaDocList64();

	cout << "$$$$$$$$$$$$$$$$$$$$$" << endl;
	return 0;
}

void testDLSad(ParamProgram *par){
	uint m, t, i;
	uint nDocsSad, nDocsFMI;
	bool *V = new bool[par->index->nDocs];
	uint *occSad = new uint[par->index->nDocs];
	uchar* pat;

	for (m=1; m<=MAX_M; m++){
		createPatterns(par, m+1);

		for (t=0; t<N_REP; t++){
			nDocsSad = nDocsFMI = 0;
			for (i=0; i<par->index->nDocs; i++)
				V[i]=par->index->V[i]=0;

			nDocsFMI = searchPattInFMI(par, par->seq+par->patterns[t], m, V);
			par->index->documentListing(par->seq+par->patterns[t], m, occSad, &nDocsSad);

			if(nDocsFMI != nDocsSad){
				cout << "ERROR: m=" << m << ", test=" << t << ", nDocsFMI = " << nDocsFMI << " != nDocsSad = " << nDocsSad << endl;
				pat = par->seq+par->patterns[t];
				cout << "patt = [";
				for(i=0; i<m; i++)
					cout << pat[i];
				cout << "]" << endl;
				for(i=0; i<par->index->nDocs; i++){
					if(V[i] != par->index->V[i])
						cout << "Document " << i << ". par->index->V[i] == " << par->index->V[i] << " != V[i] = " << V[i] << endl;
				}
				exit(1);
			}
			//cout << "________________________________" << endl;
		}
		delete [] (par->patterns);
	}
}

uint searchPattInFMI(ParamProgram *par, uchar* pat, uint m, bool *V){
	ulong doc, i;
	uint nDocsFMI=0;
	string query = string((char *)pat);
	size_t occs = sdsl::count(par->index->fmi, query.begin(), query.begin()+m);
	auto locations = locate(par->index->fmi, query.begin(), query.begin()+m);

	if (TRACE){
		cout << "Total occurrences found with FMI : " << occs << endl;
		cout << "locations..." << endl;
	}

	for(i=0; i<occs; i++){
		doc = par->index->searchDocument(locations[i]);
		//if (TRACE) cout << locations[i] << "(" << doc << ") " << endl;

		if (V[doc]==0){
			V[doc]=1;
			nDocsFMI++;
		}
	}
	if (TRACE){
		cout << endl;
		cout << "nDocs found with FMI : " << nDocsFMI << endl;
	}

	return nDocsFMI;
}

void createPatterns(ParamProgram *par, uint m){
	ulong i, j, k;
	par->patterns = new ulong[N_REP];
	bool eq;

	//cout << "Creating patterns of length m=" << m << endl;
	for(k=0; k<N_REP; k++){
		eq = true;
		while(eq){
			i = (rand() % (par->n-(m+1)))+1;
			for(j=i; j<i+(m+1); j++){
				if (par->seq[j] == par->cutDoc){
					i = (rand() % (par->n-(m+1)))+1;
					j = i-1;
				}
				if(j==0) break;
				else{
					if (j>i && par->seq[j-1] != par->seq[j])
						eq = false;
				}
			}
		}
		par->patterns[k] = i;
		//cout << "["<<k<<"] " << i << endl;
	}
	//cout << "Patterns created !!" << endl;
}

