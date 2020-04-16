# ping_cli
small Ping CLI application for MacOS or Linux

- Operating System: Mac OS X, product version: 10.15.3, build version: 19D76
- written and compiled using: Apple clang version 11.0.0 (clang-1100.0.33.17)

## Usage

1. Compile the source code: `gcc main.c -o pingcli`.
2. Run the compiled binary with the required "-h" flag: `pingcli -h www.example.com`.

Addtional options available:

1) Set the IP Time To Live for outgoing packets: `-m ttl`
2) Stop after sending (and receiving) count ECHO_RESPONSE packets: `-c count`
