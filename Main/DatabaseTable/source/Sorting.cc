
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs, 
    function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

    auto cmp = [&lhs, &rhs, &comparator](pair<MyDB_RecordPtr, int> a, pair<MyDB_RecordPtr, int> b) { 
        lhs = a.first;
        rhs = b.first;
        return !comparator();
    };

    priority_queue<pair<MyDB_RecordPtr, int>, vector<pair<MyDB_RecordPtr, int>>, decltype(cmp)> pq(cmp);

    MyDB_SchemaPtr schema = lhs->getSchema();
    std::cout << "mergeus size: " << mergeUs.size() << std::endl;
    for (int i = 0; i < mergeUs.size(); i++) {
        MyDB_RecordPtr temp = make_shared<MyDB_Record>(schema);
        mergeUs[i]->getCurrent(temp);
        std::cout << "i: " << i << " temp: " << temp << std::endl;
        pq.push({temp, i});
    }

    int total = 0;
    bool adv = false;
    while (!pq.empty()) {
        total++;

        auto [record, idx] = pq.top();
        pq.pop();

        sortIntoMe.append(record);
        if (mergeUs[idx]->advance()) {
            MyDB_RecordPtr temp = make_shared<MyDB_Record>(schema);
            mergeUs[idx]->getCurrent(temp);
            pq.push({temp, idx});
        }
    }
    std::cout << "Total records merged: " << total << std::endl;
}

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter, 
	MyDB_RecordIteratorAltPtr rightIter, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

    // std::cout << "Merging two runs..." << std::endl;
    
    vector <MyDB_PageReaderWriter> sortedPages;
    MyDB_PageReaderWriter anonyPage = MyDB_PageReaderWriter(*parent);

    bool hasLeft = true;
    bool hasRight = true;
    // int total = 0;
    while (hasLeft || hasRight) {
        MyDB_RecordPtr appendMe;
        if (!hasLeft) {
            rightIter->getCurrent(rhs);
            appendMe = rhs;
            hasRight = rightIter->advance();
        } else if (!hasRight) {
            leftIter->getCurrent(lhs);
            appendMe = lhs;
            hasLeft = leftIter->advance();
        } else {
            leftIter->getCurrent(lhs);
            rightIter->getCurrent(rhs);
            if (comparator()) {
                appendMe = lhs;
                hasLeft = leftIter->advance();
            } else {
                appendMe = rhs;
                hasRight = rightIter->advance();
            }
        }

        if (!anonyPage.append(appendMe)) { 
            // Add to result vector and get a new anonymous page to append to
            sortedPages.push_back(anonyPage);
            anonyPage = MyDB_PageReaderWriter(*parent);
            
            // This should never happen
            if (!anonyPage.append(appendMe)) {
                std::cout << "Could not append record to new anonymous page" << std::endl;
            }
        }
        // total += 1;
    }

    sortedPages.push_back(anonyPage);
    // cout << "Appended " << total << " records" << endl;
	return sortedPages; 
} 
	
void sort (int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe, 
	function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	vector<MyDB_RecordIteratorAltPtr> sortedRunPtrs;
    int i = 0;
    while (i < sortMe.getNumPages()) {
        // cout << "starting new run" << endl;
        int end = min(i + runSize, sortMe.getNumPages());
        vector<vector<MyDB_PageReaderWriter>> run;
        for (; i < end; i++) {
            MyDB_PageReaderWriter page = sortMe[i];

            if (page.getIteratorAlt()->hasNext() == false) {
                continue; // Skip empty pages
            }
            
            // Sort each individual page in the vector

            page.sortInPlace(comparator, lhs, rhs);
            vector<MyDB_PageReaderWriter> pageList = { page };
            run.push_back(pageList);
        }
 
        // go through the list, for each adjacent pair of pages in the list, use the mergeIntoList function
        while (run.size() > 1) {
            vector<vector<MyDB_PageReaderWriter>> newVector;
            // cout << "sorting run" << endl;
            for (int j = 0; j < run.size(); j += 2) {
                if (j == run.size() - 1) {
                    newVector.push_back(run[j]);
                    continue;
                }
                MyDB_RecordIteratorAltPtr leftIter = getIteratorAlt(run[j]);
                MyDB_RecordIteratorAltPtr rightIter = getIteratorAlt(run[j + 1]);
                vector <MyDB_PageReaderWriter> sortedPages = mergeIntoList(sortMe.getBufferMgr(), leftIter, rightIter, comparator, lhs, rhs);
                newVector.push_back(sortedPages);
            }
 
            run = newVector;
        }
 
        // there should only be one vector of pageReaderWriters now in the run
        if (run.size() == 0) {
            continue;
        }
        MyDB_RecordIteratorAltPtr sortedRunPtr = getIteratorAlt(run[0]);
        sortedRunPtrs.push_back(sortedRunPtr);
    }

    std::cout << "Merging " << sortedRunPtrs.size() << " runs..." << std::endl;

    mergeIntoFile(sortIntoMe, sortedRunPtrs, comparator, lhs, rhs);
} 

#endif
