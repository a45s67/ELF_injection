
all: out out_pie got_out got_out_pie

out out_pie: inject a_pie a.out payload
	./inject a.out payload out
	./inject a_pie payload out_pie
	chmod +x out
	chmod +x out_pie

payload: shell.py
	python shell.py

a.out: 1.c
	gcc 1.c -no-pie

inject: inject.c
	gcc inject.c -o inject

got: ./got_inject.c
	gcc -g3 ./got_inject.c -o got_inject 

got_out: ./got_inject
	./got_inject ./a.out ./payload got_out

a_pie: 1.c
	gcc 1.c -o a_pie


got_out_pie: a_pie got_inject
	./got_inject a_pie payload got_out_pie
	chmod +x got_out_pie

