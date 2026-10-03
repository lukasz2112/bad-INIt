#ifndef INI_PARSER
#define INI_PARSER
#define IO_BUFFER_SIZE 512

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

typedef struct IniValue
{
    char *Name;
    char *Value;
} iniValue;

/*
 @attention May be NULL. Check for NULL before accessing values directly
*/
typedef struct iniSection
{
    char *Name;
    int value_amount;
    int local_value_amount;
    int section_amount;
    iniValue *Values;
} iniSection;

/*
 @attention May be NULL. Check for NULL before accessing values directly
*/
typedef iniSection iniFile;

bool ini_write_file(iniFile *ini, char *filename);
bool ini_create_key(iniFile **ini_pnt, char *section, char *name, char *value);
bool ini_create_section(iniFile **ini, char *name);
bool ini_remove_key(iniFile *ini, char *section, char *key);
bool ini_remove_section(iniFile **ini_pnt, char *section);
int ini_get_section_index_by_name(iniFile *ini, char *name);
const char *ini_get_key_value_by_name(iniFile *ini, char *section, char *key);
void ini_free(iniFile *ini);
unsigned strstrip(char *s);
iniFile *ini_get_file(FILE *file) __attribute__((warn_unused_result));

/* Parses a file into an array of iniSection structs. The returned value must be freed with ini_free()
 * @attention The returned value must be freed with ini_free()
 * @param file   File to parse
 * @return   Pointer to a dynamically-allocated array of iniSection (iniFile) structs
 */
iniFile *ini_get_file(FILE *file)
{
    iniFile *dest;
    // rewind file pointer
    rewind(file);
    // input buffer
    dest = malloc(sizeof(iniFile));
    int buf = 0;
    char name_buf[IO_BUFFER_SIZE];
    int section_n = 0, key_n = 0, local_key_n = 0;
    while (buf != EOF)
    {
        // dest[section_n + 1].Values = NULL;

        buf = fgetc(file);
        if (buf == ';' || buf == '#') // ignore comments
        {
            while (fgetc(file) != '\n')
                continue;
        }
        if (buf == '[') // get section name
        {
            int i = 0;
            local_key_n = 0;
            name_buf[i] = fgetc(file);
            while (name_buf[i] != ']' && i < IO_BUFFER_SIZE - 1)
            {
                i++;
                name_buf[i] = fgetc(file);
            }
            // if ']' wasn't reached, advance file pointer until newline
            if (!(i < IO_BUFFER_SIZE - 1))
                while (fgetc(file) != '\n')
                    continue;
            // replace ']' with a string terminator
            name_buf[i] = '\0';
            //   allocate memory
            dest = realloc(dest, (section_n + 1) * sizeof(iniSection));
            dest[section_n].Values = NULL;
            dest[section_n].local_value_amount = 0;
            dest[section_n].Name = malloc(strlen(name_buf) * sizeof(char) + 1);

            strcpy(dest[section_n].Name, name_buf);
            // printf("%lu\n", strlen(dest[n].Name));
            section_n++;
        }
        if (isalnum(buf)) // get key name value
        {

            int i = 0, actual_section_n = section_n - 1;
            name_buf[i] = buf;
            while (name_buf[i] != '=' && i < IO_BUFFER_SIZE - 1)
            {
                i++;
                name_buf[i] = fgetc(file);
            }
            name_buf[i] = '\0';
            // allocate memory

            dest[actual_section_n].Values = realloc(dest[actual_section_n].Values, (local_key_n + 1) * sizeof(iniValue));
            strstrip(name_buf);
            dest[actual_section_n].Values[local_key_n].Name = malloc(strlen(name_buf) * sizeof(char) + 1);
            strcpy(dest[actual_section_n].Values[local_key_n].Name, name_buf);
            i = 0;
            name_buf[i] = fgetc(file);
            while (buf != '\n' && buf != ';' && i < IO_BUFFER_SIZE - 1 && buf != EOF)
            {
                i++;
                buf = fgetc(file);
                name_buf[i] = buf;
            }
            /*do
            {
                i++;
                buf = fgetc(file);
                name_buf[i] = buf;
            } while (buf != '\n' && !(buf == ';' && isspace(name_buf[i - 1])) && i < IO_BUFFER_SIZE - 1 && buf != EOF);*/
            if (name_buf[i] != '\n')
                while (fgetc(file) != '\n')
                    continue;
            name_buf[i] = '\0';
            strstrip(name_buf);
            dest[actual_section_n].Values[local_key_n].Value = malloc(strlen(name_buf) * sizeof(char) + 1);
            strcpy(dest[actual_section_n].Values[local_key_n].Value, name_buf);
            local_key_n++;
            dest[actual_section_n].local_value_amount = local_key_n;
            key_n++;
        }
    }
    dest->section_amount = section_n;
    dest->value_amount = key_n;
    if (section_n == 0)
    {
        dest->local_value_amount = 0;
        dest->Name = NULL;
        dest->Values = NULL;
    }
    return dest;
}

