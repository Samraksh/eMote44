/*
 * runFAAVLBSTTest.h
 *
 *  Created on: Feb 10, 2015
 *      Author: Bora Karaoglu
 */

#ifndef FAAVLBSTTEST_H_
#define FAAVLBSTTEST_H_

#include <Samraksh/Buffer.h>

#define NUMITEMS 50

typedef UINT16 uint16_t;

class FAAVLBSTTest {
  FAAVLBST_T<NUMITEMS, uint16_t> *shortFAAVLBST;
//  UniformRandomInt *rnd;
  FAAVLBST_T<NUMITEMS, uint16_t> tree;
public:
  FAAVLBSTTest();
  void Execute();
  void Level_0A();
//  void Level_0B();
  void Level_0C();
  void Level_0D();

  void PrintElemnentsofTreeStructured(FAAVLBSTElement<uint16_t> *curElement, FAAVLBSTHeight_t offsetheight );
  void PrintElemnentsofTreeDepthFirst(FAAVLBSTElement<uint16_t> *curElement );
};

void OMACTest_Initialize();


#endif /* FAAVLBSTTEST_H_ */
