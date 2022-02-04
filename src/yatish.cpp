#include "yatish.hpp"

static const yatish_folder froot{0, "ROOT", 0} /*, fdesktop{1, "DESKTOP", 0}*/;
const yatish_folder *const root = &froot /*, *const desktop = &fdesktop*/;
const char *yatish_savename = "YATISH01";
const char *yatish_prgmname = "YATISH";

// static const uint8_t default_password[8] = {sk_Yequ, sk_Window, sk_Zoom, sk_Trace, sk_Graph, 0, 0, 0};

static yatish_save *save;

static char buf[512];

template <typename T>
static void swap(T &a, T &b)
{
    T c = a;
    a = b;
    b = c;
}

template <typename Iter1, typename Iter2>
static void iter_swap(Iter1 a, Iter2 b) { swap(*a, *b); }

static uint32_t djb2(const char *str)
{
    uint32_t hash = 5381;
    char c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

constexpr yatish_entrytype yatish_TypeToFiletype(uint8_t ti_type)
{
    if (ti_type == TI_APPVAR_TYPE)
        return yatish_entrytype_appvar;
    if (ti_type == TI_PRGM_TYPE)
        return yatish_entrytype_nprgm;
    if (ti_type == TI_PPRGM_TYPE)
        return yatish_entrytype_pprgm;
    return yatish_entrytype_unknown;
}

constexpr uint8_t yatish_FiletypeToType(yatish_entrytype type)
{
    if (type == yatish_entrytype_appvar)
        return TI_APPVAR_TYPE;
    if (type == yatish_entrytype_nprgm)
        return TI_PRGM_TYPE;
    if (type == yatish_entrytype_pprgm)
        return TI_PPRGM_TYPE;
    return 0;
}

constexpr yatish_folder *yatish_GetFolder(yatish_save *save, uint8_t id)
{
    yatish_folder *f = save->folders;
    uint8_t nfolds = save->num_folders;
    while (nfolds--)
    {
        if (f->id == id)
            return f;
        f++;
    }
    return NULL;
}

uint16_t yatish_FolderList(yatish_save *save, uint8_t par, yatish_entry *entries, uint16_t maxentries, uint16_t skip, yatish_entrytype typ)
{
    if (!maxentries)
        return 0;
    if (typ & yatish_entrytype_folder)
    {
        yatish_folder *fold = save->folders;
        uint16_t nfolds = save->num_folders;
        while (nfolds--)
        {
            if (fold->par == par)
            {
                if (skip)
                    skip--;
                else
                {
                    entries->typ = yatish_entrytype_folder;
                    entries->ptr.folder = fold;
                    entries++;
                    maxentries--;
                    if (!maxentries)
                        return 0;
                }
            }
            fold++;
        }
    }
    if (typ & yatish_entrytype_files)
    {
        yatish_file *file = save->files;
        uint16_t nfiles = save->num_files;
        while (nfiles--)
        {
            if (file->par == par && (typ & yatish_TypeToFiletype(file->typ)))
            {
                if (skip)
                    skip--;
                else
                {
                    entries->typ = yatish_TypeToFiletype(file->typ);
                    entries->ptr.file = file;
                    entries++;
                    maxentries--;
                    if (!maxentries)
                        return 0;
                }
            }
            file++;
        }
    }
    return maxentries;
}

static const yatish_file *yatish_FindInSave(const yatish_save *save, uint8_t typ, const char *name)
{
    uint16_t n = save->num_files;
    const yatish_file *f = save->files;
    while (n--)
    {
        if (f->typ == typ && !strcmp(f->name, name))
            return f;
        f++;
    }
    if ((typ == TI_PRGM_TYPE || typ == TI_PPRGM_TYPE) && !(name[0] & 64))
    {
        strcpy(buf, name);
        buf[0] ^= 64;
        return yatish_FindInSave(save, typ, buf);
    }
    return NULL;
}

static int yatish_Compare(const void *a, const void *b)
{
    const char *namea = ((const yatish_folder *)a)->name, *nameb = ((const yatish_folder *)b)->name;
    if (!(namea[0] & 64))
    {
        memcpy(buf, namea, 8);
        namea = buf;
    }
    if (!(nameb[0] & 64))
    {
        memcpy(buf + 64, nameb, 8);
        nameb = buf + 64;
    }
    return strncmp(namea, nameb, 8);
}

static void yatish_SortFolders(yatish_save *save)
{
    qsort(save->folders, save->num_folders, sizeof(yatish_folder), yatish_Compare);
}

static void yatish_SortFiles(yatish_save *save)
{
    qsort(save->files, save->num_files, sizeof(yatish_file), yatish_Compare);
}

// -=-=-=-=-=-=-=-=-

bool yatish_Init(bool enter)
{
    var_t *v = os_GetAppVarData(yatish_savename, NULL);
    if (v == NULL)
    {
        v = os_CreateAppVar(yatish_savename, sizeof(yatish_save));
        // memcpy(((yatish_save *)v->data)->password, default_password, 8);
        ((yatish_save *)v->data)->pwdhash = 213684212U;
        ((yatish_save *)v->data)->num_files = ((yatish_save *)v->data)->num_folders = 0;
    }
    save = (yatish_save *)malloc(v->size);
    if (save == NULL)
        os_ThrowError(OS_E_MEMORY);
    memcpy(save, v->data, v->size);

    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(palette, sizeof_palette, 0);
    var_t *x = os_GetAppVarData("YATISH02", NULL);
    if (x != NULL)
        gfxutils_SetBackgroundSprite((const gfx_sprite_t *)x->data);
    // gfxutils_SetBackgroundSprite(bgimage);
    gfxutils_SetBackgroundColor(255);
    gfxutils_SetForegroundColor(0);
    gfxutils_ClearScreen();
    if (enter)
    {
        gfxutils_GetTextInput("PASSWORD", buf, 8, 3);
        if (djb2(buf) != save->pwdhash)
        {
            gfxutils_ClearScreen();
            gfxutils_TextBox("ERROR", 1, (const char *[]){"Password incorrect!"});
            gfx_SwapDraw();
            while (!os_GetCSC())
                ;
            return false;
        }
    }
    gfxutils_TextBox("STATUS", 1, (const char *[]){"Please wait..."});
    gfx_SwapDraw();

    uint8_t typ;
    void *ptr = NULL;
    char *name;
    while ((name = ti_DetectAny(&ptr, NULL, &typ)))
    {
        if (typ != TI_APPVAR_TYPE && typ != TI_PRGM_TYPE && typ != TI_PPRGM_TYPE)
            continue;
        if (yatish_FindInSave(save, typ, name) == NULL)
        {
            // if (!(name[0] & 64))
            // {
            //     strcpy(buf, name);
            //     buf[0] ^= 64;
            //     name = buf;
            // }
            save = (yatish_save *)realloc(save, sizeof(yatish_save) + (save->num_files + 1) * sizeof(yatish_file));
            if (save == NULL)
                os_ThrowError(OS_E_MEMORY);
            strcpy(save->files[save->num_files].name, name);
            save->files[save->num_files].typ = typ;
            save->files[save->num_files].par = root->id;
            save->num_files++;
        }
    }
    for (uint16_t i = 0; i < save->num_files; i++)
    {
        if (!os_ChkFindSym(save->files[i].typ, save->files[i].name, NULL, NULL))
        {
            swap(save->files[i], save->files[save->num_files - 1]);
            // save = (yatish_save *)realloc(save, sizeof(yatish_save) + (save->num_files - 1) * sizeof(yatish_file));
            save->num_files--;
        }
    }
    yatish_SortFolders(save);
    yatish_SortFiles(save);

    gfxutils_ClearScreen();
    if (enter)
    {
        sprintf(buf, "Files: %u", save->num_files);
        sprintf(buf + 32, "Folders: %u", save->num_folders);
        gfxutils_TextBox("STATUS", 2, (const char *[]){buf, buf + 32});
        gfx_SwapDraw();
        while (!os_GetCSC())
            ;
        gfxutils_ClearScreen();
    }
    gfx_SwapDraw();
    return true;
}

void yatish_Deinit()
{
    ti_Delete(yatish_savename);
    uint16_t varsize = sizeof(yatish_save) + save->num_files * sizeof(yatish_file);
    var_t *v = os_CreateAppVar(yatish_savename, varsize);
    if (v == NULL)
    {
        free(save);
        os_ThrowError(OS_E_MEMORY);
    }
    memcpy(v->data, save, varsize);
    free(save);
    ti_var_t a = ti_Open(yatish_savename, "r");
    ti_SetArchiveStatus(true, a);
    ti_Close(a);
    gfx_End();
}

static uint8_t flag = 0;
#define yatish_inclock ((uint8_t)(1 << 0))
#define yatish_instart ((uint8_t)(1 << 1))
static uint8_t start_entry;
const char *start_entries[] = {"Explorer", "Settings", "Quit"};
#define START_ENTRIES (sizeof(start_entries) / sizeof(start_entries[0]))

static void yatish_Draw()
{
    gfxutils_ClearScreen();
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, LCD_HEIGHT - 12, LCD_WIDTH, 12);
    if (flag & yatish_inclock)
    {
        gfx_FillRectangle_NoClip(LCD_WIDTH - 118, LCD_HEIGHT - 32, 114, 20);
        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(LCD_WIDTH - 114, LCD_HEIGHT - 28);
        gfx_PrintUInt(rtc_Hours, 2);
        gfx_PrintChar(':');
        gfx_PrintUInt(rtc_Minutes, 2);
        gfx_PrintChar(':');
        gfx_PrintUInt(rtc_Seconds, 2);
        gfx_SetTextScale(1, 1);
    }
    if (flag & yatish_instart)
    {
        gfx_FillRectangle_NoClip(0, LCD_HEIGHT - 120, 120, 108);
        gfx_PrintStringXY("Yet Another", 17, LCD_HEIGHT - 116);
        gfx_PrintStringXY("TI Shell", 29, LCD_HEIGHT - 106);
        for (uint8_t i = 0; i < START_ENTRIES; i++)
        {
            if (start_entry == i)
            {
                gfxutils_InvertColors();
                gfx_SetColor(0);
            }
            gfx_FillRectangle_NoClip(0, LCD_HEIGHT - 95 + 20 * i, 120, 20);
            gfx_PrintStringXY(start_entries[i], 6, LCD_HEIGHT - 89 + 20 * i);
            if (start_entry == i)
            {
                gfxutils_InvertColors();
                gfx_SetColor(255);
            }
        }
    }
    gfx_SetColor(0);
    gfx_VertLine_NoClip(45, LCD_HEIGHT - 12, 12);
    gfx_PrintStringXY("START", 2, LCD_HEIGHT - 10);
    gfx_SetTextXY(LCD_WIDTH - 40, LCD_HEIGHT - 10);
    gfx_PrintUInt(rtc_Hours, 2);
    gfx_PrintChar(':');
    gfx_PrintUInt(rtc_Minutes, 2);
}

