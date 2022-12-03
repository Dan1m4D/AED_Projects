#
# Tomás Oliveira e Silva, AED, September 2022
#
# makefile to compile the A.01 assignment (speed run)
#

clean:
	rm -rf a.out example.pdf speed_run speed_run_with_zlib solution_speed_run solution_speed_run_with_zlib

sol_SpeedRun:		sol_SpeedRun.c make_custom_pdf.c
	cc -Wall -O2 -D_use_zlib_=0 sol_SpeedRun.c -o sol_SpeedRun -lm