unsigned strstrip(char *s)
{
    char *last = NULL;
    char *dest = s;

    if (s == NULL)
        return 0;

    last = s + strlen(s);
    while ((isspace((unsigned char)*s) || *s == '"') && *s)
        s++;
    while (last > s)
    {
        if (!isspace((unsigned char)*(last - 1)) && *(last - 1) != '"')
            break;
        last--;
    }
    *last = (char)0;

    memmove(dest, s, last - s + 1);
    return last - s;
}

/* Frees an array of iniSection (iniFile) structs. Does nothing if ini is NULL
 * @param ini iniFile to free
 */
void ini_free(iniFile *ini)

{
    if (ini == NULL)
        return;

    int total_sections = ini[0].section_amount;
    for (int i = 0; i < total_sections; i++)
    {
        free(ini[i].Name);
        for (int k = 0; k < ini[i].local_value_amount; k++)
        {
            free(ini[i].Values[k].Name);
            free(ini[i].Values[k].Value);
        }
        free(ini[i].Values);
    }
    free(ini);
}

/* Gets the array index of the iniSection in "ini" that is named "name"
 * @param ini iniFile to look in
 * @param name The name to look for
 * @return On failure, or if the section is not found, returns -1. On success, returns the array index of the iniSection in "ini" that is named "name"
 */
int ini_get_section_index_by_name(iniFile *ini, char *name)
{
    if (ini == NULL)
    {
        return -1;
    }
    for (int i = 0; i < ini->section_amount; i++)
    {
        if (ini[i].Name != NULL && strcmp(name, ini[i].Name) == 0)
            return i;
    }
    return -1;
}

/* Gets the array index of the iniValue in a specified section that carries a specified name
 * @param ini iniFile to look in
 * @param section the IniSection to look in
 * @param key the name of the key to look for
 * @return On failure, or if the key is not found, returns -1. On success, returns the array index of the iniValue in a specified section that carries a specified name
 */
int ini_get_key_index_by_name(iniFile *ini, char *section, char *key)
{
    int i = ini_get_section_index_by_name(ini, section);
    if (i == -1)
        return -1;
    int ii = 0;
    while (ii < ini[i].local_value_amount && strcmp(key, ini[i].Values[ii].Name) != 0)
        ii++;
    if (strcmp(key, ini[i].Values[ii].Name) != 0)
        return -1;
    return ii;
}

/* Gets the value of key in section "section" that is named "key"
 * @param ini iniFile to look in
 * @param section The INI section too look in
 * @param key The key to look for
 * @return On failure, or if the key is not found, returns NULL. On success, returns a pointer to key's value inside the supplied iniFile struct. The returned string will be freed with the rest of the iniFile and must not be modified or freed seperately
 */
const char *ini_get_key_value_by_name(iniFile *ini, char *section, char *key)
{

    int i = ini_get_section_index_by_name(ini, section);
    if (i == -1)
        return NULL;
    int ii = 0;
    while (ii < ini[i].local_value_amount && strcmp(key, ini[i].Values[ii].Name) != 0)
        ii++;
    if (strcmp(key, ini[i].Values[ii].Name) != 0)
        return NULL;
    return ini[i].Values[ii].Value;
}

