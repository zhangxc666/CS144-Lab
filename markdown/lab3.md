## class timer
- 模拟时间流逝
- 当当前RTO时间为负时，表示此时超时，将会触发警报（注意：时间流逝并不是真正的时间，通过tick方法改变时间）
- 每次发送新的tcp段时，若当前计时器没有运行，则运行它
- 当所有的tcp段都被ack时，停止计时
- 若当前已经超时，重发最早的segment
- 不断重发未接受的segment，还需时间二进制退避算法，修改RTO
- 重置RTO为初始值
