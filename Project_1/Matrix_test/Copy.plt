set terminal png
set output "image for bufferSize.png"
set title "Time and BufferSize" font "Time New Roman,20"
set ylabel "Time\n(milliseconds)"
set xrange [0.90:10100]
set logscale x
set yrange [-1:12]
set xlabel "BufferSize"
set grid
plot	'Copy.dat' using 1:2 title "bufSize = 1" with points lc "slateblue1" pointtype 6 ps 1,\
	'Copy.dat' using 3:4 title "bufSize = 5" with points lc "web-blue" pointtype 6 ps 1,\
	'Copy.dat' using 5:6 title "bufSize = 25" with points lc "forest-green" pointtype 6 ps 1,\
	'Copy.dat' using 7:8 title "bufSize = 50" with points lc "spring-green" pointtype 6 ps 1,\
	'Copy.dat' using 9:10 title "bufSize = 100" with points lc "yellow" pointtype 6 ps 1,\
	'Copy.dat' using 11:12 title "bufSize = 250" with points lc "gold" pointtype 6 ps 1,\
	'Copy.dat' using 13:14 title "bufSize = 500" with points lc "orange" pointtype 6 ps 1,\
	'Copy.dat' using 15:16 title "bufSize = 1000" with points lc "red" pointtype 6 ps 1,\
	'Copy.dat' using 17:18 title "bufSize = 10000" with points lc "brown" pointtype 6 ps 1
