#ifndef _YATISH_H
#define _YATISH_H

#include <tice.h>
#include <fileioc.h>
#include <string.h>
#include "gfxutils.h"
#include "gfx/gfx.h"

#pragma pack(push, 1)

struct yatish_folder
{
    uint8_t id;
    char name[9];
    uint8_t par;
};

enum yatish_entrytype : uint8_t
{
    yatish_entrytype_unknown = 0b0000,
    yatish_entrytype_folder = 0b0001,
    yatish_entrytype_appvar = 0b0010,
    yatish_entrytype_nprgm = 0b0100,
    yatish_entrytype_pprgm = 0b1000,
    yatish_entrytype_files = 0b1110,
    yatish_entrytype_all = 0b1111
};

constexpr yatish_entrytype yatish_TypeToFiletype(uint8_t ti_type);
constexpr uint8_t yatish_FiletypeToType(yatish_entrytype type);

struct yatish_file
{
    uint8_t typ;
    char name[9];
    uint8_t par;
};

struct yatish_save
{
    uint32_t pwdhash;
    uint8_t num_folders;
    yatish_folder folders[256];
    uint16_t num_files;
    yatish_file files[0];
};

struct yatish_entry
{
    yatish_entrytype typ;
    union
    {
        yatish_folder *folder;
        yatish_file *file;
    } ptr;
};

extern const yatish_folder *const root /*, *const desktop*/;
extern const char *yatish_savename, *yatish_prgmname;

constexpr yatish_folder *yatish_GetFolder(yatish_save *save, uint8_t id);
/**
 * @returns Number of entries NOT found.
 */
uint16_t yatish_FolderList(yatish_save *save, uint8_t par, yatish_entry *entries, uint16_t maxentries, uint16_t skip, yatish_entrytype typ);

bool yatish_Init(bool enter = true);
void yatish_FileExplorer(uint16_t sel = 0);
void yatish_Run();
void yatish_Deinit();

#pragma pack(pop)
#endif