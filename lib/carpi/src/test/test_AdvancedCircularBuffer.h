#pragma once

#ifndef DISABLE_UNITY
    #error "This test is ment to be done with a human controlling the test, not as automated test!"
#endif

#define STEROIDO_DISABLE_LOOP

#include "../steroido/src/Steroido.h"
#include "TestingTimer.h"

#include "../steroido/src/Common/AdvancedCircularBuffer.h"


uint16_t implicitCounter = 1000;

class TestObj {
    public:
        TestObj() { 
            _i = implicitCounter++;
            printf("ImplicitConstruct %i\n", _i);
        }

        TestObj(uint16_t i) : _i(i) {
            printf("Construct %i\n", _i);
        }
        uint16_t get() { return _i; }
        
        TestObj(const TestObj &other) {
            _i = other._i;
            printf("CopyConstruct %i\n", _i);
        }
        
        TestObj& operator=(const TestObj& other) {
            _i = other._i;
            printf("CopyAssign %i\n", _i);
            
            return *this;
        }
        
        ~TestObj() {
            printf("Destruct %i\n", _i);
        }

        bool operator==(TestObj& other) {
            return _i == other._i;
        }
    
    private:
        uint16_t _i;
};

#define BUF_SIZE 5
AdvancedCircularBuffer<TestObj, BUF_SIZE, uint8_t> testBuffer;

