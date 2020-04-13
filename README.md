# bulkmt 
### пакетной обработчик команд ###

input | output
------|------------------
cmd1  |
cmd2  |
cmd3  | bulk: cmd1, cmd2, cmd3
{     |
cmd4  |
cmd5  |
}     | bulk: cmd4, cmd5
cmd6  |
EOF   | bulk: cmd6
-------------------------

## metrics
 ```
main thread  - 8 line, 3 block, 6 command
log thread   - 3 block, 6 command
file1 thread - 2 block, 4 command
file2 thread - 1 block, 2 command
```
