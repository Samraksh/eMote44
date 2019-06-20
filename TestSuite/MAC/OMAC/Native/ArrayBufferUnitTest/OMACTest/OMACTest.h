/*
 *
 *      Author: Bora Karaoglu
 */

#ifndef FAAVLBSTTEST_H_
#define FAAVLBSTTEST_H_

#include <Samraksh/Buffers/ArrayBuffer.h>
#include <Samraksh/MAC/OMAC/SeedGenerator.h>

#define NUMITEMS 10

typedef UINT16 uint16_t;


class UniformRandomInt{
	uint16_t	m_nextSeed, m_mask; // m_nextSeed stores the next seed to be used in calculating the next wakeup slot and the m_mask is used as a mask in the pseduo random function
	uint16_t minnum;
	uint16_t maxnum;

	SeedGenerator m_seedGenerator;
public:
	UniformRandomInt(uint16_t _minnum, uint16_t _maxnum){
		m_mask = (uint16_t)((float)137 * (float)29 * (31688 + 1));
		m_nextSeed = (uint16_t)((float)119 * (float)119 * (float)(31688 + 1)); // The initial seed
		minnum = _minnum;
		maxnum = _maxnum;
	}
	void Initialize(uint16_t _minnum, uint16_t _maxnum){
		m_mask = (uint16_t)((float)137 * (float)29 * (31688 + 1));
		m_nextSeed = (uint16_t)((float)119 * (float)119 * (float)(31688 + 1)); // The initial seed
		minnum = _minnum;
		maxnum = _maxnum;
	}
	uint16_t GetNext(){
		return minnum + m_seedGenerator.RandWithMask(&m_nextSeed, m_mask)% (maxnum+1-minnum);
	}
};

class Buffer_15_4_4_test_extension : public ArrayBuffer<10>{
//class Buffer_15_4_4_test_extension : public Buffer_15_4_4{
	Message_15_4_t tx_msg;
public:
	bool InserPacketWithDestination(UINT16 dest){
		tx_msg.GetHeader()->dest = dest;
		return this->InsertMessage(&tx_msg);

	}
	bool SearchPacketWithDest(UINT16 address){
		if(IsThereADataPacketWithDest((const UINT16)address) || this->IsThereATSRPacketWithDest(address) ){
			return true;
		}
		else{
			return false;
		}
	}
	bool DeletePacketWithDest(UINT16 address){
		if(this->DeletePacket(this->FindDataPacketForNeighbor(address))) return true;
		else if(this->DeletePacket(this->FindTSRPacketForNeighbor(address))) return true;
		else return false;
	}

	void PrintElemnentsofBuffer(){
		hal_printf( "\n ********BEGIN ELEMENTS*********NumElemnents = %d \n", this->GetNumberofElements());fflush(stdout);

		hal_printf( "Traversing from begin to end\n ");
		for(UINT16 j = 0; j < 10; ++j){
			hal_printf( "CurElement = %d ", this->send_packet_buffer[j].GetHeaderConst()->dest);fflush(stdout);
		}
		hal_printf( "\n ");

		hal_printf( "\n  --------END ---------\n");fflush(stdout);
	}
};

class OMAC_Common_Buffer_Test {

	Buffer_15_4_4_test_extension *buffer_ptr;
	UniformRandomInt *rnd;
	Buffer_15_4_4_test_extension tree;
	UniformRandomInt RandomIntObject;

public:
	void Initialize();
	OMAC_Common_Buffer_Test();
	void Execute();
	void Level_0A();

};



void OMACTest_Initialize();


#endif /* FAAVLBSTTEST_H_ */