static sk_key_t getkey()
{
    sk_key_t k;
    while (!(k = os_GetCSC()))
        ;
    return k;
}

static uint16_t yatish_FilterEntries(const yatish_save *save, yatish_entrytype filter)
{
    uint16_t ret = 0;
    if (filter & yatish_entrytype_folder)
        ret += save->num_folders;
    if (filter & yatish_entrytype_files)
        for (uint16_t i = 0; i < save->num_files; i++)
            if (filter & yatish_TypeToFiletype(save->files[i].typ))
                ret++;
    return ret;
}

static int yatish_Callback(void *data, int)
{
    os_GetCSC();
    yatish_Init(false);
    yatish_Draw();
    yatish_FileExplorer(*(uint16_t *)data);
    yatish_Run();
    yatish_Deinit();
    return 0;
}

#define ENTRIES_PER_SCREEN 11

static yatish_entry entries[ENTRIES_PER_SCREEN + 1];
static yatish_entry clipboard;
static bool clipmode = false;

static void yatish_DeleteEntry(yatish_entry &entry)
{
    if (entry.typ == yatish_entrytype_folder && !yatish_FolderList(save, entry.ptr.folder->id, entries + ENTRIES_PER_SCREEN, 1, 0, yatish_entrytype_all))
    {
        gfxutils_TextBox("ERROR", 3, (const char *[]){"Folder not empty.", "Please empty folder", "and try again."});
        gfx_SwapDraw();
        while (!os_GetCSC())
            ;
    }
    else if (gfxutils_Select("DELETE?", 2, no_yes) == 1)
    {
        if (entry.typ == yatish_entrytype_folder)
        {
            uint8_t id = entry.ptr.folder->id;
            iter_swap(entry.ptr.folder, save->folders + save->num_folders - 1);
            for (id++; id < save->num_folders; id++)
                yatish_GetFolder(save, id)->id--;
            save->num_folders--;
            yatish_SortFolders(save);
        }
        else
        {
            ti_DeleteVar(entry.ptr.file->name, entry.ptr.file->typ);
            iter_swap(entry.ptr.file, save->files + --save->num_files);
            save = (yatish_save *)realloc(save, sizeof(yatish_save) + save->num_files * sizeof(yatish_file));
            yatish_SortFiles(save);
        }
    }
}

