/*
 * runFAAVLBSTTest.h
 *
 *  Created on: Feb 10, 2015
 *      Author: Bora Karaoglu
 */

#ifndef FAAVLBSTTEST_H_
#define FAAVLBSTTEST_H_

#include <Samraksh/Buffer.h>
#include <Samraksh/MAC/OMAC/SeedGenerator.h>

#define NUMITEMS 50

typedef UINT16 uint16_t;


class UniformRandomInt{
	uint16_t	m_nextSeed, m_mask; // m_nextSeed stores the next seed to be used in calculating the next wakeup slot and the m_mask is used as a mask in the pseduo random function
	uint16_t minnum;
	uint16_t maxnum;

	SeedGenerator m_seedGenerator;
public:
	UniformRandomInt(uint16_t _minnum, uint16_t _maxnum){
		m_mask = 137 * 29 * (31688 + 1);
		m_nextSeed = 119 * 119 * (31688 + 1); // The initial seed
		minnum = _minnum;
		maxnum = _maxnum;
	}
	uint16_t GetNext(){
		return minnum + m_seedGenerator.RandWithMask(&m_nextSeed, m_mask)% (maxnum+1-minnum);
	}
};

class FAAVLBSTTest {
	Message_15_4_t tx_msg;
  Buffer_15_4_3<NUMITEMS> *shortFAAVLBST;
  UniformRandomInt *rnd;
  Buffer_15_4_3<NUMITEMS> tree;
  UniformRandomInt RandomIntObject;
public:
  FAAVLBSTTest();
  void Execute();
  void Level_0A();
//  void Level_0B();
  void Level_0C();
  void Level_0D();

  void PrintElemnentsofTreeStructured(FAAVLBSTElement<Message_15_4_t> *curElement, FAAVLBSTHeight_t offsetheight );
  void PrintElemnentsofTreeDepthFirst(FAAVLBSTElement<Message_15_4_t> *curElement );
};

FAAVLBSTTest gTest;

void OMACTest_Initialize();


#endif /* FAAVLBSTTEST_H_ */
