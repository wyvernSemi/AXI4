proc mk_vproc {srcrootdir testname opdir} {

  exec make -C $srcrootdir USRCDIR=$testname clean
  exec make -C $srcrootdir USRCDIR=$testname OPDIR=$opdir
  file copy -force $srcrootdir/$testname/VProc.so $opdir
}