/* Create a section with the specified name
 * @param ini_pnt Pointer to a pointer to an iniFile
 * @param name The name of the to-be-created section
 * @return On failure returns false (0). On success returns true (1)
 */
bool ini_create_section(iniFile **ini_pnt, char *name)
{
    iniFile *ini = *ini_pnt;
    if (ini == NULL || name == NULL)
        return false;
    void *tmp;
    tmp = realloc(ini, (ini->section_amount + 1) * sizeof(iniSection));
    if (tmp == NULL)
        return false;
    ini = tmp;
    ini[ini->section_amount].Name = malloc(strlen(name) * sizeof(char) + 1);
    ini[ini->section_amount].Values = NULL;
    ini[ini->section_amount].local_value_amount = 0;
    strcpy(ini[ini->section_amount].Name, name);
    ini->section_amount += 1;
    *ini_pnt = ini;
    return true;
}

/* Create a key
 * @param ini_pnt Pointer to a pointer to an iniFile
 * @param section The name of the section to place the to-be-created key into
 * @param name The name is the to-be-created key
 * @param value The value of the to-be-created key
 * @return On failure returns false (0). On success returns true (1)
 */
bool ini_create_key(iniFile **ini_pnt, char *section, char *name, char *value)
{
#define ini_Values ini[section_index].Values[ini[section_index].local_value_amount]
    iniFile *ini = *ini_pnt;
    int section_index = ini_get_section_index_by_name(ini, section);
    void *tmp;
    tmp = realloc(ini[section_index].Values, (ini[section_index].local_value_amount + 1) * sizeof(iniValue));
    if (tmp == NULL)
        return false;
    // an additional pointer will make this cleaner but im lazy
    ini[section_index].Values = tmp;
    ini_Values.Name = malloc(strlen(name) * sizeof(char) + 1);
    if (ini_Values.Name == NULL)
        return false;
    strcpy(ini_Values.Name, name);
    ini_Values.Value = malloc(strlen(value) * sizeof(char) + 1);
    if (ini_Values.Value == NULL)
        return false;
    strcpy(ini_Values.Value, value);
    ini[section_index].local_value_amount++;
    return true;
}

/* Be careful – overwrites specified file! Write iniFile as a valid INI file to a specified file
 * @param ini iniFile to write. If ini is NULL, erases the file and returns true
 * @param filename File to write to (file extension is not appended automatically)
 * @return On failure returns false (0). On success returns true (1)
 */
bool ini_write_file(iniFile *ini, char *filename)
{
    if (filename == NULL)
        return false;
    FILE *file = fopen(filename, "w");
    if (file == NULL)
        return false;
    if (ini == NULL)
    {
        fclose(file);
        return true;
    }
    for (int i = 0; i < ini->section_amount; i++)
    {
        fprintf(file, "[%s]\n", ini[i].Name);
        for (int ii = 0; ii < ini[i].local_value_amount; ii++)
            fprintf(file, "%s = %s\n", ini[i].Values[ii].Name, ini[i].Values[ii].Value);
    }
    fclose(file);
    return true;
}

/* Removes a section from an iniFile, including all keys assigned to that section.
 * @param ini_pnt Pointer to a pointer to an iniFile to operate on (e.g. &my_ini)
 * @param section Section to remove
 * @return On failure returns false (0). On success returns true (1). Terminates the program if memory reallocation fails.
 */
