/* list of dummy objects to make c++ happy */

/*
 * we're better with declaring this stub explicitly
 * otherwise c++ wants to link `abort()` with whole 9 yards
 * bloating the code by 50K
 */
void __cxa_pure_virtual(void) {};

/*
 * some math functions need errno
 */
int __errno;
