/*
 * SadaDocList64.cpp
 *
 *  Created on: 13-05-2015
 *      Author: hector
 */

#include "SadaDocList64.h"
bool SadaDocList64::TRACE = false;
bool SadaDocList64::TEST = false;

SadaDocList64::SadaDocList64(char dirSaveLoad[400]) {
	strcpy(dirStore, dirSaveLoad);
	loadDS(true);
}

SadaDocList64::SadaDocList64(char *inputFile, bool filesInList, char boundS, char cutDocCode, char dirSaveLoad[300]) {

	char fileName[300];

	// size for variables
	this->sizeDS = 2*sizeof(ulong) + sizeof(uint) + sizeof(char) + sizeof(uchar);	// size for variables
	cout << " ** Size of Variables " << sizeDS << " = " << sizeDS*8.0/(float)n << " bpc" << endl;
	this->cutDoc = cutDocCode;
	strcpy(dirStore, dirSaveLoad);
	this->nDocs = this->n = 0;

	if (filesInList){
		readListFiles(inputFile);
	}else
		readUniqueFile(inputFile, boundS);

	strcpy(fileName, "");
	strcpy(fileName, dirSaveLoad);
	strcat(fileName, "sequence.test");
	ofstream os (fileName, ios::binary);
	os.write((const char*)seq, n*sizeof(uchar));
	if(TRACE) cout << " .- Seq[] " << n*sizeof(uchar) << " Bytes" << endl;
	os.close();
	delete []seq;

	this->lgD = ceilingLog64(nDocs, 2);
	if(!lgD) lgD=1;

	this->V = new bool[nDocs];
	sizeDS += nDocs*sizeof(bool);
	cout << " ** size of V[1..nDocs] " << nDocs*sizeof(bool) << " Bytes = " << nDocs*sizeof(bool)*8.0/(float)n << " bpc" << endl;

	// create the FMI
	cout << "____________________________________________________" << endl;
	cout << " Make fmi..." << endl;
	strcpy(fileName, "");
	strcpy(fileName, inputFile);
	strcat(fileName, "_copy.txt");
	cout << " Reading... " << fileName << endl;
	construct(fmi, fileName, 1); // generate index
	sizeFMI = size_in_bytes(fmi);
	sizeDS += sizeFMI;
	cout << " **  FMI size " << sizeFMI << " Bytes = " << (float)sizeFMI/(float)n << "|T| = " << (float)sizeFMI*8.0/(float)n << " bpc" << endl;

	cout << " **  FMI length " << fmi.size() << endl;
	if (fmi.size() != n){
		cout << "ERROR. FMI length != n = " << n << endl;
		exit(1);
	}
	strcpy(fileName, "");
	strcpy(fileName, dirStore);
	strcat(fileName, "fmi.dls");
	store_to_file(fmi, fileName);

	if(false){
		cout << "FMI.." << endl;
		for(ulong i=0; i<n; i++)
			cout << fmi[i] << " ";
		cout << endl;
		//exit(0);
	}

	createEstructure();
}

