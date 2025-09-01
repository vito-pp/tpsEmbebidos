# Rules

This repository follows the **Google C Style Guide** and uses **Doxygen** for documentation.

---

## Style Guide

- **File Organization**
  - One `.c` file per implementation.
  - Matching `.h` file for declarations.
- **Naming**
  - Constants: `kConstantName`
  - Macros: `MACRO_NAME`
  - Types: `TypeName`
  - Functions: `FunctionName`
  - Variables: `variable_name`
- **Formatting**
  - Line length: max 80–100 characters.
  - Braces: Allman style.
    ```
       foo (x)
       { 
             ... ;
       }
    ```
- **Pointers**
  - Attach `*` to variable, not type (`int *ptr;`).
- **Comments**
  - Use `//` for single line, `/* ... */` for multi-line.

---

## Doxygen Documentation

Use **Doxygen** to generate documentation from annotated source code.

### Example

```c
/**
 * @file gpio.h
 * @brief GPIO driver interface.
 */

/**
 * @brief Initializes the GPIO module.
 *
 * Sets up registers and enables clock gating.
 *
 * @param port The GPIO port to initialize.
 * @return 0 on success, negative on error.
 */
int GPIO_Init(int port);

