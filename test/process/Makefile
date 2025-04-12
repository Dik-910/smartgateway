fork: 01_fork.c
	-$(CC) $^ -o $@
	-./$@
	-rm $@

other: 02_other.c
	-$(CC) $^ -o $@

execve: 02_execve.c other
	-$(CC) $< -o $@
	-./$@
	-rm $@ $(word 2, $^)

# $(word N, TEXT) 函数用于提取 TEXT 中的第 N 个单词

waitpid: 03_waitpid.c
	-$(CC) $^ -o $@
	-./$@
	-rm $@

