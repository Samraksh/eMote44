/*
 * TimesyncTest.cpp
 * runFAAVLBSTTest.h
 *
 *  Created on: Feb 10, 2015
 *      Author: Bora Karaoglu
 *
 */

#include "OMACTest.h"



FAAVLBSTTest::FAAVLBSTTest() : RandomIntObject(0, NUMITEMS){
}

void FAAVLBSTTest:: Execute(){
  //Sequential test
	hal_printf("\n\n************Sequential test************* \n");
 Level_0A();

 //Generate random numbers
//	hal_printf("\n\n************Duplicate number insertion test************* \n");
//
// Level_0B();

  //Generate random numbers
	hal_printf("\n\n************Random Number test************* \n");

  Level_0C();

  //Multiple times initialize and destruct trees
	hal_printf("\n\n************Destruction with remaining elements test************* \n");

  Level_0D();

  hal_printf("\n\n************ALL TESTS COMPLETED ************* \n");

}

void FAAVLBSTTest::PrintElemnentsofTreeStructured(FAAVLBSTElement<uint16_t> *curElement, FAAVLBSTHeight_t offsetheight=0 ){
	if(curElement){
		if(curElement->left) {
			PrintElemnentsofTreeStructured(curElement->left,curElement->DepthL() < curElement->DepthR() ? offsetheight + (curElement->DepthR() - curElement->DepthL())  : offsetheight);
			//hal_printf("            ");
		}
		else {
		//	hal_printf("\n");
		}
		for(int i=0; i < ((curElement->CalculateDepth()-1)+ offsetheight); i++){
			hal_printf("            ");
		}
		if (curElement->parent) {
			hal_printf("%d(%d_%d_%d)",curElement->data, curElement->parent->data,curElement->DepthL(),curElement->DepthR());
		}
		else{
			hal_printf("%d(--_%d_%d)",curElement->data,curElement->DepthL(),curElement->DepthR());
		}
		hal_printf("\n");
		if(curElement->right) {
			PrintElemnentsofTreeStructured(curElement->right,curElement->DepthR() < curElement->DepthL() ? offsetheight + (curElement->DepthL() - curElement->DepthR())  : offsetheight);
		}
		else{
			//hal_printf("\n");
		}
	}
}


