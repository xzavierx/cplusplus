## 在单处理器上，也需要保证原子性，否则如果中断了，也会导致一致性状态问题
## 要预防ABA问题，参见[wiki](https://en.wikipedia.org/wiki/ABA_problem)