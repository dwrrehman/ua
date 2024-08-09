// 202406086.154106: dwrr
//   the binary space partitioning  srnfgpr   version of the search utiltiy. 
//    designed to optimally split up the jobs in the search space   amoung a set of threads, 
//    while splitting up jobs the least, and incurring the least amount 
//     of sychronization between threads. 

// old 202403041.192520: by dwrr         
//      the prthead  cpu-parellelized version of the srnfgpr.
//



/*

202407184.193355:

	 we want/required in the su:


	x	- zero synchronization between workker threads

			- - we need to find some alternative to bsp, which uses a lottt of synchronization!




	x	- low memory usage 

			- - no writing millions of z values to a file or to a memory array

		



	x	- no z indicies, or 128 or 64bit math     (64bit is fine for pm counts or something not that important lol)

		 	 - - use nf increments on the MSB bits     to      "modulo partition"   the space    
					and then give each subspace to a worker_thread / core   to process.




	x	- more efficient application of GA across all NF increments, 

			- - make GA and NF completely intermingled and completely merged, so there is no distinctifying them. 




	










generate():

	GA and NF 


   
 |
 |  <------- this pipe to outptu all of the generated z values would result in a large memory consumption. 
 v


prune():

	execute_graph(....);








-----------------printing current job allocations (largest_remaining: 0)-------------------

	1.0000000000%

  * 00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=2, R=0]:
	space_size=118689037748575
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 118689037748575 zvs
	using 6 threads
	in     183.00s [1202407184.190108:1202407184.190411],
	at 648573976768.17 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684831586913		
pm_fea: 0       		pm_ns0: 83983170		
pm_pco: 100828958		pm_zr5: 1317245335		
pm_zr6: 1816477735		pm_ndi: 353148163		
pm_oer: 17280   		pm_r0i: 646753  		
 pm_h0: 1085966 		pm_f1e: 41668   		
pm_erc: 11215047		pm_rmv: 36132   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 1980    		pm_bdl2: 144     		
pm_erw: 0       		pm_mcal: 520119953		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 12      		 pm_h3: 0       		
pm_per: 1312160 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 1206    		
[done_pm]
bsp_su: b release
bsp_su: ./run
SRNFGPR: searching [D=1, R=0] space....













  * 00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=1, R=0]:
	space_size=10883911680
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 10883911680 zvs
	using 6 threads
	in       4.00s [1202407184.190436:1202407184.190440],
	at 2720977920.00 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 10817125368		
pm_fea: 0       		pm_ns0: 1739834 		
pm_pco: 2800900 		pm_zr5: 27093079		
pm_zr6: 17348752		pm_ndi: 7306120 		
pm_oer: 0       		pm_r0i: 7363    		
 pm_h0: 24241   		pm_f1e: 696     		
pm_erc: 413664  		pm_rmv: 20      		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 10023611		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 0       		 pm_h3: 0       		
pm_per: 28030   		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 2       		
[done_pm]
bsp_su: b
c.c:950:12: warning: unused function 'fea_execute_graph' [-Wunused-function]
static nat fea_execute_graph(byte* graph, nat* array) {
           ^
1 warning generat






-----------------printing current job allocations (largest_remaining: 0)-------------------

	1.0000000000%

  * 00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=1, R=1]:
	space_size=10883911680
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 10883911680 zvs
	using 6 threads
	in       8.00s [1202407184.190511:1202407184.190519],
	at 1360488960.00 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 10817125368		
pm_fea: 0       		pm_ns0: 7098282 		
pm_pco: 9566443 		pm_zr5: 6985124 		
pm_zr6: 18479970		pm_ndi: 5149376 		
pm_oer: 0       		pm_r0i: 30606   		
 pm_h0: 31407   		pm_f1e: 2868    		
pm_erc: 1050912 		pm_rmv: 380     		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 10      		
pm_erw: 0       		pm_mcal: 18360851		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 25      		 pm_h3: 0       		
pm_per: 29401   		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 657     		
[done_pm]
bsp_su: b release
bsp_su: ./run
SRNFGPR: searching [D=2, R=1] space....













00000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=2, R=1]:
	space_size=118689037748575
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 118689037748575 zvs
	using 6 threads
	in     308.00s [1202407184.190714:1202407184.191222],
	at 385354018664.20 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684830226071		
pm_fea: 0       		pm_ns0: 382081582		
pm_pco: 341374227		pm_zr5: 524205356		
pm_zr6: 1885368617		pm_ndi: 249703893		
pm_oer: 0       		pm_r0i: 1571759 		
 pm_h0: 906382  		pm_f1e: 196013  		
pm_erc: 29332544		pm_rmv: 40183   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 116     		
pm_erw: 0       		pm_mcal: 791652761		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 1286    		 pm_h3: 0       		
pm_per: 1074691 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 13094   		
[done_pm]
bsp_su:  






















new su structure    nf ga gp    no zi    execute stage  gpu

	--->





			the     no zi      is related to the parrelization over all the cores- 


						if we could figure out some way to distribute zv over the cores, in a way that uses all cores for the full su call,


										then that would be the solution, 







			heres a z value (zv):


					0122105021433161400115264505

					
					0122 1050 2143 3161 4001 1526 450[5]     <---- bracketed digit is the msb digit, which we would use  in   "no zi"'s partition of the space, to 


					in actuality, what we'll do, because some subsearch spaces   will be immediately pruned by ga,


							so we wanto evaluate GA on the msb   (or however many msb's we are using to do the modulo parition (mp))


									we want to run the GA passes for the positions of the msb digits we are using in the mp
		
							

					


202407184.195949:
	fundemental requirements:

		the mmount that execute_graph (exg)  is pruning makes it so that we fundemental cannot simply output z values to a file,

			if they just simply pass ga, 

				we need to be the doing the quite parts of exg    ie, a low el,   to lower    in some way    the amount of zv  that we output to the file/memory array




				


		--> so one possible solution is to basically have the el  be small during generate()    but then    a large el  (which will maybe cause the most nonlinearaity wrt the zi / timetillprune plot)


					the large el causes    some z values to take 30 seconds, with others to take 30 years           we don't like that 

									at all thats bad,  for parel.



						instead, we want to randomize/mp   the zv bag that is ooutput by generate(),     and so prune will take on average,  (because of the randomization/mp)   it will take thte same amount of time per core, and thus require zero synchronization, and use our full parel. factor. 




												this is good 






			but the main problem now, is that   we need to have the   generate() have of the search call     take as little as possible,


						because parallelzing generate()                 MUST involve the bsp machinery,   because things are just highly nonlinear   when it comes to GA pruning 







				note:
						this is a problem becuase one tihng i definitely don't want is for bsp to make global_range_begin/end   a  zv   as opposed to a zi, 

						because then memory atomicity becomes           much more complicated 


							and much more expensive

	

			lukcily  GA     is      at least,   in theory,  fast       so       like,   in theory we could do         stage 1     ie generate()
 						single threaded 

							or, like, just perform a simple        no zi      partitioning of the space    in  main()    


					and then do   generate()  parel   over   all the cores that we have.



			that could work 

				we just, wouldnt be using our full parelelization factor   ie number of cores,   because some would finsh their subsearchspace  early... 


				because GA is just highly nonlinear 







simplest possible approach taht would  ALMOST   just about solve this prolbem    quite well:   (possiblyyyyyy   possibly not lol)



	step 1. make main start up 10 search calls, 

			each saerch call has      range_begin   range_end    

			and,  range_begin   is a zv

			and,   range_end is a zv. 


	step 2. in the generate() worker thread function       "thread_instance_for_generate(rb, re)"


		we do  something like the current nf and ga sitaution   in bsp, 

				but we merge nf and ga completely   so that its wayyy more efficient in how it traverses and checks the space. 


	step 3.      inside of this thread_instance_for_generate function,   we must MUST   run exg  substage    "execute_graph(zv)" (exg).

				we can give a quiteeee low EL   for this, however,   its just simply an explicit trade off of how much memory we want to allocate, 
				to the pipe/buffer    that is allowing the output of stage1, "generate()"   to go into   stage2,  "prune()".

						this buffer transfers the zv list output from generate   into prune. its required. 


					by giving a higher EL  to stage1's exg,    you make the required size of the buffer    smaller. 


							giving a lower   El   to stage1 exg   will cause more output z values    from generate()   to emmited.. 


	note about step 3: 

					we are having an explicit trade off between "nonlinear behavior of zv's causing our pf to effectively decrease"  and memory consumtpion. 

						note:  pf = parelelization factor

							


					ie, if you want lower memory consutpion,    then you need a worse  (on average)   pf  to happen.  during stage 1. 

						and then theres less for stage2,  prune(),    to do, basically. 


					but if you want a better pf,     then you needddd to output more zv, and thus use more memory










		


okay waittt


	this idea mightt solve this and allow us to have only one stage1. heres basically the idea:


			so 

				wait 
						so the reason why we have eveng having tow stages to begin with, is because we want some kind of guarantee that    MOST of the searchcall's time will spent  USING all the cores.    we want a guareantee that this is the case.  to maximize out searching hardware efficiency. (effective pf (epf))


					to get this guarantee         WITOUTH   anyyyyyy sychronizatoin of any kind,    we need to somehow give        similar typesss of jobs   to each  core, 


							ie,  each   bag of work    we give to each core, should    in theory   be     similarly sized,  and take a similar amount of time to complete, 


							thast the only way-     i don't think theres another way to get   this guarentee about our epf   without doign any synchr. 



					


		okay,s o we need to give a similar bag of work   to each core!    how do we do that??


			IFFFF the total search space     has the property that zv close to each other IN ZI space   will    

								on average   
											be similar  in their  "time-till-prune" value    ish             mostly.  ish. ... kinda.. 




			thennnnn



					welll a modulo partition    is the way to        split up the search space's work   into    equally sized parts  each of which       take the same amount of time to complete for any given core. (which im just going to call, the equialization requirement of our job partitioning (or, eq req for short))



							
						
						buttt! we can go even further tahn this, to try to guarantee    the  eq req           and thats  to actually give a sequecne of jobs to each core, 

					which are spread out over the entire search space. 



							
					




		basically,  

					mp to get good pf during stage1:

							ss = pf * njobspercore * sizeofeachjob



202407184.203059:
lets say this was our search space:



		|		|		|		|		|		|		|
		[========================================================================================================]

		^													 ^
		zi=0													zi = 118 trillion ish













note:




	it would be really nice if we could somehow statically partition this space (in main(), before we spin up any worker thread) 

				in a way      that gives us    ROUGHLY SPEAKING   the    eq  req. 


					


	note:

			RUNNING GRAPHS    FOR A LONG EL during generate()              IS        HORRIBLE   for our chances of maintaining the eq req during generate(). 



							we want to minimize the amount of nonlinaeratiy taht could happen during the call. 



										so that any particular z value really doesnt take drastically longer than any other one. 


											thast the goal. 





						


				A               B
		|		|		|		|		|		|		|        |
		[========================================================================================================]

		^													 ^
		zi=0													zi = 118 trillion ish


							ie, i divided the full search space into 7 parts. 







so, 

		now, lets zoom into one section, say the section  between A and B.           ie one of the 7 parts:








				A	|	 |       |       |       |       |       |  B
				|===========================================================|

				^~~~~~~~ ^~~~~~~ ^~~~~~~ ^~~~~~~ ^~~~~~~ ^~~~~~~ ^~~~~~~ ^~~~
				core 0    core 1  core 2  core 3   ...                    core 9


						here, we actually want to divide this sub portion of the search space     into      CORE_COUNT   or thread_count      number of picees. 




						we then want to name each piece                [subrange_begin::subrange_end]       and then push this    to  the element #CORE_INDEX in the array   core_jobs



				ie,      core_jobs[CORE_INDEX].jobs[core_jobs[CORE_INDEX].job_count++] = (struct job) { .subrange_begin = ..zv.., subrange_end = ..zv.. };


						so this push  idiom   instance          is basically just giving mulitple jobs to a core        but where the distribution of these  jobs is across the entire search space, 


				



		so like, the portion between A and B    contains     10   (if we have 10 cores lets say)        jobs   each of which is given toa different core. 


				



				each core will end up with 7 jobs!         but in diffferent parts of the search space!




	and hopefullyyyy




	because each part is like... similar to the parts arround it, 


			we will like   on average  maintain a good          epf    the for the whole call,   without doing any synchronization!



						



			


						
		
so basically, this is getting to the actual equaation i gave earlier, 


	which is of course:



		
							ss = pf * njobspercore * sizeofeachjob





			the search_space_size =   core_count    *    number_of_jobs_per_core    *   number_of_zv_per_job;



			



	bsp    is essentially the algohithm that maximizes        TO THE MAX    the  epf     



			i think we just need to comprimise on this- 

			using ALL the cores  ALLLLLL the time    for the full search call 		is maybe good, but not required, 



				if comprimising on this causes us to not have any synchreonoization requirements,   AND allows us to use MORE than just the cpu cores,  than it might be WAYYYYY  WAYYY worth it   to comprimise on  maximing epf. 



						core count is kind of more important technically, assuming we have more cores we can run this on,


								which like,  the gpu exists-     and is quite useful for this-   as is  any other machine we want to runthis algorithm on lol 



													and not requiring sych of any kind  is HUGEEE for allowing all of those cores to be used  

																for a single su call. 







								


so wait 


	so thats the other part of this revision to the search utility:



				we want to actually somehow divde the search space into     10   or 70 parts   like i described above, using   the whole    pictures and   A-B thingy 



					like using a modulo partition   and a number of jobs to allocate per core ,


						we need to use          the NF!!!!      to do this partitioning. 



								which             is the hard part. 





		so note, 

						we need to compute the  zv   that corresponds to point A         thats the problem that is maybe a little bit nonobvious how oyu would do that 


					because lkie, without using      any z indicies  its quite difficult lol 


			buttt


						if we were to use z indicies JUST   for determining the     subrange_begin_zv = ....     and subrange_end_zv = ...



								and then use z indicies for no other part of the program!


								thennnn that would be fine, becuase the job allocation stage (stage0)   happening single threaded   at the start of main, 


									is not time sensitive, we can take however long we want to determine that, and theres no problems with using zi there, 




				so yeah, we'll probably use like      256 or 512 bit    zi indicies     to numerically represent what zv      we are loooking at,

					becuase of course zi's   you can do math on them, which is super useful if you are trying to arithemetically partition the space into a bunch of equal sized chunks,


												nice!!!



						cool 




	





	quick note:

		params to this revision of the su:

			param0:  cc

				core count


			param1:  jcpc

				job count per core


				














202407184.211422:


so wait 



		is this a two stage algorithm anymore???



						should make the exg   stage          into its own seperate stage,    ie


	







	okay actually wait 


			i think it actually makes sense      to NOT      make this a two stage algortihm,  and to actually just tick down our  el a tonnn becuase basically, the root of this, is that     liek, 

					ideally, GA would prunue everything - or close to           and  like we'll get there eventually, but for now,  it makes sense to give a low EL 

							and then have a ton of z values  (not too many, but more than 0 ...)



									output      to   a file    or multiple (for each core of course)



									BUTTTT    we can run     the iterative pruning utility (ip)      on that list!


								and that ISSSSS OUR STAGE 2 


							ANDDDD  we can GUARANTEEE  out epf  in that sort of situation!   theres no mystery about the distribution of z values, kinad of, 


										our epf is much more determined, kinda    than the epf  during generate()




														or should i say generate_prune()   because stage1 does actually run exg

													its just, not for a very long el  to maximise our epf




							

					gp (generate_prune()) is like the hard   situation    to maximize your epf  in      


						but doing so   in ip     isnt taht bad       quite easy actually 



					and so basically the limiting factor, really is just the pipeline/buffer    that connects the su  and ip  


							we don't want to use a tonnn of memory or filessystem space      to store all the zv that are in  the pipeline/buffer   after we finish stage1


							so yeah, 


								we need  a   decent ishhhh      el      in      the su    gp     exg 




							but it can't be crazy,    lest we risk loosing   epf a little bit    or alot,  depending on the effect that increasing our el will have on the call. 





			
						
	



		


note:


		i thik it will actually make sense to     do a  seperate little       prune()  stage2    after   gp     (which is stage1) 


				basically, so tha twe don't have to offset all of this work to   the ip utiltiy, directly,     where the pipeline taht we will be using   to transfer this zv data   is of course, files, lol. 



					so like, while its still in memory,   it makes sense to try to run graphs for a billion timesteps or so,       BECAUSEEEE

								we can still maintain a good epf     even if we do that,    BECAUSE we have an explicit list of z values. 


									and we definitely didnt before,  in stage1.    so yeah, we can run a large EL  now,   in stage2. 

							with    very little risk at loosing  our epf   or making   lower






				thennnn the zv   taht are output    from  prune()      WILLLL actually be writte to a file. 





















things todo:

	- max pointer     <-------    relevant  for when we implement stage 2. 

	- lower el

	- 




x ripping out fea ex    start call 

x go over search calls










202407011.083111:


	the 3,0 call we started in the video:



...

using [D=3, R=0]:
        space_size=2251799813685248000
        thread_count=64
        cache_line_size=8
        minimum_split_size=6
        display_rate=0
        fea_execution_limit=5000
        execution_limit=10000000000
        array_size=100000


        searched 2251799813685248000 zvs
        using 64 threads
        in    7295.00s [1202406307.214536:1202406307.234711],
        at 308677150607984.62 z/s.


pm counts:
z_is_good: 0                     pm_ga: 2251799624808126246
pm_fea: 0                       pm_ns0: 223012
pm_pco: 143969                  pm_zr5: 14074631
pm_zr6: 14376165                pm_ndi: 768996
pm_oer: 481071                  pm_r0i: 7108731
 pm_h0: 26759558                pm_f1e: 20945
pm_erc: 191110021               pm_rmv: 928853
 pm_ot: 0                       pm_csm: 0
 pm_mm: 0                       pm_snm: 0
pm_bdl: 0                       pm_bdl2: 0
pm_erw: 0                       pm_mcal: 4431836
pm_snl: 0                        pm_h1: 0
 pm_h2: 56                       pm_h3: 32
pm_per: 33519                   pm_snco: 0
pmf_fea: 23808093               pmf_ns0: 2637069555
pmf_pco: 2861076452             pmf_zr5: 45932147442
pmf_zr6: 105849276783           pmf_ndi: 12858316008
pmf_per: 47465655               pmf_mcal: 18407494899
pmf_snco: 0                     pm_r1i: 5472
[done_pm]
bsp_su: 












202407011.083111:

when we had run 3,1 the first time, call number 6:


....
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=3, R=1]:
        space_size=2251799813685248000
        thread_count=64
        minimum_split_size=6
        range_update_frequency=0
        display_rate=5
        fea_execution_limit=5000
        execution_limit=10000000000
        array_size=100000


        searched 2251799813685248000 zvs
        using 64 threads
        in  165857.00s [1202406226.175653:1202406241.160110],
        at 13576754756719.63 z/s.


pm counts:
z_is_good: 0                     pm_ga: 2251798731928012790
pm_fea: 0                       pm_ns0: 16122643
pm_pco: 9618402                 pm_zr5: 2095135937
pm_zr6: 3194124287              pm_ndi: 471017394
pm_oer: 0                       pm_r0i: 216772087
 pm_h0: 96179642                pm_f1e: 22798540
pm_erc: 3084645930              pm_rmv: 43274999
 pm_ot: 0                       pm_csm: 0       
 pm_mm: 1760                    pm_snm: 32      
pm_bdl: 0                       pm_bdl2: 0       
pm_erw: 0                       pm_mcal: 78770346
pm_snl: 58833603278              pm_h1: 0       
 pm_h2: 91828                    pm_h3: 0       
pm_per: 31754609                pmf_fea: 154657405
pmf_ns0: 63816927559            pmf_pco: 55718697783
pmf_zr5: 75134928080            pmf_zr6: 584800879956   
pmf_ndi: 37422343619            pmf_per: 409296364
pmf_mcal: 196105592730  
[done]
6_D3_R1_call: 










and then heres also the 3,0 output the first time we searched 3,0:





    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=3, R=0]:
        space_size=2251799813685248000
        thread_count=64
        minimum_split_size=6
        range_update_frequency=0
        display_rate=3
        fea_execution_limit=5000
        execution_limit=10000000000
        array_size=100000


        searched 2251799813685248000 zvs
        using 64 threads
        in  117571.00s [1202406145.012212:1202406156.100143],
        at 19152680624348.25 z/s.


pm counts:
z_is_good: 240                   pm_ga: 2251798695353080354
pm_fea: 0                       pm_ns0: 10171957
pm_pco: 14235301                pm_zr5: 988729591
pm_zr6: 1906588863              pm_ndi: 64010438
pm_oer: 706734                  pm_r0i: 31549582
 pm_h0: 76781722                pm_f1e: 541489  
pm_erc: 4133477717              pm_rmv: 44540790
 pm_ot: 0                       pm_csm: 0       
 pm_mm: 172                     pm_snm: 162     
pm_bdl: 19536                   pm_bdl2: 0       
pm_erw: 0                       pm_mcal: 118230867
pm_snl: 25203840280              pm_h1: 0       
 pm_h2: 28352                    pm_h3: 372     
pm_per: 7040647                 pmf_fea: 100965901
pmf_ns0: 12778474385            pmf_pco: 15412494376
pmf_zr5: 291036707345           pmf_zr6: 607513521202
pmf_ndi: 64284941527            pmf_per: 266841885
pmf_mcal: 94337726213
[done]






117571 / 7295 ---> 16 times speed up for 3,0














notes


uaj iter add    profile bsp




0 ohm jumper  in pencil for 2d wiring
wire mount for pencil pcb




   loop unrolling is just cte


x    remove printing from output file








	*0    *1    *2    *3    *4    *5   ... 
	
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]  
	[ ]   [x]   [x]   [ ]   [1]   [ ]                 ...   ER(3;5)
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]     <----
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]  
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]  
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]  
	      .....
	[ ]   [x]   [ ]   [ ]   [ ]   [ ]  




one:
	if (pointer == 1) {

		if ((last_mcal_op == three) {

			counter++;
			if (counter >= stuff) return pm;
		}

		else last_mcal_op == one) counter = 0;

	}

















	
a
	op = 3
	last_mcal_op = 1
	last_last_mcal_op = 5


	op = 1
	last_mcal_op = 1
	last_last_mcal_op = 5
	

	

































using [D=2, R=0]:
	space_size=118689037748575
	thread_count=10
	cache_line_size=100           <------------------- here!!
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 118689037748575 zvs
	using 10 threads
	in     192.00s [1202407022.222308:1202407022.222620],
	at 618172071607.16 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684826926031		
pm_fea: 0       		pm_ns0: 3576    		
pm_pco: 4194    		pm_zr5: 451374  		
pm_zr6: 366533  		pm_ndi: 15604   		
pm_oer: 17280   		pm_r0i: 251054  		
 pm_h0: 855817  		pm_f1e: 1301    		
pm_erc: 11215047		pm_rmv: 5795    		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 129164  		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 0       		 pm_h3: 0       		
pm_per: 3723    		pm_snco: 0       		
pmf_fea: 582491  		pmf_ns0: 84187522		
pmf_pco: 101168869		pmf_zr5: 1318887193		
pmf_zr6: 1816994126		pmf_ndi: 353989121		
pmf_per: 1180252 		pmf_mcal: 520512346		
pmf_snco: 0       		pm_r1i: 162     		
[done_pm]
bsp_su: 













using [D=2, R=0]:
	space_size=118689037748575
	thread_count=10
	cache_line_size=1             <----------------------- and here!!
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 118689037748575 zvs
	using 10 threads
	in     213.00s [1202407022.222656:1202407022.223029],
	at 557225529336.03 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684828703799		
pm_fea: 0       		pm_ns0: 3576    		
pm_pco: 4194    		pm_zr5: 451374  		
pm_zr6: 367589  		pm_ndi: 15604   		
pm_oer: 17280   		pm_r0i: 251054  		
 pm_h0: 855886  		pm_f1e: 1301    		
pm_erc: 11215047		pm_rmv: 5795    		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 129164  		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 0       		 pm_h3: 0       		
pm_per: 3723    		pm_snco: 0       		
pmf_fea: 582502  		pmf_ns0: 84094233		
pmf_pco: 101047911		pmf_zr5: 1317528532		
pmf_zr6: 1817565481		pmf_ndi: 353475392		
pmf_per: 1180242 		pmf_mcal: 520248734		
pmf_snco: 0       		pm_r1i: 162     		
[done_pm]
bsp_su: 






and the other half:


using [D=2, R=1]:
	space_size=118689037748575
	thread_count=10
	cache_line_size=100        <----------
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 118689037748575 zvs
	using 10 threads
	in     357.00s [1202407022.223155:1202407022.223752],
	at 332462290612.25 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684830666219		
pm_fea: 0       		pm_ns0: 3562    		
pm_pco: 109     		pm_zr5: 2606250 		
pm_zr6: 190868  		pm_ndi: 41398   		
pm_oer: 0       		pm_r0i: 355498  		
 pm_h0: 514646  		pm_f1e: 8466    		
pm_erc: 29332544		pm_rmv: 10021   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 14912   		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 288     		 pm_h3: 0       		
pm_per: 4525    		pm_snco: 0       		
pmf_fea: 1437745 		pmf_ns0: 382194912		
pmf_pco: 341146696		pmf_zr5: 521612409		
pmf_zr6: 1885026337		pmf_ndi: 250073141		
pmf_per: 889176  		pmf_mcal: 791617297		
pmf_snco: 0       		pm_r1i: 1556    		
[done_pm]
bsp_su: o















-----------------printing current job allocations (largest_remaining: 0)-------------------

	1.0000000000%

  * 00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 
    00000000000000000000 : 00000000000000000000 :: 

using [D=0, R=0]:
	space_size=1953125
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 1953125 zvs
	using 6 threads
	in       1.00s [1202407184.193547:1202407184.193548],
	at 1953125.00 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 1912949 		
pm_fea: 0       		pm_ns0: 744     		
pm_pco: 1680    		pm_zr5: 18138   		
pm_zr6: 11904   		pm_ndi: 2356    		
pm_oer: 0       		pm_r0i: 0       		
 pm_h0: 0       		pm_f1e: 0       		
pm_erc: 0       		pm_rmv: 0       		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 5354    		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 0       		 pm_h3: 0       		
pm_per: 0       		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 0       		
[done_pm]
bsp_su: sub c.c
bsp_su: b release
bsp_su: ./run







using [D=0, R=1]:
	space_size=1953125
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000000
	array_size=100000


	searched 1953125 zvs
	using 6 threads
	in       1.00s [1202407184.193609:1202407184.193610],
	at 1953125.00 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 1912949 		
pm_fea: 0       		pm_ns0: 3720    		
pm_pco: 6080    		pm_zr5: 399     		
pm_zr6: 13392   		pm_ndi: 1494    		
pm_oer: 0       		pm_r0i: 32      		
 pm_h0: 37      		pm_f1e: 0       		
pm_erc: 0       		pm_rmv: 0       		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 0       		pm_bdl2: 0       		
pm_erw: 0       		pm_mcal: 14984   		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 0       		 pm_h3: 0       		
pm_per: 38      		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 0       		
[done_pm]
bsp_su: 




*/

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 2;        // the duplication count (operation_count = 5 + D)
static const byte R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

