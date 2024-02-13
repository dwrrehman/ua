#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t nat;
const nat unknown = 11223344556677;

const nat _ = unknown;

int main(void) {
	

	const nat operation_count = 2 + 3; 		// 2 starting nodes, and 4 extension nodes!
	const nat graph_count = 4 * operation_count;
	nat* graph = calloc(graph_count, sizeof(nat));

	nat partial_graph[graph_count ] = {
/*0*/		0,    1,  [0],  1,
/*1*/		1,   [0],  0,   0,
/*2*/	       [0],  [1], [2],  _,
/*3*/		_,    _,   _,   _,
/*4*/		_,    _,   _,   _,
/*5*/		_,    _,   _,   _,
/*6*/		_,    _,   _,   _,
	}


	memcpy(graph, partial_graph, graph_count * sizeof(nat));

        const nat m = 134;
        



        nat* nf_array = calloc((size_t) n + 1, sizeof(nat)), i = 0;

        goto print;






loop:
	if (nf_array[i] < m) goto increment;
        if (i < n) goto reset;

        goto done;



increment:    

	nf_array[i]++;
	i = 0; 

print:  
	for (int _ = 0; _ < n + 1; _++) printf("%d ", nf_array[_]); putchar(10);

        goto loop;

reset:
	nf_array[i] = 0; 
	i++; 

	goto loop;

done:
	puts("looked at all possibilites!");
}








/*
	202306294.155521:



	the first thing that this algorithm needs to do, is look over all posibilities  (edge connections)  between nodes that exist in the graph, already!! don'tttttt add any more new nodes YET.


	then, once youve done that,  
			then you are clear to add a new node,         and you need to keep track of the fact that you have to look over all possible operations for that node!


										so thats like, part of the possible fill-in's for that hole. 

										it includes looking at the hole's operations.


		


	











FACT: 

		the nodes in the graph,     are filled in           as a strick stack, 

			ie, you never have a case, where you fill in     the     address=4  node       beforeeeeee filling in the address 2 or 3 node. 

				ie, if you fill in a node with address          k          then   all nodes at addresses   0...k-1 are filled.




				so yeah, its a stack
	cool 




	


	ALSOO

			FACT:

								once you start filling in a particular node, 


						there is no going back,   on it 


						ie,   you can never make that node's operation        "unknown"     ie      _


					again 

								ie, once you increase your node count in the graph,   it stays increased, 

						and you continue with that many nodes, always,    from there. 






		

	
okay 
	so 
	i think this is starting form 



	

		






*/





























0 0 0
1 0 0
2 0 0
0 1 0
1 1 0
2 1 0
0 2 0
1 2 0
2 2 0
0 0 1
1 0 1
2 0 1
0 1 1
1 1 1
2 1 1
0 2 1
1 2 1
2 2 1
0 0 2
1 0 2
2 0 2
0 1 2
1 1 2
2 1 2
0 2 2
1 2 2
2 2 2

0 0 0 0
1 0 0 0
2 0 0 0
0 1 0 0
1 1 0 0
2 1 0 0
0 2 0 0
1 2 0 0
2 2 0 0
0 0 1 0
1 0 1 0
2 0 1 0
0 1 1 0
1 1 1 0
2 1 1 0
0 2 1 0
1 2 1 0
2 2 1 0
0 0 2 0
1 0 2 0
2 0 2 0
0 1 2 0
1 1 2 0
2 1 2 0
0 2 2 0
1 2 2 0
2 2 2 0


0 0 0 1
1 0 0 1
2 0 0 1
0 1 0 1
1 1 0 1
2 1 0 1
0 2 0 1
1 2 0 1
2 2 0 1
0 0 1 1
1 0 1 1
2 0 1 1
0 1 1 1
1 1 1 1
2 1 1 1
0 2 1 1
1 2 1 1
2 2 1 1
0 0 2 1
1 0 2 1
2 0 2 1
0 1 2 1
1 1 2 1
2 1 2 1
0 2 2 1
1 2 2 1
2 2 2 1





