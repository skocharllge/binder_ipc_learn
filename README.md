# binder_ipc_learn

First step of implementation(DONE)

Character driver to send across c struct's across process boundaries.

Figuring out how to send huge data across process boundaries


Yet to implement(TO BE DONE)


Implement cirucular buffer of structures so that process can communicate using circ buffer in kernel

Implement white list & black list map of processes communicating

Implement ioctl to update the black list map and allow IPC if it is part of whitelist and block if it is part of blacklist
