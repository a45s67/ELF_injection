
all: payload inject got_inject 

payload: ./reverse_reverse_shell.py
	python ./reverse_reverse_shell.py


inject: inject.c
	gcc inject.c -o inject

got_inject: ./got_inject.c
	gcc $^ -o $@ 




