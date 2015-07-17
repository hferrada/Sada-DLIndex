/*
 * loadDL_Sada.cpp
 *
 *  Created on: 27-05-2015
 *      Author: hector
 */

#include "SadaDocList64.h"
#include <ConfigFile.h>

bool PRINT = false;			// true: print all details for console
bool TEST_IND = true;		// true: apply exhaustive test
bool RUN_EXP = false;
uint REPET = 5;				// number of repetitions for each experiment
uint M_MAX = 10;			// maximum length pattern value to run test

// Structure with all globals parameters program
typedef struct {
	string configFile;		// properties file

	uchar *seq;				// original sequence (1 byte for symbol)
	ulong n;				// Length of generalize Text = T1$T2$...TD$
	char cutDoc;			// symbol to separate documents
	bool lowerCPatt;		// 1: transform the patterns to lowercase

	SadaDocList64 *index;

	bool pattFromFile;		// 0: random patterns, 1: from file (as in todoCL)
	char dirStore[300];		// directory to save/load the data structure (files *.tk)
	char dirResult[300];	// directory to save tables

	ulong* patterns;
	uchar **patt;
	suint* lenPat;
} ParamProgram;

void timeCount(ParamProgram *par, uint m);
void runExperiments(ParamProgram *par);
void testDLSadLoad(ParamProgram *par);
uint searchPattInFMILoad(ParamProgram *par, uchar* pat, uint m, bool *V);

int main(int argc, char *argv[]) {
	ParamProgram *par = new ParamProgram();
	char fileName[300];

	cout << "Running loadDL_Sada.cpp ..." << endl;

	if(argc != 2){
		cout << "ERRORR !! " << endl;
		cout << "loadDL_Sada.cpp's usage requires the Properties File as parameter !! " << endl;
		cout << "Example for the file 'config.txt' in the same directory: ./loadDL_Sada.cpp config.txt" << endl;
		exit(1);
	}
	par->configFile = string(argv[1]);
	cout << "Congif file: " << par->configFile << endl;
	ConfigFile cf(par->configFile);

	PRINT = cf.Value("GLOBALS","TRACE");
	TEST_IND = cf.Value("GLOBALS","TEST");
	REPET = cf.Value("GLOBALS","N_REP");
	RUN_EXP = cf.Value("GLOBALS","RUN_EXP");

	strcpy(par->dirStore, ((string)(cf.Value("DL","dirStore"))).c_str());
	strcpy(par->dirResult, ((string)(cf.Value("DL","dirResult"))).c_str());
	par->pattFromFile = cf.Value("DL","pattFromFile");	// 0:random
	par->lowerCPatt = cf.Value("DL","lowercase");

	cout << "loadAppTopkLZ parameters..." << endl;
	cout << "dirStore: " << par->dirStore << endl;
	cout << "dirResult: " << par->dirResult << endl;
	cout << "patterns from file: " << par->pattFromFile << endl;
	cout << "lowercase patterns: " << par->lowerCPatt << endl;

	par->index = new SadaDocList64(par->dirStore);
	par->n = par->index->n;
	par->cutDoc = par->index->cutDoc;

	cout << "____________________________________________________" << endl;
	cout << "***  Index size " << par->index->sizeDS << " bytes = " << (float)par->index->sizeDS*8.0/(float)par->n << " bpc" << endl;
	cout << "====================================================" << endl;

	strcpy(fileName, "");
	strcpy(fileName, par->dirStore);
	strcat(fileName, "sequence.test");
	ifstream is(fileName, ios::binary);
	par->seq = new uchar[par->n];
	is.read((char*)par->seq, par->n*sizeof(uchar));
	is.close();

	if (TEST_IND){
		cout << "Test Index..." << endl;
		testDLSadLoad(par);
		cout << "Test Index OK !!" << endl;
	}

	if (RUN_EXP){
		cout << "Experiments..." << endl;
		runExperiments(par);
		cout << "Experiments OK !!" << endl;
	}


	cout << "$$$$$$$$$$$$$$$$$$$$$" << endl;
	return 0;
}