// static const nat range_update_frequency = 0;

static const nat minimum_split_size = 6;

static const nat thread_count = 6;
static const nat display_rate = 0;

static const nat cache_line_size = 100;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good, pm_ga, 
	pm_fea, pm_ns0, 

	pm_pco, pm_zr5, 
	pm_zr6, pm_ndi, 

	pm_oer, pm_r0i, 
	pm_h0, pm_f1e, 

	pm_erc, pm_rmv, 
	pm_ot, pm_csm, 

	pm_mm, pm_snm, 
	pm_bdl, pm_bdl2, 

	pm_erw, pm_mcal, 
	pm_snl, pm_h1, 

	pm_h2, pm_h3, 
	pm_per, pm_snco,

	pmf_fea, pmf_ns0, 
	pmf_pco, pmf_zr5, 

	pmf_zr6, pmf_ndi, 
	pmf_per, pmf_mcal, 

	pmf_snco, pm_r1i,

	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_ga", 
	"pm_fea", "pm_ns0", 

	"pm_pco", "pm_zr5", 
	"pm_zr6", "pm_ndi", 

	"pm_oer", "pm_r0i", 
	"pm_h0", "pm_f1e", 

	"pm_erc", "pm_rmv", 
	"pm_ot", "pm_csm", 

	"pm_mm", "pm_snm", 
	"pm_bdl", "pm_bdl2", 

	"pm_erw", "pm_mcal", 
	"pm_snl", "pm_h1", 

	"pm_h2", "pm_h3", 
	"pm_per", "pm_snco",

	"pmf_fea", "pmf_ns0", 
	"pmf_pco", "pmf_zr5", 

	"pmf_zr6", "pmf_ndi", 
	"pmf_per", "pmf_mcal", 

	"pmf_snco", "pm_r1i",
};


