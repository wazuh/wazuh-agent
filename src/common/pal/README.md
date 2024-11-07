# Platform Abstraction Layer (PAL)

The aim of this module is to provide a the tools necessary to independize wazuh agent's code from the platform it will be compile for.
Since we are currently using GNU C for Linux. AppleClang for MacOS and MSVC for Windows, this will be used to identify the correspoding platform during compile time.

### Hierarchy

```console
pal
 ├── README.md
 ├── CMakeLists.txt
 ├── include 
 │   ├── Linux
 │   │    ├── pal_file.h
 │   │    ├── pal_thread.h
 │   │    ├── pal_time.h
 │   │    └── pal.h
 │   ├── macos
 │   │    ├── pal_file.h
 │   │    ├── pal_thread.h
 │   │    ├── pal_time.h
 │   │    └── pal.h
 │   └── windows
 │        ├── pal_file.h
 │        ├── pal_thread.h
 │        ├── pal_time.h
 │        └── pal.h
 └── src
     ├── Linux
     │    ├── pal_file.c
     │    ├── pal_thread.c
     │    ├── pal_time.c
     │    └── pal.c
     ├── macos
     │    ├── pal_file.c
     │    ├── pal_thread.c
     │    ├── pal_time.c
     │    └── pal.c
     └── windows
          ├── pal_file.c
          ├── pal_thread.c
          ├── pal_timee.c
          └── pal.c
```

### Usage

A commom public file named "pal.h" should be included in every file that will be platform dependant. No more includes should be necessary.
Thos definitions, prototypes, etc, provided by the PAL should be used in Wazuh agent's code, avoiding whenever possible the usage of pre-processor in higher level code.
If situations not considered in the PAL are found during development, it should be prioritized to extend the PAL than taking shortcuts as ad-hoc solutions, except if this option is no available for implementation reasons.