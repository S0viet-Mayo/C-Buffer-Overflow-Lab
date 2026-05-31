# Vulnerability Analysis: Stack-Based Buffer Overflow and Memory Corruption

## Project Overview
This repository contains a deep-dive security analysis and documentation of a stack-based buffer overflow and string handling vulnerability compiled target application ('buffy'). By analyzing the differences between boundede and unbounded input functions, this lab demonstrats hands-on proficiency in low-level memory layouts, stack behaviors, and how software flaws directly violate the core pillars of the CIA triad (Confidentiality, Integrity, and Avalibility). 

The application processes user credintals in a maximum expected buffer size of 10 characters for adjacent 'username' and 'password' variables on the stack.

---

## Vulnerability Analysis & Code Mechanics

The application applications implements two input tracking behaviors: 'get_input_safe()' and 'get_input_unsafe()'.

### 1. Bounded Mitigation: 'get_input_safe()' The safe input enforces strict boundary validation logic: 

```text

while (count <len ){...}