static const byte _ = 0;

static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

#define _63R_hole_count 9
static const byte _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};

static const byte _36R[5 * 4] = {
	0,  1, 2, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  _, 4, _,      //  9    11
	3,  _, _, _,      // 13 14 15
	4,  0, 0, _,      //       19
};

#define _36R_hole_count 9
static const byte _36R_hole_positions[_36R_hole_count] = {3, 6, 7, 9, 11, 13, 14, 15, 19};

static const byte initial = R ? _63R_hole_count : _36R_hole_count;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const byte hole_count = initial + 4 * D;

static const nat fea_execution_limit = 5000;
static const nat execution_limit = 10000000;
static const nat array_size = 100000;

static const byte max_er_repetions = 50;
static const byte max_erw_count = 100;
static const byte max_modnat_repetions = 15;
static const byte max_consecutive_s0_incr = 30;
static const byte max_consecutive_s1_incr = 30;
static const byte max_consecutive_small_modnats = 200;
static const byte max_bdl_er_repetions = 25;
static const byte max_sn_loop_iterations = 100 * 2;
static const byte max_consecutive_h0_bouts = 10;
static const byte max_consecutive_h2_bouts = 30;
static const byte max_consecutive_h3_bouts = 30;

// static const byte max_consecutive_h1_bouts = 30;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;