0 0 0 2
1 0 0 2
2 0 0 2
0 1 0 2
1 1 0 2
2 1 0 2
0 2 0 2
1 2 0 2
2 2 0 2
0 0 1 2
1 0 1 2
2 0 1 2
0 1 1 2
1 1 1 2
2 1 1 2
0 2 1 2
1 2 1 2
2 2 1 2
0 0 2 2
1 0 2 2
2 0 2 2
0 1 2 2
1 1 2 2
2 1 2 2
0 2 2 2
1 2 2 2
2 2 2 2


0 0 0 0 0
1 0 0 0 0
2 0 0 0 0
0 1 0 0 0
1 1 0 0 0
2 1 0 0 0
0 2 0 0 0
1 2 0 0 0
2 2 0 0 0
0 0 1 0 0
1 0 1 0 0
2 0 1 0 0
0 1 1 0 0
1 1 1 0 0
2 1 1 0 0
0 2 1 0 0
1 2 1 0 0
2 2 1 0 0
0 0 2 0 0
1 0 2 0 0
2 0 2 0 0
0 1 2 0 0
1 1 2 0 0
2 1 2 0 0
0 2 2 0 0
1 2 2 0 0
2 2 2 0 0


0 0 0 1 0
1 0 0 1 0
2 0 0 1 0
0 1 0 1 0
1 1 0 1 0
2 1 0 1 0
0 2 0 1 0
1 2 0 1 0
2 2 0 1 0
0 0 1 1 0
1 0 1 1 0
2 0 1 1 0
0 1 1 1 0
1 1 1 1 0
2 1 1 1 0
0 2 1 1 0
1 2 1 1 0
2 2 1 1 0
0 0 2 1 0
1 0 2 1 0
2 0 2 1 0
0 1 2 1 0
1 1 2 1 0
2 1 2 1 0
0 2 2 1 0
1 2 2 1 0
2 2 2 1 0























//
//
//	nat n = operation_count - 1;          n, as used in the nf,     corresponds to this. 
//
//      nat m = ????;   

///////////////////

nf(1, 1):

	 0 0
	 0 1
	 1 0
	 1 1


/////////////////






/*
 o#   :=      {          the sequential values  of     an M_2  variable/hole.

		0   	mean "i++",    (1)
		1 	means "(*n)++" (2)
		2 	means "(*i)++" (3)
		3 	means "i = 0"  (5)
		4 	means "*n = 0" (6)

	}


*/
	
//	------------------------------
//	        o#    <    >    =
//	------------------------------
/*0*/		0,    1,  [0],  1,
/*1*/		1,   [0],  0,   0,
/*2*/	       [0],  [1], [2], [0],
/*3*/		_,   _, _, _,
/*4*/		_,   _, _, _,
/*5*/		_,   _, _, _,
/*6*/		_,   _, _, _,
//	------------------------------


	variable:  type=addresss   ->    LRS(operation_count) = M_1          <---------- THIS  MODULUS   INCREASES OVER TIME!!!!!!

	variable:  type=operation   ->    LRS(unique_oc) = M_2






all of the holes that are of type address, 


	they will be at the beginning of the nf_array      will all be of type     address    ie,  have modulus of    M_1


		the values at the end of the nfarray    will be of type operation,   and have modulus M_2


	


when you add a new operation, 


		you add               ONE new hole/variable/element-in-the-nfarray      of type    M_2

				and you addd     THREE    variables/etc     of type   M_1




		upon  addding those three holes     of type   M_1         you set     the selected option  for all of them,  
					to be  
								0 

										ie, the first option. 


				they are NOTTTTT added into graph, as still unknowns. you immediately fill them. 

					upon   adding their spot  into the nfarray. 





	nf array:


		[M_1] [M_1] [M_1] [M_1] [M_1] [M_1] [M_1]      [M_2] [M_2] [M_2] [M_2] [M_2] 




when we add variables, 


		we add them like this:



		[M_1] [M_1] [M_1] [M_1] [M_1] [M_1] [M_1] [(NEW)M_1] [(NEW)M_1] [(NEW)M_1]      [M_2] [M_2] [M_2] [M_2] [M_2] [(NEW)M_2]


	


	notice how i added exactlyyyy 3   M_1's   and    1    M_2           the M_2 was put at the veryyyyy end,    definitely 

			but the M_1's were put at the end of the the M_1 variable section.  those all always belong together. 



			
			


legend:

	(tX){ blah }      is the timestep that we added    the varibles given by blah    into the nfarray.
	t0   is done before the algorithm starts.






