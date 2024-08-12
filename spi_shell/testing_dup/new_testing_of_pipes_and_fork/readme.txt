1202408117.235648
finallyyy got this asych shell functionality working. 
it allows you to send and receive strings from a running process, 
through a cli of sort.  its super nice to use, and works with anything in general, basically. 
except signals.. doesnt quite work with that lol lololo

but yeah, ignoring that. (we should probably figure those out lololol..)



we got this working   using only the following system calls:



	execve
	fork 
	chdir

	read
	write
	close

	poll
	pipe
	dup2
	signal




using those, we are able to construct a shell, using this code.
which is pretty cool! we will be able to do this in our language, probably, soon. 


but yeah, generally speaking this code is useful for whenever you have a process that you want to talk to asychronously from another process. perhaps even muiltple processes you want to manage, and talk to asychr. superrrr duper useful. 



its cool becuase you get to decide when you want to issue the reads and writes, and the system makes sure you don't wait around for a read,  or crash from a bad write,    while doing so lol.









