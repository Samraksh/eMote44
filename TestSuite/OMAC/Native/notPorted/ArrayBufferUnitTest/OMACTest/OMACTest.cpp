/*
 *
 *      Author: Bora Karaoglu
 *
 */

#include "OMACTest.h"

OMAC_Common_Buffer_Test gTest;

OMAC_Common_Buffer_Test::OMAC_Common_Buffer_Test() : RandomIntObject(0, NUMITEMS){
	Initialize();

}

void OMAC_Common_Buffer_Test::Initialize(){
	buffer_ptr = NULL;
	buffer_ptr =&tree ;
	RandomIntObject.Initialize(0, NUMITEMS);
	rnd = &RandomIntObject;

}

void OMAC_Common_Buffer_Test:: Execute(){
	//Sequential test
	hal_printf("\n\n************Sequential test************* \n");
	Level_0A();
}



/*
 * Level_0A: Sequential test
 * 		1-	Insert non-random non-continuous NUMITEMS elements one by one
 * 		2- 	Insert some number multiple times
 * 		3- 	Traverse the tree from front to end
 * 		4- 	Traverse the tree from end to front
 * 		5-	Delete even numbered elements  in a non-random non-continuous fashion
 * 		6-	Draw random elements from [0,NUMITEMS] and delete them
 */
void OMAC_Common_Buffer_Test::Level_0A(){
	int l1=5;
	int l2=NUMITEMS/l1;

	//
	//  rnd = new UniformRandomInt(0,NUMITEMS);
	rnd = &RandomIntObject;

	hal_printf("\n***TESTING INITIALIZATION****\n");fflush(stdout);
	buffer_ptr =&tree ;
	if(buffer_ptr ->GetNumberofElements() != 0){
		hal_printf("TEST FAIL: Initial size not 0\n");fflush(stdout);
	}
	uint32_t cur_num_items = 0;


	hal_printf("\n***TESTING: InsertPacketWithDesting deterministic but non-continuous elements *** \n");fflush(stdout);
	hal_printf("Inserting uint16_t ... \n");fflush(stdout);
	for(uint16_t i = l1; i >= 1; i--)
	{
		for(uint16_t j = (i * l2) - l2+1 ; j <= i * l2; j++)
		{
			hal_printf("Inserting %d NumElemnents = %d \n", j, buffer_ptr->GetNumberofElements());fflush(stdout);
			if(! buffer_ptr->InserPacketWithDestination(j) ) {
				hal_printf("TEST FAIL: Insertion FAILED \n");fflush(stdout);
			}
			else if(buffer_ptr->GetNumberofElements() != ++cur_num_items){
				hal_printf("TEST FAIL : NUM ITEMS NOT INCREMENTED  FAILED \n");fflush(stdout);
			}
			buffer_ptr->PrintElemnentsofBuffer();
		}
	}

	hal_printf("\n***TESTING: Inserting an element beyon the capacity *** \n");fflush(stdout);
	if(buffer_ptr->InserPacketWithDestination(NUMITEMS+1) ) {
		hal_printf("TEST FAIL: Insertion beyond capacity \n");fflush(stdout);
	}
	else if(buffer_ptr->GetNumberofElements() != NUMITEMS){
		hal_printf("TEST FAIL : NUMITEMS beyond capacity \n");fflush(stdout);
	}

	buffer_ptr->PrintElemnentsofBuffer();


	hal_printf("\n***TESTING: Delete even numbered elements*** \n");fflush(stdout);
	for(uint16_t i = l1; i >= 1; i--)
	{
		for(uint16_t j = (i * l2) - l2+1 ; j <= i * l2; j++)
		{
			if(j %2 ){

			}
			else{
				if(buffer_ptr->SearchPacketWithDest(j)){
					hal_printf( "\n Extracting %d NumElemnents = %d ... \n", j, buffer_ptr->GetNumberofElements());fflush(stdout);
					if(!buffer_ptr->DeletePacketWithDest(j)) {
						hal_printf( "\n TEST FAIL: Delete failed for a found element =  %d NumElemnents = %d ... \n", j, buffer_ptr->GetNumberofElements());fflush(stdout);
						buffer_ptr->PrintElemnentsofBuffer();
					}
					else{
						hal_printf( "\n Delete successful for %d NumElemnents = %d ... \n", j, buffer_ptr->GetNumberofElements());fflush(stdout);
						buffer_ptr->PrintElemnentsofBuffer();
					}
				}
			}
		}
	}

	hal_printf("\n***TESTING: Random deletion *** \n");fflush(stdout);
	uint16_t value;
	while(buffer_ptr->GetNumberofElements() > 0){
		value = rnd->GetNext();
		hal_printf( "\n Searching %d NumElemnents = %d ... \n", value, buffer_ptr->GetNumberofElements());fflush(stdout);
		if(buffer_ptr->SearchPacketWithDest(value)){
			hal_printf( "\n Extracting %d NumElemnents = %d ... \n", value, buffer_ptr->GetNumberofElements());fflush(stdout);
			if(!buffer_ptr->DeletePacketWithDest(value)) {
				hal_printf( "\n TEST FAIL: Delete failed for a found element =  %d NumElemnents = %d ... \n", value, buffer_ptr->GetNumberofElements());fflush(stdout);
				buffer_ptr->PrintElemnentsofBuffer();
			}
			else{
				hal_printf( "\n Delete successful for %d NumElemnents = %d ... \n", value, buffer_ptr->GetNumberofElements());fflush(stdout);
				buffer_ptr->PrintElemnentsofBuffer();
			}
		}
		else{
			hal_printf( "\n %d does not exist NumElemnents = %d ... \n", value, buffer_ptr->GetNumberofElements());fflush(stdout);

		}
	}

	hal_printf("\n***TESTING: Tree destruction *** \n");fflush(stdout);
}




void OMACTest_Initialize(){
	gTest.Initialize();
	gTest.Execute();
}


