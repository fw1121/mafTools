/* 
 * Copyright (C) 2012 by 
 * Dent Earl (dearl@soe.ucsc.edu, dentearl@gmail.com)
 * ... and other members of the Reconstruction Team of David Haussler's 
 * lab (BME Dept. UCSC).
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 */
#ifndef MAFTRANSITIVECLOSURE_H_
#define MAFTRANSITIVECLOSURE_H_
#include "sonLib.h"
#include "stPinchGraphs.h"
#include "common.h"
#include "CuTest.h"
#include "sharedMaf.h"

typedef struct mafTcSeq {
    // maf tc (trasitive closure) sequence
    char *name;
    char *sequence;
    uint32_t length;
} mafTcSeq_t;
typedef struct mafTcRegion {
    // region or interval
    uint32_t start;
    uint32_t end;
    struct mafTcRegion *next;
} mafTcRegion_t;
typedef struct mafTcComparisonOrder {
    // This struct is used in adding alignment information into the thread set.
    /* for this sequence matrix:
         0123456789
       0 AC---ACG-G
       1 ACG--ACGGC
       2 A-G-TACGGC
       3 ACGTTACGGC
       a comparison order is {3: [3, 3]}, {2: [4, 4]},
       {1: [8, 8]}, {1: [2, 2]}, {0: [9, 9]}, {0: [5, 7]}, {0: [0, 1]}
       [note that the ordering of these values is arbitrary, though consistent with the
       output of the algo in the code which appends new structs to the head of the list]
       e.g. the first block to process uses 0 as its reference and it starts at column 1 
       and ends at column 1. The second block to process uses 1 as its reference and it
       starts at column 2 and ends at column 2 (it is only one column), etc.
     */
    uint32_t ref; // 
    mafTcRegion_t *region;
    struct mafTcComparisonOrder *next;
} mafTcComparisonOrder_t;
typedef struct mafCoordinatePair {
    /* this struct is used to store pairs of coordinates
    */
    int64_t a;
    int64_t b;
} mafCoordinatePair_t;
typedef struct mafBlockSort {
    /* this struct is used to sort a sequence matrix by the number of gaps in each row
     */
    int64_t value; // value to sort upon
    mafLine_t *ml;
} mafBlockSort_t;

void usage(void);
mafTcSeq_t* newMafTcSeq(char *name, unsigned length);
mafTcComparisonOrder_t* newMafTcComparisonOrder(void);
mafTcRegion_t* newMafTcRegion(uint32_t start, uint32_t end);
mafCoordinatePair_t* newCoordinatePairArray(uint32_t numSeqs, char **seqs);
void destroyMafTcSeq(void *p);
void destroyMafTcRegionList(mafTcRegion_t *r);
void destroyMafTcRegion(mafTcRegion_t *r);
void destroyMafTcComparisonOrder(mafTcComparisonOrder_t *c);
void destroyCoordinatePairArray(mafCoordinatePair_t *cp);
uint32_t hashMafTcSeq(const mafTcSeq_t *mtcs);
int hashCompareMafTcSeq(const mafTcSeq_t *m1, const mafTcSeq_t *m2);
char* createNSequence(unsigned length);
void reverseComplementSequence(char *s, size_t  n);
void complementSequence(char *s, size_t n);
char complementChar(char c);
void addSequenceValuesToMtcSeq(mafLine_t *ml, mafTcSeq_t *mtcs);
void parseOptions(int argc, char **argv, char *filename);
stPinchThreadSet* buildThreadSet(stHash *hash);
void walkBlockAddingAlignments(mafBlock_t *mb, stPinchThreadSet *threadSet);
void addAlignmentsToThreadSet(mafFileApi_t *mfa, stPinchThreadSet *threadSet);
void createSequenceHash(mafFileApi_t *mfa, stHash **hash, stHash **nameHash);
mafTcRegion_t* getComparisonOrderFromRow(char **mat, uint32_t row, mafTcComparisonOrder_t **done, 
                                         mafTcRegion_t *todo, int containsGaps);
mafTcComparisonOrder_t *getComparisonOrderFromMatrix(char **mat, uint32_t rowLength, uint32_t colLength, 
                                                     uint32_t *lengths, int **vizMat);
void processPairForPinching(stPinchThreadSet *threadSet, stPinchThread *a, uint32_t aGlobalStart, 
                            uint32_t aGlobalLength, int aStrand, 
                            char *aSeq, stPinchThread *b, uint32_t bGlobalStart, uint32_t bGlobalLength,
                            int bStrand, char *bSeq, uint32_t regionStart, uint32_t regionEnd,
                            mafCoordinatePair_t aBookmark, mafCoordinatePair_t bBookmark, 
                            int aContainsGaps, int bContainsGaps,
                            void (*pinchFunction)(stPinchThread *, stPinchThread *, int64_t, int64_t, int64_t, bool));
int64_t localSeqCoords(uint32_t p, char *s, mafCoordinatePair_t *bookmark, int containsGaps);
int64_t localSeqCoordsToGlobalPositiveCoords(int64_t c, uint32_t start, uint32_t sourceLength, char strand);
int64_t localSeqCoordsToGlobalPositiveStartCoords(int64_t c, uint32_t start, uint32_t sourceLength, 
                                                  char strand, uint32_t length);
void mafBlock_sortBlockByIncreasingGap(mafBlock_t *mb);
// debugging tools
int** getVizMatrix(mafBlock_t *mb, unsigned n, unsigned m);
void updateVizMatrix(int **mat, mafTcComparisonOrder_t *co);
void printVizMatrix(int **mat, uint32_t n, uint32_t m);
void printTodoArray(mafTcRegion_t *reg, unsigned max);
// test suite
CuSuite* mafTransitiveClosure_TestSuite(void);
int mafTransitiveClosure_RunAllTests(void);

#endif // MAFTRANSITIVECLOSURE_H_