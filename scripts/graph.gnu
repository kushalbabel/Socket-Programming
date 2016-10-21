set term postscript eps color

set output '../combined.eps'

#used to make the fonts appear larger;  makes the figure smaller but keeps the fonts same size
set size 0.6, 0.6
set key right bottom
set title "Avg Time vs No. of workers"
set yrange [0:200]

set xlabel "Number of workers"
set ylabel "Avg time (in secs)"

set grid ytics lt 0 lw 1 lc rgb "#bbbbbb"
set grid xtics lt 0 lw 1 lc rgb "#bbbbbb"
show grid
plot \
'../data/8.dat' using 1:2 t'PasswordLength : 8' with linespoints pt 1, \
'../data/7.dat' using 1:2 t'PasswordLength : 7' with linespoints pt 1, \
'../data/6.dat' using 1:2 t'PasswordLength : 6' with linespoints pt 1