// generates the DA
void SadaDocList64::createEstructure(){
	ulong i, j, val;
	long int *C;
	long int *Aux;
	uint doc, *DocArray;

	cout << " Create the Document Array..." << endl;
	DocArray = new uint[n];
	//cout << " CSA..." << endl;
	for(j=0; j<n; j++){
		val = fmi[j];
		//cout << val << " ";
		doc = searchDocument(val);
		if (doc > nDocs-1){
			cout << " ERROR, doc = " << doc << " for fmi[" << j << "] = " << fmi[j] << endl;
			exit(1);
		}
		DocArray[j]= doc;
	}
	//cout << endl;

	{	// free memory
		decltype(fmi) empty;
		fmi.swap(empty);
	}

	ulong *aux = new ulong[nDocs];
	for(j=0; j<nDocs; j++)
		aux[j] = 0;
	for(j=0; j<n; j++)
		(aux[DocArray[j]])++;

	//cout << " ## Validate document array..." << endl;
	ulong sum = 0;
	for(j=0; j<nDocs; j++){
		//cout << "E[" << j << "] = " << aux[j] << endl;
		sum += aux[j];
	}
	//cout << " sum = " << sum << endl;
	if (sum != n){							// VALIDATE DOCUMENT ARRAY !!
		cout << "ERROR. sum = " << sum << " != n = " << n << endl;

		cout << "EndDocs..." << endl;
		for(i=0; i<nDocs; i++)
			cout << EndDocs[i] << " ";
		cout << endl;

		ulong sum = 0;
		for(i=0; i<nDocs; i++){
			sum += aux[i];
			cout << sum << " ";
		}
		cout << endl;

		exit(1);
	}
	delete [] aux;

	if(TRACE){
		cout << " ## Document array..." << endl;
		cout << DocArray[0] << " ";
		for(j=1; j<n; j++){
			if (j%10 == 0)
				cout << " - ";
			cout << DocArray[j] << " ";
		}
		cout << endl;
	}

	cout << "____________________________________________________" << endl;
	cout << " Create the C pointer array on Document Array..." << endl;

	Aux = new long int[nDocs];
	C = new long int[n];

	for(i=0; i<nDocs; i++) Aux[i] = -1;
	for(j=0; j<n; j++){
		doc = DocArray[j];
		C[j] = Aux[doc];
		Aux[doc] = j;
	}
	delete [] Aux;
	delete [] DocArray;

	if (TRACE){
		cout << "C = ";
		for(j=1; j<n; j++){
			if (j%10 == 0)
				cout << " - ";
			cout << C[j] << " ";
		}
		cout << endl;
	}
	cout << " Create RMQ of C..." << endl;
	rmqC = new RMQRMM64(C,n);
	if(TEST){
		cout << "Test RMQ..." << endl;
		ulong beg, end, min, rmq_min;
		for (uint t=0; t<1000; t++){
			beg = (rand() % (n/2));
			end = n/2 + (rand() % (n/2)-1);
			if (end > beg+1){
				for (min=beg, i=beg+1; i<=end; i++){
					if (C[i] < C[min])
						min = i;
				}
				if (beg==end)
					rmq_min = beg;
				else
					rmq_min = rmqC->queryRMQ(beg,end);
				if (rmq_min < beg || rmq_min > end){
					cout << "ERROR... rmq_min = " << rmq_min << " out of range [" << beg << " , " << end << " ]" << endl;
					exit(1);
				}else{
					if (C[rmq_min] != C[min]){
						cout << "ERROR... (" << beg << " , " << end << " ) = " << rmq_min << " != " << min << endl;
						exit(1);
					}
				}
			}
		}
	}
	delete [] C;

	cout << " ** rmqC[1..n] " << rmqC->getSize() << " Bytes = " << rmqC->getSize()*8.0/(float)n << " bpc" << endl;
	sizeDS += rmqC->getSize();
}

// it determines the doc Id for the val
uint SadaDocList64::searchDocument(ulong val){
	uint m, l, r;
	m=l=0;
	r=nDocs-1;

	// binary search in the interval endDocs[0..nPhra-1]
	while(l <= r){
		m = l+(r-l)/2;
		if (val > EndDocs[m])
			l = m+1;
		else{
			if(m){
				if (val > EndDocs[m-1])
					return m;
				else
					r = m-1;
			}else
				return 0;
		}
	}

	cout << "ERROR... doc no encontrado para val = " << val;
	exit(1);

	return m;
}

void SadaDocList64::docListingMuthu(ulong l, ulong r, uint* occ, uint* nDocs){
	if (l > r) return;

	ulong x;
	if (l == r) x= l;
	else x = rmqC->queryRMQ(l,r);

	// 0 <= fmi[x] <= n-1
	uint doc = searchDocument(fmi[x]);
	if (V[doc] == 0){
		V[doc] = 1;
		//cout << " * doc: " << doc << ", nOcc " << par->nOcc << endl;
		occ[*nDocs] = doc;
		(*nDocs)++;
		if (x > 0)
			docListingMuthu(l, x-1, occ, nDocs);
		if (x < n-1)
			docListingMuthu(x+1, r, occ, nDocs);
	}

}