void runExperimentsOne(ParamProgram *par, uint m){
	uint nDocsSad, k, i;
	uint *occSad = new uint[par->index->nDocs];
	double t, avgTime;
	float avgnOcc;
	char aFile[400];
	char str[20];

	cout << "____________________________________________________" << endl;
	cout << "Start DL for patterns of length m = " << m << endl;
	avgTime = 0.0;
	avgnOcc = 0.0;
	for (i=0; i<par->index->nDocs; i++)
		par->index->V[i] = 0;

	for (k=0; k<REPET; k++){
		nDocsSad =0;
		t = getTime_ms();
		par->index->documentListing(par->seq+par->patterns[k], m, occSad, &nDocsSad);
		t = getTime_ms() - t;
		avgTime += t/(double)REPET;
		avgnOcc += nDocsSad;

		// It cleans the V[] vector or unmarks the documents reported !!
		for (i=0; i<nDocsSad; i++)
			par->index->V[occSad[i]] = 0;
	}
	avgnOcc /= (float)REPET;
	cout << "Average CPU time for execution: " << avgTime*1000.0 << " Microseconds" << endl;
	cout << "Average nDocs found : " << avgnOcc << endl;
	cout << "Size : " << par->index->sizeDS*8.0/(float)par->n << endl;
	cout << "____________________________________________________" << endl;

	strcpy(aFile, par->dirResult);
	strcpy(str, "");
	sprintf(str, "SadaDL_m%d", m);
	strcat(aFile, str);

	FILE *fp = fopen(aFile, "a+" );
	// [m] [SAMPLE SA] [SAMPLE ISA] [size] [nDocs] [time]
	fprintf(fp, "%d %d %d %f %f %G\n", m, SaS, SaI, par->index->sizeDS*8.0/(float)par->n, avgnOcc, avgTime*1000.0);
	fclose(fp);
}

void createPatternsLoad(ParamProgram *par, uint m){
	ulong i, j, k;
	par->patterns = new ulong[REPET];
	bool eq;

	//cout << "Creating patterns of length m=" << m << endl;
	for(k=0; k<REPET; k++){
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


void timeCount(ParamProgram *par, uint m){
	double t, avgTime;
	uchar* pat;
	uint k;
	string query;
	uint64_t lb, rb, l, r;

	avgTime = 0.0;
	for (k=0; k<REPET; k++){
		pat = par->seq+par->patterns[k];
		query = string((char *)pat);
		l=lb=0;
		r=rb=par->index->fmi.size()-1;

		t = getTime_ms();
		backward_search(par->index->fmi, lb, rb, query.begin(), query.begin()+m, l, r);
		t = getTime_ms() - t;
		avgTime += t/(double)REPET;
	}
	cout << "____________________________________________________" << endl;
	cout << "Time for count..." << endl;
	cout << "Average CPU time for count m=" << m << " = " << avgTime*1000.0 << " Microseconds" << endl;
	cout << "____________________________________________________" << endl;
}

void runExperiments(ParamProgram *par){
	uint m;
	cout << "====================================================" << endl;

	par->patterns = new ulong[REPET];
	m = 6;
	createPatternsLoad(par, m);
	cout << "Patterns created for m = " << m << endl;
	timeCount(par, m);
	cout << "Document Listing..." << endl;
	runExperimentsOne(par, m);

	m = 10;
	createPatternsLoad(par, m);
	cout << "Patterns created for m = " << m << endl;
	timeCount(par, m);
	cout << "Document Listing..." << endl;
	runExperimentsOne(par, m);

	delete [] (par->patterns);
}

void testDLSadLoad(ParamProgram *par){
	uint m, t, i;
	uint nDocsSad, nDocsFMI;
	bool *V = new bool[par->index->nDocs];
	uint *occSad = new uint[par->index->nDocs];
	uchar* pat;

	for (m=1; m<=M_MAX; m++){
		createPatternsLoad(par, m+1);

		for (t=0; t<REPET; t++){
			nDocsSad = nDocsFMI = 0;
			for (i=0; i<par->index->nDocs; i++)
				V[i]=par->index->V[i]=0;

			nDocsFMI = searchPattInFMILoad(par, par->seq+par->patterns[t], m, V);
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
		}
		delete [] (par->patterns);
	}
}


uint searchPattInFMILoad(ParamProgram *par, uchar* pat, uint m, bool *V){
	ulong doc, i;
	uint nDocsFMI=0;
	string query = string((char *)pat);
	size_t occs = sdsl::count(par->index->fmi, query.begin(), query.begin()+m);
	auto locations = locate(par->index->fmi, query.begin(), query.begin()+m);

	if (PRINT){
		cout << "Total occurrences found with FMI : " << occs << endl;
		cout << "locations..." << endl;
	}

	for(i=0; i<occs; i++){
		doc = par->index->searchDocument(locations[i]);
		if (PRINT) cout << locations[i] << "(" << doc << ") " << endl;

		if (V[doc]==0){
			V[doc]=1;
			nDocsFMI++;
		}
	}
	if (PRINT){
		cout << endl;
		cout << "nDocs found with FMI : " << nDocsFMI << endl;
	}

	return nDocsFMI;
}
