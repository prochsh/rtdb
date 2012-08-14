#!/bin/tsch
set host_list=(248 249 250)
@ hostnum = $1
while($hostnum < 3)
	shift host_list
	@ hostnum++
end
foreach host ($host_list)
    ssh "d5000@192.168.1.$host"   ~/src/fes/bin/fes_simu_test $2 1 $3 >/dev/null &
end
