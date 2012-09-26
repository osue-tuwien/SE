#ifndef COMMON_H
#define COMMON_H

#define EC0 (0xE1820)
#define SEQ0 (0xE1821)

pid_t run_child(int (*fp)(void));

#endif /* COMMON_H */
