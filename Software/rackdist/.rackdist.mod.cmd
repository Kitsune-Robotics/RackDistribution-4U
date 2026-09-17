savedcmd_rackdist.mod := printf '%s\n'   rackdist.o | awk '!x[$$0]++ { print("./"$$0) }' > rackdist.mod