// runtime constants:
static nat space_size = 0;
static byte* positions = NULL; 
static pthread_t* threads = NULL; 

// runtime variables:
static _Atomic nat* global_range_begin = NULL;
static _Atomic nat* global_range_end = NULL;
static pthread_mutex_t mutex;


static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

static void get_graphs_z_value(char string[64], byte* graph) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, nat* max_pointer) {

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	e = 0,  xw = 0,  pointer = 0,  
		er_count = 0, 
		mcal_index = 0, 
		bout_length = 0, 
		walk_ia_counter = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		BDL_er_at = 0,
		BDL2_er_at = 0;

	byte 	mcal_path = 0, R1I_counter = 0,
		ERW_counter = 0, SNL_counter = 0,  OER_counter = 0,  BDL_counter = 0, 
		BDL2_counter = 0,  R0I_counter = 0, 
		H0_counter = 0, H2_counter = 0, H3_counter = 0, 
		RMV_counter = 0, CSM_counter = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;



/*	
	char string[64] = {0};
	get_graphs_z_value(string, graph);



	if (not strcmp(string, "0122105025433156400510102455")) {
		puts("found the errant z value!");
		abort();
	}

*/

	

	for (; e < execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return pm_f1e;
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return pm_erc;
		}
		
		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return pm_fea;
			if (not array[pointer]) return pm_ns0; 

			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;
			
			if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s1_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			bout_length++;
			pointer++;

			if (pointer > *max_pointer) *max_pointer = pointer;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_er_repetions) return pm_oer;

			if (BDL_er_at and pointer == BDL_er_at - 1) { BDL_counter++; BDL_er_at--; }
			else { BDL_er_at = pointer; BDL_counter = 0; }
			if (BDL_counter >= max_bdl_er_repetions)  return pm_bdl;

			if (BDL2_er_at > 1 and pointer == BDL2_er_at - 2) { BDL2_counter++; BDL2_er_at -= 2; }
			else { BDL2_er_at = pointer; BDL2_counter = 0; }
			if (BDL2_counter >= max_bdl_er_repetions) return pm_bdl2;

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv;
			}

			if (walk_ia_counter == 1) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {
			// if (array[n] >= 65535) { *out_e = e; return pm_snm; }

			if (last_op == six) SNL_counter++; 
			else if (last_op != two) SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;
			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			if (last_op == two) SNL_counter++; 
			else SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (last_mcal_op == five) {
				R0I_counter++;
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (bout_length == 3) {
				H3_counter++;
				if (H3_counter >= max_consecutive_h3_bouts) return pm_h3; 
			} else H3_counter = 0;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

		//	if (pointer == 1) {
		//		incr++;
		//	}

			bout_length = 0;
			walk_ia_counter++;

			// if (array[pointer] >= 65535) { *out_e = e; return pm_mm; }

			array[pointer]++;
		}

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	// *out_e = e; 


	return z_is_good;
}

static nat execute_graph(byte* graph, nat* array, byte* origin, nat* max_pointer) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		//nat e = 0;
		pm = execute_graph_starting_at(o, graph, array, max_pointer);
		//if (e > max_e[pm]) max_e[pm] = e;
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return pm;
}

static nat fea_execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 
	byte ip = origin, last_mcal_op = 255, mcal_path = 0, last_op = 255;
	nat pointer = 0, e = 0, xw = 0, mcal_index = 0;
	nat did_ier_at = (nat)~0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return pmf_fea;
			if (not array[pointer]) return pmf_ns0;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pmf_pco;
			if (not pointer) return pmf_zr5;

			did_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return pmf_zr6;

			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pmf_snco;

			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return pmf_ndi;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pmf_per;
				did_ier_at = (nat) ~0;
			}

			array[pointer]++;
		}

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pmf_mcal; 
		if (mcal_index == 2  and last_mcal_op != one) 	return pmf_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pmf_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pmf_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pmf_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pmf_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pmf_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pmf_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pmf_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pmf_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pmf_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pmf_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good; 
}

