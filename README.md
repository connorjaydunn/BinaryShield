# BinaryShield

**BinaryShield** is an open-source, bin-to-bin x86-64 code virtualizer designed to offer strong protection against reverse engineering efforts. It translates commonly used x86-64 instructions into a custom bytecode, which is executed by a secure, purpose-built virtual machine. For more information on virtualization and the technical details of how the BinaryShield VM works, click [here](https://connorjaydunn.github.io/blog/posts/binaryshield-a-bin2bin-x86-64-code-virtualizer/).

Features
----
* _Bytecode encryption (soon)_
* Multi-Thread safe VM
* _VM handler mutation (soon)_
* Stack-Based, RISC VM
* _Multiple VM handler instances (soon)_
* Wide range of supported opcodes
* Trivial to implement support for new opcodes
* _VM handler integrity checks (soon)_
* Over 60+ VM handlers

Screenshots
---

<p align="center">
  <img src="https://github.com/connorjaydunn/BinaryShield/blob/main/screenshots/before.png"/>
  <br>
  before virtualization
</p>

<p align="center">
  <img src="https://github.com/connorjaydunn/BinaryShield/blob/main/screenshots/after.png"/>
  <br>
  after virtualization
</p>

Dependencies
---
* C++14 or higher,
* [Zydis](https://github.com/zyantific/zydis)

Usage
----
```bash
binaryshield.exe <target binary path> <start-rva> <end-rva>
```

Example:

```bash
binaryshield.exe calc.exe 0x16D0 0x16E6
```

TODO
----

* Bytecode encryption
* ~~VM context collision check~~
* VM handler mutation
* VM handler integrity checks
* Multiple VM handler instances
* Anti-Debugger checks
* Add function by code markers
* Randomised VM context
* Ability to virtualize areas of code, not just functions

Disclaimer
---
**BinaryShield** is currently in a very early stage of development and is **not suitable for commercial use** at this time. While the core functionality is in place, there may still be bugs, incomplete features, and potential security vulnerabilities.

I am actively working on improving and expanding the tool, and will continue to release updates regularly. Feedback and contributions are welcome.