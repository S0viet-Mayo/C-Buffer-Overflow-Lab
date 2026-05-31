# Vulnerability Analysis: Stack-Based Buffer Overflow and Memory Corruption

## Project Overview
This repository contains a deep-dive security analysis and documentation of a stack-based buffer overflow and string handling vulnerability in a  compiled target application (`buffy`). By analyzing the differences between bounded and unbounded input functions, this lab demonstrats hands-on proficiency in low-level memory layouts, stack behaviors, and how software flaws directly violate the core pillars of the CIA triad (Confidentiality, Integrity, and Avalibility). 

The application processes user credintals in a maximum expected buffer size of 10 characters for adjacent `username` and `password` variables on the stack.

---

## Vulnerability Analysis & Code Mechanics

The application applications implements two input tracking behaviors: `get_input_safe()` and `get_input_unsafe()`.

### 1. Bounded Mitigation: `get_input_safe()` The safe input enforces strict boundary validation logic: 

```c

while (count <len ){...}
```
Because `count < len` prevents the user from writing past the allocated buffer size, inputs that exceed the limit are safely terminated. Testing confirms when passing inputs larger than 10 characers, the buffer reserves space for the newline character (`\n`) or a null terminator, saftely restricting data retention to exactly 9 characters of a user input.

### 2. Unbound Flaw: `get_input_safe()` The vulnerability lies within the unsafe input tracking loop, which lacks array bounds checking: 

```c 
while (c !='\n'){...}

```
This loop evaluates input termination based *only* when encountering a newline character (`\n`) via the enter key. It does not validate how many bytes are being written into the buffer. 

Furthermore, the application admits string NULL Terminators (`\0`). Because the C standard library string manupulation routines rely on finding a `\0` byte to determine where the string ends, the absence of null termination forces operations to read across the continuous stack space. When the loop fails to find a terminator, it slams adjacent data structures together in memory, reading past the inteded boundaries. 

---

## Empirical Test Cases & Execution Observations

The application was subjected to four input tracking test vectors to map runtime stack behavior. 

### Test Case A: Standard Normal Input
* Input Username: `mjacks11` (8 characters)
* Behavior: The input sits at 8 characters safely within the 10-byte boundary. The application executes normally without leaking memory.

### Test Case B: Boundary Collision & Adjacent Memory leak
* Input Username: `david12345` (Exactly 10 Characters)
* Observed Output:
 ```text David12345david12345Secret:It could be bunnies!```
* Analysis: Because the input filled the entire buffer without leaving room for a null terminator, string printing routines bled directly into adjacent memory on the stack. This leaked the string payload of the password buffer and exposed and embedded application credential (`Secret:It could be bunnies!`).

### Test Case C: Arbitrary Memory Corruption & Overwrite
* Input Monologue: `ifwardoesntchangemenmustchangeandsomusttheirsymbols`
* Observed Output:
 ```text ifwardoesndddddddddddgeandsomusttheirsymbols```
* Analysis: The lack of bounds checking allowed the input string to overwrite adjacent variables. By shortening the input to `ifwardoesnt` and the password string to `changemenm`, the application confirmed memory alignment by printing the password injection cleanly inside of the corrupted username space. This revealed exactly where the variables neighbor each other on the stack. 

### Test Case D: Stack Smashing & Segmentation Fault
* Input: 50 random characters
* Observed Output:
 ```text Secret:It could be bunnies! ***stack smashing detected***: terminated Aboted (core dump).```
* Analysis: An input string of this volume fills both the username and password buffers (10 bytes each) and continues writing sequentially up the stack until it overwrites the Stack Canary. The canary is a secret guard variable placed on the stack right above local variables. Just before the function returns, the compiler executes a hidden integrity check to evaluate the canary's memory address. If the check fails, the program bypasses the standard return path and jumps to the C standard library error handler, `__stack_chk_fail()`, forcing a controlled crash. 

## Technical Appendix: Why Canaries Fail
While Stack Canaries are a vital defensive layer, they represent an indirect tripwire rather than a physical barrier. Attackers circumvent this mitigation using two primary methodologies:

### 1. Information Disclosure (Canary Leaking): 
If an application contains a string format vulnerability or lacks null terminators (as seen in Test Case B), an attacker can trick the program into printing out the secret 8-byte canary value. Once leaked, the attacker crafts a payload that overwrites the stack but inserts the exact, original canary value back into its designated slot. The integrity check passes, allowing the payload to hijack the Return Address completely undetected.

### 2. Brute Forcing (32-Bit Enviroments): 
On 32-bit systems, a canary is only 4 bytes long. To prevent string functions from reading it, the first byte is typically a null byte `(0x00)`, leaving 3 bytes of actual randomness (roughly 16 million variations). If a vulnerable network service forks and restarts automatically upon crashing, an attacker can brute-force the canary byte-by-byte in a matter of hours.

---


## Impact Assessment (CIA Triad Breakdown)
The vulnerabilities withing the unsafe execution path cause failures across all core tenets of security: 

### 1. Confidentiality Violation
Due to the ommision of the null terminator and lacks of bounds checking, a user can read memory in adjacent spaces that they are not authorized to see. In a real world senario, this allows threat actors to scrape active session memory, expose passwords, leak cryptographic API keys, or harvest authorization tokens. 

### 2. Integrity Violation
Because the application permits writing past array boundaries, an attacker can manipulate program data structures. By overwriting values stored in adjacent variables or altering function pointers, the state of the data is corrupted, allowing unauthorized modifications to the program. 

## 3. Avaliability Violation (Stack Smashing)
Fuzzing the application with a high volume of characters corrupts critical frame managment structures on the runtime stack, such as the Frame Pointer (EBP/RBP) and the Return Address. Upon severe corruption, modern compiler defenses trigger a crash mitigation response:

```text 
*** stack smashing detected ***: terminated Aborted (core dump)
```
While the compiler defense successfully blocks an attacker from gaining control of instruction pointer, the execution terminates in a Segmentation Fault. This results in a complete Denial of Service(DoS) for the application.

---

## Remediation & Hardening Guide
To protect the binary against memory corruption and framework execution explotation, the following defensive engineering priciples must be enforced/followd:

### 1. Enforce strict Verifcation: 
Depreceate loops only checking for `\n`. Implement robust, standard bounded string operations such as `fgets()` or `strncpy()`, which require a max length modifier.

### 2. Explicit Null Termination: 
Ensure that all arrays handling string manipulation reserve the final byte `(buffer[len -1] = '\0')` to prevent buffer bleeding and out-of-bounds memory reads.

### 3. Compiler Mitigation Defenses: 
Ensure that code is compiled with modern defensive flags enabled:
** ASLR (Address Space Layout Randomization) 
** DEP/NX (Data Execution Prevention)
** SSP (Stack Smashing Protector / Compiler Canaries)

