//
//  BinarySearch.h
//  HappyPetStory
//
//  Created by Yow Feng Nyon on 23/7/14.
//  Copyright (c) 2014 Happy Labs. All rights reserved.
//

#ifndef HAPPYPETSTORY_BINARYSEARCH_H
#define HAPPYPETSTORY_BINARYSEARCH_H

#include <functional>

template <class T>
//static INT BinarySearch(const T* pDataSet, INT nSetSize, const T& value, BOOLEAN (*PREDICATE_LESS_THAN)(const T& lhs, const T& rhs))
static INT BinarySearch(const T* pDataSet, INT nSetSize, const T& value, std::function<BOOLEAN(const T& lhs, const T& rhs)> PREDICATE_LESS_THAN)
{
    // binary search
    INT nTopBound = nSetSize;
    INT nLowBound = 0;
    INT nSearch = 0;
    
    while (nLowBound < nTopBound) {
        
        nSearch = (nLowBound + nTopBound) / 2;
        
        if (PREDICATE_LESS_THAN(pDataSet[nSearch], value)) {
            nLowBound = nSearch + 1;
        }
        else {
            if (PREDICATE_LESS_THAN(value, pDataSet[nSearch])) {
                nTopBound = nSearch;
            }
            else {
                return nSearch;
            }
        }
    }
    
    // safety
    //if (0 <= nSearch && nSetSize > nSearch && pDataSet[nSearch].uID == uID) {
//    if (0 <= nSearch && nSetSize > nSearch && (!PREDICATE_LESS_THAN(value, pDataSet[nSearch]) && !PREDICATE_LESS_THAN(pDataSet[nSearch], value))) {
//        return nSearch;
//    }
    
    //TRACE("[CQuestData::GetQuestData] no data with id(%d) found\n", uID);
    
    return -1;
}


#endif
