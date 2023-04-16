# *crack-hash*
*crack-hash* `clone` 自 [*hashcat*](https://github.com/hashcat/hashcat)，*hashcat*的详细介绍请参考[hashcat_READEME.md](./HASHCAT_README.md)。

## *crack-hash* 任务
*crack-hash* 的目的使用 *hashcat* 作为破解工具对用户的输入的哈希值进行破解，为了直观的展示破解的过程，为用户提供了 *GUI* 界面。

## *crack-hash* 支持的功能

1. 哈希值的破解，支持的哈希算法：
   + *MD5*
   + *SHA1*
   + *SHA256*
2. GUI 输入哈希值
3. GUI 展示破解进度条

## *crack-hash* 依赖的库

1. [GLFW](https://www.glfw.org/)
2. [IMGUI](https://github.com/ocornut/imgui)

## *crack-hash* 相对 hashcat 的改动

1. *hashcat* 使用 `make` 作为其构建工具，能在各类 *unix-based* 系统、*CYGWIN*、*MSYS* 和 *WSL* 上进行构建，在 *crack-hash* 中，使用 `cmake` 作为构建工具，目前只支持 *unix-based* 系统上的构建；