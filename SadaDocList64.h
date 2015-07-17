/*
 * SadaDocList64.h
 *
 *  Created on: 13-05-2015
 *      Author: hector
 */

#ifndef SADADOCLIST64_H_
#define SADADOCLIST64_H_

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/int_vector.hpp>
#include <Basic_drf64.h>
#include <assert.h>
#include <RMQRMM64.h>

using namespace sdsl;
using namespace std;
using namespace drf64;

#define SaS 4
#define SaI 4

const uint W32 = 32;

class SadaDocList64 {

private:

	uchar lgD;				// Binary logarithm (ceiling) of nDocs
	RMQRMM64 *rmqC;			// RMQ structure for Document array, with length n

public:
	static bool TRACE;		// true: print all details for console
	static bool TEST;		// true: print all details for console

	ulong sizeDS;			// size in bytes for all data structure (this include sizeRMQRange)
	uchar *seq;				// original sequence (1 byte for symbol)
	ulong n;				// Length of generalize Text = T1$T2$...TD$
	uint nDocs;				// Number of distinct documents D

	sdsl::csa_wt<wt_huff<rrr_vector<127>>, SaS, SaI> fmi;
	ulong sizeFMI;			// size in bytes of FMI

	bool *V;				// In this we marking the reported documents.
	ulong* EndDocs;			// this store the final phrase number for each document. This is used only in time construction and it will be no include in the final structure

	char cutDoc;			// symbol to separate documents
	char dirStore[200];		// directory to save/load the data structure (files *.tk)

	SadaDocList64(char dirSaveLoad[400]);
	SadaDocList64(char *inputFile, bool filesInList, char boundS, char cutDocCode, char dirSaveLoad[300]);
	virtual ~SadaDocList64();

	// generates the DA
	void createEstructure();

	// it determines the doc Id for the val
	uint searchDocument(ulong val);

	// read the list of input file "inputFile", count the documents of these files
	// and store it in a the sequence seq[]. in Seq[] the files are separated by "cutDoc" symbol
	void readListFiles(char *inputFile);

	// read the input file "inputFile", count the documents in this file which are separated by "boundS" symbol
	// and store it in a the sequence seq[]. in Seq[] the files are separated by "cutDoc" symbol
	void readUniqueFile(char *inputFile, char boundS);

	// DL of Muthukrisman
	void docListingMuthu(ulong l, ulong r, uint* occ, uint* nDocs);

	// DL for pattern p of length m
	void documentListing(uchar* pat, uint m, uint* occ, uint* nDocs);

	// save the Data Structure in folder 'dirStore'
	void saveDS(bool showSize);

	// load the Data Structure from the file 'fileName'
	void loadDS(bool showSize);

};

#endif /* SADADOCLIST64_H_ */
