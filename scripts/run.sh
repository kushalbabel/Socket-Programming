while IFS='' read -r row || [[ -n "$row" ]]; do
	echo "$row"
	../exec/user 10.105.12.37 5000 $row 8 100 >> avg.txt
done  < "$1"

python avg.py