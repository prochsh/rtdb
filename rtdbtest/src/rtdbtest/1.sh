#!/bin/tcsh
foreach item in ( apple peal peach )
	echo $item  > $item.foo
end

