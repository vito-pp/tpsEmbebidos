# Rules

---

## Style Guide

- **File Organization**
  - One `.c` file per implementation.
  - Matching `.h` file for declarations.
- **Naming**
  - Constants: `kConstantName`
  - Macros: `MACRO_NAME`
  - Typedefs: `TypeName_t`
  - Enum with tiypedefs: `EnumName_e`
  - Functions: `functionName`
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
  - Comments in english.
---

## Doxygen Documentation

Use **Doxygen** format to document code.

### Example

```c
/**
 * @file foo.h
 * @brief Super cool foo API.
 */

/**
 * @brief Explain what foo does.  
 * @param var Explain what var is for.
 * @return true on success, false on error.
 */
bool foo(int var);