bool testPushPop() {
    bool success = true;

    for (uint16_t i = 0; i < 8; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    for (uint16_t i = 8 - BUF_SIZE; i < 8; i++) {
        TestObj curr;
        if (testBuffer.pop(curr)) {
            if (curr.get() != i) {
                printf("Error: wrong element popped!\n");
                success = false;
            }
        } else {
            printf("Error: can't pop!\n");
            success = false;
        }
    }

    if (success) {
        printf("testPushPop success!\n");
    } else {
        printf("testPushPop failure!\n");
    }

    return success;
}

bool testGetters() {
    #if BUF_SIZE < 5
        #error "Buffer Size has to be at least 5"
    #endif

    bool success = true;

    for (uint16_t i = 0; i < 3*BUF_SIZE + 5; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    // Head

    TestObj getHeadRes;
    TestObj getHeadPlusTwo;
    testBuffer.getHead(getHeadRes);
    testBuffer.getFromHead(2, getHeadPlusTwo);

    if (testBuffer.getHeadReference().get() == 3*BUF_SIZE+4 &&
        testBuffer.getReferenceFromHead(2).get() == 3*BUF_SIZE+2 &&
        getHeadRes.get() == 3*BUF_SIZE+4 &&
        getHeadPlusTwo.get() == 3*BUF_SIZE+2) {
        printf("getHead success!\n");
    } else {
        printf("getHead failure!\n");
        success = false;
    }

    // Tail

    testBuffer.dropLast();

    TestObj getTailRes;
    TestObj getTailPlusTwo;
    testBuffer.getTail(getTailRes);
    testBuffer.getFromTail(2, getTailPlusTwo);

    if (testBuffer.getTailReference().get() == 3*BUF_SIZE+1 &&
        testBuffer.getReferenceFromTail(2).get() == 3*BUF_SIZE+3 &&
        getTailRes.get() == 3*BUF_SIZE+1 &&
        getTailPlusTwo.get() == 3*BUF_SIZE+3) {
        printf("getTail success!\n");
    } else {
        printf("getTail failure!\n");
        success = false;
    }


    if (success) {
        printf("testGetters success!\n");
    } else {
        printf("testGetters failure! \n");
    }

    return success;
}

bool testSize() {
    bool success = true;

    if (!testBuffer.empty()) {
        printf("Error: buffer not empty! (0)\n");
        success = false;
    }

    if (testBuffer.full()) {
        printf("Error: buffer already full! (0)\n");
        success = false;
    }

    if (testBuffer.size() != 0) {
        printf("Error: wrong size! (0)\n");
        success = false;
    }

    TestObj testObj(99);
    testBuffer.push(testObj);

    if (testBuffer.empty()) {
        printf("Error: buffer still empty! (0)\n");
        success = false;
    }

    if (testBuffer.full()) {
        printf("Error: buffer already full! (1)\n");
        success = false;
    }

    if (testBuffer.size() != 1) {
        printf("Error: wrong size! (1)\n");
        success = false;
    }

    for (uint16_t i = 0; i < 3*BUF_SIZE; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    if (testBuffer.empty()) {
        printf("Error: buffer still empty! (1)\n");
        success = false;
    }

    if (!testBuffer.full()) {
        printf("Error: buffer not full!\n");
        success = false;
    }

    if (testBuffer.size() != BUF_SIZE) {
        printf("Error: wrong size! (2)\n");
        success = false;
    }

    testBuffer.dropLast();

    if (testBuffer.empty()) {
        printf("Error: buffer still empty! (2)\n");
        success = false;
    }

    if (testBuffer.full()) {
        printf("Error: buffer full!\n");
        success = false;
    }

    if (testBuffer.size() != BUF_SIZE - 1) {
        printf("Error: wrong size! (3)\n");
        success = false;
    }

    bool empty = false;
    for (uint16_t i = 0; i < BUF_SIZE; i++) {
        TestObj curr;
        if (!testBuffer.pop(curr)) {
            if (!empty) {
                empty = true;
            } else {
                printf("Error: Not enough elements!\n");
                success = false;
            }
        }
    }

    if (!empty) {
        printf("Error: Not empty py pop return!\n");
        success = false;
    }

    if (!testBuffer.empty()) {
        printf("Error: buffer not empty! (2)\n");
        success = false;
    }

    if (testBuffer.full()) {
        printf("Error: buffer already full! (2)\n");
        success = false;
    }

    if (testBuffer.size() != 0) {
        printf("Error: wrong size! (4)\n");
        success = false;
    }

    if (success) {
        printf("testSize success!\n");
    } else {
        printf("testSize failure!\n");
    }

    return success;
}

bool testDelete() {
    bool success = true;

    #if BUF_SIZE < 5
        #error "Buffer Size has to be at least 5"
    #endif

    // Fill up / prep
    for (uint8_t i = 0; i < 5; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    TestObj elementToDel(3);
    printf("Delete 3 by deleteElement\n");
    if (!testBuffer.deleteElement(elementToDel)) {
        printf("Error: bad return val!\n");
        success = false;
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 3) {
            printf("Error: deleted element still there!\n");
            success = false;
        }
    }

    printf("Delete 2 by deleteFromTail(2)\n");
    if (!testBuffer.deleteFromTail(2)) {
        printf("Error: bad return val! (1)\n");
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 3) {
            printf("Error: already deleted Element reappeared!\n");
            success = false;
        }

        if (testBuffer.getReferenceFromTail(i).get() == 2) {
            printf("Error: deleted element still there! (1)\n");
            success = false;
        }
    }

    // Reset buffer
    for (uint8_t i = 0; i < 5; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    TestObj secondElement(1);
    printf("Delete 1 by deleteElement\n");
    if (!testBuffer.deleteElement(secondElement)) {
        printf("Error: bad return val!\n");
        success = false;
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 1) {
            printf("Error: deleted element still there! (1)\n");
            success = false;
        }
    }

    printf("Delete 2 by deleteFromHead(2)\n");
    if (!testBuffer.deleteFromHead(2)) {
        printf("Error: bad return val! (1)\n");
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 1) {
            printf("Error: already deleted Element reappeared! (1)\n");
            success = false;
        }

        if (testBuffer.getReferenceFromTail(i).get() == 2) {
            printf("Error: deleted element still there! (2)\n");
            success = false;
        }
    }



    // Reset buffer
    for (uint8_t i = 0; i < 5; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    printf("Delete 2 by deleteFromTailAmount(2)\n");
    if (!testBuffer.deleteAmountFromTail(2)) {
        printf("Error: bad return val! (2)\n");
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 0 || testBuffer.getReferenceFromTail(i).get() == 1) {
            printf("Error: deleted element still there! (3)\n");
            success = false;
        }
    }



    // Reset buffer
    for (uint8_t i = 0; i < 5; i++) {
        TestObj curr(i);
        testBuffer.push(curr);
    }

    printf("Delete 3 by deleteFromHeadAmount(3)\n");
    if (!testBuffer.deleteAmountFromHead(3)) {
        printf("Error: bad return val! (3)\n");
    }

    for (uint8_t i = 0; i < testBuffer.size(); i++) {
        if (testBuffer.getReferenceFromTail(i).get() == 4 || testBuffer.getReferenceFromTail(i).get() == 3 || testBuffer.getReferenceFromTail(i).get() == 2) {
            printf("Error: deleted element still there! (4)\n");
            success = false;
        }
    }


    if (success) {
        printf("testDelete success!\n");
    } else {
        printf("testDelete failure!\n");
    }

    return success;
}


void setup() {
    printf("Start\n");

    bool allSuccess = true;

    printf("testPushPop\n");
    allSuccess &= testPushPop();

    testBuffer.reset();

    printf("testGetters\n");
    allSuccess &= testGetters();

    testBuffer.reset();

    printf("testSize\n");
    allSuccess &= testSize();

    testBuffer.reset();

    printf("testDelete\n");
    allSuccess &= testDelete();

    if (allSuccess) {
        printf("All tests done with success!\n");
    } else {
        printf("One or more tests done with failures!\n");
    }

    printf("End of Test\n");
}