void FAAVLBSTTest::PrintElemnentsofTreeDepthFirst(FAAVLBSTElement<uint16_t> *curElement ){
	hal_printf("CurElement %d \n", curElement->data);
	if(curElement->left) {
		PrintElemnentsofTreeDepthFirst(curElement->left);
	}else{
		hal_printf("%d No Left\n", curElement->data);
	}
	if(curElement->right) {
		PrintElemnentsofTreeDepthFirst(curElement->right);
	}else{
		hal_printf("%d No Right\n", curElement->data);
	}
	if(curElement->parent) {
		hal_printf("%d 's parent %d \n", curElement->data, curElement->parent->data);
	}else{
		hal_printf("%d has no Parent\n", curElement->data);
	}
	hal_printf("%d DepthL() = %d DepthR() = %d \n",curElement->data, curElement->DepthL(), curElement->DepthR());
	fflush(stdout);
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
void FAAVLBSTTest::Level_0A(){
int l1=5;
int l2=NUMITEMS/l1;

//
//  rnd = new UniformRandomInt(0,NUMITEMS);
rnd = &RandomIntObject;

hal_printf("\n***TESTING INITIALIZATION****\n");fflush(stdout);
shortFAAVLBST =&tree ;
if(shortFAAVLBST->Size() != 0){
	 hal_printf("TEST FAIL: Initial size not 0\n");fflush(stdout);
}
uint32_t cur_num_items = 0;


 hal_printf("\n***TESTING: Inserting deterministic but non-continuous elements *** \n");fflush(stdout);
 hal_printf("Inserting uint16_t ... \n");fflush(stdout);
  for(uint16_t i = l1; i >= 1; i--)
  {
    for(uint16_t j = (i * l2) - l2+1 ; j <= i * l2; j++)
    {
     hal_printf("Inserting %d NumElemnents = %d \n", j, shortFAAVLBST->Size());fflush(stdout);
      if(! shortFAAVLBST->Insert(j) ) {
     	 hal_printf("TEST FAIL: Insertion FAILED \n");fflush(stdout);
      }
      else if(shortFAAVLBST->Size() != ++cur_num_items){
     	 hal_printf("TEST FAIL : NUM ITEMS NOT INCREMENTED  FAILED \n");fflush(stdout);
      }

      hal_printf( "\n runFAAVLBST ********BEGIN Tree*********NumElemnents = %d \n", shortFAAVLBST->Size());fflush(stdout);
      //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
      PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
      //shortFAAVLBST->PrintTreeStructured();
      hal_printf( "\n runFAAVLBST --------END  Tree---------\n");fflush(stdout);
    }
  }

  hal_printf("\n***TESTING: Inserting an element beyon the capacity *** \n");fflush(stdout);
  if(shortFAAVLBST->Insert(NUMITEMS+1) ) {
      	 hal_printf("TEST FAIL: Insertion beyond capacity \n");fflush(stdout);
  }
	else if(shortFAAVLBST->Size() != NUMITEMS){
	 hal_printf("TEST FAIL : NUMITEMS beyond capacity \n");fflush(stdout);
	}


  hal_printf( "\n runFAAVLBST ********BEGIN Tree*********NumElemnents = %d \n", shortFAAVLBST->Size());fflush(stdout);
  //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
  PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
  //shortFAAVLBST->PrintTreeStructured();
  hal_printf( "\n runFAAVLBST --------END  Tree---------\n");fflush(stdout);

  hal_printf( "Traversing from begin to end\n ");
  for(FAAVLBSTElement<uint16_t>* i = shortFAAVLBST->Begin(); i != NULL; i = shortFAAVLBST->Next(i) ){
		hal_printf( "CurElement = %d ", i->data);fflush(stdout);
  }
  hal_printf( "\n ");

  hal_printf( "Traversing from end to begin\n ");
  for(FAAVLBSTElement<uint16_t>* i = shortFAAVLBST->End(); i != NULL; i = shortFAAVLBST->Previous(i) ){
		hal_printf( "CurElement = %d ", i->data);fflush(stdout);
  }
  hal_printf( "\n ");


  hal_printf("\n***TESTING: Delete even numbered elements*** \n");fflush(stdout);
  for(uint16_t i = l1; i >= 1; i--)
  {
    for(uint16_t j = (i * l2) - l2+1 ; j <= i * l2; j++)
    {
    	if(j %2 ){

    	}
    	else{
    		  if(shortFAAVLBST->Search(j)){
    			 hal_printf( "\n Extracting %d NumElemnents = %d ... \n", j, shortFAAVLBST->Size());fflush(stdout);
    			  if(!shortFAAVLBST->Delete(j)) {
    				 hal_printf( "\n TEST FAIL: Delete failed for a found element =  %d NumElemnents = %d ... \n", j, shortFAAVLBST->Size());fflush(stdout);
    				  hal_printf( "\n runFAAVLBST ********BEGIN Tree*********\n");
    				  //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
    				  PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
    				  //shortFAAVLBST->PrintTreeStructured();
    				  hal_printf( "\n runFAAVLBST --------END  Tree---------\n");
    			  }
    			  else{
    				 hal_printf( "\n Delete successful for %d NumElemnents = %d ... \n", j, shortFAAVLBST->Size());fflush(stdout);
    				  hal_printf( "\n runFAAVLBST ********BEGIN Tree*********\n");
    				  //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
    				  PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
    				  //shortFAAVLBST->PrintTreeStructured();
    				  hal_printf( "\n runFAAVLBST --------END  Tree---------\n");
    			  }
    		  }
    	}
    }
  }

  hal_printf("\n***TESTING: Random deletion *** \n");fflush(stdout);
  uint16_t value;
  while(shortFAAVLBST->Size() > 0){
	  value = rnd->GetNext();
	  if(shortFAAVLBST->Search(value)){
		 hal_printf( "\n Extracting %d NumElemnents = %d ... \n", value, shortFAAVLBST->Size());fflush(stdout);
		  if(!shortFAAVLBST->Delete(value)) {
			 hal_printf( "\n TEST FAIL: Delete failed for a found element =  %d NumElemnents = %d ... \n", value, shortFAAVLBST->Size());fflush(stdout);
		      hal_printf( "\n runFAAVLBST ********BEGIN Tree*********\n");
		      //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
		      PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
		      //shortFAAVLBST->PrintTreeStructured();
		      hal_printf( "\n runFAAVLBST --------END  Tree---------\n");
		  }
		  else{
			 hal_printf( "\n Delete successful for %d NumElemnents = %d ... \n", value, shortFAAVLBST->Size());fflush(stdout);
		      hal_printf( "\n runFAAVLBST ********BEGIN Tree*********\n");
		      //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
		      PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
		      //shortFAAVLBST->PrintTreeStructured();
		      hal_printf( "\n runFAAVLBST --------END  Tree---------\n");
		  }
	  }
  }

  hal_printf("\n***TESTING: Tree destruction *** \n");fflush(stdout);
//  delete(shortFAAVLBST);
}


/*
 * Level_0B: Duplicate number insertion test
 * 		1-	Insert non-random non-continuous NUMITEMS elements one by one
 * 		2- 	Insert some number multiple times
 * 		3- 	Traverse the tree from front to end
 * 		4- 	Traverse the tree from end to front
 * 		5-	Draw random elements from [0,NUMITEMS] and delete them
 */
/* BK: Cancelling test since we allow duplicate numbers as long as comparator works
void FAAVLBSTTest::Level_0B(){
uint16_t test_element = 42;
uint32_t cur_num_items;
hal_printf("\n***TESTING INITIALIZATION****\n");fflush(stdout);
shortFAAVLBST = new FAAVLBST_T<NUMITEMS, uint16_t >() ;
if(shortFAAVLBST->Size() != 0){
	 hal_printf("TEST FAIL: Initial size not 0\n");fflush(stdout);
}

 hal_printf("\n***TESTING Element insertion ****\n");fflush(stdout);
 hal_printf("Inserting %d NumElemnents = %d \n", test_element, shortFAAVLBST->Size());fflush(stdout);
 cur_num_items = shortFAAVLBST->Size();
 if(! shortFAAVLBST->Insert(test_element) ) {
	 hal_printf("TEST FAIL: Insertion FAILED \n");fflush(stdout);
 }
 else if(shortFAAVLBST->Size() != ++cur_num_items){
	 hal_printf("TEST FAIL : NUM ITEMS NOT INCREMENTED  FAILED \n");fflush(stdout);
 }

  hal_printf( "\n runFAAVLBST ********BEGIN Tree*********NumElemnents = %d \n", shortFAAVLBST->Size());fflush(stdout);
  //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
  PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
  //shortFAAVLBST->PrintTreeStructured();
  hal_printf( "\n runFAAVLBST --------END  Tree---------\n");fflush(stdout);


  cur_num_items = shortFAAVLBST->Size();
  hal_printf("\n***TESTING  Duplicate Element insertion ****\n");fflush(stdout);
  hal_printf("Inserting %d NumElemnents = %d \n", test_element, shortFAAVLBST->Size());fflush(stdout);
  if( shortFAAVLBST->Insert(test_element) ) {
 	 hal_printf("TEST FAIL: Insertion succeeded on duplicate element \n");fflush(stdout);
  }
  else if(shortFAAVLBST->Size() != cur_num_items){
 	 hal_printf("TEST FAIL : size incremented after inserting duplicate element \n");fflush(stdout);
  }


  hal_printf( "Traversing from begin to end\n ");
  for(FAAVLBSTElement<uint16_t>* i = shortFAAVLBST->Begin(); i != NULL; i = shortFAAVLBST->Next(i) ){
		hal_printf( "CurElement = %d ", i->data);fflush(stdout);
  }
  hal_printf( "\n ");

  hal_printf( "Traversing from end to begin\n ");
  for(FAAVLBSTElement<uint16_t>* i = shortFAAVLBST->End(); i != NULL; i = shortFAAVLBST->Previous(i) ){
		hal_printf( "CurElement = %d ", i->data);fflush(stdout);
  }
  hal_printf( "\n ");


  hal_printf("\n***TESTING: Tree destruction *** \n");fflush(stdout);
  delete(shortFAAVLBST);
}
 */


void FAAVLBSTTest::Level_0C(){
//  int l1=0;
//  rnd = new UniformRandomInt(0,NUMITEMS);
//
//
//  hal_printf("\n***TESTING INITIALIZATION****\n");fflush(stdout);
//  shortFAAVLBST = new FAAVLBST_T<NUMITEMS, uint16_t >() ;
//  if(shortFAAVLBST->Size() != 0){
//  	 hal_printf("TEST FAIL: Initial size not 0\n");fflush(stdout);
//  }
//
//
//  hal_printf("\n***TESTING RANDOM INSERTION ****\n");fflush(stdout);
//  uint16_t value;
//  for(uint16_t i = NUMITEMS; i >= 1; i--)
//  {
//    value = rnd->GetNext();
//   hal_printf("Inserting %d \n", value);fflush(stdout);
//    //MyData *value=new MyData(key);
//    if(shortFAAVLBST->Insert(value)){
//    	++l1;
//    }
//    hal_printf( "\n runFAAVLBST ********BEGIN Tree*********\n");
//    PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
//    hal_printf( "\n runFAAVLBST --------END  Tree---------\n");
//   hal_printf( "top number is %d \n", shortFAAVLBST->PeekRoot());
//  }
// // IterateElements(shortFAAVLBST);
// hal_printf( "\n Extracting ... \n");fflush(stdout);
//  uint16_t newMax =0;
//  for(uint16_t i = l1; i >= 1; i--)
//  {
//    newMax = shortFAAVLBST->ExtractRoot();
//   hal_printf( "Extracted num is %d top number is %d, elements left in heap %d \n", newMax, shortFAAVLBST->PeekRoot(),  shortFAAVLBST->Size());
//  }
//
//  if(shortFAAVLBST->Size() != 0){
//	  hal_printf("TEST FAIL: Remianing elements after test \n");fflush(stdout);
//  }
//  delete(shortFAAVLBST);
}


void FAAVLBSTTest::Level_0D(){
//
//	int l1=5;
//	int l2=NUMITEMS/l1;
//	uint16_t cur_num_items;
//
//	for(uint16_t a = 0; a < NUMITEMS; ++a){
//	  hal_printf("\n***TESTING INITIALIZATION****\n");fflush(stdout);
//	  shortFAAVLBST = new FAAVLBST_T<NUMITEMS, uint16_t >() ;
//	  if(shortFAAVLBST->Size() != 0){
//	  	 hal_printf("TEST FAIL: Initial size not 0\n");fflush(stdout);
//	  }
//	  cur_num_items = 0;
//
//	  hal_printf("\n***TESTING: Inserting deterministic but non-continuous elements *** \n");fflush(stdout);
//	  hal_printf("Inserting uint16_t ... \n");fflush(stdout);
//	   for(uint16_t i = l1; i >= 1; i--)
//	   {
//	     for(uint16_t j = (i * l2) - l2+1 ; j <= i * l2; j++)
//	     {
//	      hal_printf("Inserting %d NumElemnents = %d \n", j, shortFAAVLBST->Size());fflush(stdout);
//	       if(! shortFAAVLBST->Insert(j) ) {
//	      	 hal_printf("TEST FAIL: Insertion FAILED \n");fflush(stdout);
//	       }
//	       else if(shortFAAVLBST->Size() != ++cur_num_items){
//	      	 hal_printf("TEST FAIL : NUM ITEMS NOT INCREMENTED  FAILED \n");fflush(stdout);
//	       }
//
//	       hal_printf( "\n runFAAVLBST ********BEGIN Tree*********NumElemnents = %d \n", shortFAAVLBST->Size());fflush(stdout);
//	       //rintElemnentsofTreeDepthFirst(shortFAAVLBST->GetRootPtr());
//	       PrintElemnentsofTreeStructured(shortFAAVLBST->GetRootPtr(),0);
//	       //shortFAAVLBST->PrintTreeStructured();
//	       hal_printf( "\n runFAAVLBST --------END  Tree---------\n");fflush(stdout);
//	     }
//	   }
//	   hal_printf("\n***TESTING: Tree destruction *** \n");fflush(stdout);
//	   delete(shortFAAVLBST);
//	}


}



void OMACTest_Initialize(){
	  FAAVLBSTTest hTest;
	  hTest.Execute();
}