static nat fea_execute_graph(byte* graph, nat* array) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = fea_execute_graph_starting_at(o, graph, array);
		if (not pm) return z_is_good;
	}
	return pm;
}

static void append_to_file(char* filename, size_t size, byte* graph, byte origin) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, graph); 
	char o[16] = {0};    snprintf(o, sizeof o, "%hhu", origin);

	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("[%s]: [z=%s]: failed to create filename = \"%s\"\n", dt, z, filename);
			fflush(stdout);
			abort();
		}
		snprintf(filename, size, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, z, strlen(z));
	write(file, " ", 1);
	write(file, o, strlen(o));
	write(file, " ", 1);
	write(file, dt, strlen(dt));
	write(file, "\n", 1);
	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",
		dt, permissions ? "created" : "wrote", z, filename
	);
}

static void* worker_thread(void* raw_argument) {


	/********************************************



	note:

		202408084.172117: i found out that there is a bug with this code, somewhere.
				not sure where, 

				but we know that this  bsp su is not processing the following z value,



  0  1  2  2  				this z value should have been output    but  was not (it was usin the no-synch utility though! nice)

  1  0  5  0

  2  1  4  3

  3  1  1  6

  4  0  0  1


  1  1  1  1

  2  4  1  0


			which i have checked and   found it shouldnt be pruned by GA, or execute(), 

			
				and thus we should have output it   but        bsp did not output it, 

				so we are skipping over this z value,   ie, theres a bug with how we are partitioning the search space up 

				amoungst all the cores, via the bsp job-split machinery, and the sychr and other stuff like that. 


			so yeah. keep this bug in mind, if you want to try to use this thing again ...  maybe solve this bug first lol 







	*********************************/




	char filename[4096] = {0};
	nat* pms = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);
	byte* graph = raw_graph;
	byte* end = raw_end;
	nat* graph_64 = raw_graph;
	nat* end_64 = raw_end;
	memcpy(graph, R ? _63R : _36R, 20);
	memcpy(end, R ? _63R : _36R, 20);
	byte pointer = 0;
	// nat publish_counter = 0;

	nat max_pointer = 0;

	const nat thread_index = *(nat*) raw_argument;
	free(raw_argument);

	nat range_begin = atomic_load_explicit(global_range_begin + cache_line_size * thread_index, memory_order_relaxed);
	nat range_end = atomic_load_explicit(global_range_end + thread_index, memory_order_relaxed);

	//printf("worker_thread[%llu]: starting with job:  [range_begin=%llu, range_end=%llu]\n", thread_index, range_begin, range_end);
	
next_job:;
	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	goto init;

loop:	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
		if (graph_64[i] < end_64[i]) goto process;
		if (graph_64[i] > end_64[i]) break;
	}

done:
	//printf("worker_thread[%llu]: finished with job! waiting for mutex...[%llu : %llu]\n", thread_index, range_begin, range_end);
	pthread_mutex_lock(&mutex);

	nat largest_remaining = 0, chosen_thread = 0;

	for (nat thread = 0; thread < thread_count; thread++) {
		const nat b = atomic_load_explicit(global_range_begin + cache_line_size * thread, memory_order_relaxed);
		const nat e = atomic_load_explicit(global_range_end + thread, memory_order_relaxed);
		const nat size =  e >= b ? e - b : 0;
		if (size > largest_remaining) { largest_remaining = size; chosen_thread = thread; }
	}

	if (largest_remaining < minimum_split_size) {
		printf("worker_thread[%llu]: largest remaining (%llu) was less than minimum split size of %llu\n", thread_index, largest_remaining, minimum_split_size);
		pthread_mutex_unlock(&mutex);
		goto terminate_thread;
	}

	//printf("worker_thread[%llu]: performing a split: using thread #%llu, which had a largest remaining of %llu\n", thread_index, chosen_thread, largest_remaining);
	
	const nat subtract_off_amount = (largest_remaining >> 1);
	const nat job_ends_at = atomic_fetch_sub_explicit(global_range_end + chosen_thread, subtract_off_amount + 1, memory_order_relaxed);

	range_begin = job_ends_at - subtract_off_amount;
	range_end = job_ends_at;

	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, range_begin, memory_order_relaxed);
	atomic_store_explicit(global_range_end + thread_index, range_end, memory_order_relaxed);

	pthread_mutex_unlock(&mutex);

	//printf("worker_thread[%llu]: performed a split, now processing this job:  [range_begin=%llu, range_end=%llu]\n", thread_index, range_begin, range_end);

	goto next_job;

process:
	if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
	if (pointer < hole_count - 1) goto reset_;
	goto done;

increment:
	graph[positions[pointer]]++;
init:  	pointer = 0;

	//if (publish_counter >= range_update_frequency) {
		
	const nat local_range_end = atomic_load_explicit(global_range_end + thread_index, memory_order_relaxed);
	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((local_range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, zindex, memory_order_relaxed);

		
	// printf("worker_thread[%llu]: updating begin and end,   pulled range_end=%llu, 
	// published range_begin=%llu]\n", thread_index, local_range_end, zindex);
	//	publish_counter = 0;
	//} else publish_counter++;


	u16 was_utilized = 0;
	byte at = 0;


	for (byte index = 20; index < graph_count - 4; index += 4) {
		if (graph[index] > graph[index + 4]) { at = index + 4; goto bad; } 
	}

	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  
			at = 4 * index + 3; goto bad; 
		}
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == six   and graph[4 * index + 2]) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  
			at = 4 * index + 2 * (index == two); goto bad; 
		}
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  
			at = 4 * index + 1 * (index == three); goto bad; 
		}
		
	//	if (graph[4 * index] == six and graph[4 * l] == two and graph[4 * l + 1] == index) { 
	//		at = 4 * l; goto bad; 
	//	}    // redundant, to the 2->6 GA check that we just added just now. 202406307.182147

		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == one) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == five) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad; 
		}
 
		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == one and graph[4 * e] == one) {
			if (index == one) { at = 4 * index + 3; goto bad; }
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad;
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == one) {
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == three) {
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == three) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == two)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) {
					if (index == two) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { at = 4 * index; goto bad; }
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}
	
		if (graph[4 * index] == one)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == one) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { at = 4 * index; goto bad; }
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}
		
		if (graph[4 * index] == five) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == five) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == six) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) { 
					if (index == six) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == three) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == three) { 
					if (index == three) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == three) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		
		if (l != index) was_utilized |= 1 << l;
		if (g != index) was_utilized |= 1 << g;
		if (e != index) was_utilized |= 1 << e;
	}

	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto loop;
	goto try_executing;
bad:
	for (byte i = 0; i < hole_count; i++) {
		if (positions[i] == at) { pointer = i; goto loop; } else graph[positions[i]] = 0;
	}
	abort();
	
try_executing:;
	//nat pm = fea_execute_graph(graph, array);
	//if (pm) { pms[pm]++; goto loop; } 
	byte origin;
	nat pm = execute_graph(graph, array, &origin, &max_pointer);
	if (pm) { pms[pm]++; goto loop; } 

	pms[z_is_good]++; 
	append_to_file(filename, sizeof filename, graph, origin);

	printf("\n    FOUND:  z = "); 
	print_graph_raw(graph); 
	printf("\n"); 
	fflush(stdout);
        goto loop;

reset_:
	graph[positions[pointer]] = 0; 
	pointer++;
	goto loop;

terminate_thread:
	pthread_mutex_lock(&mutex);
	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, 0, memory_order_relaxed);
	atomic_store_explicit(global_range_end   + thread_index, 0, memory_order_relaxed);
	pthread_mutex_unlock(&mutex);

	printf("worker_thread[%llu]: terminating thread!  (thread with filename=\"%s\")\n", thread_index, filename);
	printf("worker_thread[%llu]: max_pointer = %llu\n", thread_index, max_pointer);
	free(raw_graph);
	free(raw_end);
	free(array);
	return pms;
}


static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

static void print(char* filename, size_t size, const char* string) {
	char dt[32] = {0};   get_datetime(dt);

	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("print: [%s]: failed to create filename = \"%s\"\n", dt, filename);
			fflush(stdout);
			abort();
		}
		snprintf(filename, size, "%s_D%hhuR%hhu_%08x%08x%08x%08x_output.txt", dt, D, R,
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, string, strlen(string));
	close(file);
	printf("%s", string);
	fflush(stdout);
}

