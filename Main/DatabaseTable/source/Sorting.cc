
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

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr, MyDB_RecordIteratorAltPtr, MyDB_RecordIteratorAltPtr, function <bool ()>, 
	MyDB_RecordPtr, MyDB_RecordPtr) {return vector <MyDB_PageReaderWriter> (); } 
	
void sort (int, MyDB_TableReaderWriter &, MyDB_TableReaderWriter &, function <bool ()>, MyDB_RecordPtr, MyDB_RecordPtr) {} 

#endif