static void yatish_CreateFolder(const yatish_folder *par)
{
    if (save->num_folders == 255)
    {
        gfxutils_TextBox("ERROR", 2, (const char *[]){"Maximum folders (255)", "reached."});
        while (!os_GetCSC())
            ;
        return;
    }
    gfxutils_GetTextInput("NAME", buf, 8, 3);
    save->folders[save->num_folders].id = save->num_folders + 1;
    strcpy(save->folders[save->num_folders].name, buf);
    save->folders[save->num_folders].par = par->id;
    save->num_folders++;
}

static void yatish_Paste(const yatish_folder *cur)
{
    if (!clipmode)
    {
        gfxutils_TextBox("ERROR", 1, (const char *[]){"Nothing in clipboard!"});
        gfx_SwapDraw();
        while (!os_GetCSC())
            ;
    }
    else
    {
        clipboard.ptr.file->par = cur->id;
    }
}

void yatish_FileExplorer(uint16_t sel)
{
    flag = 0;

    yatish_entrytype show = yatish_entrytype_all;
    uint16_t skip = sel;
    const yatish_folder *cur = root;

    while (true)
    {
        yatish_Draw();
        gfx_SetColor(255);
        gfx_FillRectangle_NoClip(15, 15, LCD_WIDTH - 30, LCD_HEIGHT - 42);
        gfx_SetColor(0);
        gfx_HorizLine_NoClip(20, 31, LCD_WIDTH - 40);
        gfx_SetColor(255);
        gfx_PrintStringXY("File Explorer", 24, 20);
        uint8_t disp = ENTRIES_PER_SCREEN - yatish_FolderList(save, cur->id, entries, ENTRIES_PER_SCREEN, skip, show);
        while (disp--)
        {
            if (sel == skip + disp)
            {
                gfxutils_InvertColors();
                gfx_SetColor(0);
            }
            gfx_FillRectangle_NoClip(20, 35 + disp * 15, LCD_WIDTH - 75, 15);
            const char *name = entries[disp].ptr.folder->name;
            if ((entries[disp].typ & yatish_entrytype_files) && (!(name[0] & 64)))
            {
                strcpy(buf, name);
                buf[0] ^= 64;
                name = buf;
            }
            gfx_PrintStringXY(name, 35, 39 + disp * 15);
            if (entries[disp].typ == yatish_entrytype_folder)
            {
                gfx_Sprite_NoClip(folder_icon, 25, 40 + disp * 15);
            }
            else if (entries[disp].typ == yatish_entrytype_appvar)
            {
                gfx_Sprite_NoClip(file_icon, 26, 39 + disp * 15);
            }
            else
            {
                gfxutils_ColoredSprite(file_icon, 26, 39 + disp * 15, 2, 255);
            }
            // gfx_PrintString(" (Type: ");
            // gfx_PrintUInt(entries[disp].ptr.file->typ, 1);
            // gfx_PrintChar(')');
            if (sel == skip + disp)
            {
                gfxutils_InvertColors();
                gfx_SetColor(255);
            }
        }
        gfx_SwapDraw();

        sk_key_t key = getkey();
        gfxutils_ClearScreen();
        yatish_entry &selent = entries[sel - skip];
        if (key == sk_Clear)
            break;
        if (key == sk_Up)
        {
            if (sel != 0)
            {
                sel--;
                if (skip > sel)
                    skip--;
            }
        }
        else if (key == sk_Down)
        {
            if (sel != yatish_FilterEntries(save, show) - 2)
            {
                sel++;
                if (sel >= skip + ENTRIES_PER_SCREEN)
                    skip++;
            }
        }
        else if (key == sk_Enter)
        {
            if (selent.typ & yatish_entrytype_files)
            {
                if (selent.ptr.file->typ == TI_PRGM_TYPE || selent.ptr.file->typ == TI_PPRGM_TYPE)
                {
                    if (!strcmp(selent.ptr.file->name, "YATISH"))
                    {
                        gfxutils_TextBox("NOPE!", 2, (const char *[]){"Please do NOT run", "YATISH from YATISH."});
                        gfx_SwapDraw();
                        while (!os_GetCSC())
                            ;
                    }
                    else
                    {
                        strcpy(buf, selent.ptr.file->name);
                        yatish_Deinit();
                        os_RunPrgm(buf, &sel, sizeof(sel), yatish_Callback);
                    }
                }
            }
            else
            {
                cur = entries->ptr.folder;
            }
        }
        else if (key == sk_Alpha)
        {
            gfxutils_TextBox("WAITING", 2, (const char *[]){"Please type a", "letter key"});
            gfx_SwapDraw();
            sk_key_t k = getkey();
            if (capital_chars[k] >= 'A' && capital_chars[k] <= 'Z')
            {
                uint16_t ind = 0;
                while (ind < save->num_files)
                {
                    if (capital_chars[k] >= (save->files[ind].name[0] & 64))
                        break;
                    ind++;
                }
                if (ind == save->num_files)
                    ind--;
                sel = skip = ind;
            }
        }
        else if (key == sk_Del)
        {
            yatish_DeleteEntry(selent);
        }
        else if (key == sk_Mode)
        {
            uint8_t s = gfxutils_Select("ACTION", 5, (const char *[]){"Rename", "Cut", "Delete", "Paste", "New folder"});
            gfxutils_ClearScreen();
            switch (s)
            {
            case 0:
                strcpy(buf, selent.ptr.folder->name);
                gfxutils_GetTextInput("RENAME", selent.ptr.folder->name, 8, 3);
                if (selent.typ & yatish_entrytype_files)
                    ti_RenameVar(buf, selent.ptr.file->name, selent.ptr.file->typ);
                break;
            case 1:
                clipboard = selent;
                clipmode = true;
                break;
            case 2:
                yatish_DeleteEntry(selent);
                break;
            case 3:
                yatish_Paste(cur);
                break;
            case 4:
                yatish_CreateFolder(cur);
                break;
            }
        }
        else if (key == sk_Left)
        {
            if (cur != root)
            {
                if (cur->par == root->id)
                    cur = root;
                else
                    cur = yatish_GetFolder(save, cur->par);
            }
        }
    }
}

