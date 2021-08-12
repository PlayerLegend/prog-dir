(load-int (int 0))

(ifelse
 ( (load-float (float 0))
   (+float (float 2))
   (+float (float 55))
   (-float (float 23.5)) )
 
 ( (load-float (float 0))
   (+float (float 3))
   (+float (float 3))
   (+float (float 5)) ) )

(exit)
