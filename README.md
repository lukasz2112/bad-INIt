# bad-INIt
A simple single-header INI (.conf) library I wrote in C with no AI usage.\
In its current state, **should not be used for production** (overallocates memory, may halt your program etc).

# Features:
Everything you'd expect from a CRUD parser
- Creating INI files
- Opening INI files
- Creating sections
- Creating values
- Modifying values
- Modifying value names (coming soon)
- Modifying section names (coming soon)
- Deleting sections
- Deleting values
- Writing INI files

# Architecture
(sorry for stupid variable names)\
(ASCII art by Gemini)

**Warning:** The value_amount and section_amount (but not local_value_amount) are only allocated in iniSection[0].\
In subsequent sections are **left uninitialized** (garbage). I know this is a not a good way to do this.

```
 iniFile (typedef to iniSection) (Dynamically Allocated Array)
 ┌──────────────────────────────────────────────────────────────────┐
 │ iniSection[0]                                                    │
 │  ├── char *Name                                                  │
 │  ├── int value_amount (the amount of values in all sections      |
 │  ├── int local_value_amount (the amount of values in this section|
 │  ├── int section_amount (the total amount of iniSections)        │
 │  └── iniValue *Values         ──┐                                │
 └─────────────────────────────────┼────────────────────────────────┘
                                   │
                                   ▼  (Dynamically Allocated Array)
                             ┌───────────────────────────────────┐
                             │ Values[0]                         │
                             │  ├── char *Name                   │
                             │  └── char *Value                  │
                             ├───────────────────────────────────┤
                             │ Values[1]                         │
                             │  ├── char *Name                   │
                             │  └── char *Value                  │
                             └───────────────────────────────────┘
```

# Usage
## Example
```
#include "ini_parser.h"
int main(void)
{
    FILE *file = fopen("test.ini", "w+b"); // create file
    iniFile *inifile = ini_get_file(file); // parse file
    ini_create_section(&inifile, "a section"); // create a section
    ini_create_key(&inifile, "a section", "a key", "a value"); // create a key
    for (int i = 0; i < inifile->section_amount; i++) // print the IniFile contents
    {
        printf("[%s]\n", inifile[i].Name);
        for (int ii = 0; ii < inifile[i].local_value_amount; ii++)
            printf("%s = %s\n", inifile[i].Values[ii].Name, inifile[i].Values[ii].Value);
    }
    ini_remove_key(inifile, "a section", "a key"); // remove a key
    ini_remove_section(&inifile, "a test"); // remove a section
    ini_write_file(inifile, "test.ini"); // write the file
    ini_free(inifile); // Free memory
    fclose(file); // close the file handle
}
```

# Contributing
First of all, I'm surprised that you decided to contribute, but remember that
fully AI-generated contributions are **banned** and to keep your PRs concise. Thanks.
