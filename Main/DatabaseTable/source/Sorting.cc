
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
    auto cmp = [&lhs, &rhs, &comparator](MyDB_RecordPtr a, MyDB_RecordPtr b) { 
        lhs = a;
        rhs = b;
        return comparator();
    };
    map<MyDB_RecordPtr, int, decltype(cmp)> heap(cmp);
    for (int i = 0; i < mergeUs.size(); i++) {
        MyDB_RecordPtr temp = make_shared<MyDB_Record>(lhs->getSchema());
        mergeUs[i]->getCurrent(temp);
        heap[temp] = i;
    }
    while (!heap.empty()) {
        auto pop = heap.begin();
        MyDB_RecordPtr record = pop->first;
        int idx = pop->second;
        heap.erase(pop);
        sortIntoMe.append(record);
        if (mergeUs[idx]->advance()) {
            MyDB_RecordPtr temp = make_shared<MyDB_Record>(lhs->getSchema());
            mergeUs[idx]->getCurrent(temp);
            heap[temp] = idx;
        }
    }
}

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter, 
	MyDB_RecordIteratorAltPtr rightIter, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	// Get anonymous storage from buffer manager
 
    // Iterate through both iterators, do a comparison, and add to anonymous storag
	return vector <MyDB_PageReaderWriter> (); 
} 
	
void sort (int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe, 
	function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	vector<MyDB_RecordIteratorAltPtr> sortedRunPtrs;
    int i = 0;
    while (i < sortMe.getNumPages() - 1) {
        // Load a run of pages into RAM .
        // Each page will correspond to a MyDB_PageReaderWriter object, and
        // all of those objects will be stored in a std :: vector.
        vector<vector<MyDB_PageReaderWriter>> run;
        for (; i < i + runSize && i < sortMe.getNumPages(); i++) {
            MyDB_PageReaderWriter page = sortMe[i];
 
            // Sort each individual page in the vector
            page.sortInPlace(comparator, lhs, rhs);
            vector<MyDB_PageReaderWriter> pageList = { page };
            run.push_back(pageList);
        }
 
        // go through the list, for each adjacent pair of pages in the list, use the mergeIntoList function
        while (run.size() > 1) {
            vector<vector<MyDB_PageReaderWriter>> newVector;
 
            for (int j = 0; j < run.size(); j += 2) {
                if (j == run.size() - 1) {
                    newVector.push_back(run[j]);
                    continue;
                }
                MyDB_RecordIteratorAltPtr leftIter = getIteratorAlt(run[j]);
                MyDB_RecordIteratorAltPtr rightIter = getIteratorAlt(run[j + 1]);
                vector <MyDB_PageReaderWriter> sortedPages = mergeIntoList(sortMe.getBufferMgr(), leftIter, rightIter, comparator, lhs, rhs);
            }
 
            run = newVector;
        }
 
        // there should only be one vector of pageReaderWriters now in the run
        MyDB_RecordIteratorAltPtr sortedRunPtr = getIteratorAlt(run[0]);
        sortedRunPtrs.push_back(sortedRunPtr);
    }
} 

#endif
