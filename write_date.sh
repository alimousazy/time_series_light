
#!/bin/bash 
s_date=1480138844
e_date=1480655019
COUNTER=0
while [  $s_date -lt $e_date ]; do
  rand=`awk -v min=1 -v max=80 'BEGIN{srand(); print int(min+rand()*(max-min+1))}'`
  echo "w:$s_date:new_era:$rand" >> tm 
  d=`wc -l tm `
  set -- $d
  if [ $1 -eq 1000 ] ; then
    cat tm | nc localhost 5555
    exit;
    echo '' > tm
    echo 'flushing'
  fi
  let s_date=s_date+1 
done
