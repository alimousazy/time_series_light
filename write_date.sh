
#!/bin/bash 
s_date=1480460000
e_date=1480467200
COUNTER=0
while [  $s_date -lt $e_date ]; do
  echo The counter is $s_date 
	 let s_date=s_date+1 
done