void SadaDocList64::documentListing(uchar* pat, uint m, uint* occ, uint* nDocs){
	string query = string((char *)pat);
	uint64_t lb=0, rb=fmi.size()-1;
	uint64_t l, r;
	backward_search(fmi, lb, rb, query.begin(), query.begin()+m, l, r);
	if(r>n-1) r=n-1;

	*nDocs = 0;
	docListingMuthu(l, r, occ, nDocs);
}


void SadaDocList64::readListFiles(char *inputFile){
	ulong i, len, lenText;
	uint cantD = 0;
	char fileName[300];

	std::ifstream in(inputFile);
	string line;
	std::getline(in,line);
	while(in){
	    strcpy(fileName,line.c_str());
	    //cout << "File: " << fileName << endl;
	    std::ifstream input(fileName);
		assert(input.good());
		input.seekg(0, ios_base::end);
		len = (size_t)input.tellg();
		if(len > 1){
			n += len;
			nDocs++;
		}
		input.close();
		std::getline(in,line);
	}
	in.close();

	cout << "Length of generalize text(n): " << n << ", in " << nDocs << " Documents" << endl;
	// allocate to memory for text...
	seq = new uchar[n];
	char *aux;
	std::ifstream in2(inputFile);
	lenText = 0;

	this->EndDocs = new ulong[nDocs];
	sizeDS += nDocs*sizeof(ulong);
	cout << " ** size of EndDocs[1..nDocs] " << nDocs*sizeof(ulong) << " Bytes = " << nDocs*sizeof(ulong)*8.0/(float)n << " bpc" << endl;

	for(ulong texts=0; texts < nDocs;){
		std::getline(in2,line);
		strcpy(fileName,line.c_str());
		std::ifstream input(fileName); 			// open file
		//cout << "... File: " << fileName << endl;
		assert(input.good()); 				// check open
		input.seekg(0, ios_base::end);			// move to the end
		len = (size_t)input.tellg();			// add the final symbol (smallest)
		if(len > 1){
			aux = new char[len];
			//if (TRACE)
			//cout << "To read " << fileName << " pos: " << lenText << "..." << lenText+len-1 << endl;

			input.seekg(0, ios_base::beg);		// move back to the beginning
			if (input.good()){
				input.read(aux, len);

				len--;
				aux[len] = cutDoc;
				//cout << aux << endl;
				for (i=0; i<len; i++, lenText++){
					if((uchar)(aux[i]) <= cutDoc)
						seq[lenText] = ' ';
					else
						seq[lenText] = (uchar)(aux[i]);
				}
				seq[lenText] = cutDoc;
				EndDocs[cantD] = lenText;
				lenText++;
				cantD++;
				assert(!input.fail());
				input.close();
			}else{
				cout << "Can't to open the file: <" << fileName << ">";
				exit(1);
			}
			delete [] aux;
			texts++;
		}
	}
	seq[n-1] = '\0';
	in2.close();
	char fileCpy[300];
	strcpy(fileCpy,inputFile);
	strcat(fileCpy, "_copy.txt");
	ofstream myfile;
	myfile.open (fileCpy);
	myfile << seq;
	myfile.close();
	seq[n-1] = cutDoc;

	if(TEST){
		uint DD = 0;
		for(i=0; i<n; i++){
			if (seq[i] == cutDoc)
				DD++;
		}
		if(nDocs != DD){
			cout << "ERROR with nDocs in Sequence !! " << endl;
			cout << "nDocs = " << nDocs << " != " << DD << endl;
			exit(1);
		}
		if(nDocs != cantD){
			cout << "ERROR with nDocs in Sequence !! " << endl;
			cout << "nDocs = " << nDocs << " != cantD = " << cantD << endl;
			exit(1);
		}
	}
	if(TRACE){		// listing original sequence
		cout << endl << "T[0.." << n-1 << "]:" << endl;
		for(i=0; i<n; i++){
			if (seq[i] == cutDoc)
				cout << "$";
			else
				cout << seq[i];
		}
		cout << endl;

		cout << "EndDocs..." << endl;
		for(i=0; i<nDocs; i++)
			cout << EndDocs[i] << " ";
		cout << endl;
	}
}

