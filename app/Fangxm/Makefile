dirs	=  `ls -d xdr_handler xdr_sender_emu xdr_merge_emu`

all:
	@for dir in $(dirs); do \
		cd $$dir; make; cd ..; \
	done
	
clean:
	@for dir in $(dirs); do \
		cd $$dir; make clean; cd ..; \
	done