int main(void) {
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};
	
	srand((unsigned)time(0)); rand();
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	positions = calloc(hole_count, 1);
	threads = calloc(thread_count, sizeof(pthread_t));
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	snprintf(output_string, 4096, "SRNFGPR: searching [D=%hhu, R=%hhu] space....\n", D, R);
	print(output_filename, 4096, output_string);

	global_range_begin = calloc(1, cache_line_size * thread_count * sizeof(_Atomic nat));
	global_range_end = calloc(thread_count, sizeof(_Atomic nat));

	pthread_mutex_init(&mutex, NULL);

	const nat width = space_size / thread_count;
	nat begin = 0;
	for (nat i = 0; i < thread_count; i++) {
		atomic_init(global_range_begin + cache_line_size * i, begin);
		atomic_init(global_range_end + i, i < thread_count - 1 ? begin + width - 1 : space_size - 1);
		begin += width;
	}

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		nat* thread_index = malloc(sizeof(nat));
		*thread_index = i;
		pthread_create(threads + i, NULL, worker_thread, thread_index);
	}

	nat* local_begin = calloc(thread_count, sizeof(nat));
	nat* local_end = calloc(thread_count, sizeof(nat));

	nat resolution = 360;

	while (1) {

		byte printed_one_char = 0;

		for (nat i = 0; i < thread_count; i++) {
			local_begin[i] = atomic_load_explicit(global_range_begin + cache_line_size * i, memory_order_relaxed);
			local_end[i] = atomic_load_explicit(global_range_end + i, memory_order_relaxed);
		}
		
		nat largest_remaining = 0, chosen_thread = 0;

		for (nat i = 0; i < thread_count; i++) {
			if (local_begin[i] > local_end[i]) local_begin[i] = local_end[i];
			const nat size = local_end[i] - local_begin[i];
			if (size > largest_remaining) { largest_remaining = size; chosen_thread = i; }
		}

		snprintf(output_string, 4096, "\033[H\033[2J");
		printf("%s", output_string);

		snprintf(output_string, 4096, "\n-----------------printing current job allocations (largest_remaining: %llu)-------------------\n", largest_remaining);
		printf("%s", output_string);

		nat sum = 0;

		for (nat i = 0; i < thread_count; i++) {
			const nat size = local_end[i] - local_begin[i];
			sum += size;
		}

		snprintf(output_string, 4096, "\n\t%1.10lf%%\n\n", (double) (space_size - sum) / (double) space_size);
		print(output_filename, 4096, output_string);

		for (nat i = 0; i < thread_count; i++) { 

			snprintf(output_string, 4096, "  %c %020llu : %020llu :: ", i == chosen_thread ? '*' : ' ', local_begin[i], local_end[i]);
			printf("%s", output_string);

			const nat diff = local_end[i] - local_begin[i];
			const nat zs_per_char = space_size / resolution;
			const nat amount = diff / zs_per_char;

			for (nat j = 0; j < amount; j++) {
				snprintf(output_string, 4096, "#");
				printf("%s", output_string);
				printed_one_char = 1;
			}
			snprintf(output_string, 4096, "\n");
			printf("%s", output_string);
		}

		if (not printed_one_char) resolution *= 512;
		if (resolution > space_size) resolution = space_size;
		
		snprintf(output_string, 4096, "\n");
		printf("%s", output_string);

		if (not largest_remaining) break;
		sleep(1 << display_rate);
	}

	nat counts[pm_count] = {0};
	//nat max_e[pm_count] = {0};

	for (nat i = 0; i < thread_count; i++) {

		nat* local_counts = NULL;
		pthread_join(threads[i], (void**) &local_counts);

		for (nat j = 0; j < pm_count; j++) {
			counts[j] += local_counts[j];
		}

		//for (nat j = 0; j < pm_count; j++) {
		//	if (local_counts[j + pm_count] > max_e[j]) 
		//		max_e[j] = local_counts[j + pm_count];
		//}

		free(local_counts);
	}

	nat sum = 0;
	for (nat i = 0; i < pm_count; i++) sum += counts[i];
	counts[pm_ga] = space_size - sum;

	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec), zthroughput = (double) space_size / seconds;
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	snprintf(output_string, 4096, "using [D=%hhu, R=%hhu]:"
			"\n\tspace_size=%llu"
			"\n\tthread_count=%llu"
			"\n\tcache_line_size=%llu"
			"\n\tminimum_split_size=%llu"
			// "\n\trange_update_frequency=%llu"
			"\n\tdisplay_rate=%llu"
			"\n\tfea_execution_limit=%llu"
			"\n\texecution_limit=%llu"
			"\n\tarray_size=%llu"
			"\n\n"
			"\n\t""searched %llu zvs"
			"\n\tusing %llu threads"
			"\n\tin %10.2lfs [%s:%s],"
			"\n\tat %10.2lf z/s."
			"\n\n", 

			D, R,   space_size,  thread_count,   cache_line_size,
			minimum_split_size,  display_rate,
			fea_execution_limit,  execution_limit,  array_size,  space_size, 
			thread_count,  seconds,  time_begin_dt,  time_end_dt,  zthroughput
	);
	print(output_filename, 4096, output_string);

	snprintf(output_string, 4096, "\npm counts:\n");
	print(output_filename, 4096, output_string);
	for (nat i = 0; i < pm_count; i++) {
		if (i and not (i % 2)) {
			snprintf(output_string, 4096, "\n");
			print(output_filename, 4096, output_string);
		}
		snprintf(output_string, 4096, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
		print(output_filename, 4096, output_string);
	}
	snprintf(output_string, 4096, "\n[done_pm]\n");
	print(output_filename, 4096, output_string);

/*	snprintf(output_string, 4096, "\nmax execution time till prune:\n");
	print(output_filename, 4096, output_string);
	for (nat i = 0; i < pm_count; i++) {
		if (i and not (i % 2)) {
			snprintf(output_string, 4096, "\n");
			print(output_filename, 4096, output_string);
		}
		snprintf(output_string, 4096, "%6s: %-8lld\t\t", pm_spelling[i], max_e[i]);
		print(output_filename, 4096, output_string);
	}
	snprintf(output_string, 4096, "\n[done_max_e]\n");
	print(output_filename, 4096, output_string);*/


}
 
























		/*


				*n = 0
				*i = 4
				(*n)++;  -----[*n < *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;




				*n = 0
				*i = 1
				(*n)++;  -----[*n == *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;



				*n = 0
				*i = 0
				(*n)++;  -----[*n > *i]----->    *n = 0;   ------[*n == *i]----->   (*n)++;












			LA:		5		              6		                  7
--------------------------------------------------------------------------------------------------------------------------


		 ......	     DOL = {    1 : { .l=7 .g=X .e=X }       4 : { .l=X .g=X .e=X }      4 : { .l=5 .g=X .e=X }      }






				*n = 0
				*i = 4
				(*n)++;  -----[*n < *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;

		*/