bool ini_remove_section(iniFile **ini_pnt, char *section)
{
    if (ini_pnt == NULL || *ini_pnt == NULL || section == NULL)
        return false;
    iniFile *ini = *ini_pnt;
    void *tmp;
    int index = ini_get_section_index_by_name(ini, section);
    if (index == -1)
        return false;
    // these variables are needed in case the section in question is at index 0
    int local_value_n = ini[index].local_value_amount,
        section_n = ini->section_amount, value_n = ini->value_amount;
    // get the pointer to the to-be-removed section
    iniSection *section_pnt = &ini[index];
    // free some of the allocated memory
    free(ini[index].Name);
    for (int k = 0; k < ini[index].local_value_amount; k++)
    {
        free(ini[index].Values[k].Name);
        free(ini[index].Values[k].Value);
    }
    free(ini[index].Values);
    if (section_n == 1)
    {
        free(ini);
        *ini_pnt = NULL;
        return true;
    }
    // shrink the array
    // realloc() shrinks from the end, so we need to overwrite removed section
    // by moving the remaining array elements left
    memmove(section_pnt, section_pnt + 1, (section_n - index - 1) * sizeof(iniSection));
    // section_pnt is dangling after we call realloc(), so set it to NULL
    section_pnt = NULL;
    // shrink allocated memory
    tmp = realloc(ini, (section_n - 1) * sizeof(iniSection));
    // if realloc() failed, exit (ini_pnt is now corrupted and WILL cause a memory bugs)
    if (!tmp)
    {
        printf("FATAL: ini_remove_section(): realloc() failed: %s\n", strerror(errno));
        exit(errno);
    }
    ini = tmp;
    // correct variables
    ini->section_amount = --section_n;
    ini->value_amount = value_n - local_value_n;
    // overwrite the original pointer
    *ini_pnt = ini;
    return true;
}

/* Removes a key from an iniFile.
 * @param ini iniFile to operate on
 * @param section Section of the to-be-removed key
 * @param key Key to remove
 * @return On failure returns false (0). On success returns true (1). Terminates the program if memory reallocation fails.
 */
bool ini_remove_key(iniFile *ini, char *section, char *key)
{
    if (ini == NULL || section == NULL || key == NULL)
        return false;
    int section_index = ini_get_section_index_by_name(ini, section),
        key_index = ini_get_key_index_by_name(ini, section, key);
    if (section_index == -1 || key_index == -1)
        return false;
    iniValue *key_pnt = &ini[section_index].Values[key_index];
    void *tmp;
    // free some of the allocated memory
    free(key_pnt->Name);
    free(key_pnt->Value);
    // shrink the array
    // realloc() shrinks from the end, so we need to overwrite removed section
    // by moving the remaining array elements left
    memmove(key_pnt, key_pnt + 1, (ini[section_index].local_value_amount - key_index - 1) * sizeof(iniValue));
    tmp = realloc(ini[section_index].Values, (ini[section_index].local_value_amount - 1) * sizeof(iniSection));
    // if realloc() failed, exit (key_pnt is now corrupted and WILL cause memory bugs)
    if (!tmp)
    {
        printf("FATAL: ini_remove_key(): realloc() failed: %s\n", strerror(errno));
        exit(errno);
    }
    // update variables
    ini[section_index].Values = tmp;
    ini[section_index].local_value_amount--;
    ini->value_amount--;
    // return true
    return true;
}

/* Modifies the value of a key
 * @param ini iniFile to operate on
 * @param section Section of the to-be-modified key
 * @param key Key to modify
 * @param new_value The value to assign to the key
 * @return On failure returns false (0). On success returns true (1). Terminates the program if memory reallocation fails.
 */
bool ini_modify_key_value(iniFile *ini, char *section, char *key, char *new_value)
{
#define KEY_VALUE ini[section_index].Values[key_index].Value
    if (new_value == NULL || ini == NULL)
        return false;
    int section_index = ini_get_section_index_by_name(ini, section);
    if (section_index == -1)
    {
        return false;
    }
    int key_index = ini_get_key_index_by_name(ini, section, key);
    if (section_index == -1)
    {
        return false;
    }
    if (strlen(KEY_VALUE) != strlen(new_value))
    {
        void *tmp;
        tmp = realloc(KEY_VALUE, strlen(new_value) + 1);
        if (!tmp)
            return false;
        KEY_VALUE = tmp;
    }
    strcpy(KEY_VALUE, new_value);
    return true;
}

#endif