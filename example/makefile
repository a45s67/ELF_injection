
all: out out_pie got_out got_out_pie

out out_pie: ../inject a_pie a ../payload
	../inject a ../payload out
	../inject a_pie ../payload out_pie
	chmod +x out
	chmod +x out_pie


a: 1.c
	gcc 1.c -no-pie -o a
a_pie: 1.c
	gcc 1.c -o a_pie


got_out: ../got_inject a ../payload
	$< a ../payload got_out
	chmod +x got_out

got_out_pie: ../got_inject a_pie ../payload 
	$< a_pie ../payload got_out_pie
	chmod +x got_out_pie