/*
202405293.180937:

implementing the binary space partition (BSP) opt:   a solution to 
	parellelizing while maining sequential executeion 
		as much as posisble for z skips. 


	the implementation goes as follows:


			first partition the space into 64 parts, each one given to a worker thread, 

				run each one using the sr,      supplying it a range_begin and range_end   

					both of which are global and atomic, 

						on initial execution:

						the thread reads the range begin, inits graph to it via a reduce()


						the thread reads range_end and inits end[] to it, via a reduce()

					the graph then does the standard loop containing ga, fea execute, and execute, 


					every 100th (or so) iteration,   we will "publish the current graph state, 

						via this code:





					nat zindex = 0;
					p = 1;
					for (byte i = 0; i < hole_count; i++) {
						zindex += p * graph[positions[i]];
						p *= (nat) (positions[i] & 3 ? operation_count : 5);
					}
					atomic_store_explicit(&range_begin[this_thread], zindex, memory_order_relaxed);
				


				



				ie, we are doing a unreduce on the graph, to get a zindex  and then we are storing that zi into our thread's global atomic range_begin state.   ie, we loose the original value of range_begin, set by main.  we don't need it anymore. 

				


				additionally, after updating  or "publishing"   our progress, via range_begin, 


				we update our local copy of    end[]  to reflect the current state of range_end   which might have been edited by other worker threads, from them taking half of our job's z values. 



				to do this, we just use this code:








				const nat range_end = atomic_load_explicit(global__range_end, memory_order_relaxed);
				p = 1;
				for (nat i = 0; i < hole_count; i++) {
					end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
					p *= (nat) (positions[i] & 3 ? operation_count : 5);
				}
				








				so end    and thus end_64      are now reflecting the true values   


					ie     in this case,   the global atomic values     global__range_end, are the master copy, 


					but in the case of range_begin,    the local_copies are master copies,   


						and synch'ing up again involves ovewrwritting the non-master copy with the master copy, of course


					
				



		so once all of this in place, 

			now the binary space partitoning part   comes in-

				when a thread finishes, it simply checks the global_range_begin  and global_range_end   arrays, 


			and searches it and finds the i such that    global_range_end[i] - global_range_begin[i]   is the largest



				and then it takes     that difference    divides it in half,   and then subtracts that much from global_range_end[i]



						this is the thread taking half of the work from the thread that has the least amount done!



				thus binary partitioning the search space!    pretty cool 





		then, the thread continues with this job-  given by simply:    

		local_range_begin = global_range_end[i]     (but after the subtraction was performed)
		
		local_range_end = global_range_end[i]       (but BEFORE  the subtraction was performed)



		then the process starts over, using those local range begin and end. 





















publish_begin() {

	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	atomic_store_explicit(&global_range_begin[thread_index], zindex, memory_order_relaxed);
}



pull_end() {

	const nat range_end = atomic_load_explicit(global_range_end[thread_index], memory_order_relaxed);
	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
				
}





main


	pthread_mutex_init(&mutex, 0);





	
worker_thread() {

start_first_job:

	// load range_begin and range_end   
	// from global arrays  at thread index (TI)

	nat range_begin = atomic_load(global_range_begin[thread_index], mor);
	nat range_end = atomic_load(global_range_end[thread_index], mor);


next_job:
	...reduce range_begin and range_end   to be graph   and graph_end  (end)...
	// now we are ready to start the job

	loop:

		
		check if job is done, (ie graph is at end) if so goto done

	init:
		if (publish_counter >= 500) {

			publish_begin();
			pull_end();

			publish_counter = 0;
		} else publish_counter++;
		...do job...

		goto loop;

	done:

		pthread_mutex_lock(&mutex);

		nat max_diff = 0;
		nat max_ti = 0;
		for (each thread index   ti < 64) {
			const nat b = atomic_load(global_range_begin[ti], mor);
			const nat e = atomic_load(global_range_end[ti], mor);
			const nat diff =  e - b;
			if (diff > max_diff) { max_diff = diff; max_ti = ti; }
		}

		if (max_diff < minimum_split_size) goto terminate;
		
		const nat amount = (max_diff >> 1);
		const nat e = atomic_fetch_sub(global_range_end[max_ti], amount + 1, mor);
		pthread_mutex_unlock(&mutex);
		range_begin = e - amount;
		range_end = e;
		goto next_job;
terminate:
	...
	pthread_exit(0);
	free(...);
}










[ 0 ... N )     [ N ... W )                W = 10


bob's range_begin = 0
bob's range_end = 9


          2 3 4 5     6     9
      D D X X X X     X X X X 
          ^







bob:
       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX P XXXXXXXXXXX......M..............
							     |
      zi=0                             zi=1000              -|-          zi=5000
						             |
						             |
					         joe:         XXXXXXXXXXXXXX









*/





























































































/*


	// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
				// CHANGED:   yup, its correct i think.











static nat execute_graph_starting_at(byte origin) {


}










static nat execute_graph(void) {
	
}

static nat fea_execute_graph_starting_at(byte origin) {

	
}

static nat fea_execute_graph(void) {

}












*/

























			// update progress bar here!!!   // ie, atomic load of "head".
			// (would detect if head is >= spacesize, and then start the joining process, if it is.
















/*




static void write_graph(nat b, nat e) {

	get_datetime(buffer[buffer_count].dt);
	get_graphs_z_value(buffer[buffer_count].z);
	buffer_count++;

	if (buffer_count == max_buffer_count) {
		append_to_file(b, e);
		buffer_count = 0;
	}
}










	//for (byte i = 0; i < graph_count; i += 4) {
	//
	//	if (	executed[i + 1] < 5 and graph[i + 1] or         // slightly concerning here... think about this more...? 
	//		executed[i + 2] < 5 and graph[i + 2] or 
	//		executed[i + 3] < 5 and graph[i + 3]) { a = pm_eda; goto bad; }
	//}



///////////////
//	if (not (display_counter & ((1 << display_rate) - 1))) { print_graph_raw(); putchar(10); fflush(stdout); }
//	display_counter++;
//	nat zindex = 0;
//	nat p = 1;
//	for (byte i = 0; i < hole_count; i++) {
//		zindex += p * graph[positions[i]];
//		p *= (nat) (positions[i] & 3 ? operation_count : 5);
//	}
////////////////










x	array = calloc(array_size + 1, sizeof(nat));
x	timeout = calloc(operation_count, sizeof(nat));

x	positions = calloc(hole_count, 1); 
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);

	graph = raw_graph;
	end = raw_end;
	uint64_t* graph_64 = raw_graph;
	uint64_t* end_64 = raw_end;
	nat display_counter = 0, found = 0;
	byte pointer = 0;

	memcpy(graph, R ? _63R : _36R, 20);
	memcpy(end, R ? _63R : _36R, 20);


	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); }

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_end >= p) { puts("range_end is too big!"); printf("range_end = %llu, p = %llu\n", range_end, p); abort(); }

	
	goto init;










	if (argc != 2) { printf("./pth-srnfgpr [D=%hhu][R=%hhu] <threadcount:nat>\n", D, R); exit(0); }

	char* threadcount_invalid = NULL;
	const nat threadcount = strtoull(argv[1], &threadcount_invalid, 10);
	if (*threadcount_invalid) {
		printf("ERROR: error parsing threadcount near \"%s\" aborting...\n", threadcount_invalid);
		abort();
	}




// printf("sleeping for %5.2lfs until:\n\n\talarm: %s\n\tnow:   %s\n\n", difference, alarm_spelling, now_spelling);





static volatile int quit = 0;
static noreturn void handler(int __attribute__((unused))_) {
	puts("\ncanceled alarm.");
	quit = 1;
	abort();
}



int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: ./alarm <string> <datetime:1%Y%m%d%u.%H%M%S>");

	struct sigaction action = {.sa_handler = handler}; 
	sigaction(SIGINT, &action, NULL);

	const char* purpose = argv[1];
	const char* alarm_string = argv[2];
	char now_spelling[32] = {0};
	char alarm_spelling[32] = {0};

	struct tm alarm_details = {0};
	strptime(alarm_string, "1%Y%m%d%u.%H%M%S", &alarm_details);
	struct timeval the_alarm = { .tv_sec = timelocal(&alarm_details), .tv_usec = 0 };
	
	struct timeval now;
	gettimeofday(&now, NULL);
	const double difference = difftime(the_alarm.tv_sec, now.tv_sec);

	strftime(alarm_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&the_alarm.tv_sec));
	strftime(  now_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&now.tv_sec));
	printf("sleeping for %5.2lfs until:\n\n\talarm: %s\n\tnow:   %s\n\n", difference, alarm_spelling, now_spelling);

	puts("\tasleep...");
	sleep((unsigned) difference);

	gettimeofday(&now, NULL);
	strftime(now_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&now.tv_sec));
	printf("\n\n\t\t[ \033[1;31mALARM\033[0m ] : \033[1m %s \033[0m : \033[1m %s \033[0m\n\n", purpose, now_spelling);
	while (not quit) { system("afplay bell.wav"); sleep(1); }

}













typedef long long nat;

static nat thread_count = 9;
static const nat job_count = 1000000;

struct thread_arguments {
	nat index;
};

static pthread_mutex_t input_mutex;
static pthread_mutex_t output_mutex;

static _Atomic nat output_count = 0;
static _Atomic nat input_count = 0;

static nat* output = NULL; 
static nat* input = NULL;


int main(int argc, const char** argv) {
	if (argc == 1) return puts("give the thread count as an argument!");

	thread_count = atoi(argv[1]);

	srand(42);

	//clock_t begin = clock();

	pthread_mutex_init(&output_mutex, NULL);
	pthread_mutex_init(&input_mutex, NULL);

	input = calloc(job_count, sizeof(nat));
	output = calloc(job_count, sizeof(nat));
	
	for (nat i = 0; i < job_count; i++) 
		input[input_count++] = (nat) (rand() % 400);
	
	pthread_t* thread = calloc((size_t) thread_count, sizeof(pthread_t));

	for (nat i = 0; i < thread_count; i++) {
		struct thread_arguments* args = calloc(1, sizeof(struct thread_arguments));
		args->index = i;
		pthread_create(thread + i, NULL, worker_thread, args);
	}

	for (nat i = 0; i < thread_count; i++) 
		pthread_join(thread[i], NULL);

	printf("info: number of jobs completed = %llu\n", output_count);

	pthread_mutex_destroy(&output_mutex);
	pthread_mutex_destroy(&input_mutex);

	free(input);
	free(output);

	//clock_t end = clock();
	//double total_time = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("\t time for %llu threads:   %10.10lfs\n", thread_count, total_time);
}








// atomic_fetch_add_explicit(&acnt, 1, memory_order_relaxed);










































static void append_to_file(char* filename, size_t sizeof_filename, byte* graph) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, graph);
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("filename=%s ", newfilename);
			close(dir); 	
			123456_ERROR:   DO SOMETHING TO FIX THIS!?!!?!
			abort();
		}
		char created_at[32] = {0};
		get_datetime(created_at);
		snprintf(filename, sizeof_filename, "%s_%08x%08x_z.txt", 
			created_at, arc4random(), arc4random()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, z, strlen(z));
	write(file, " ", 1);
	write(file, dt, strlen(dt));
	write(file, "\n", 1);
	close(file);
	printf("[%s]: write: %s z = %s to file \"%s\"\n", 
		dt, permission ? "created" : "wrote", z, filename
	);
}














static void write_file(const char* directory, char* name, size_t maxsize) {
	int flags = O_WRONLY | O_TRUNC;
	mode_t permission = 0;
	if (not *name) {
		srand((unsigned)time(0)); rand();
		char datetime[32] = {0};
		struct timeval t = {0};
		gettimeofday(&t, NULL);
		struct tm* tm = localtime(&t.tv_sec);
		strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);
		snprintf(name, maxsize, "%s%s_%08x%08x.txt", directory, datetime, rand(), rand());
		flags |= O_CREAT | O_EXCL;
		permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	}
	int file = open(name, flags, permission);
	if (file < 0) { perror("save: open file"); puts(name); getchar(); }
	write(file, text, count);
	close(file);
}










char newfilename[4096] = {0};
	strncpy(newfilename, filename, sizeof newfilename);
	const int dir = open(directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", directory); 
		return; 
	}




//     srand((unsigned)time(0)); rand();





char datetime[32] = {0};
		struct timeval t = {0};
		gettimeofday(&t, NULL);
		struct tm* tm = localtime(&t.tv_sec);
		strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);





perror("write openat file");
		printf("filename=%s\n", newfilename);






	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, b, e);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strncpy(filename, newfilename, sizeof filename);

	close(dir);








*/

















