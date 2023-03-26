set terminal png
set term png size 1000, 750
set output "image for multithread.png"
set title "Time and Multithread" font"Time New Roman,20"
set ylabel "Time\n(milliseconds)"
set xrange [1:1050]
set logscale x
set xlabel "SizeofMatrix" 
set yrange[0.001:10000]
set logscale y
set grid
set key left top inside
plot	'Matrix.dat' using 1:2 title "singlethread" with linespoints lc "slateblue1" pointtype 6 ps 1,\
	'Matrix.dat' using 1:3 title "Multithread = 1" with linespoints lc "web-blue" pointtype 6 ps 1,\
	'Matrix.dat' using 1:4 title "Multithread = 2" with linespoints lc "forest-green" pointtype 6 ps 1,\
	'Matrix.dat' using 1:5 title "Multithread = 3" with linespoints lc "spring-green" pointtype 6 ps 1,\
	'Matrix.dat' using 1:6 title "Multithread = 4" with linespoints lc "yellow" pointtype 6 ps 1,\
	'Matrix.dat' using 1:7 title "Multithread = 5" with linespoints lc "gold" pointtype 6 ps 1,\
	'Matrix.dat' using 1:8 title "Multithread = 6" with linespoints lc "orange" pointtype 6 ps 1,\
	'Matrix.dat' using 1:9 title "Multithread = 7" with linespoints lc "red" pointtype 6 ps 1,\
	'Matrix.dat' using 1:10 title "Multithread = 8" with linespoints lc "brown" pointtype 6 ps 1
