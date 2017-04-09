/**
 * This is a bridge between STM32 HAL generated C code and our C++ event handling loop
 * @see Program
 * @author Denis Kokarev
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * our app execution point defined in program.cpp
 * this function is to be invoked from Src/main.c generated module
 */
void exec();

#ifdef __cplusplus
}
#endif
