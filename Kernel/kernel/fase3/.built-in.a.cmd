savedcmd_kernel/fase3/built-in.a := rm -f kernel/fase3/built-in.a;  printf "kernel/fase3/%s " syscall3.o | xargs ar cDPrST kernel/fase3/built-in.a
