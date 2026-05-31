# Vulnerability Analysis: Stack-Based Buffer Overflow and Memory Corruption

## Project Overview
This repository contains a deep-dive security analysis and documentation of a stack-based buffer overflow and string handling vulnerability in a  compiled target application ('buffy'). By analyzing the differences between bounded and unbounded input functions, this lab demonstrats hands-on proficiency in low-level memory layouts, stack behaviors, and how software flaws directly violate the core pillars of the CIA triad (Confidentiality, Integrity, and Avalibility). 

The application processes user credintals in a maximum expected buffer size of 10 characters for adjacent 'username' and 'password' variables on the stack.

---

## Vulnerability Analysis & Code Mechanics

The application applications implements two input tracking behaviors: 'get_input_safe()' and 'get_input_unsafe()'.

### 1. Bounded Mitigation: 'get_input_safe()' The safe input enforces strict boundary validation logic: 

```c

while (count <len ){...}
```
Because count < len prevents the user from writing past the allocated buffer size, inputs that exceed the limit are safely terminated. Testing confirms when passing inputs larger than 10 characers, the buffer reserves space for the newline character (\n) or a null terminator, saftely restricting data retention to exactly 9 characters of a user input.

### 2. Unbound Flaw: 'get_input_safe()' The vulnerability lies within the unsafe input tracking loop, which lacks array bounds checking: 

```c 
while (c !='\n'){...}

```
This loop evaluates input termination based *only* when encountering a newline character (\n) via the enter key. It does not validate how many bytes are being written into the buffer. 

Furthermore, the application admits string NULL Terminators(\0). Because the C standard library string manupulation routines rely on finding a \0 byte to determine where the string ends, the absence of null termination forces operations to read across the continuous stack space. When the loop fails to find a terminator, it slams adjacent data structures together in memory, reading past the inteded boundaries. 

---

## Empirical Test Cases & Execution Observations

The application was subjected to three input tracting test vectors to map stack behavior. 

### Test Case A: Standard Normal Input
*Input Username: mjacks11(8 characters)
*Behavior: The input sits at 8 characters safely within the 10-byte boundary. The application executes normally without leaking memory.

### Test Case B: Boundary Collision & Adjacent Memory leak
*Input Username: david12345 (Exactly 10 Characters)
*Observed Output: ```text David12345david12345Secret:It could be bunnies!```
*Analysis: Because the input filled the entire buffer without leaving room for a null terminator, string printing routines bled directly into adjacent memory on the stack. This leaked the string payload of the password buffer and exposed and embedded application credential (Secret:It could be bunnies!).

### Test Case C: Arbitrary Memory Corruption & Overwrite
*Input Monologue: ifwardoesntchangemenmustchangeandsomusttheirsymbols
*Observed Output: ```text ifwardoesndddddddddddgeandsomusttheirsymbols```
*Analysis: The lack of bounds checking allowed the input string to overwrite adjacent variables. By shortening the input to ifwardoesnt and the password string to changemenm, the application confirmed memory alignment by printing the password injection cleanly inside of the corrupted username space. This revealed exactly where the variables neighbor each other on the stack. 

### Test Case D: Stack Smashing & Segmentation Fault
*Input: 50 random characters
*Observed Output: ```text Secret:It could be bunnies! ***stack smashing detected***: terminated Aboted (core dump).```
*Analysis: We've already stated before that the lack of bounds checking allows the input string to overwrite adjacent variables. When you have an input string with an insane amount of characters, it fills the username and password buffers (10 bytes each) and it keeps writing up the memory space until it hits the Stack Canary. The Stack Canary is a secret variable placed on the stack right above the our variables (Username and Password). Just before the program returns, the compiler inputs a hidden if statement that looks at the memory address of the canary. If the integrity check fails, instead of allowing the program to hit the retunr instruction, the program explicitly jumps to a built-in error handler function in the C standard library called __stack_chk_fail(). 
*The Catch: You may be asking, so why do we still see Buffer Overflows as a means of attack today? The Canary is not a physical barrier, it is an indirect tripwire. It only works if the attacker is playing by the rules. Attackers can absolutely cause the canary to leak it's own 8-byte value. When they craft their payload, when they make it to the canary in memory, they drop in it's 8-byte value which passes the integrity checks. Attackers can also brute force the canary on 32-bit systems. On 32-bit systems the canary is only 4-bytes long. To prevent it from being read by string functions the first byte is almost always a null byte (0x00), leaving only 3 bytes of randomness. 

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
Depreceate loops only checking for \n. Implement robust, standard bounded string operations such as fgets() or strncopy(), which require a max length modifier.

### 2. Explicit Null Termination: 
Ensure that all arrays handling string manipulation reserve the final byte (buffer[len -1] = '\0') to prevent buffer bleeding and out-of-bounds reads.

### 3. Compiler Mitigation Defenses: 
Ensure that code is compiled with modern defensive flags such as, ASLR (Address Space Layout Randomization), DEP/NX (Data Execution Prevention), and SSP (Stack Smashing Protector / Compiler Canaries).