static void yatish_EditSettings()
{
    gfxutils_ClearScreen();
    gfxutils_GetTextInput("PASSWORD", buf, 8, 3);
    if (djb2(buf) != save->pwdhash)
    {
        gfxutils_ClearScreen();
        gfxutils_TextBox("ERROR", 1, (const char *[]){"Password incorrect!"});
        gfx_SwapDraw();
        while (!os_GetCSC())
            ;
        return;
    }
    while (true)
    {
        gfxutils_ClearScreen();
        uint8_t s = gfxutils_Select("Settings", 1, (const char *[]){"Set password"});
        if (s == 255)
            return;
        if (s == 0)
        {
            gfxutils_GetTextInput("PASSWORD", buf, 8, 3);
            save->pwdhash = djb2(buf);
        }
    }
}

void yatish_Run()
{
    while (true)
    {
        yatish_Draw();
        gfx_SwapDraw();

        sk_key_t key;
        uint8_t v = 75;
        while (!(key = os_GetCSC()) && v--)
            delay(10);
        if (key == sk_Clear)
            break;
        if (key == sk_Graph)
            flag ^= yatish_inclock;
        else if (key == sk_Yequ)
            flag ^= yatish_instart, start_entry = 0;
        else if (key == sk_Up)
        {
            if (flag & yatish_instart)
            {
                if (start_entry == 0)
                    start_entry = START_ENTRIES;
                start_entry--;
            }
        }
        else if (key == sk_Down)
        {
            if (flag & yatish_instart)
            {
                start_entry++;
                if (start_entry == START_ENTRIES)
                    start_entry = 0;
            }
        }
        else if (key == sk_Enter)
        {
            if (flag & yatish_instart)
            {
                if (start_entry == 0)
                    yatish_FileExplorer();
                else if (start_entry == 1)
                    yatish_EditSettings();
                else if (start_entry == 2)
                    break;
            }
            else
                yatish_FileExplorer();
        }
    }
}