void SadaDocList64::readUniqueFile(char *inputFile, char boundS){
	ulong i, j, len;
	n = nDocs = 0;
	ifstream input(inputFile);			// open file
	assert(input.good()); 				// check open
	input.seekg(0, ios_base::end);		// move to the end
	n = (size_t)input.tellg();
	seq = new uchar[n];
	char *aux = new char[n];

	input.seekg(0, ios_base::beg);		// move back to the beginning
	if (input.good()){
		input.read(aux, n-1);
		for (i=0; i<n-1; i++){
			if((uchar)(aux[i]) == boundS){
				nDocs++;
				while((uchar)(aux[i+1]) == boundS){
					seq[i] = '\n';
					i++;
				}
				seq[i] = cutDoc;
			}else{
				if((uchar)(aux[i]) <= cutDoc)
					seq[i] = '\n';
				else
					seq[i] = (uchar)aux[i];
			}
		}
		if(seq[n-2] == cutDoc){
			seq[n-2] = '\n';
			nDocs--;
		}
		seq[n-1] = cutDoc;
		nDocs++;
		//cout << seq << endl;
		assert(!input.fail());
		input.close();
	}else{
		cout << "Can't to open the file: <" << inputFile << ">";
		exit(1);
	}
	input.close();
	delete [] aux;
	cout << "Length of generalize text(n): " << n << ", in " << nDocs << " Documents" << endl;

	this->EndDocs = new ulong[nDocs];
	sizeDS += nDocs*sizeof(ulong);
	cout << " ** size of EndDocs[1..nDocs] " << nDocs*sizeof(ulong) << " Bytes = " << nDocs*sizeof(ulong)*8.0/(float)n << " bpc" << endl;

	for (i=j=len=0; i<n; i++){
		if(seq[i] == cutDoc || seq[i] =='\0'){
			EndDocs[j] = i;
			len=0;
			j++;
		}else
			len++;
	}
	if (j != nDocs){
		cout << "Error cutDocs Symbols = " << j << " != nDocs = " << nDocs << endl;
		exit(1);
	}

	seq[n-1] = '\0';
	char fileCpy[300];
	strcpy(fileCpy,inputFile);
	strcat(fileCpy, "_copy.txt");
	ofstream myfile;
	myfile.open (fileCpy);
	myfile << seq;
	myfile.close();

	if(TRACE){		// listing original sequence
		cout << "T[0.." << n-1 << "]:" << endl;
		for(i=0; i<n; i++){
			if (seq[i] == cutDoc || seq[i] =='\0')
				cout << "$";
			else
				cout << seq[i];
		}
		cout << endl;

		cout << "EndDocs..." << endl;
		for(i=0; i<nDocs; i++)
			cout << EndDocs[i] << " ";
		cout << endl;
	}
}

// save the Data Structure in file 'fileName'
void SadaDocList64::saveDS(bool showSize){
	char *fileName = new char[300];
	cout << "Save data structure in folder " << dirStore << endl;

	strcpy(fileName, dirStore);
	strcat(fileName, "dataStructures.dls");
	ofstream os (fileName, ios::binary);
	cout << "   Saving. Data structure size: " << sizeDS << endl;

	os.write((const char*)&n, sizeof(ulong));
	os.write((const char*)&nDocs, sizeof(uint));
	os.write((const char*)&lgD, sizeof(uchar));
	os.write((const char*)&cutDoc, sizeof(char));

	ulong sizeDSav = 2*sizeof(ulong) + sizeof(uint) + sizeof(char) + sizeof(uchar);	// size for variables
	if(showSize) cout << " .- Variables " << sizeDSav << " Bytes = " << sizeDSav*8.0/(float)n << " bpc" << endl;

	sizeDSav += nDocs*sizeof(bool);
	if(showSize) cout << " .- V[1..nDocs] " << nDocs*sizeof(bool) << " Bytes = " << nDocs*sizeof(bool)*8.0/(float)this->n << " bpc" << endl;

	os.write((const char*)EndDocs, nDocs*sizeof(ulong));
	sizeDSav += nDocs*sizeof(ulong);
	if(showSize) cout << " .- EndDocs[] " << nDocs*sizeof(ulong) << " Bytes = " << nDocs*sizeof(ulong)*8.0/(float)this->n << " bpc" << endl;

	// RMQ
	strcpy(fileName, "");
	strcpy(fileName, dirStore);
	strcat(fileName, "rmq.dls");
	rmqC->saveDS(fileName);
	sizeDSav += rmqC->getSize();
	if(showSize) cout << " .- rmqC " << rmqC->getSize() << " Bytes" << endl;

	sizeDSav += sizeFMI;
	if(showSize) cout << " .-  FMI size " << sizeFMI << " bytes = " << (float)sizeFMI/(float)n << "|T|" << endl;

	cout << "   Total bytes saved from data structure DL_LZ = " << sizeDSav << endl;
	cout << "______________________________________________________________" << endl;
}