/*static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}*/









// if (not array[pointer])    { a = PM_ne0; goto bad; }     // delete me!!!!     redundant becuaes of pco.

/*










for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not fea_execute_graph_starting_at(o, graph, array)) return false;
	}
	return true;
















	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0; 
	memset(timeout, 0, operation_count * sizeof(nat));

	byte ip = origin, last_mcal_op = 0;

	nat 	e = 0,  xw = 0,
		pointer = 0,  er_count = 0, 
	    	OER_er_at = 0,  OER_counter = 0, 
		R0I_counter = 0, H_counter = 0,
		RMV_counter = 0, RMV_value = 0, CSM_counter = 0;

	for (; e < execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return true; 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return true; 
		}
		
		const byte I = ip * 4, op = graph[I];

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return true; 
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) return true; 
			if (not array[pointer]) return true; 

			if (last_mcal_op == one) H_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			pointer++;

			if (pointer > xw and pointer < n) {    // <--------- ALSO CHANGED THIS ONE TOO.
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return true; 
			if (not pointer) return true; 
				
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_acceptable_er_repetions) return true; 
			
			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_acceptable_consequtive_small_modnats) return true; 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_acceptable_modnat_repetions) return true; 
			}

			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
			if (array[n] >= 65535) return true; 
		}

		else if (op == six) {  
			if (not array[n]) return true; 
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return true; 

			if (last_mcal_op == one) {
				H_counter++;
				if (H_counter >= max_acceptable_run_length) return true; 
			}

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_acceptable_consecutive_s0_incr) return true; 
			}

			if (array[pointer] >= 65535) return true; 
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return false;








for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not execute_graph_starting_at(o, graph, array, timeout)) return false;
	}
	return true;














	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 

	byte ip = origin, last_mcal_op = 0;
	nat pointer = 0, e = 0, xw = 0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return true;
			if (not array[pointer]) return true;
			pointer++;

			// new correct lazy zeroing:

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}


			// WAS:  if (pointer > xw) { xw = pointer; array[pointer] = 0; }   // ERROR HERE!!!!
			
			

			/////////////////////////////////////////////////////////////////////////////////////////
			

				//  WHAT IF pointer ALIAS's   STAR N!!!!!!!   CRAPPP

				// we'll reset it, and thus change the graphs behavior!!!!





					// we had added the lazy zeroing opt to the fea pass,  and thus the "i == n" alias condition will occur MUCH more likely!!!

			/////////////////////////////////////////////////////////////////////////////////////////




		}

		else if (op == five) {
			if (last_mcal_op != three) return true;
			if (not pointer) return true;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return true;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return true;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return false; 








*/




//if (executed[I + state] < 253) executed[I + state]++;







// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
						// CHANGED: yup, we can just use memory_order_relaxed. 





// only open part of the multithreading-zskip solution that we devised:    the sychronization part-
// we need this to COME BEFORE the atomic_fetch_add_explicit() call. 
// (using better  memory orderings  probably???...)






// {0 1 3 [4} ]
	// byte previous_op = graph[20];
	// make this not use this temporary variable, by using   index and index + 4   
	// (except if index+4==graphcount, then we will  just say its index.. yeah)

	// constructive GA here





     // here, we know we are doing a zskip, becuase we are over the end.




/*
if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); } // delete me?
	if (range_end >= p) { puts("range_end is too big!"); printf("range_end = %llu, p = %llu\n", range_end, p); abort(); }  // delete me?
*/




/*	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	atomic_store_explicit(&head, zindex, memory_order_relaxed);














const nat h = atomic_fetch_add_explicit(&head, job_size, memory_order_relaxed);
	if (h >= space_size) goto terminate_thread;

// if (range_end >= space_size) range_end = space_size - 1; // is this neccessary now?...







if (h >= space_size) {
			snprintf(output_string, 4096, "info: [all jobs allocated to threads. waiting for them to finish.]\n");
			print(output_filename, 4096, output_string);
			break;
		}





*/


// memset(timeout, 0, operation_count * sizeof(nat));



		/*for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return pm_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;*/



			/*if (bout_length == 1) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;*/



/*
// WRONG: was     at = index   which is incorrect. 
	}                                                                      //       don't edit this to get the original semantics.

*/





/*
(MCALOP) [2]... 6 [2]... (MCAL_OP) [2]... 6 [2]... (MCALOP) [2]... 6 [2]...

(MCALOP) [2]... 6 [2]... 6 [2]... (MCALOP) [2]... (MCALOP) [2] [2] [2] [2] [2] 6 [2]...

const nat op = ...

if (six) {
	if (bit) return pm_snco;
	bit = 1;

} else if (two) {
	...
}

...

if (op == one or three or five) {
	last_mcal_op = op;
	mcal_index++;
	bit = 0;
}

last_op = op;
///////////////////////////////////////

implementation:


const nat op = ...

if (op == six) {
	if (	last_op != one and 
		last_op != three and 
		last_op != five
	) return pm_snco;
	...
}






////////////////////////






*/




/*



1067/60  17.7833333333

000000000000 : 00000000000000000000 :: 

using [D=2, R=0]:
        space_size=118689037748575
        thread_count=64
        minimum_split_size=6
        range_update_frequency=0
        display_rate=3
        fea_execution_limit=5000
        execution_limit=10000000000
        array_size=100000


        searched 118689037748575 zvs
        using 64 threads
        in    1067.00s [1202406296.041605:1202406296.043352],
        at 111236211573.17 z/s.


pm counts:
z_is_good: 0                     pm_ga: 118673515889217 
pm_fea: 0                       pm_ns0: 91740   
pm_pco: 197031                  pm_zr5: 12574641
pm_zr6: 13909703                pm_ndi: 950355  
pm_oer: 21708                   pm_r0i: 669957  
 pm_h0: 2047223                 pm_f1e: 5954    
pm_erc: 34413942                pm_rmv: 956741  
 pm_ot: 0                       pm_csm: 0       
 pm_mm: 18                      pm_snm: 18      
pm_bdl: 720                     pm_bdl2: 0       
pm_erw: 0                       pm_mcal: 1474341 
pm_snl: 346981997                pm_h1: 0       
 pm_h2: 1014                     pm_h3: 12      
pm_per: 220234                  pmf_fea: 1840527 
pmf_ns0: 271515602              pmf_pco: 390249243
pmf_zr5: 4979848460             pmf_zr6: 6560756663
pmf_ndi: 1091898281             pmf_per: 5315232 
pmf_mcal: 1805918001
[done]

*/





