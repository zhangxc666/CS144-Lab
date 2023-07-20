Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`

==============================

### Finish List

- `Lab0`：用时2h
- `Lab1`：用时2h
- `Lab2`：用时4h
- `Lab3`：用时12h（主要根本没看懂函数组织逻辑）
- `Lab4`：用时4h（仅debug一次就通过所有样例。跟着文档走就行，文档情况写的特别全）
- `Lab5`：用时3h（挺简单，就是有个坑点，进行route更新ttl时，需要更新校验和）
- `Lab6和Lab7`：无需写代码，至此cs144完成，8天成功完成lab。