// load the Data Structure from the file 'fileName'
void SadaDocList64::loadDS(bool showSize){
	cout << " Load data structure from " << dirStore << endl;
	char *fileName = new char[300];

	strcpy(fileName, "");
	strcpy(fileName, dirStore);
	strcat(fileName, "dataStructures.dls");
	ifstream is(fileName, ios::binary);

	is.read((char*)&n, sizeof(ulong));
	is.read((char*)&nDocs, sizeof(uint));
	is.read((char*)&lgD, sizeof(uchar));
	is.read((char*)&cutDoc, sizeof(char));

	cout << " n = " << n << endl;
	cout << " nDocs = " << nDocs << endl;
	cout << " lgD = " << (uint)lgD << endl;
	cout << " cutDoc (= " << (uint)cutDoc << ")" << endl;

	sizeDS = 2*sizeof(ulong) + sizeof(uint) + sizeof(char) + sizeof(uchar);	// size for variables
	if(showSize) cout << " ** Size of Variables " << sizeDS << " Bytes = " << sizeDS*8.0/(float)n << " bpc" << endl;

	this->V = new bool[nDocs];
	sizeDS += nDocs*sizeof(bool);
	if(showSize) cout << " ** V[1..nDocs] " << nDocs*sizeof(bool) << " Bytes = " << nDocs*sizeof(bool)*8.0/(float)this->n << " bpc" << endl;

	EndDocs = new ulong[nDocs];
	is.read((char*)EndDocs, nDocs*sizeof(ulong));
	sizeDS += nDocs*sizeof(ulong);
	if(showSize) cout << " ** size of EndDocs[] " << nDocs*sizeof(ulong) << " Bytes = " << nDocs*sizeof(ulong)*8.0/(float)this->n << " bpc" << endl;

	// RMQ
	strcpy(fileName, "");
	strcpy(fileName, dirStore);
	strcat(fileName, "rmq.dls");
	rmqC = new RMQRMM64(fileName);
	sizeDS += rmqC->getSize();
	if(showSize) cout << " ** size of rmqC " << rmqC->getSize() << " Bytes" << endl;

	//FMI
	strcpy(fileName, "");
	strcpy(fileName, dirStore);
	strcat(fileName, "fmi.dls");
	load_from_file(fmi, fileName);
	sizeFMI = size_in_bytes(fmi);
	if(showSize) cout << " **  FMI (sampling=" << SaS << ") size " << sizeFMI << " Bytes = " << (float)sizeFMI/(float)n << "|T| = " << (float)sizeFMI*8.0/(float)n << " bpc" << endl;
	sizeDS += sizeFMI;

	cout << sizeDS << " bytes loaded !!" << endl;
	cout << "___________________________" << endl;
}


SadaDocList64::~SadaDocList64() {
	sizeDS = n = nDocs = lgD = cutDoc = 0;

	delete []EndDocs;
	delete []V;
	cout << "deleting rmqC..." << endl;
	rmqC->~RMQRMM64();
}
