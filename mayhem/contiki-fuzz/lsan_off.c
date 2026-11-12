/* LeakSanitizer is off by default for every target of this fleet. */
int __lsan_is_turned_off(void) { return 1; }