(t0){ [M_1] [M_1] }     (t1){ [M_2] [M_1] [M_1] [M_1] }   (t2){ [M_2] [M_1] [M_1] [M_1] }   (t3){ [M_2] [M_1] [M_1] [M_1] }
				0     0     0     0 
				init of zeros!!!
				never _  ever again!
				

								note:	-------> just from a performance standpoint, 

									this approach is better than the   M-grouped approach, i think. 



	we run the algorithm, after t0,  

	looking at all posibilies, for    these two little holes


	and then, when we exhausted all of those, 
		we arrive at t1, 


			where we add 4 more new posibilies. 


					AND NOTE:    ALL THESE POSS.  WILL HAVE     INIT  VALUES OF 0         

						ie, at first option. 


								ANDDDD they are never empty    (ie, unknown, ever again)


	
							


			
		








FACT:	

		when we change the length of   nfarray 




  (ie, add   4  new holes,     (3 of the M_1's,   (which will internally use the new value of M_1!!!! becase now M_1  has been incremented because we added an M_2 variable.)

											and 1 of the M_2 variables.  because we need to try all possible operations this node could be. 
)




		then it is the case that           THE ENTIRE NFARRAY     will be 0's.        ie,  all options are at their first option, 
							
						so thus, its fine that we literally need to increment M_1. 



							because all existing options, are    0    (ie, at first option!)


								so    0 is valid for all M_1  values,   no matter what the new M_1 is. 




				cool









	



























//	------------------------------
//	        o#    <    >    =
//	------------------------------
/*0*/		0,    1,  [0],  1,
/*1*/		1,   [0],  0,   0,
/*2*/	       [0],  [1], [2], [0],
/*3*/		_,   _, _, _,
/*4*/		_,   _, _, _,
/*5*/		_,   _, _, _,
/*6*/		_,   _, _, _,
//	------------------------------


































graph A  :

//	------------------------------
//	        o#    <    >    =
//	------------------------------
/*0*/		0,    1,  [0],  1,
/*1*/		1,   [2],  0,   0,
/*2*/	       [3],  [1], [2], [0],
/*3*/		_,   _, _, _,
/*4*/		_,   _, _, _,
/*5*/		_,   _, _, _,
/*6*/		_,   _, _, _,
//	------------------------------


-----------------------------------------------------------------------

graph B:

//	------------------------------
//	        o#    <    >    =
//	------------------------------
/*0*/		0,    1,  [0],  1,
/*1*/		1,   [2],  0,   0,
/*2*/	       [3],  [1], [2], _, 
/*3*/		_,   _, _, _,
/*4*/		_,   _, _, _,
/*5*/		_,   _, _, _,
/*6*/		_,   _, _, _,
//	------------------------------









#
# #
#   #
# # # 
  # # #
#   # #
#   # #
-----------------------
  # # # #
#   # #
#   # #
  # # # # #  ##   # #
#   # #
  # # # ##   # #
#   # # # # # 
  # # # ##   # #
#   # #
  # # # ##   # #
#   # #
  # # # #          
[--------EL---------]












two types of partial graphs:


	type 1               

/*0*/		0,    1,  [0],  1,
/*1*/		1,   [2],  0,   0,
/*2*/	       [3],  [1], [2], [0], 







	type 2

/*0*/		0,    1,  [0],  1,
/*1*/		1,   [2],  0,   0,
/*2*/	       [3],  [1], [2],  _, 









currently, the algorithm can only generate type 1 graphs. 


	the algorithm needs to be execution based, 

	and allow for type 2  graphs, i think. 


	those two points are related!




i think the way it will work is to	only push an M_1  variable       to the nfarray 

				when we execute it. 


	i think. 

	

			until then, 

								that var    is       _            ie, unknown. 




			when we psuh it, 


			it no longer ever becomes   unknown, ever. 




		note, 



			we must keep track of the number of      M_2  var's that we add 

	because that tells the modulus   for all of our  M_1 vars.



	
		and when we reach all posibilities for all executed holes      ie          2 2 2 2 2 2 2  


						ie, all existing nfarray   values/variables    are at their modulus,  (it won't be the same for all variables of course! some are of type M_1, some of type M_2) 




				so when we reach the state of all poss being at their max, 
				then

					we NEED to push the M_2 var    onto the back of the  nfarray



							ANDDDDD at that point, is when the modulus of   all M_1 variables, gets incrmented, 
						because its always the number of non-unknown-valued operations in the graph.   (ie, nodes in the graph)




 

						but then, from there,	    we only ever add an   M_1  variable,   if we execute it,

								


		


202306294.173517:

ACTUALLYYYYY

	


	turns out there is a problem with this approach!!!


			this would cause the NF array to change during looking through all poss   fro the current length of the nf array 



			and the way that the nfarray works,   
				we always look over all possibilies, 


							THENNNN only after we done that          do we expandddd the nfarray, 

							ie, push (possibly multiple) new variables onto the end of the array. 


								




	so

				




						yaeh, having the M_1    be pushed    via execution, 


			
							basically means the array is changing   while we are looking at a particular possibility 




						which, 

								note 


								execution   only ever occurs while looking at a paricular poss, 


									and so 



							yeah, thats a completely different situation then what we are wanting to be the caes, 
								for when we add a new variable. 

	


							we want it to be that we finihsed the last poss set    ie, nfarr configuration, 
										ie,   we finished    2 2 2 2 2 2 

											
									

										in order to add a new var, 

									


								notttttt

										we are still working on one of poss, 

										and we found it executed a hole during running the el ins 
								





this problem seems pretty fundedental... idk. 



		but yeah, doesnt seem good for adding in  an execution-based    component to this algorithm,... 

		i think we mightttt have to go without it, if we don't see a solution... 


	


	but even, just having this whole algorithm        as a non-execution based approach, 
	is already good, because we know it gets alll possibilities that are possible to have,
			

					and most importantly        can generate partial type1 graphs!




	
	so yeah thats good 


	kinda







	202306294.180532:

	WaIT

				i found a bug   


				the bug is that 


						you don't want duplicate possibilies,  


						  when you exchange the order of  two given operations!



					thats something we were accounting for, with the previous gp algirothm,  namely, the part where we look over the different operation possibilies,  looking only at the unique combinations,     in the genrate_D_space()  function,



		so yeah 

					that algorithm treats        [5 5 6]       the exact same   as     [5 6 5]


						and   thus ends up deleting  the combination     5 6 5  



							because its superfluous,  and would just add to the number of z values, needlessly 



			we need to make the current nfgp  algorithm we are working on, here,    deal with that uniquifcation too 




					how?




			.. 
						i think to start, we need to pull out all M_2 variables from the nfarray, 



					they should be in their own array, 



			which we change   

							according to          a         NOT nf algorithm 


									ie, 




								a different algordithm, that is able to  


										given a array state,   return the next array state that is one of the unique combinations of values of variables  in the array






				ie, its like a "already pruned"  version of   nf       where the pruned possibilies are things like  



						[5 6 5]




				those possibilies        are not even outputted by this        "combination-nf  algorithm"





						which is useful 






				then, 
					we can just put all the M_2 variables     in their own array, 



					and we can   apply            M2_array = NEXT_combination-nf(M2_array);



						each time we roll over    the M_1 array, 



					and then, when the M2_array rolls over too,        (ie,   when we reach the last combination! given from NEXT_combination-nf()

)
							then, 


								we do the thing where we add    1  M_2 to M2_array, 
								and add 3 M_1   to the M1_array, 



			and note, 



						the ACTUAL nf alrogihtmh 




								ONLY OPERATES        ON         the M1_array. 



										only. 




	



		


so 
	ie, 

				there is no        single monolithic    	nf array


				there is only          two    nf-like  arrays 



						one,  which is actually used by nf,         M1_array

						and one, which is used by the NEXT_combination-nf() function,        M2_array


							and NEXT_combination-nf()    is only called    when nf   reaches the maximum for all variables    in the M1_array.



picture:



		M1_array:                                                    M2_array:
------------------------------------------------------------------------------------------------------------------

		[M_1] [M_1] [M_1] [M_1] [M_1] [M_1] [M_1]                    [M_2] [M_2] [M_2] [M_2] [M_2] 









pretty clear    i think 



				this division will allow us to utilize the fact that     the order that the operations appear in the graph, 


						is not important. 


						what only matters is the actual connections. 





	











								



		



						






		












	 




















































