#include "global.h"
#include "ui_battle_menu.h"
#include "strings.h"
#include "bg.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_util.h"
#include "battle_script_commands.h"
#include "battle_anim.h"
#include "battle_ai_main.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "item_menu.h"
#include "item_menu_icons.h"
#include "list_menu.h"
#include "item_icon.h"
#include "item_use.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "overworld.h"
#include "event_data.h"
#include "constants/abilities.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/field_weather.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "mgba_printf/mgba.h"
#include "mgba_printf/mini_printf.h"
#include "trig.h"

//==========DEFINES==========//
enum
{
    SPRITE_ARR_ID_FIELD_ICON,
    SPRITE_ARR_ID_SELECTOR,
    SPRITE_ARR_ID_MON_ICON_1,
    SPRITE_ARR_ID_MON_ICON_2,
    SPRITE_ARR_ID_MON_ICON_3,
    SPRITE_ARR_ID_MON_ICON_4,
    SPRITE_ARR_ID_STATUS,
    //SPRITE_ARR_ID_TYPE_1,
    //SPRITE_ARR_ID_TYPE_2,
    //SPRITE_ARR_ID_TYPE_3,
    SPRITE_ARR_HELD_ITEM,
    SPRITE_ARR_ID_MON_ICON_1_SPEED,
    SPRITE_ARR_ID_MON_ICON_2_SPEED,
    SPRITE_ARR_ID_MON_ICON_3_SPEED,
    SPRITE_ARR_ID_MON_ICON_4_SPEED,
    NUM_SPRITES,
};

enum
{
    FIELD_INFO_WEATHER,
    FIELD_INFO_TERRAIN,
    FIELD_INFO_TRICK_ROOM,
    FIELD_INFO_GRAVITY,
    FIELD_INFO_WONDER_ROOM,
    FIELD_INFO_MAGIC_ROOM,
    NUM_FIELD_INFO,
};

struct MenuResources
{
    MainCallback savedCallback;     // determines callback to run when we exit. e.g. where do we want to go after closing the menu
    u8 gfxLoadState;
    u8 battlerId;
    u8 modeId;
    u8 tabId;
    u8 fieldTabId;
    u8 tabModeId;
    u8 moveModeId;
    u8 spriteIds[NUM_SPRITES];
    u8 fieldInfo[NUM_FIELD_INFO];
    u8 currentFieldInfo;
    u8 numFields;
};

enum WindowIds
{
    WINDOW_1,
};

enum battler_TabIds
{
    TAB_STATS,
    TAB_ABILITIES,
    TAB_MOVES,
    TAB_DAMAGE_CALCULATOR,
    TAB_STATUS,
    NUM_TABS,
};

enum field_TabsIds
{
    TAB_FIELD,
    TAB_SPEED,
    NUM_FIELD_TABS,
};

enum menu_colors
{
    MENU_COLOR_BLUE,
    MENU_COLOR_RED,
    MENU_COLOR_YELLOW,
    MENU_COLOR_GREEN,
    NUM_COLORS,
};

enum modes
{
    MODE_BATTLER2,
    MODE_BATTLER0,
    MODE_FIELD,
    MODE_BATTLER1,
    MODE_BATTLER3,
    NUM_MODES
};

enum move_moves
{
    MOVE_MODE_NORMAL,
    MOVE_MODE_DESCRIPTION,
    //MOVE_MODE_DAMAGE_CALCULATOR,
    MOVE_MODE_AFFECTED_1,
    MOVE_MODE_AFFECTED_2,
    NUM_MOVE_MODES
};

//==========EWRAM==========//
static EWRAM_DATA struct MenuResources *sMenuDataPtr = NULL;
static EWRAM_DATA u8 *sBg1TilemapBuffer = NULL;

//==========STATIC=DEFINES==========//
static void Battle_Menu_RunSetup(void);
static bool8 Menu_DoGfxSetup(void);
static bool8 Menu_InitBgs(void);
static void Menu_FadeAndBail(void);
static bool8 Menu_LoadGraphics(void);
static void Menu_InitWindows(void);
static void PrintToWindow(u8 windowId, u8 colorIdx);
static void PrintFieldTab(void);
static void PrintSpeedTab(void);
static void PrintStatsTab(void);
static void PrintAbilityTab(void);
static void PrintStatusTab(void);
static void PrintDamageCalulatorTab(void);
static void PrintDamageCalculation(u8 battler, u8 target, u16 move);
static void Task_MenuWaitFadeIn(u8 taskId);
static void Task_MenuMain(u8 taskId);
static void PrintMoveInfo(u16 move, u8 x, u8 y, u8 moveIdx);

static u8 ShowSpeciesIcon(u8 num);
static u8 ShowSpeciesIconSpeed(u8 battler, u8 x, u8 y);
static void FreeEveryMonIconSprite(void);
static void FreeSpeciesIconSprite(u8 battler);
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible);
static void SetMonTypeIcons(void);
static void SetTypeIconSpritePosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId);
static void CreateSetStatusSprite(void);
static u8 DestroyBattleMenuSprite(u8 spriteArrayId);
static void setBattler(void);
static void LoadTilemapFromMode(void);
static void PrintPage(void);

static void ShowFieldIcon(void);
static void CreateSelectorSprite(void);
static void FreeItemIconSprite(void);
void FreeFieldSprite(void);
void FreeSelectorSprite(void);

//==========CONST=DATA==========//
static const struct BgTemplate sMenuBgTemplates[] =
{
    {
        .bg = 0,    // windows, etc
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 1
    }, 
    {
        .bg = 1,    // this bg loads the UI tilemap
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .priority = 2
    },
    {
        .bg = 2,    // this bg loads the UI tilemap
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .priority = 0
    }
};

static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [WINDOW_1] = 
    {
        .bg = 0,            // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 30,        // width (per 8 pixels)
        .height = 20,       // height (per 8 pixels)
        .paletteNum = 0,    // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    },
};

static const u32 sMenuTiles[]      = INCBIN_U32("graphics/ui_menus/battle_menu/tiles.4bpp.lz");
static const u8 sStatDownArrow[]   = INCBIN_U8("graphics/ui_menus/battle_menu/stat_down_arrow.4bpp");
static const u8 sStatUpArrow[]     = INCBIN_U8("graphics/ui_menus/battle_menu/stat_up_arrow.4bpp");
static const u8 sCheck[]           = INCBIN_U8("graphics/ui_menus/battle_menu/check.4bpp");

//Palettes
static const u16 sMenuPalette[]        = INCBIN_U16("graphics/ui_menus/battle_menu/palette.gbapal");
static const u16 sMenuPalette_Blue[]   = INCBIN_U16("graphics/ui_menus/battle_menu/palette_blue.gbapal");
static const u16 sMenuPalette_Yellow[] = INCBIN_U16("graphics/ui_menus/battle_menu/palette_yellow.gbapal");
static const u16 sMenuPalette_Green[]  = INCBIN_U16("graphics/ui_menus/battle_menu/palette_green.gbapal");
static const u16 sMenuPalette_Red[]    = INCBIN_U16("graphics/ui_menus/battle_menu/palette_red.gbapal");

//Battler Tabs
static const u32 sMenu_Tilemap_Singles_Battler_Status[]     = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_singles_battler_status.bin.lz");
static const u32 sMenu_Tilemap_Doubles_Battler_Status[]     = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_doubles_battler_status.bin.lz");
//
static const u32 sMenu_Tilemap_Singles_Battler_Abilities[]  = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_singles_battler_abilities.bin.lz");
static const u32 sMenu_Tilemap_Doubles_Battler_Abilities[]  = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_doubles_battler_abilities.bin.lz");
//Field Tabs
static const u32 sMenu_Tilemap_Singles_Field[]              = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_singles_field.bin.lz");
static const u32 sMenu_Tilemap_Doubles_Field[]              = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_doubles_field.bin.lz");
//
static const u32 sMenu_Tilemap_Singles_Speed[]              = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_singles_field_speed.bin.lz");
static const u32 sMenu_Tilemap_Doubles_Speed[]              = INCBIN_U32("graphics/ui_menus/battle_menu/titlemap_doubles_field_speed.bin.lz");

enum Colors
{
    FONT_BLACK,
    FONT_WHITE,
    FONT_RED,
    FONT_BLUE,
};
static const u8 sMenuWindowFontColors[][3] = 
{
    [FONT_BLACK]  = {TEXT_COLOR_TRANSPARENT,  3,  2},
    [FONT_WHITE]  = {TEXT_COLOR_TRANSPARENT,  1,  3},
    [FONT_RED]    = {TEXT_COLOR_TRANSPARENT,  14, 2},
    [FONT_BLUE]   = {TEXT_COLOR_TRANSPARENT,  13, 2},
};

//==========FUNCTIONS==========//
// UI loader template
void Task_OpenBattleMenuFromStartMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        UI_Battle_Menu_Init(CB2_ReturnToFieldWithOpenMenu);
        DestroyTask(taskId);
    }
}

// This is our main initialization function if you want to call the menu from elsewhere
void UI_Battle_Menu_Init(MainCallback callback)
{
    u8 i, j;
    bool8 isExtraInfoShown;

    if ((sMenuDataPtr = AllocZeroed(sizeof(struct MenuResources))) == NULL)
    {
        SetMainCallback2(callback);
        return;
    }

    // initialize stuff
    sMenuDataPtr->gfxLoadState     = 0;
    sMenuDataPtr->moveModeId       = 0;
    sMenuDataPtr->tabModeId        = 0;
    sMenuDataPtr->tabId            = 0;
    sMenuDataPtr->fieldTabId       = 0;
    sMenuDataPtr->currentFieldInfo = 0;
    sMenuDataPtr->numFields        = 0;
    if(IsDoubleBattle())
        sMenuDataPtr->modeId = MODE_BATTLER2;
    else
        sMenuDataPtr->modeId = MODE_BATTLER0;
    setBattler();
    for(i = 0; i < NUM_SPRITES; i++)
        sMenuDataPtr->spriteIds[i] = SPRITE_NONE;
    sMenuDataPtr->savedCallback = callback;

    for(i = 0; i < NUM_FIELD_INFO; i++)
        sMenuDataPtr->fieldInfo[i] = NUM_FIELD_INFO;

    //
    for(i = 0; i < NUM_FIELD_INFO; i++){
        isExtraInfoShown = FALSE;
        switch(i){
            case FIELD_INFO_WEATHER:
                if((gBattleWeather & WEATHER_ANY))
                    isExtraInfoShown = TRUE;
            break;
            case FIELD_INFO_TERRAIN:
                if(gFieldStatuses & STATUS_FIELD_TERRAIN_ANY)
                    isExtraInfoShown = TRUE;
            break;
            case FIELD_INFO_TRICK_ROOM:
                if(gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
                    isExtraInfoShown = TRUE;
            break;
            case FIELD_INFO_GRAVITY:
                if((gFieldStatuses & STATUS_FIELD_GRAVITY))
                    isExtraInfoShown = TRUE;
            break;
            case FIELD_INFO_WONDER_ROOM:
                if(gFieldStatuses & STATUS_FIELD_WONDER_ROOM)
                    isExtraInfoShown = TRUE;
            break;
            case FIELD_INFO_MAGIC_ROOM:
                if(gFieldStatuses & STATUS_FIELD_MAGIC_ROOM)
                    isExtraInfoShown = TRUE;
            break;
        }

        if(isExtraInfoShown){
            //mgba
            #ifdef DEBUG_BUILD
            if(FlagGet(FLAG_SYS_MGBA_PRINT)){
                MgbaOpen();
                MgbaPrintf(MGBA_LOG_WARN, "sMenuDataPtr->fieldInfo[%d] = %d", sMenuDataPtr->numFields, i);
                MgbaClose();
            }
            #endif
            sMenuDataPtr->fieldInfo[sMenuDataPtr->numFields] = i;
            sMenuDataPtr->numFields++;
        }
    }

    SetMainCallback2(Battle_Menu_RunSetup);
}

static void Battle_Menu_RunSetup(void)
{
    while (1)
    {
        if (Menu_DoGfxSetup() == TRUE)
            break;
    }
}

static void Menu_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Menu_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

#define POKEMON_ICON_X   (1 * 8)  + (2 * 8)

#define POKEMON_ICON_1_Y (4 * 8)  + (2 * 8)
#define POKEMON_ICON_2_Y (12 * 8) + (2 * 8)
#define POKEMON_ICON_3_Y (0 * 8)  + (2 * 8)
#define POKEMON_ICON_4_Y (16 * 8) + (2 * 8)

static bool8 Menu_DoGfxSetup(void)
{
    u8 taskId;
    switch (gMain.state)
    {
    case 0:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000)
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 2:
        if (Menu_InitBgs())
        {
            sMenuDataPtr->gfxLoadState = 0;
            gMain.state++;
        }
        else
        {
            Menu_FadeAndBail();
            return TRUE;
        }
        break;
    case 3:
        if (Menu_LoadGraphics() == TRUE)
            gMain.state++;
        break;
    case 4:
        LoadMessageBoxAndBorderGfx();
        Menu_InitWindows();
        gMain.state++;
        break;
    case 5:
        ShowSpeciesIcon(0);
        ShowSpeciesIcon(1);
        if(IsDoubleBattle()){
            ShowSpeciesIcon(2);
            ShowSpeciesIcon(3);
        }
        PrintStatsTab();
        taskId = CreateTask(Task_MenuWaitFadeIn, 0);
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 6:
        CreateSelectorSprite();
        ShowFieldIcon();
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(Menu_VBlankCB);
        SetMainCallback2(Menu_MainCB);
        return TRUE;
    }
    return FALSE;
}

#define try_free(ptr) ({        \
    void ** ptr__ = (void **)&(ptr);   \
    if (*ptr__ != NULL)                \
        Free(*ptr__);                  \
})

static void Menu_FreeResources(void)
{
    try_free(sMenuDataPtr);
    try_free(sBg1TilemapBuffer);
    FreeAllWindowBuffers();
}

void LoadTilemapFromMode(void)
{
    try_free(sBg1TilemapBuffer);
    sBg1TilemapBuffer == NULL;

    ResetAllBgsCoordinates();
    sBg1TilemapBuffer = Alloc(0x800);

    memset(sBg1TilemapBuffer, 0, 0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sMenuBgTemplates, NELEMS(sMenuBgTemplates));
    SetBgTilemapBuffer(1, sBg1TilemapBuffer);
    ScheduleBgCopyTilemapToVram(1);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);

    //ResetTempTileDataBuffers();
    DecompressAndCopyTileDataToVram(1, sMenuTiles, 0, 0, 0);
    FreeTempTileDataBuffersIfPossible();

    if(sMenuDataPtr->modeId != MODE_FIELD){
        switch(sMenuDataPtr->tabId){
            case TAB_STATS:
                if(IsDoubleBattle())
                    LZDecompressWram(sMenu_Tilemap_Doubles_Battler_Status, sBg1TilemapBuffer);
                else
                    LZDecompressWram(sMenu_Tilemap_Singles_Battler_Status, sBg1TilemapBuffer);
            break;
            case TAB_ABILITIES:
            case TAB_MOVES:
            case TAB_DAMAGE_CALCULATOR:
                if(IsDoubleBattle())
                    LZDecompressWram(sMenu_Tilemap_Doubles_Battler_Abilities, sBg1TilemapBuffer);
                else
                    LZDecompressWram(sMenu_Tilemap_Singles_Battler_Abilities, sBg1TilemapBuffer);
            break;
            default:
                if(IsDoubleBattle())
                    LZDecompressWram(sMenu_Tilemap_Doubles_Field, sBg1TilemapBuffer);
                else
                    LZDecompressWram(sMenu_Tilemap_Singles_Field, sBg1TilemapBuffer);
            break;
        }
    }
    else{
        switch(sMenuDataPtr->fieldTabId){
            case TAB_FIELD:
                if(IsDoubleBattle())
                    LZDecompressWram(sMenu_Tilemap_Doubles_Field, sBg1TilemapBuffer);
                else
                    LZDecompressWram(sMenu_Tilemap_Singles_Field, sBg1TilemapBuffer);
            break;
            case TAB_SPEED:
                if(IsDoubleBattle())
                    LZDecompressWram(sMenu_Tilemap_Doubles_Speed, sBg1TilemapBuffer);
                else
                    LZDecompressWram(sMenu_Tilemap_Singles_Speed, sBg1TilemapBuffer);
            break;
        }
    }
}

static void Task_MenuWaitFadeAndBail(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeFieldSprite();
        FreeSelectorSprite();
        FreeItemIconSprite();
        SetMainCallback2(sMenuDataPtr->savedCallback);
        Menu_FreeResources();
        DestroyTask(taskId);
    }
}

static void Menu_FadeAndBail(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_MenuWaitFadeAndBail, 0);
    SetVBlankCallback(Menu_VBlankCB);
    SetMainCallback2(Menu_MainCB);
}

static bool8 Menu_InitBgs(void)
{
    ResetAllBgsCoordinates();
    sBg1TilemapBuffer = Alloc(0x800);
    if (sBg1TilemapBuffer == NULL)
        return FALSE;

    memset(sBg1TilemapBuffer, 0, 0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sMenuBgTemplates, NELEMS(sMenuBgTemplates));
    SetBgTilemapBuffer(1, sBg1TilemapBuffer);
    ScheduleBgCopyTilemapToVram(1);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    return TRUE;
}

static bool8 Menu_LoadGraphics(void)
{
    switch (sMenuDataPtr->gfxLoadState)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, sMenuTiles, 0, 0, 0);
        sMenuDataPtr->gfxLoadState++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            if(IsDoubleBattle())
                LZDecompressWram(sMenu_Tilemap_Doubles_Battler_Status, sBg1TilemapBuffer);
            else
                LZDecompressWram(sMenu_Tilemap_Singles_Battler_Status, sBg1TilemapBuffer);
            sMenuDataPtr->gfxLoadState++;
        }
        break;
    case 2:
        LoadPalette(sMenuPalette, 0, 32);
        sMenuDataPtr->gfxLoadState++;
        break;
    default:
        sMenuDataPtr->gfxLoadState = 0;
        return TRUE;
    }
    return FALSE;
}

static void Menu_InitWindows(void)
{
    u32 i;

    InitWindows(sMenuWindowTemplates);
    DeactivateAllTextPrinters();
    ScheduleBgCopyTilemapToVram(0);

    //First Window
    FillWindowPixelBuffer(WINDOW_1, 0);
    LoadUserWindowBorderGfx(WINDOW_1, 720, 14 * 16);
    PutWindowTilemap(WINDOW_1);
    CopyWindowToVram(WINDOW_1, 3);

    ScheduleBgCopyTilemapToVram(2);
}

static const u8 sText_MyMenu[] = _("HP: {STR_VAR_1}/{STR_VAR_2}");
static const u8 sText_HP[] = _("{STR_VAR_1}/{STR_VAR_2}");
static const u8 sText_PP[] = _("PP: {STR_VAR_1}/{STR_VAR_2}");

static const u8 sText_Ability[] = _("Ability:");
static const u8 sText_Innate[]  = _("Innate:");
static const u8 sText_Held_Item[] = _("Held Item:\n{STR_VAR_1}");
static const u8 sText_None[]  = _("None");
static const u8 sYourOpponentPokemonData[] = _("Your Opponent Pokémon Battle Data");
static const u8 sYourPokemonData[] = _("Your Pokémon Battle Data");

static const u8 sText_StatHP[]         = _("HP");
static const u8 sText_Attack[]         = _("Atk");
static const u8 sText_Defense[]        = _("Def");
static const u8 sText_SpecialAttack[]  = _("SpA");
static const u8 sText_SpecialDefense[] = _("SpD");
static const u8 sText_Speed[]          = _("Spd");
static const u8 sText_Accuracy[]       = _("Acc");
static const u8 sText_Evasion[]        = _("Eva");
static const u8 sText_Critical[]       = _("Crt");

static u8 statorder[NUM_BATTLE_STATS] = {
    STAT_HP,
    STAT_ATK,
    STAT_DEF,
    STAT_SPATK,
    STAT_SPDEF,
    STAT_SPEED,
    STAT_ACC,
    STAT_EVASION,
};

#define PAL_UI_SPRITES        14
#define PAL_FIELD_ICON        15

#define TAG_ITEM_ICON         4133
#define TAG_BATTLER_SPEED     4134

static const u32 gBattleFieldIconForest_Gfx[] = INCBIN_U32("graphics/ui_menus/battle_menu/fields/forest.4bpp.lz");
static const u16 gBattleFieldIconForest_Pal[] = INCBIN_U16("graphics/ui_menus/battle_menu/fields/forest.gbapal");

static const struct SpritePalette sBattleMenuFieldIconSpritePalette_Forest[] = {gBattleFieldIconForest_Pal, PAL_FIELD_ICON};

//Field Icon
void FreeFieldSprite(void)
{
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON];
    if (*spriteId != SPRITE_NONE)
    {
        FreeSpriteTilesByTag(SPRITE_ARR_ID_FIELD_ICON);
        FreeSpritePaletteByTag(SPRITE_ARR_ID_FIELD_ICON);
        FreeSpriteOamMatrix(&gSprites[*spriteId]);
        DestroySprite(&gSprites[*spriteId]);
        *spriteId = SPRITE_NONE;
        sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON] = SPRITE_NONE;
    }
}

static void ShowFieldIcon(void)
{
    u8 spriteId;
    u8 SpriteTag = SPRITE_ARR_ID_FIELD_ICON;
    struct CompressedSpriteSheet sSpriteSheet_FieldIcon = {gBattleFieldIconForest_Gfx, 0x0800, SpriteTag};
    struct SpriteTemplate TempSpriteTemplate = gDummySpriteTemplate;

    TempSpriteTemplate.tileTag = SpriteTag;
    TempSpriteTemplate.callback = SpriteCallbackDummy;

    LoadCompressedSpriteSheet(&sSpriteSheet_FieldIcon);
    LoadSpritePalette(sBattleMenuFieldIconSpritePalette_Forest);
    TempSpriteTemplate.paletteTag = PAL_FIELD_ICON;
    spriteId = CreateSprite(&TempSpriteTemplate, 4, 68, 0);
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON] = spriteId;

    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON]].oam.shape = SPRITE_SHAPE(64x32);
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON]].oam.size = SPRITE_SIZE(64x32);
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_FIELD_ICON]].oam.priority = 1;
}

static void SpriteCB_Selector(struct Sprite *sprite)
{
    u8 val = sprite->data[0];
    sprite->y2 = (gSineTable[val] / 128) + ((sMenuDataPtr->modeId * 4) * 8);
    sprite->data[0] += 8;
}

static const u32 gBattleSelector_Gfx[] = INCBIN_U32("graphics/ui_menus/battle_menu/fields/selector.4bpp.lz");
static const u16 gBattleSelector_Pal[] = INCBIN_U16("graphics/ui_menus/battle_menu/fields/selector.gbapal");

static const struct SpritePalette sBattleMenuSelectorSpritePalette[] = {gBattleSelector_Pal, PAL_UI_SPRITES};
//Selector
void FreeSelectorSprite(void)
{
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR];
    if (*spriteId != SPRITE_NONE)
    {
        FreeSpriteTilesByTag(SPRITE_ARR_ID_SELECTOR);
        FreeSpritePaletteByTag(SPRITE_ARR_ID_SELECTOR);
        FreeSpriteOamMatrix(&gSprites[*spriteId]);
        DestroySprite(&gSprites[*spriteId]);
        *spriteId = SPRITE_NONE;
        sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR] = SPRITE_NONE;
    }
}

static void CreateSelectorSprite(void)
{
    u8 spriteId;
    u8 SpriteTag = SPRITE_ARR_ID_SELECTOR;
    struct CompressedSpriteSheet sSpriteSheet_Selector = {gBattleSelector_Gfx, 0x0801, SpriteTag};
    struct SpriteTemplate TempSpriteTemplate = gDummySpriteTemplate;

    TempSpriteTemplate.tileTag = SpriteTag;
    TempSpriteTemplate.callback = SpriteCB_Selector;

    LoadCompressedSpriteSheet(&sSpriteSheet_Selector);
    LoadSpritePalette(sBattleMenuSelectorSpritePalette);
    TempSpriteTemplate.paletteTag = PAL_UI_SPRITES;
    spriteId = CreateSprite(&TempSpriteTemplate, 4, 4, 0);
    sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR] = spriteId;

    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR]].oam.shape = SPRITE_SHAPE(64x32);
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR]].oam.size = SPRITE_SIZE(64x32);
    gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_SELECTOR]].oam.priority = 0;
}

static void ShowItemIcon(u16 itemId, u8 x, u8 y)
{
    u8 itemSpriteId;
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_HELD_ITEM];

    if (*spriteId == SPRITE_NONE && itemId != ITEM_NONE)
    {
        FreeSpriteTilesByTag(TAG_ITEM_ICON - 1);
        FreeSpritePaletteByTag(TAG_ITEM_ICON - 1);
        itemSpriteId = AddItemIconSprite(TAG_ITEM_ICON, TAG_ITEM_ICON, itemId);
        sMenuDataPtr->spriteIds[SPRITE_ARR_HELD_ITEM] = itemSpriteId;
        
        if (itemSpriteId != MAX_SPRITES)
        {
            *spriteId = itemSpriteId;
            gSprites[itemSpriteId].x2 = x;
            gSprites[itemSpriteId].y2 = y;
        }
    }
}

static void FreeItemIconSprite(void)
{
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_HELD_ITEM];
    if (*spriteId != SPRITE_NONE)
    {
        FreeSpriteTilesByTag(TAG_ITEM_ICON);
        FreeSpritePaletteByTag(TAG_ITEM_ICON);
        FreeSpriteOamMatrix(&gSprites[*spriteId]);
        DestroySprite(&gSprites[*spriteId]);
        *spriteId = SPRITE_NONE;
        sMenuDataPtr->spriteIds[SPRITE_ARR_HELD_ITEM] = SPRITE_NONE;
    }
}
const u8 sText_Title_Nothing[]    = _("");

const u8 sText_Title_Battler_Stats[]    = _("Pokémon Stats");
const u8 sText_Title_Battler_Ability[]  = _("Abilities Info");
const u8 sText_Title_Battler_Moves[]    = _("Moves Info");
const u8 sText_Title_Battler_Status[]   = _("Pokémon Status");
const u8 sText_Title_Battler_Damage[]   = _("Damage Calculator");
const u8 sText_Title_Field_Stats[]      = _("Field Info");
const u8 sText_Title_Controllers[]      = _("{DPAD_UPDOWN}Switch {DPAD_LEFTRIGHT}Page");
const u8 sText_Title_Speed_Order[]      = _("Speed Order");

const u8 sText_Title_Type_One[]     = _("Type:\n{STR_VAR_1}");
const u8 sText_Title_Type_Two[]     = _("Types:\n{STR_VAR_1}/ {STR_VAR_2}");
const u8 sText_Title_Type_Three[]   = _("Types:\n{STR_VAR_1}/ {STR_VAR_2}/ {STR_VAR_3}");

const u8 sText_Title_Nature[]        = _("Nature: {STR_VAR_1}\n({STR_VAR_2}+ {STR_VAR_3}-)");
const u8 sText_Title_Nature_NoStat[] = _("Nature:\n{STR_VAR_1}");

const u8 sText_Title_Held_Item[]    = _("Held Item: {STR_VAR_1}");
const u8 gText_NewLevelSymbol[] = _("{LV}{STR_VAR_1}");
#define SPACE_BETWEEN_TYPES (5 * 8)

static void PrintStatsTab(){
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 innate1 = gBaseStats[species].innates[0];
    u16 innate2 = gBaseStats[species].innates[1];
    u16 innate3 = gBaseStats[species].innates[2];
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;
    u8 numtypes = 1;
    struct Pokemon *party, *targetParty;
    u8 nature;
    const s8 *natureMod;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = -4;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Battler_Stats);
    x  = 19;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    //Pokemon Name
    x  = 9;
    y  = 4;
    x2 = 0;
    y2 = -4;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gSpeciesNames[species]);
    //Pokemon Gender
    x = x + 8;
    switch(gender){
        case MON_MALE:
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLUE], 0xFF, gText_MaleSymbol);
            break;
        case MON_FEMALE:
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_RED], 0xFF, gText_FemaleSymbol);
            break;
    }
    //Pokemon Level
    x++;
    ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].level, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_NewLevelSymbol);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
    //Pokemon Types
    y++;
    x = 9;
    StringCopy(gStringVar1, gTypeNames[gBattleMons[sMenuDataPtr->battlerId].type1]);
    //Check if there is a second type
    if(gBattleMons[sMenuDataPtr->battlerId].type1 != gBattleMons[sMenuDataPtr->battlerId].type2){
        numtypes++;
        StringCopy(gStringVar2, gTypeNames[gBattleMons[sMenuDataPtr->battlerId].type2]);
    }
    //Check if there is a third type
    if(gBattleMons[sMenuDataPtr->battlerId].type3 != TYPE_MYSTERY && 
       gBattleMons[sMenuDataPtr->battlerId].type3 != gBattleMons[sMenuDataPtr->battlerId].type1 &&
       gBattleMons[sMenuDataPtr->battlerId].type3 != gBattleMons[sMenuDataPtr->battlerId].type2){
        numtypes++;
        if(numtypes == 2)
            StringCopy(gStringVar2, gTypeNames[gBattleMons[sMenuDataPtr->battlerId].type3]);
        else
            StringCopy(gStringVar3, gTypeNames[gBattleMons[sMenuDataPtr->battlerId].type3]);
    }   

    switch(numtypes){
        case 1:
            StringExpandPlaceholders(gStringVar4, sText_Title_Type_One);
        break;
        case 2:
            StringExpandPlaceholders(gStringVar4, sText_Title_Type_Two);
        break;
        case 3:
            StringExpandPlaceholders(gStringVar4, sText_Title_Type_Three);
        break;
    }
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);

    //Held Item
    y = y +2;
    if(gBattleMons[sMenuDataPtr->battlerId].item != ITEM_NONE)
        CopyItemName(gBattleMons[sMenuDataPtr->battlerId].item, gStringVar1);
    else
        StringCopy(gStringVar1, sText_None);
    StringExpandPlaceholders(gStringVar4, sText_Title_Held_Item);
    //StringCopy(gStringVar1, ItemId_GetDescription(gBattleMons[sMenuDataPtr->battlerId].item));
    StringExpandPlaceholders(gStringVar4, sText_Title_Held_Item);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);

    //Item Icon
    x  = 26 + 1;
    x2 = 4;
    y  = 4  + 3;
    ShowItemIcon(gBattleMons[sMenuDataPtr->battlerId].item, (x * 8) + x2, (y * 8) + y2);
    x2 = 0;

    //Stat Drops & Ups
    x = 9;
    y = 9;
    //Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Attack);
    y++;
    //Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Defense);
    y++;
    //Special Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialAttack);
    y++;
    //Special Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialDefense);
    y++;
    //Speed
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Speed);
    y = y + 2;
    //Accuracy
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Accuracy);
    //Evasion
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Evasion);
    //Critical
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Critical);
    
    x = 11;
    y = 9;
    for(i = 0; i < NUM_BATTLE_STATS - 1; i++){
        
        statStage = gBattleMons[sMenuDataPtr->battlerId].statStages[statorder[i + 1]];//HP is not taken into account
        if(statStage != DEFAULT_STAT_STAGE){
            if(statStage > DEFAULT_STAT_STAGE){
                statStageUp = TRUE;
                statStage = statStage - DEFAULT_STAT_STAGE;
            }
            else{
                statStageUp = FALSE;
                statStage = DEFAULT_STAT_STAGE - statStage;
            }

            for(j = 0; j < statStage; j++){
                if(statStageUp)
                    BlitBitmapToWindow(windowId, sStatUpArrow, ((x + j) * 8) + x2, (y * 8), 8, 8);
                else
                    BlitBitmapToWindow(windowId, sStatDownArrow, ((x + j) * 8) + x2, (y * 8), 8, 8);

            }
        }
        if(statorder[i + 1] == STAT_SPEED)
            y = y + 2;
        else
            y++;
    }

    //Stat names
    x  = 20;
    y  = 9;
    //HP
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_StatHP);
    ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, gBattleMons[sMenuDataPtr->battlerId].maxHP, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_HP);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
    y++;
    //Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Attack);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].attack, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Defense);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].defense, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Special Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialAttack);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].spAttack, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Special Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialDefense);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].spDefense, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Speed
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Speed);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].speed, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);

    //Nature
    x = 20;
    y = 16;
    if (GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;
    
    nature = GetMonData(&party[gBattlerPartyIndexes[sMenuDataPtr->battlerId]], MON_DATA_NATURE, NULL);

    CopyNatureName(nature);
    if(nature == NATURE_HARDY || nature == NATURE_DOCILE || nature == NATURE_SERIOUS || nature == NATURE_BASHFUL || nature == NATURE_QUIRKY){
        //No Stat Up or Down
        StringExpandPlaceholders(gStringVar4, sText_Title_Nature_NoStat);
    }
    else{
        natureMod = gNatureStatTable[nature];
        for(i = 0; i < NUM_NATURE_STATS; i++){
            if (natureMod[i] > 0){
                //Nature Up
                switch(i + 1){
                    case STAT_ATK:
                        StringCopy(gStringVar2, sText_Attack);
                    break;
                    case STAT_DEF:
                        StringCopy(gStringVar2, sText_Defense);
                    break;
                    case STAT_SPATK:
                        StringCopy(gStringVar2, sText_SpecialAttack);
                    break;
                    case STAT_SPDEF:
                        StringCopy(gStringVar2, sText_SpecialDefense);
                    break;
                    case STAT_SPEED:
                        StringCopy(gStringVar2, sText_Speed);
                    break;
                }
            }
            else if (natureMod[i] < 0){
                //Nature Down
                switch(i + 1){
                    case STAT_ATK:
                        StringCopy(gStringVar3, sText_Attack);
                    break;
                    case STAT_DEF:
                        StringCopy(gStringVar3, sText_Defense);
                    break;
                    case STAT_SPATK:
                        StringCopy(gStringVar3, sText_SpecialAttack);
                    break;
                    case STAT_SPDEF:
                        StringCopy(gStringVar3, sText_SpecialDefense);
                    break;
                    case STAT_SPEED:
                        StringCopy(gStringVar3, sText_Speed);
                    break;
                }
            }
        }
        StringExpandPlaceholders(gStringVar4, sText_Title_Nature);
    }
    
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}


const u8 sText_PrintAbilityTab_Ability[] = _("Ability");
const u8 sText_PrintAbilityTab_Innate[] = _("Innate");
#define SPACE_BETWEEN_ABILITY_AND_NAME (8 * 8)
#define SPACE_BETWEEN_ABILITIES 3
static void PrintAbilityTab(){
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 innate1 = gBaseStats[species].innates[0];
    u16 innate2 = gBaseStats[species].innates[1];
    u16 innate3 = gBaseStats[species].innates[2];
    u16 ability = gBattleMons[sMenuDataPtr->battlerId].ability;
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;

    if(!isEnemyMon){ //Enemy Mons have disabled randomized innates/abilies 
        innate1 = RandomizeInnate(gBaseStats[species].innates[0], species, personality);
        innate2 = RandomizeInnate(gBaseStats[species].innates[1], species, personality);
        innate3 = RandomizeInnate(gBaseStats[species].innates[2], species, personality);
    }

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = 0;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Battler_Ability);
    x  = 19;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    // Main Ability
    x  = 9;
    y  = 3;
    x2 = 0;
    y2 = -4;
    //Title
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_PrintAbilityTab_Ability);
	// Name ---------------------------------------------------------------------------------------------------
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gAbilityNames[ability]);
	// Description ---------------------------------------------------------------------------------------------------
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2 + 4, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityDescriptionPointers[ability]);

    // Innate 1
    y = y + SPACE_BETWEEN_ABILITIES;
    //Title
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_PrintAbilityTab_Innate);
    // Name ---------------------------------------------------------------------------------------------------
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gAbilityNames[innate1]);
    // Description ---------------------------------------------------------------------------------------------------
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2 + 4, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityDescriptionPointers[innate1]);
    
    // Innate 2
    y = y + SPACE_BETWEEN_ABILITIES;
    //Title
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_PrintAbilityTab_Innate);
    // Name ---------------------------------------------------------------------------------------------------
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gAbilityNames[innate2]);
    // Description ---------------------------------------------------------------------------------------------------
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2 + 4, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityDescriptionPointers[innate2]);
       
    // Innate 3
    y = y + SPACE_BETWEEN_ABILITIES;
    //Title
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_PrintAbilityTab_Innate);
    // Name ---------------------------------------------------------------------------------------------------
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gAbilityNames[innate3]);
    // Description ---------------------------------------------------------------------------------------------------
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2 + 4, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityDescriptionPointers[innate3]);

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}


const u8 gText_CurrentPP[]         = _("PP: {STR_VAR_1}/{STR_VAR_2}");
const u8 gText_MoveInfo_Power[]    = _("Power: {STR_VAR_1}");
const u8 gText_MoveInfo_Accuracy[] = _("Accuracy: {STR_VAR_1}");
const u8 gText_MoveInfo_Priority[] = _("Priority: {STR_VAR_1}");
const u8 gText_MoveInfo_Split[]    = _("Split: {STR_VAR_1}");
const u8 gText_MoveInfo_STAB[]     = _("STAB: {STR_VAR_1}");

const u8 gText_Split_Physical[] = _("Physical");
const u8 gText_Split_Special[]  = _("Special");
const u8 gText_Split_Status[]   = _("Status");

const u8 gText_MoveInfo_Target1[] = _("Target 1: {STR_VAR_1}");
const u8 gText_MoveInfo_Target2[] = _("Target 2: {STR_VAR_1}");

const u8 gText_MoveInfo_Damage[] = _("Damage");

const u8 gText_Target_Nothing[]   = _("---");
const u8 sText_Title_Controllers_Move[]      = _("{DPAD_UPDOWN}Switch {DPAD_LEFTRIGHT}Page {A_BUTTON}Mode");
#define SPACE_BETWEEN_MOVES 4
#define SPACE_BETWEEN_DAMAGE (4 * 8)
#define MIN_MOVE_DAMAGE 4
#define ENABLE_DAMAGE_CALCULATION FALSE
static void PrintMoveTab(void){
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 move1   = gBattleMons[sMenuDataPtr->battlerId].moves[0];
    u16 move2   = gBattleMons[sMenuDataPtr->battlerId].moves[1];
    u16 move3   = gBattleMons[sMenuDataPtr->battlerId].moves[2];
    u16 move4   = gBattleMons[sMenuDataPtr->battlerId].moves[3];
    u16 ability = gBattleMons[sMenuDataPtr->battlerId].ability;
    u16 gBattleMoveDamage;
    u8 target = 1;
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;
    u8 mode = sMenuDataPtr->moveModeId;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    
    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = 0;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Battler_Moves);
    x  = 16;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers_Move);

    // First Move
    x  = 9;
    y  = 3;
	PrintMoveInfo(move1, x, y, 0);

    // Second Move
    y = y + SPACE_BETWEEN_MOVES;
	PrintMoveInfo(move2, x, y, 1);

    // Third Move
    y = y + SPACE_BETWEEN_MOVES;
	PrintMoveInfo(move3, x, y, 2);

    // Fourth Move
    y = y + SPACE_BETWEEN_MOVES;
	PrintMoveInfo(move4, x, y, 3);

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

const u8 gText_Makes_Contact_True[]        = _("Makes contact");
const u8 gText_Makes_Contact_False[]       = _("Does not make contact");
const u8 gText_Protect_Affected_True[]     = _("Affected by Protect");
const u8 gText_Protect_Affected_False[]    = _("Not affected by Protect");
const u8 gText_Magic_Coat_Affected_True[]  = _("Affected by Magic Coat");
const u8 gText_Magic_Coat_Affected_False[] = _("Not affected by Magic Coat");
const u8 gText_Snatch_Affected_True[]      = _("Affected by Snatch");
const u8 gText_Snatch_Affected_False[]     = _("Not affected by Snatch");
const u8 gText_Mirror_Move_Affected_True[] = _("Affected by Mirror Move");
const u8 gText_Mirror_Move_Affected_False[]= _("Not affected by Mirror Move");
const u8 gText_Kings_Rock_Affected_True[]  = _("Affected by King's Rock");
const u8 gText_Kings_Rock_Affected_False[] = _("Not affected by King's Rock");

const u8 gText_Boost_True[]          = _("True");
const u8 gText_Boost_False[]         = _("False");

const u8 gText_Move_Type_TwoTypedMoves[] = _("{STR_VAR_1}/{STR_VAR_2}");

static void MulModifier(u16 *modifier, u16 val)
{
	*modifier = UQ_4_12_TO_INT((*modifier * val) + UQ_4_12_ROUND);
}

u32 calculateTotalMoveDamage(u16 move, u8 battlerAtk, u8 battlerDef, u8 moveType){
    bool32 updateFlags = FALSE;
    u16 modifier = UQ_4_12(1.0);
    u16 movePower = CalcMoveBasePowerAfterModifiers(move, battlerAtk, battlerDef, moveType, updateFlags);
	u8 numsleepmons = 0;
    u8 atkSide = GetBattlerSide(battlerAtk);

    // attacker's abilities
    //Reckless
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_RECKLESS)){
        if (gBattleMoves[move].flags & FLAG_RECKLESS_BOOST)
            MulModifier(&modifier, UQ_4_12(1.2));
    }

    //Iron Fist/Power Fist
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_IRON_FIST) || BATTLER_HAS_ABILITY(battlerAtk, ABILITY_POWER_FISTS)){
        if (gBattleMoves[move].flags & FLAG_IRON_FIST_BOOST)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Striker
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_STRIKER)){
        if (gBattleMoves[move].flags & FLAG_STRIKER_BOOST)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Mighty Horn
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_MIGHTY_HORN)){
        if (gBattleMoves[move].flags & FLAG_HORN_BASED)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Field Explorer
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_FIELD_EXPLORER)){
        if (gBattleMoves[move].flags & FLAG_FIELD_BASED)
           MulModifier(&modifier, UQ_4_12(1.25));
    }

    //Giant Wings
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_GIANT_WINGS)){
        if (gBattleMoves[move].flags2 & FLAG_AIR_BASED)
           MulModifier(&modifier, UQ_4_12(1.25));
    }

    //Keen Edge
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_KEEN_EDGE)){
        if (gBattleMoves[move].flags & FLAG_KEEN_EDGE_BOOST)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Sheer Force
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SHEER_FORCE)){
        if (gBattleMoves[move].flags & FLAG_SHEER_FORCE_BOOST)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Sand Force
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SAND_FORCE)){
        if ((moveType == TYPE_STEEL || moveType == TYPE_ROCK || moveType == TYPE_GROUND)
            && gBattleWeather & B_WEATHER_SANDSTORM && WEATHER_HAS_EFFECT)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Analytic
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_ANALYTIC)){
        if (GetBattlerTurnOrderNum(battlerAtk) == gBattlersCount - 1 && move != MOVE_FUTURE_SIGHT && move != MOVE_DOOM_DESIRE)
           MulModifier(&modifier, UQ_4_12(1.3));
    }

    //Steelworker
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_STEELWORKER)){
        if (moveType == TYPE_STEEL)
           MulModifier(&modifier, UQ_4_12(1.3)); // was 1.5
    }

    // Strong Jaw
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_STRONG_JAW) && (gBattleMoves[move].flags & FLAG_STRONG_JAW_BOOST))
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Pixilate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_PIXILATE) && moveType == TYPE_FAIRY && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));

    // Pollinate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_POLLINATE) && moveType == TYPE_BUG && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));

    // Draconize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_DRACONIZE) && moveType == TYPE_DRAGON && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));
	
	// Galvanize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_GALVANIZE) && moveType == TYPE_ELECTRIC && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));
	
	// Refrigerate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_REFRIGERATE) && moveType == TYPE_ICE && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));
	
	// Aerilate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_AERILATE) && moveType == TYPE_FLYING && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));
	
	// Normalize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_NORMALIZE) && moveType == TYPE_NORMAL && gBattleStruct->ateBoost[battlerAtk])
        MulModifier(&modifier, UQ_4_12(1.1));
	
	// Punk Rock
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_PUNK_ROCK) && (gBattleMoves[move].flags & FLAG_SOUND))
        MulModifier(&modifier, UQ_4_12(1.3));

    // Amplifier
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_AMPLIFIER) && (gBattleMoves[move].flags & FLAG_SOUND))
        MulModifier(&modifier, UQ_4_12(1.3));
	
	// Steely Spirit
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_STEELY_SPIRIT) && moveType == TYPE_STEEL)
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Transistor
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_TRANSISTOR) && moveType == TYPE_ELECTRIC)
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Dragon's Maw
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_DRAGONS_MAW) && moveType == TYPE_DRAGON)
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Liquid Voice
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_LIQUID_VOICE) && (gBattleMoves[move].flags & FLAG_SOUND))
        MulModifier(&modifier, UQ_4_12(1.2));

	// Gorilla Tactics
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_GORILLA_TACTICS) && IS_MOVE_PHYSICAL(move))
        MulModifier(&modifier, UQ_4_12(1.5));

	// Sage Power
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SAGE_POWER) && IS_MOVE_SPECIAL(move))
        MulModifier(&modifier, UQ_4_12(1.5));

    //Exploit Weakness
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_EXPLOIT_WEAKNESS) && (gBattleMons[battlerDef].status1 & STATUS1_ANY))
        MulModifier(&modifier, UQ_4_12(1.25));

	// Avenger
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_AVENGER) && gSideTimers[atkSide].retaliateTimer == 1)
        MulModifier(&modifier, UQ_4_12(1.5));

	// Immolate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_IMMOLATE) && moveType == TYPE_FIRE && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

    // Mineralize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_MINERALIZE) && moveType == TYPE_ROCK && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

    // Spectral Shroud
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SPECTRAL_SHROUD) && moveType == TYPE_GHOST && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

    // Spectralize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SPECTRALIZE) && moveType == TYPE_GHOST && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

    // Solar Flare
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_SOLAR_FLARE) && moveType == TYPE_FIRE && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

	// Crystallize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_CRYSTALLIZE) && moveType == TYPE_ICE && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));

	// Fight Spirit
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_FIGHT_SPIRIT) && moveType == TYPE_FIGHTING && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));
	
	// Tectonize
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_TECTONIZE) && moveType == TYPE_GROUND && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));
	
	// Hydrate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_HYDRATE) && moveType == TYPE_WATER && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));
	
	// Intoxicate
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_INTOXICATE) && moveType == TYPE_POISON && gBattleStruct->ateBoost[battlerAtk])
		MulModifier(&modifier, UQ_4_12(1.1));
	
	// Long Reach
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_LONG_REACH) && IS_MOVE_PHYSICAL(move) && !(gBattleMoves[move].flags & FLAG_MAKES_CONTACT))
        MulModifier(&modifier, UQ_4_12(1.2));
	
	// Tough Claws & Big Pecks
	if((BATTLER_HAS_ABILITY(battlerAtk, ABILITY_TOUGH_CLAWS) ||
        BATTLER_HAS_ABILITY(battlerAtk, ABILITY_BIG_PECKS))  && 
        IsMoveMakingContact(move, battlerAtk))
           MulModifier(&modifier, UQ_4_12(1.3));

    //DreamCatcher
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_DREAMCATCHER) && numsleepmons > 0) //It used to be depending on the number of sleeping mons, but the sleep clause made it useless
		MulModifier(&modifier, UQ_4_12(2.0));

	// Rivalry
    if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_RIVALRY)){
        if (GetGenderFromSpeciesAndPersonality(gBattleMons[battlerAtk].species, gBattleMons[battlerAtk].personality) != MON_GENDERLESS
            && GetGenderFromSpeciesAndPersonality(gBattleMons[battlerDef].species, gBattleMons[battlerDef].personality) != MON_GENDERLESS)
            {
                if (GetGenderFromSpeciesAndPersonality(gBattleMons[battlerAtk].species, gBattleMons[battlerAtk].personality)
                == GetGenderFromSpeciesAndPersonality(gBattleMons[battlerDef].species, gBattleMons[battlerDef].personality))
                MulModifier(&modifier, UQ_4_12(1.25));
            }
	}
	
	// Dragonslayer
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_DRAGONSLAYER) && IS_BATTLER_OF_TYPE(battlerDef, TYPE_DRAGON)) // check if foe has Dragon-type
        MulModifier(&modifier, UQ_4_12(1.5));

    // Fae Hunter
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_FAE_HUNTER) && IS_BATTLER_OF_TYPE(battlerDef, TYPE_FAIRY)) // check if foe has Fairy-type
        MulModifier(&modifier, UQ_4_12(1.5));

    // Marine Apex
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_MARINE_APEX) && IS_BATTLER_OF_TYPE(battlerDef, TYPE_WATER)) // check if foe has Water-type
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Huge Power & Pure Power
	if((BATTLER_HAS_ABILITY(battlerAtk, ABILITY_HUGE_POWER)  ||
        BATTLER_HAS_ABILITY(battlerAtk, ABILITY_PURE_POWER)) && 
        IS_MOVE_PHYSICAL(move))
           MulModifier(&modifier, UQ_4_12(2.0));

	// Feline Prowess
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_FELINE_PROWESS) && IS_MOVE_SPECIAL(move))
        MulModifier(&modifier, UQ_4_12(2.0));
	
	// Majestic Bird
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_MAJESTIC_BIRD) && IS_MOVE_SPECIAL(move))
        MulModifier(&modifier, UQ_4_12(1.5));
	
	//Toxic Boost
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_TOXIC_BOOST) && (gBattleMons[battlerAtk].status1 & STATUS1_PSN_ANY) && IS_MOVE_PHYSICAL(move))
        MulModifier(&modifier, UQ_4_12(1.5));
	
	//Flare Boost
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_FLARE_BOOST) && (gBattleMons[battlerAtk].status1 & STATUS1_BURN) && IS_MOVE_SPECIAL(move))
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Mega Launcher
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_MEGA_LAUNCHER) && (gBattleMoves[move].flags & FLAG_MEGA_LAUNCHER_BOOST))
        MulModifier(&modifier, UQ_4_12(1.5));

    // Iron Barrage
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_IRON_BARRAGE) && (gBattleMoves[move].flags & FLAG_MEGA_LAUNCHER_BOOST))
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Hustle
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_HUSTLE))
        MulModifier(&modifier, UQ_4_12(1.4));
	
	// Technician
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_TECHNICIAN) && movePower <= 60)
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Water Bubble
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_WATER_BUBBLE) && moveType == TYPE_WATER)
        MulModifier(&modifier, UQ_4_12(2.0));

    // Hydro Circuit
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_HYDRO_CIRCUIT) && moveType == TYPE_ELECTRIC)
        MulModifier(&modifier, UQ_4_12(1.5));
	
	// Violent Rush
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_VIOLENT_RUSH) && (gDisableStructs[battlerAtk].isFirstTurn))
        MulModifier(&modifier, UQ_4_12(1.2));
	
    // Field Abilities
    if ((IsAbilityOnField(ABILITY_DARK_AURA) && moveType == TYPE_DARK)
        || (IsAbilityOnField(ABILITY_FAIRY_AURA) && moveType == TYPE_FAIRY))
    {
        if (IsAbilityOnField(ABILITY_AURA_BREAK))
            MulModifier(&modifier, UQ_4_12(0.75));
        else
            MulModifier(&modifier, UQ_4_12(1.33));
    }

    // Pretty Princess
	if(BATTLER_HAS_ABILITY(battlerAtk, ABILITY_PRETTY_PRINCESS)){
        if(!BATTLER_HAS_ABILITY(battlerAtk, ABILITY_UNAWARE) &&
           !BattlerHasInnate(battlerDef, ABILITY_UNAWARE) &&
           HasAnyLoweredStat(battlerDef))
            MulModifier(&modifier, UQ_4_12(1.5));
    }

    // Attacker Partner's Abilities
    if (IsBattlerAlive(BATTLE_PARTNER(battlerAtk)))
    {
		// Battery
		if(BATTLER_HAS_ABILITY(BATTLE_PARTNER(battlerAtk), ABILITY_BATTERY)){
			if (IS_MOVE_SPECIAL(move))
                MulModifier(&modifier, UQ_4_12(1.3));
		}
		
		// Power Spot
		if(BATTLER_HAS_ABILITY(BATTLE_PARTNER(battlerAtk), ABILITY_POWER_SPOT)){
			MulModifier(&modifier, UQ_4_12(1.3));
		}
		
		// Steely Spirit
		if(BATTLER_HAS_ABILITY(BATTLE_PARTNER(battlerAtk), ABILITY_STEELY_SPIRIT)){
			if (moveType == TYPE_STEEL)
                MulModifier(&modifier, UQ_4_12(1.5));
		}
    }

    // Target's Abilities
    //Heatproof & Water Bubble
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_HEATPROOF || BATTLER_HAS_ABILITY(battlerDef, ABILITY_WATER_BUBBLE))){
        if (moveType == TYPE_FIRE)
            MulModifier(&modifier, UQ_4_12(0.5));
    }

    //Dry Skin
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_DRY_SKIN)){
        if (moveType == TYPE_FIRE)
            MulModifier(&modifier, UQ_4_12(1.25));
    }

    //Fluffy
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_FLUFFY)){
        if (moveType == TYPE_FIRE)
            MulModifier(&modifier, UQ_4_12(2.0));
    }

    //Liquified
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_LIQUIFIED)){
        if (IsMoveMakingContact(move, battlerAtk))
            MulModifier(&modifier, UQ_4_12(0.5));
        
        if (moveType == TYPE_WATER)
            MulModifier(&modifier, UQ_4_12(2.0));
    }

    //Christmas Spirit
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_CHRISTMAS_SPIRIT)){
        if (WEATHER_HAS_EFFECT && gBattleWeather & WEATHER_HAIL_ANY)
            MulModifier(&modifier, UQ_4_12(0.5));
    }

    //Dune Terror
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_DUNE_TERROR)){
        if (WEATHER_HAS_EFFECT && gBattleWeather & WEATHER_SANDSTORM_ANY)
            MulModifier(&modifier, UQ_4_12(0.5));
    }

    //Shell Armor & Battle Armor
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_SHELL_ARMOR || BATTLER_HAS_ABILITY(battlerDef, ABILITY_BATTLE_ARMOR))){
        MulModifier(&modifier, UQ_4_12(0.8));
    }

    //Lead Coat
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_LEAD_COAT)){
        MulModifier(&modifier, UQ_4_12(0.6));
    }

    //Parry
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_PARRY)){
        MulModifier(&modifier, UQ_4_12(0.8));
    }

    //Immunity
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_IMMUNITY)){
        if (moveType == TYPE_POISON)
            MulModifier(&modifier, UQ_4_12(0.5));
    }

    //Fossilized
    if(BATTLER_HAS_ABILITY(battlerDef, ABILITY_FOSSILIZED)){
        if (moveType == TYPE_ROCK)
            MulModifier(&modifier, UQ_4_12(0.5));
    }

    MulModifier(&movePower, modifier);

    return movePower;
}

static void PrintMoveInfo(u16 move, u8 x, u8 y, u8 moveIdx){
    u8 i, j;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 ability = gBattleMons[sMenuDataPtr->battlerId].ability;
    u16 gBattleMoveDamage;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;
    u8 mode = sMenuDataPtr->moveModeId;
    u8 x2 = 0;
    u8 y2 = -4;
    u8 target = BATTLE_OPPOSITE(sMenuDataPtr->battlerId);
    u32 movePower = gBattleMoves[move].power;
    u32 moveAccuracy = GetTotalAccuracy(sMenuDataPtr->battlerId, target, move);
    u8 moveType  = gBattleMoves[move].type;
    u8 moveType2 = TYPE_MYSTERY;
    bool32 updateFlags = FALSE;
    bool8 isStatusMove = gBattleMoves[move].split == SPLIT_STATUS;
    bool8 isStab = FALSE;
    struct Pokemon *party;

    //Party
    if (!isEnemyMon)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    //Sets move type depending on the mon ability/stats
    SetTypeBeforeUsingMove(move, sMenuDataPtr->battlerId);
    GET_MOVE_TYPE(move, moveType);

    //Sets move power depending on the mon ability/stats
    movePower = calculateTotalMoveDamage(move, sMenuDataPtr->battlerId, target, moveType);

    //MoveType2
    if(gBattleMoves[move].type2 != TYPE_MYSTERY && gBattleMoves[move].type2 != TYPE_NORMAL)
        moveType2 = gBattleMoves[move].type2;

    //Check Stab
    if((IS_BATTLER_OF_TYPE(sMenuDataPtr->battlerId, moveType) || (IS_BATTLER_OF_TYPE(sMenuDataPtr->battlerId, moveType2) && moveType2 != TYPE_MYSTERY)) && !isStatusMove)
        isStab = TRUE;
    
    //Move Name
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gMoveNamesLong[move]);
    //Type
    if(gBattleMoves[move].type2 == TYPE_MYSTERY || gBattleMoves[move].type2 == TYPE_NORMAL)
        StringCopy(gStringVar4, gTypeNames[moveType]);
    else{
        StringCopy(gStringVar1, gTypeNames[moveType]);
        StringCopy(gStringVar2, gTypeNames[gBattleMoves[move].type2]);
        StringExpandPlaceholders(gStringVar4, gText_Move_Type_TwoTypedMoves);
    }
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 1) * 8) + SPACE_BETWEEN_ABILITY_AND_NAME + 16, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar4);
    y++;

    //Stab Boost
    if(isStab)
        movePower = movePower * 1.5;
    
    switch(mode){
        case MOVE_MODE_NORMAL:
            // Move Power ---------------------------------------------------------------------------------------------------
            if(!isStatusMove)
                ConvertIntToDecimalStringN(gStringVar1, movePower, STR_CONV_MODE_LEFT_ALIGN, 3);
            else
                StringCopy(gStringVar1, gText_Target_Nothing);
            StringExpandPlaceholders(gStringVar4, gText_MoveInfo_Power);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            // Move Accuracy
            if(moveAccuracy != 0)
                ConvertIntToDecimalStringN(gStringVar1, moveAccuracy, STR_CONV_MODE_LEFT_ALIGN, 3);
            else
                StringCopy(gStringVar1, gText_Target_Nothing);
            StringExpandPlaceholders(gStringVar4, gText_MoveInfo_Accuracy);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            y++;
            // Move Priority ---------------------------------------------------------------------------------------------------
            if(gBattleMoves[move].priority >= 0)
                ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].priority, STR_CONV_MODE_LEFT_ALIGN, 3);
            else
                ConvertIntToDecimalStringN(gStringVar1, 0, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringExpandPlaceholders(gStringVar4, gText_MoveInfo_Priority);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            // Move Split
            switch(gBattleMoves[move].split){
                case SPLIT_PHYSICAL:
                    StringCopy(gStringVar1, gText_Split_Physical);
                break;
                case SPLIT_SPECIAL:
                    StringCopy(gStringVar1, gText_Split_Special);
                break;
                case SPLIT_STATUS:
                default:
                    StringCopy(gStringVar1, gText_Split_Status);
                break;
            }
            StringExpandPlaceholders(gStringVar4, gText_MoveInfo_Split);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            y++;
            //PP
            ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].pp[moveIdx],  STR_CONV_MODE_LEFT_ALIGN, 2); //Current PP
            ConvertIntToDecimalStringN(gStringVar2, CalculatePPWithBonus(move, GetMonData(&party[gBattlerPartyIndexes[sMenuDataPtr->battlerId]], MON_DATA_PP_BONUSES, NULL), moveIdx), STR_CONV_MODE_LEFT_ALIGN, 2); //Max PP
            StringExpandPlaceholders(gStringVar4, gText_CurrentPP);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            //Stab
            if(!isStatusMove){ //No stab boost for status moves
                if(isStab)
                    StringCopy(gStringVar1, gText_Boost_True);
                else
                    StringCopy(gStringVar1, gText_Boost_False);
                StringExpandPlaceholders(gStringVar4, gText_MoveInfo_STAB);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + SPACE_BETWEEN_ABILITY_AND_NAME, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            }
        break;
        case MOVE_MODE_DESCRIPTION:
            y2 = y2 + 4;
            StringCopy(gStringVar4, gMoveDescriptionPointers[move - 1]);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
        break;
        /*case MOVE_MODE_DAMAGE_CALCULATOR:
            if(gBattleMoves[move].split == SPLIT_STATUS || movePower == 0){
                StringCopy(gStringVar4, gText_Target_Nothing);
            }
            else{
                PrintDamageCalculation(sMenuDataPtr->battlerId, target, move);
            }
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
        break;*/
        case MOVE_MODE_AFFECTED_1:
            //Makes Contact
            if(gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
                StringCopy(gStringVar4, gText_Makes_Contact_True);
            else
                StringCopy(gStringVar4, gText_Makes_Contact_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            
            //Protect Affected 
            y++;
            if(gBattleMoves[move].flags & FLAG_PROTECT_AFFECTED)
                StringCopy(gStringVar4, gText_Protect_Affected_True);
            else
                StringCopy(gStringVar4, gText_Protect_Affected_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            //Magic Coat Affected
            y++;
            if(gBattleMoves[move].flags & FLAG_MAGIC_COAT_AFFECTED)
                StringCopy(gStringVar4, gText_Magic_Coat_Affected_True);
            else
                StringCopy(gStringVar4, gText_Magic_Coat_Affected_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
        break;
        case MOVE_MODE_AFFECTED_2:
            //Snatch Affected
            if(gBattleMoves[move].flags & FLAG_SNATCH_AFFECTED)
                StringCopy(gStringVar4, gText_Snatch_Affected_True);
            else
                StringCopy(gStringVar4, gText_Snatch_Affected_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            
            //Mirror Move Affected
            y++;
            if(gBattleMoves[move].flags & FLAG_PROTECT_AFFECTED)
                StringCopy(gStringVar4, gText_Mirror_Move_Affected_True);
            else
                StringCopy(gStringVar4, gText_Mirror_Move_Affected_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            //King's Rock Affected
            y++;
            if(gBattleMoves[move].flags & FLAG_MAGIC_COAT_AFFECTED)
                StringCopy(gStringVar4, gText_Kings_Rock_Affected_True);
            else
                StringCopy(gStringVar4, gText_Kings_Rock_Affected_False);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
        break;
    }
}

static void PrintStatusTab(void){
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 move1   = gBattleMons[sMenuDataPtr->battlerId].moves[0];
    u16 move2   = gBattleMons[sMenuDataPtr->battlerId].moves[1];
    u16 move3   = gBattleMons[sMenuDataPtr->battlerId].moves[2];
    u16 move4   = gBattleMons[sMenuDataPtr->battlerId].moves[3];
    u16 ability = gBattleMons[sMenuDataPtr->battlerId].ability;
    u16 gBattleMoveDamage;
    u8 target = 1;
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = 0;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Battler_Status);
    x  = 19;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

static void PrintDamageCalulatorTab(void){
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 moves[MAX_MON_MOVES] = {gBattleMons[sMenuDataPtr->battlerId].moves[0], 
                                gBattleMons[sMenuDataPtr->battlerId].moves[1],
                                gBattleMons[sMenuDataPtr->battlerId].moves[2], 
                                gBattleMons[sMenuDataPtr->battlerId].moves[3]};
    u16 ability = gBattleMons[sMenuDataPtr->battlerId].ability;
    u16 gBattleMoveDamage;
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;
    u16 move;
    u8 target = BATTLE_OPPOSITE(sMenuDataPtr->battlerId);

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = 0;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Battler_Damage);
    x  = 20;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    // First Move
    x  = 9;
    y  = 3;
    x2 = 0;
    y2 = -4;
    for(i = 0; i < MAX_MON_MOVES; i++){
        //Move Name
        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gMoveNamesLong[moves[i]]);

        // Move Damage ---------------------------------------------------------------------------------------------------
        y++;
        if(gBattleMoves[moves[i]].split == SPLIT_STATUS || gBattleMoves[moves[i]].power <= 2){
            StringCopy(gStringVar4, gText_Target_Nothing);
        }
        else{
            PrintDamageCalculation(sMenuDataPtr->battlerId, target, moves[i]);
        }
        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
        y = y + 3;
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

#define HP_EV_INDEX    0
#define ATK_EV_INDEX   1
#define DEF_EV_INDEX   2
#define SPEED_EV_INDEX 3
#define SPATK_EV_INDEX 4
#define SPDEF_EV_INDEX 5

const u8 gText_SmogonDamageCalculator_FirstPartText_SpecialDefense[]              = _("{STR_VAR_1} SpA {STR_VAR_2} {STR_VAR_3} vs.");
const u8 gText_SmogonDamageCalculator_FirstPartText_SpecialDefense_NatureUp[]     = _("{STR_VAR_1} SpA+ {STR_VAR_2} {STR_VAR_3} vs.");
const u8 gText_SmogonDamageCalculator_FirstPartText_SpecialDefense_NatureDown[]   = _("{STR_VAR_1} SpA- {STR_VAR_2} {STR_VAR_3} vs.");
const u8 gText_SmogonDamageCalculator_FirstPartText_PhysicalDefense[]             = _("{STR_VAR_1} Atk {STR_VAR_2} {STR_VAR_3} vs.");
const u8 gText_SmogonDamageCalculator_FirstPartText_PhysicalDefens_NatureUp[]     = _("{STR_VAR_1} Atk+ {STR_VAR_2} {STR_VAR_3} vs.");
const u8 gText_SmogonDamageCalculator_FirstPartText_PhysicalDefens_NatureDown[]   = _("{STR_VAR_1} Atk- {STR_VAR_2} {STR_VAR_3} vs.");
//{STR_VAR_1}+ = Special Attack EV, {STR_VAR_2} = Pokemon Name, {STR_VAR_3} = Move Name
const u8 gText_SmogonDamageCalculator_SecondPartText_SpecialDefense[]             = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} SpD");
const u8 gText_SmogonDamageCalculator_SecondPartText_SpecialDefense_NatureUp[]    = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} SpD+");
const u8 gText_SmogonDamageCalculator_SecondPartText_SpecialDefense_NatureDown[]  = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} SpD-");
const u8 gText_SmogonDamageCalculator_SecondPartText_PhysicalDefense[]            = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} Def");
const u8 gText_SmogonDamageCalculator_SecondPartText_PhysicalDefense_NatureUp[]   = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} Def+");
const u8 gText_SmogonDamageCalculator_SecondPartText_PhysicalDefense_NatureDown[] = _("{STR_VAR_1}\n{STR_VAR_2} HP/ {STR_VAR_3} Def-");
//{STR_VAR_1} = First Part {STR_VAR_2} = HP EV, {STR_VAR_3} = Special Defense EV
const u8 gText_SmogonDamageCalculator_ThirdPart[] = _("{STR_VAR_1} {STR_VAR_2}: {STR_VAR_3}");
//{STR_VAR_1} = Second Part {STR_VAR_2} = Target Name, {STR_VAR_3} = Min Damage
const u8 gText_SmogonDamageCalculator_FourthPart[] = _("{STR_VAR_1}-{STR_VAR_2}\n({STR_VAR_3} - ");
//{STR_VAR_1} = Third Part {STR_VAR_2} = Max Damage, {STR_VAR_3} = Min Percent
const u8 gText_SmogonDamageCalculator_FifthPart[] = _("{STR_VAR_1} {STR_VAR_2}%) -- {STR_VAR_3}% chance to ");
//{STR_VAR_1} = Foruth Part {STR_VAR_2} = Max Percent, {STR_VAR_3} = Chance
const u8 gText_SmogonDamageCalculator_SixthPart[] = _("{STR_VAR_1} {STR_VAR_2}HKO");
//{STR_VAR_1} = Fifth Part {STR_VAR_2} = 2HKO or 3HKO

#define MAX_DAMAGE_FACTOR 0
#define MIN_DAMAGE_FACTOR 16

static void PrintDamageCalculation(u8 battler, u8 target, u16 move){
    s32 dmg, moveType, critDmg, minDamage, maxDamage, critChance;
    u8 percentage, chance, i, j;
    struct Pokemon *party, *targetParty;
    u8 TargetEvs[NUM_STATS];
    u8 BattlerEvs[NUM_STATS];
    u16 targetCurrentHp = gBattleMons[target].hp;

    //252+ SpA Abomasnow Blizzard vs. 168 HP / 0 SpD Abomasnow: 178-211 (49 - 58.1%) -- 97.7% chance to 2HKO

    //To Calculate EVs
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        party = gPlayerParty;
    else
        party = gEnemyParty;

    if (GetBattlerSide(target) == B_SIDE_PLAYER)
        targetParty = gPlayerParty;
    else
        targetParty = gEnemyParty;

    for(i = 0 ; i < NUM_STATS; i++){
        BattlerEvs[i] = GetMonData(&party[gBattlerPartyIndexes[battler]],      MON_DATA_HP_EV + i, NULL);
        TargetEvs[i]  = GetMonData(&targetParty[gBattlerPartyIndexes[target]], MON_DATA_HP_EV + i, NULL);
    }

    //Damage Calculation
    minDamage = DoMoveDamageCalcBattleMenu(move, sMenuDataPtr->battlerId, target, moveType, FALSE, MIN_DAMAGE_FACTOR);
    maxDamage = DoMoveDamageCalcBattleMenu(move, sMenuDataPtr->battlerId, target, moveType, FALSE, MAX_DAMAGE_FACTOR);

    //First Part
    StringCopy(gStringVar2, gSpeciesNames[gBattleMons[sMenuDataPtr->battlerId].species]);
    StringCopy(gStringVar3, gMoveNames[move]);
    if(gBattleMoves[move].split == SPLIT_SPECIAL){
        ConvertIntToDecimalStringN(gStringVar1, BattlerEvs[SPATK_EV_INDEX], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_FirstPartText_SpecialDefense);
    }
    else{
        ConvertIntToDecimalStringN(gStringVar1, BattlerEvs[ATK_EV_INDEX], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_FirstPartText_PhysicalDefense);
    }

    //Second Part
    StringCopy(gStringVar1, gStringVar4);
    ConvertIntToDecimalStringN(gStringVar2, TargetEvs[HP_EV_INDEX], STR_CONV_MODE_LEFT_ALIGN, 3);
    if(gBattleMoves[move].split == SPLIT_SPECIAL){
        ConvertIntToDecimalStringN(gStringVar3, TargetEvs[SPDEF_EV_INDEX], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_SecondPartText_SpecialDefense);
    }
    else{
        ConvertIntToDecimalStringN(gStringVar3, TargetEvs[DEF_EV_INDEX], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_SecondPartText_PhysicalDefense);
    }
        
    //Third Part
    StringCopy(gStringVar1, gStringVar4);
    StringCopy(gStringVar2, gSpeciesNames[gBattleMons[target].species]);
    ConvertIntToDecimalStringN(gStringVar3, minDamage, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_ThirdPart);

    //Fourth Part gText_SmogonDamageCalculator_FourthPart
    StringCopy(gStringVar1, gStringVar4);
    ConvertIntToDecimalStringN(gStringVar2, maxDamage, STR_CONV_MODE_LEFT_ALIGN, 4);
    percentage = (minDamage * 100)/ targetCurrentHp; // Min Damage Percentage
    ConvertIntToDecimalStringN(gStringVar3, percentage, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_FourthPart);

    //Fifth Part 
    StringCopy(gStringVar1, gStringVar4);
    percentage = (maxDamage * 100)/ targetCurrentHp; // Max Damage Percentage
    ConvertIntToDecimalStringN(gStringVar2, percentage, STR_CONV_MODE_LEFT_ALIGN, 3);
    chance = 100; //Todo: Calculate Chance
    ConvertIntToDecimalStringN(gStringVar3, chance, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_FifthPart);

    //Sixth Part
    StringCopy(gStringVar1, gStringVar4);
    chance = targetCurrentHp / maxDamage;
    ConvertIntToDecimalStringN(gStringVar2, chance, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_SmogonDamageCalculator_SixthPart);
}

const u8 sText_Title_Field_Turns_Left[]                   = _("Turns Left:");
const u8 sText_Title_Field_Weather[]                      = _("Weather: {STR_VAR_1}");
const u8 sText_Title_Field_Weather_Rain[]                 = _("Rain");
const u8 sText_Title_Field_Weather_Sun[]                  = _("Sun");
const u8 sText_Title_Field_Weather_Sandstorm[]            = _("Sandstorm");
const u8 sText_Title_Field_Weather_Hail[]                 = _("Hail");
const u8 sText_Title_Field_Weather_Snow[]                 = _("Snow");
const u8 sText_Title_Field_Weather_Strong_Winds[]         = _("Strong Winds");
const u8 sText_Title_Field_Weather_Rain_Primal[]          = _("Primal Rain");
const u8 sText_Title_Field_Weather_Sun_Primal[]           = _("Primal Sun");

const u8 sText_Title_Field_Weather_Description_Rain[]         = _("Strengthens Water-type moves by\n"
                                                                  "50% while weakening Fire-type\n"
                                                                  "moves by 50%.");
const u8 sText_Title_Field_Weather_Description_Sun[]          = _("Strengthens the power of Fire-type\n"
                                                                  "moves by 50% and weakens Water-type\n"
                                                                  "moves by 50%.");
const u8 sText_Title_Field_Weather_Description_Sandstorm[]    = _("Any Pokémon that is not Rock, Ground\n"
                                                                  "or Steel-type will be damaged for\n"
                                                                  "1/16 of its maximum HP each turn.");
const u8 sText_Title_Field_Weather_Description_Hail[]         = _("Any Pokémon that is not Ice-type\n"
                                                                  "will be damaged each turn, Pokémon\n"
                                                                  "are twice as likely to get frostbite.");
const u8 sText_Title_Field_Weather_Description_Strong_Winds[] = _("Causes Electric, Ice, and Rock-type\n"
                                                                  "moves to deal neutral damage to \n"
                                                                  "Flying-type Pokémon.");
const u8 sText_Title_Field_Weather_Description_Primal_Rain[]  = _("Boosts the power of Water-type moves\n"
                                                                  "and protects Pokémon from Fire-type\n"
                                                                  "moves.");
const u8 sText_Title_Field_Weather_Description_Primal_Sun[]  = _("Boosts the power of Fire-type moves\n"
                                                                 "and protects Pokémon from Water\n"
                                                                 "-type moves.");

const u8 sText_Title_Field_Terrain[]                      = _("Terrain: {STR_VAR_1}");
const u8 sText_Title_Field_Terrain_Electric[]             = _("Electric");
const u8 sText_Title_Field_Terrain_Psychic[]              = _("Psychic");
const u8 sText_Title_Field_Terrain_Misty[]                = _("Misty");
const u8 sText_Title_Field_Terrain_Grassy[]               = _("Grassy");

const u8 sText_Title_Field_Terrain_Description_Electric[] = _("Pokémon on the ground won't fall\n"
                                                              "asleep. The power of Electric-type\n"
                                                              "moves is boosted.");
const u8 sText_Title_Field_Terrain_Description_Psychic[]  = _("Pokémon on the ground won't be hit\n"
                                                              "by priority moves. The power of\n"
                                                              "Psychic-type moves is boosted.");
const u8 sText_Title_Field_Terrain_Description_Misty[]    = _("Pokémon on the ground won't get any\n"
                                                              "status conditions, Damage from Dragon\n"
                                                              "type moves is halved.");
const u8 sText_Title_Field_Terrain_Description_Grassy[]   = _("Increases the power of Grass-type\n"
                                                              "moves, Restores 1/16 HP to all\n"
                                                              "Pokémon on the ground each turn.");

const u8 sText_Title_Field_Room[]                         = _("Room Type:");
const u8 sText_Title_Field_Trick_Room[]                   = _("Trick Room");
const u8 sText_Title_Field_Room_Description_Trick[]       = _("The move order is reversed, slower\n"
                                                              "Pokémon will attack before faster\n"
                                                              "Pokémon.");
const u8 sText_Title_Field_Wonder_Room[]                  = _("Wonder Room:");
const u8 sText_Title_Field_Room_Description_Wonder[]       = _("The move order is reversed, slower\n"
                                                              "Pokémon will attack before faster\n"
                                                              "Pokémon.");
const u8 sText_Title_Field_Magic_Room[]                   = _("Magic Room:");
const u8 sText_Title_Field_Room_Description_Magic[]       = _("The move order is reversed, slower\n"
                                                              "Pokémon will attack before faster\n"
                                                              "Pokémon.");
const u8 sText_Title_Field_Gravity[]                      = _("Gravity");
const u8 sText_Title_Field_Gravity_Description   []       = _("The move order is reversed, slower\n"
                                                              "Pokémon will attack before faster\n"
                                                              "Pokémon.");

const u8 sText_Title_Field_Nature_Power[]                 = _("Nature Power:");
const u8 sText_Title_Field_Secret_Power[]                 = _("Secret Power:");
const u8 sText_Title_Field_Paralysis[]                    = _("Causes Paralysis");

const u8 sText_Title_Field_Active[]                       = _("Active");
const u8 sText_Title_Field_Not_Active[]                   = _("Not Active");
const u8 sText_Title_Field_None[]                         = _("None");

#define SPACE_BETWEEN_LINES_FIELD ((6 * 8) + 4)
#define MAX_DESCRIPTION_LINES 3
#define LINES_BETWEEN_STUFF 1
static void PrintFieldTab(void)
{
    u8 i, j;
    u8 x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u8 turnsLeft = 5;
    bool8 printedInfo = FALSE;
    u8 maxLines = 3;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = -4;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Stats);
    x  = 19;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    // Weather
    x  = 9;
    y  = 3;
    x2 = 0;
    y2 = -4;

    if(sMenuDataPtr->numFields < maxLines)
        maxLines = sMenuDataPtr->numFields;
    
    for(i = 0; i < maxLines; i++){
        printedInfo = FALSE;
        //switch(sMenuDataPtr->fieldInfo[(i ) % NUM_FIELD_INFO]){
        switch(sMenuDataPtr->fieldInfo[(i + sMenuDataPtr->currentFieldInfo) % sMenuDataPtr->numFields]){
            case FIELD_INFO_WEATHER:
                //Title
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Weather);
                if((gBattleWeather & WEATHER_RAIN_ANY) && !(gBattleWeather & WEATHER_RAIN_PRIMAL))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Rain);
                else if((gBattleWeather & WEATHER_SUN_ANY) && !(gBattleWeather & WEATHER_SUN_PRIMAL))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Sun);
                else if((gBattleWeather & WEATHER_SANDSTORM_ANY))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Sandstorm);
                else if((gBattleWeather & WEATHER_HAIL_ANY))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Hail);
                else if((gBattleWeather & WEATHER_STRONG_WINDS))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Strong_Winds);
                else if((gBattleWeather & WEATHER_RAIN_PRIMAL))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Rain_Primal);
                else if((gBattleWeather & WEATHER_SUN_PRIMAL))
                    StringCopy(gStringVar1, sText_Title_Field_Weather_Sun_Primal);
                else
                    StringCopy(gStringVar1, sText_Title_Field_None);

                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + SPACE_BETWEEN_LINES_FIELD, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
            
                if((gBattleWeather & WEATHER_ANY)){
                    //Turns Left
                    if(!(gBattleWeather & WEATHER_RAIN_PRIMAL) && !(gBattleWeather & WEATHER_SUN_PRIMAL)){
                        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Turns_Left);
                        ConvertIntToDecimalStringN(gStringVar1, gWishFutureKnock.weatherDuration, STR_CONV_MODE_LEFT_ALIGN, 4);
                        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                    }

                    //Weather Description
                    if((gBattleWeather & WEATHER_STRONG_WINDS))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Strong_Winds);
                    else if((gBattleWeather & WEATHER_RAIN_PRIMAL))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Primal_Rain);
                    else if((gBattleWeather & WEATHER_SUN_PRIMAL))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Primal_Sun);
                    else if((gBattleWeather & WEATHER_RAIN_ANY))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Rain);
                    else if((gBattleWeather & WEATHER_SUN_ANY))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Sun);
                    else if((gBattleWeather & WEATHER_SANDSTORM_ANY))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Sandstorm);
                    else if((gBattleWeather & WEATHER_HAIL_ANY))
                        StringCopy(gStringVar1, sText_Title_Field_Weather_Description_Hail);
                    
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);
                }

                printedInfo = TRUE;
            break;
            case FIELD_INFO_TERRAIN:
                //Terrain
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Terrain);
                if(gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN)
                    StringCopy(gStringVar1, sText_Title_Field_Terrain_Electric);
                else if(gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN)
                    StringCopy(gStringVar1, sText_Title_Field_Terrain_Grassy);
                else if(gFieldStatuses & STATUS_FIELD_MISTY_TERRAIN)
                    StringCopy(gStringVar1, sText_Title_Field_Terrain_Misty);
                else if(gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
                    StringCopy(gStringVar1, sText_Title_Field_Terrain_Psychic);
                else
                    StringCopy(gStringVar1, sText_Title_Field_None);

                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + SPACE_BETWEEN_LINES_FIELD, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                //Turns Left
                if(gFieldStatuses & STATUS_FIELD_TERRAIN_ANY){
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Turns_Left);
                    ConvertIntToDecimalStringN(gStringVar1, gFieldTimers.terrainTimer, STR_CONV_MODE_LEFT_ALIGN, 4);
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                    //Terrain Description
                    if((gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN))
                        StringCopy(gStringVar1, sText_Title_Field_Terrain_Description_Electric);
                    else if((gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN))
                        StringCopy(gStringVar1, sText_Title_Field_Terrain_Description_Grassy);
                    else if((gFieldStatuses & STATUS_FIELD_MISTY_TERRAIN))
                        StringCopy(gStringVar1, sText_Title_Field_Terrain_Description_Misty);
                    else if((gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN))
                        StringCopy(gStringVar1, sText_Title_Field_Terrain_Description_Psychic);

                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);
                }

                printedInfo = TRUE;
            break;
            case FIELD_INFO_GRAVITY:
                StringCopy(gStringVar1, sText_Title_Field_Gravity);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);

                //Turns Left
                StringCopy(gStringVar1, sText_Title_Field_Turns_Left);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                turnsLeft = gFieldTimers.gravityTimer;
                ConvertIntToDecimalStringN(gStringVar1, turnsLeft, STR_CONV_MODE_LEFT_ALIGN, 4);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                
                //Description
                StringCopy(gStringVar1, sText_Title_Field_Gravity_Description);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);

                printedInfo = TRUE;
            break;
            case FIELD_INFO_TRICK_ROOM:
                StringCopy(gStringVar1, sText_Title_Field_Trick_Room);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);

                //Turns Left
                StringCopy(gStringVar1, sText_Title_Field_Turns_Left);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                turnsLeft = gFieldTimers.trickRoomTimer;
                ConvertIntToDecimalStringN(gStringVar1, turnsLeft, STR_CONV_MODE_LEFT_ALIGN, 4);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                
                //Description
                StringCopy(gStringVar1, sText_Title_Field_Room_Description_Trick);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);

                printedInfo = TRUE;
            break;
            case FIELD_INFO_WONDER_ROOM:
                StringCopy(gStringVar1, sText_Title_Field_Wonder_Room);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);

                //Turns Left
                StringCopy(gStringVar1, sText_Title_Field_Turns_Left);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                turnsLeft = gFieldTimers.wonderRoomTimer;
                ConvertIntToDecimalStringN(gStringVar1, turnsLeft, STR_CONV_MODE_LEFT_ALIGN, 4);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                
                //Description
                StringCopy(gStringVar1, sText_Title_Field_Room_Description_Wonder);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);

                printedInfo = TRUE;
            break;
            case FIELD_INFO_MAGIC_ROOM:
                StringCopy(gStringVar1, sText_Title_Field_Magic_Room);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);

                //Turns Left
                StringCopy(gStringVar1, sText_Title_Field_Turns_Left);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 2), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                turnsLeft = gFieldTimers.magicRoomTimer;
                ConvertIntToDecimalStringN(gStringVar1, turnsLeft, STR_CONV_MODE_LEFT_ALIGN, 4);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 3), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
                
                //Description
                StringCopy(gStringVar1, sText_Title_Field_Room_Description_Magic);
                AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, ((y + 1) * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLACK], 0xFF, gStringVar1);

                printedInfo = TRUE;
            break;
        }
        if(printedInfo)
            y = y + MAX_DESCRIPTION_LINES + 2;
    }

    /*//Nature Power
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Field_Nature_Power);
    StringCopy(gStringVar1, gMoveNamesLong[MOVE_THUNDERBOLT]);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 1.5), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);

    //Secret Power
    y++;
    StringCopy(gStringVar1, sText_Title_Field_Secret_Power);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
    StringCopy(gStringVar1, sText_Title_Field_Paralysis);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2 + (SPACE_BETWEEN_LINES_FIELD * 1.5), (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, gStringVar1);
    */

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

//gText_MoveInfo_Priority
const u8 gText_SpeedInfo_CanKOThisTurn[] = _("Can KO {STR_VAR_1} this turn:\n{STR_VAR_2}");
#define SPEED_POKEMON_ICON_X (9 * 8) + 4
#define SPEED_POKEMON_ICON_Y (6 * 8) + 1
#define SPEED_POKEMON_SPACE  (8 * 8)

const u8 gText_BattlerLevelString[] = _("{LV}{STR_VAR_1}");
const u8 gText_PrintSpeedTabStats[] = _("Total Speed: {STR_VAR_1}");

static void PrintSpeedTab(void)
{
    //Single Battle Only
    u16 targetCurrentHp, move, moveDamage, species, targetSpecies;
    u8 i, j, target, x, y, x2, y2;
    u8 windowId = WINDOW_1;
    u8 colorIdx = FONT_BLACK;
    u8 sBattlerByTurnOrder[gBattlersCount];

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //Calculate Order
    for (i = 0; i < gBattlersCount; i++)
        sBattlerByTurnOrder[i] = i;

    for (i = 0; i < gBattlersCount - 1; i++)
    {
        for (j = i + 1; j < gBattlersCount; j++)
        {
            if (GetWhoStrikesFirst(sBattlerByTurnOrder[i], sBattlerByTurnOrder[j], TRUE) != 0)
            {
                u8 temp = sBattlerByTurnOrder[i];
                sBattlerByTurnOrder[i] = sBattlerByTurnOrder[j];
                sBattlerByTurnOrder[j] = temp;
            }
        }
    }

    //Mon Icon
    for(i = 0; i < gBattlersCount; i++){
        if(IsBattlerAlive(sBattlerByTurnOrder[i])){
            ShowSpeciesIconSpeed(sBattlerByTurnOrder[i], SPEED_POKEMON_ICON_X, SPEED_POKEMON_ICON_Y + (i * SPEED_POKEMON_SPACE));
        }
    }

    //Title
    x  = 9;
    y  = 0;
    x2 = 0;
    y2 = -4;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Speed_Order);
    x  = 19;
    AddTextPrinterParameterized4(windowId, FONT_SMALL, (x * 8), (y * 8), 0, 0, sMenuWindowFontColors[FONT_WHITE], 0xFF, sText_Title_Controllers);

    //Speed Order
    x  = 13;
    y  = 4;
    x2 = 0;
    y2 = -4;

    for(i = 0; i < gBattlersCount; i++){
        if(IsBattlerAlive(sBattlerByTurnOrder[i])){
            u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sBattlerByTurnOrder[i]].species, gBattleMons[sBattlerByTurnOrder[i]].personality);
            u16 speed = GetBattlerTotalSpeedStat(sBattlerByTurnOrder[i]);
            species = gBattleMons[sBattlerByTurnOrder[i]].species;
            target = BATTLE_OPPOSITE(sBattlerByTurnOrder[i]);
            targetCurrentHp = gBattleMons[target].hp;
            
            //Species Name
            StringCopy(gStringVar1, gSpeciesNames[species]);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
            x = x + 8;

            //Level
            ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sBattlerByTurnOrder[i]].level, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringExpandPlaceholders(gStringVar4, gText_BattlerLevelString);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            x = x + 4;

            switch(gender){
                case MON_MALE:
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLUE], 0xFF, gText_MaleSymbol);
                    break;
                case MON_FEMALE:
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_RED], 0xFF, gText_FemaleSymbol);
                    break;
            }
            x = 13;
            y++;
            //Total Stats 9 x 8
            ConvertIntToDecimalStringN(gStringVar1, speed, STR_CONV_MODE_LEFT_ALIGN, 3);
            //StringExpandPlaceholders(gStringVar4, gText_PrintSpeedTabStats);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (8 * 8) + 4, (10 * 8) - 4 + (SPEED_POKEMON_SPACE * i), 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
            y = y + 2;

            //Move Names
            for(j = 0; j < MAX_MON_MOVES; j++){
                move = gBattleMons[sBattlerByTurnOrder[i]].moves[j];
                if(move != MOVE_NONE){
                    StringCopy(gStringVar1, gMoveNames[move]);
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
                    x = x + 9;
                    //Priority
                    ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].priority, STR_CONV_MODE_LEFT_ALIGN, 3);
                    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
                    x = x + 3;
                    x2 = 4;
                    //Can KO - Todo: Check calculation
                    if(gBattleMoves[move].split != SPLIT_STATUS && gBattleMoves[move].power > 0){
                        u8 moveType = gBattleMoves[move].type;
                        SetTypeBeforeUsingMove(move, sBattlerByTurnOrder[i]);
                        GET_MOVE_TYPE(move, moveType);
                        moveDamage = CalculateMoveDamage(move, sBattlerByTurnOrder[i], target, moveType, 0, FALSE, FALSE, FALSE);
                        if(targetCurrentHp <= moveDamage)
                            BlitBitmapToWindow(windowId, sCheck, (x * 8) + x2, (y * 8), 8, 8);
                    }
                    x = x - 12;
                    x2 = 0;
                    //
                    y++;
                }
            }
            y++;
        }
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

static void PrintToWindow(u8 windowId, u8 colorIdx)
{
    const u8 *str = sText_MyMenu;
    u8 i, j;
    u8 x, y, x2, y2;
    u16 species = gBattleMons[sMenuDataPtr->battlerId].species;
    u16 innate1 = gBaseStats[species].innates[0];
    u16 innate2 = gBaseStats[species].innates[1];
    u16 innate3 = gBaseStats[species].innates[2];
    u32 personality = gBattleMons[sMenuDataPtr->battlerId].personality;
    u8 gender = GetGenderFromSpeciesAndPersonality(gBattleMons[sMenuDataPtr->battlerId].species, gBattleMons[sMenuDataPtr->battlerId].personality);
    u8 statStage;
    bool8 statStageUp = FALSE;
    bool8 isEnemyMon = GetBattlerSide(sMenuDataPtr->battlerId) == B_SIDE_OPPONENT;
    u8 move = MOVE_NONE;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    if(!isEnemyMon){ //Enemy Mons have disabled randomized innates/abilies 
        innate1 = RandomizeInnate(gBaseStats[species].innates[0], species, personality);
        innate2 = RandomizeInnate(gBaseStats[species].innates[1], species, personality);
        innate3 = RandomizeInnate(gBaseStats[species].innates[2], species, personality);
    }

    //Title
    x  = 7;
    y  = 2;
    x2 = 0;
    y2 = -4;
    if(isEnemyMon)
        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sYourOpponentPokemonData);
    else
        AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sYourPokemonData);

    //Pokemon Name
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gSpeciesNames[species]);
    //Pokemon HP
    y++;
    ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, gBattleMons[sMenuDataPtr->battlerId].maxHP, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_HP);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
    //Pokemon Gender
    x = 2;
    y = 6;
    switch(gender){
        case MON_MALE:
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_BLUE], 0xFF, gText_MaleSymbol);
            break;
        case MON_FEMALE:
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[FONT_RED], 0xFF, gText_FemaleSymbol);
            break;
    }

    //Pokemon Level
    x++;
    ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].level, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_NewLevelSymbol);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);

    //Held Item
    x  = 22;
    y  = 3;
    if (gBattleMons[sMenuDataPtr->battlerId].item == ITEM_NONE)
        StringCopy(gStringVar1, sText_None);
    else
        StringCopy(gStringVar1, ItemId_GetName(gBattleMons[sMenuDataPtr->battlerId].item));
    StringExpandPlaceholders(gStringVar4, sText_Held_Item);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);

    //Stats
    x  = 3;
    y  = 7;
    //Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Attack);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].attack, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Defense);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].defense, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Special Attack
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialAttack);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].spAttack, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Special Defense
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_SpecialDefense);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].spDefense, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);
    y++;
    //Speed
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Speed);
	ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].speed, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 3) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar1);

    //Accuracy
    y = y + 2;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Accuracy);
    //Evasion
    y++;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Evasion);

    //Abilitites
    x = 15;
    y = 7;
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Ability);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 5) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityNames[gBattleMons[sMenuDataPtr->battlerId].ability]);
    y++;
    //Innate 1
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Innate);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 5) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityNames[innate1]);
    y++;
    //Innate 2
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Innate);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 5) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityNames[innate2]);
    y++;
    //Innate 3
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, sText_Innate);
    AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 5) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gAbilityNames[innate3]);

    //Moves
    y = y+2;
    for(i = 0; i < MAX_MON_MOVES; i++){
        move = gBattleMons[sMenuDataPtr->battlerId].moves[i];
        if(move != MOVE_NONE){
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, (x * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gMoveNames[move]);
            //Current PP
            ConvertIntToDecimalStringN(gStringVar1, gBattleMons[sMenuDataPtr->battlerId].pp[i], STR_CONV_MODE_LEFT_ALIGN, 3); //Current PP
            ConvertIntToDecimalStringN(gStringVar2, gBattleMoves[move].pp, STR_CONV_MODE_LEFT_ALIGN, 3); //Max PP, ToFix
            StringExpandPlaceholders(gStringVar4, sText_PP);
            AddTextPrinterParameterized4(windowId, FONT_SMALL_NARROW, ((x + 8) * 8) + x2, (y * 8) + y2, 0, 0, sMenuWindowFontColors[colorIdx], 0xFF, gStringVar4);
            y++;
        }
    }

    //Stat Drops & Ups
    x = 8;
    y = 7;
    for(i = 0; i < NUM_BATTLE_STATS - 1; i++){
        
        statStage = gBattleMons[sMenuDataPtr->battlerId].statStages[statorder[i + 1]];//HP is not taken into account
        if(statStage != DEFAULT_STAT_STAGE){
            if(statStage > DEFAULT_STAT_STAGE){
                statStageUp = TRUE;
                statStage = statStage - DEFAULT_STAT_STAGE;
            }
            else{
                statStageUp = FALSE;
                statStage = DEFAULT_STAT_STAGE - statStage;
            }

            for(j = 0; j < statStage; j++){
                if(statStageUp)
                    BlitBitmapToWindow(windowId, sStatUpArrow, ((x + j) * 8) + x2, (y * 8), 8, 8);
                else
                    BlitBitmapToWindow(windowId, sStatDownArrow, ((x + j) * 8) + x2, (y * 8), 8, 8);

            }
        }
        if(statorder[i + 1] == STAT_SPEED)
            y = y + 2;
        else
            y++;
    }
    
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, 3);
}

static void Task_MenuWaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_MenuMain;
}

static void Task_MenuTurnOff(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        SetMainCallback2(sMenuDataPtr->savedCallback);
        Menu_FreeResources();
        DestroyTask(taskId);
    }
}
//Status

static const union AnimCmd sSpriteAnim_StatusPoison[] = {
    ANIMCMD_FRAME(0, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusParalyzed[] = {
    ANIMCMD_FRAME(4, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusSleep[] = {
    ANIMCMD_FRAME(8, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusFrozen[] = {
    ANIMCMD_FRAME(12, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusBurn[] = {
    ANIMCMD_FRAME(16, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusPokerus[] = {
    ANIMCMD_FRAME(20, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusFaint[] = {
    ANIMCMD_FRAME(24, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_StatusFrostbite[] = {
    ANIMCMD_FRAME(28, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd *const sSpriteAnimTable_StatusCondition[] = {
    sSpriteAnim_StatusPoison,
    sSpriteAnim_StatusParalyzed,
    sSpriteAnim_StatusSleep,
    sSpriteAnim_StatusFrozen,
    sSpriteAnim_StatusBurn,
    sSpriteAnim_StatusPokerus,
    sSpriteAnim_StatusFaint,
    sSpriteAnim_StatusFrostbite,
};
static const struct OamData sOamData_StatusCondition =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x8),
    .tileNum = 0,
    .priority = 3,
    .paletteNum = 0,
    .affineParam = 0,
};
#define TAG_MON_STATUS      30001
static const struct SpriteTemplate sSpriteTemplate_StatusCondition =
{
    .tileTag = TAG_MON_STATUS,
    .paletteTag = TAG_MON_STATUS,
    .oam = &sOamData_StatusCondition,
    .anims = sSpriteAnimTable_StatusCondition,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

#define STATUS_ICON_X (7 * 8)
#define STATUS_ICON_Y (5 * 8)
static void CreateSetStatusSprite(void)
{
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_ID_STATUS];
    u8 statusAnim;

    if (*spriteId == SPRITE_NONE)
        *spriteId = CreateSprite(&sSpriteTemplate_StatusCondition, STATUS_ICON_X, STATUS_ICON_Y, 0);

    //statusAnim = gBattleMons[sMenuDataPtr->battlerId].status1;
    statusAnim = sMenuDataPtr->battlerId;
    if (statusAnim != 0)
    {
        StartSpriteAnim(&gSprites[*spriteId], statusAnim - 1);
        SetSpriteInvisibility(SPRITE_ARR_ID_STATUS, FALSE);
    }
    else
    {
        SetSpriteInvisibility(SPRITE_ARR_ID_STATUS, TRUE);
    }
}

static u8 DestroyBattleMenuSprite(u8 spriteArrayId)
{
    struct Sprite *sprite = &gSprites[sMenuDataPtr->spriteIds[spriteArrayId]];
    sMenuDataPtr->spriteIds[spriteArrayId] = SPRITE_NONE;
    DestroySpriteAndFreeResources(sprite);
}

static u8 ShowSpeciesIcon(u8 num)
{
	u16 species     = gBattleMons[num].species;
    u16 personality = gBattleMons[num].personality;
	LoadMonIconPalette(species);

    switch(num){
        case 0:
            sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1] = CreateMonIcon(species, SpriteCallbackDummy, POKEMON_ICON_X, POKEMON_ICON_1_Y, 0, personality);
                    
            gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1]].invisible = FALSE;
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1];
        break;
        case 1:
            sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2] = CreateMonIcon(species, SpriteCallbackDummy, POKEMON_ICON_X, POKEMON_ICON_2_Y, 0, personality);
                    
            gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2]].invisible = FALSE;
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2];
        break;
        case 2:
            if(IsDoubleBattle()){
                sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3] = CreateMonIcon(species, SpriteCallbackDummy, POKEMON_ICON_X, POKEMON_ICON_3_Y, 0, personality);
                        
                gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3]].invisible = FALSE;
            }
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3];
        break;
        case 3:
            if(IsDoubleBattle()){
                sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4] = CreateMonIcon(species, SpriteCallbackDummy, POKEMON_ICON_X, POKEMON_ICON_4_Y, 0, personality);
                        
                gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4]].invisible = FALSE;
            }
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4];
        break;
    }
}

static void FreeEveryMonIconSprite(void)
{
    u8 i;
    for (i = 0; i < gBattlersCount; i++)
        FreeSpeciesIconSprite(i);
}

static void FreeSpeciesIconSprite(u8 battler)
{
    u8 *spriteId = &sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1_SPEED + battler];
    if (*spriteId != SPRITE_NONE)
    {
        FreeSpriteTilesByTag(TAG_BATTLER_SPEED      + battler - 1);
        FreeSpritePaletteByTag(TAG_BATTLER_SPEED      + battler - 1);
        FreeSpriteOamMatrix(&gSprites[*spriteId]);
        DestroySprite(&gSprites[*spriteId]);
        *spriteId = SPRITE_NONE;
        sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1_SPEED + battler] = SPRITE_NONE;
    }
}

static u8 ShowSpeciesIconSpeed(u8 battler, u8 x, u8 y)
{
	u16 species     = gBattleMons[battler].species;
    u16 personality = gBattleMons[battler].personality;
	LoadMonIconPalette(species);

    switch(battler){
        case 0:
            sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1_SPEED] = CreateMonIcon(species, SpriteCallbackDummy, x, y, 0, personality);
                    
            gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1_SPEED]].invisible = FALSE;
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_1_SPEED];
        break;
        case 1:
            sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2_SPEED] = CreateMonIcon(species, SpriteCallbackDummy, x, y, 0, personality);
                    
            gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2_SPEED]].invisible = FALSE;
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_2_SPEED];
        break;
        case 2:
            if(IsDoubleBattle()){
                sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3_SPEED] = CreateMonIcon(species, SpriteCallbackDummy, x, y, 0, personality);
                        
                gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3_SPEED]].invisible = FALSE;
            }
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_3_SPEED];
        break;
        case 3:
            if(IsDoubleBattle()){
                sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4_SPEED] = CreateMonIcon(species, SpriteCallbackDummy, x, y, 0, personality);
                        
                gSprites[sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4_SPEED]].invisible = FALSE;
            }
            return sMenuDataPtr->spriteIds[SPRITE_ARR_ID_MON_ICON_4_SPEED];
        break;
    }
}
// different from pokemon_summary_screen
#define TYPE_ICON_PAL_NUM_0     13
#define TYPE_ICON_PAL_NUM_1     14
#define TYPE_ICON_PAL_NUM_2     15
static const u8 sMoveTypeToOamPaletteNum[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FIGHTING] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FLYING] = TYPE_ICON_PAL_NUM_1,
    [TYPE_POISON] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GROUND] = TYPE_ICON_PAL_NUM_0,
    [TYPE_ROCK] = TYPE_ICON_PAL_NUM_0,
    [TYPE_BUG] = TYPE_ICON_PAL_NUM_2,
    [TYPE_GHOST] = TYPE_ICON_PAL_NUM_1,
    [TYPE_STEEL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_MYSTERY] = TYPE_ICON_PAL_NUM_2,
    [TYPE_FIRE] = TYPE_ICON_PAL_NUM_0,
    [TYPE_WATER] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GRASS] = TYPE_ICON_PAL_NUM_2,
    [TYPE_ELECTRIC] = TYPE_ICON_PAL_NUM_0,
    [TYPE_PSYCHIC] = TYPE_ICON_PAL_NUM_1,
    [TYPE_ICE] = TYPE_ICON_PAL_NUM_1,
    [TYPE_DRAGON] = TYPE_ICON_PAL_NUM_2,
    [TYPE_DARK] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FAIRY] = TYPE_ICON_PAL_NUM_1,
};

static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible)
{
    gSprites[sMenuDataPtr->spriteIds[spriteArrayId]].invisible = invisible;
}

static void SetTypeIconSpritePosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId)
{
    struct Sprite *sprite = &gSprites[sMenuDataPtr->spriteIds[spriteArrayId]];
    StartSpriteAnim(sprite, typeId);
    sprite->oam.paletteNum = sMoveTypeToOamPaletteNum[typeId];
    sprite->x = x + 16;
    sprite->y = y + 8;
    SetSpriteInvisibility(spriteArrayId, FALSE);
}
/*
#define TYPE_ICON_Y (5 * 8)
#define TYPE_ICON_1_X (7 * 8)
#define TYPE_ICON_2_X TYPE_ICON_1_X + (4 * 8)
#define TYPE_ICON_3_X TYPE_ICON_2_X + (4 * 8)
static void SetMonTypeIcons(void)
{
    u8 type1 = gBattleMons[sMenuDataPtr->battlerId].type1;
    u8 type2 = gBattleMons[sMenuDataPtr->battlerId].type2;
    u8 type3 = gBattleMons[sMenuDataPtr->battlerId].type3;

    if (type1 != type2)
    {
        SetTypeIconSpritePosAndPal(type1, TYPE_ICON_1_X, TYPE_ICON_Y, SPRITE_ARR_ID_TYPE_1);
        SetTypeIconSpritePosAndPal(type2, TYPE_ICON_2_X, TYPE_ICON_Y, SPRITE_ARR_ID_TYPE_2);
        SetSpriteInvisibility(SPRITE_ARR_ID_TYPE_2, FALSE);
    }
    else
    {
        SetTypeIconSpritePosAndPal(type1, TYPE_ICON_1_X, TYPE_ICON_Y, SPRITE_ARR_ID_TYPE_1);
        SetSpriteInvisibility(SPRITE_ARR_ID_TYPE_2, TRUE);
    }
}*/

static u8 tabColors[NUM_TABS] = {
    [TAB_STATS]             = MENU_COLOR_BLUE,
    [TAB_ABILITIES]         = MENU_COLOR_RED,
    [TAB_MOVES]             = MENU_COLOR_GREEN,
    [TAB_STATUS]            = MENU_COLOR_YELLOW,
    [TAB_DAMAGE_CALCULATOR] = MENU_COLOR_RED,
};

static u8 tabColorsField[NUM_FIELD_TABS + 2] = {
    [TAB_FIELD]             = MENU_COLOR_GREEN,
    [TAB_SPEED]             = MENU_COLOR_BLUE,
};

static void setBattler(void){
    switch(sMenuDataPtr->modeId){
        case MODE_BATTLER0:
            sMenuDataPtr->battlerId = 0;
        break;
        case MODE_BATTLER1:
            sMenuDataPtr->battlerId = 1;
        break;
        case MODE_BATTLER2:
            sMenuDataPtr->battlerId = 2;
        break;
        case MODE_BATTLER3:
            sMenuDataPtr->battlerId = 3;
        break;
        case MODE_FIELD:
            sMenuDataPtr->battlerId = 0;
        break;
    }
}

static void LoadTabPalette(void){
    if(sMenuDataPtr->modeId == MODE_FIELD){
        switch(tabColorsField[sMenuDataPtr->fieldTabId]){
            case MENU_COLOR_BLUE:
                LoadPalette(sMenuPalette_Blue, 0, 32);
            break;
            case MENU_COLOR_YELLOW:
                LoadPalette(sMenuPalette_Yellow, 0, 32);
            break;
            case MENU_COLOR_RED:
                LoadPalette(sMenuPalette_Red, 0, 32);
            break;
            case MENU_COLOR_GREEN:
                LoadPalette(sMenuPalette_Green, 0, 32);
            break;
            default:
                LoadPalette(sMenuPalette, 0, 32);
            break;
        }
    }
    else{
        switch(tabColors[sMenuDataPtr->tabId]){
            case MENU_COLOR_BLUE:
                LoadPalette(sMenuPalette_Blue, 0, 32);
            break;
            case MENU_COLOR_YELLOW:
                LoadPalette(sMenuPalette_Yellow, 0, 32);
            break;
            case MENU_COLOR_RED:
                LoadPalette(sMenuPalette_Red, 0, 32);
            break;
            case MENU_COLOR_GREEN:
                LoadPalette(sMenuPalette_Green, 0, 32);
            break;
            default:
                LoadPalette(sMenuPalette, 0, 32);
            break;
        }
    }
}

static void PrintPage(void){
    LoadTilemapFromMode();
    LoadTabPalette();
    FreeItemIconSprite();
    FreeEveryMonIconSprite();
    if(sMenuDataPtr->modeId != MODE_FIELD)
        switch(sMenuDataPtr->tabId){
            case TAB_STATS:
                PrintStatsTab();
            break;
            case TAB_ABILITIES:
                PrintAbilityTab();
            break;
            case TAB_MOVES:
                PrintMoveTab();
            break;
            case TAB_STATUS:
                PrintStatusTab();
            break;
            case TAB_DAMAGE_CALCULATOR:
                PrintDamageCalulatorTab();
            break;
            default:
                PrintStatsTab();
            break;
        }
    else{
        switch(sMenuDataPtr->fieldTabId){
            case TAB_FIELD:
                PrintFieldTab();
            break;
            case TAB_SPEED:
                PrintSpeedTab();
            break;
        }
    }
}

/* This is the meat of the UI. This is where you wait for player inputs and can branch to other tasks accordingly */
static void Task_MenuMain(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_MenuTurnOff;
    }

    if (JOY_NEW(A_BUTTON))
    {
        if(sMenuDataPtr->modeId != MODE_FIELD){
            switch(sMenuDataPtr->tabId){
                case TAB_MOVES:
                    #ifdef DEBUG_BUILD
                    if(FlagGet(FLAG_SYS_MGBA_PRINT)){
                        MgbaOpen();
                        MgbaPrintf(MGBA_LOG_WARN, "sMenuDataPtr->moveModeId Detected %d", sMenuDataPtr->moveModeId);
                        MgbaClose();
                    }
                    #endif

                    if(sMenuDataPtr->moveModeId != NUM_MOVE_MODES - 1)
                        sMenuDataPtr->moveModeId++;
                    else
                        sMenuDataPtr->moveModeId = 0;
                    PrintMoveTab();
                break;
                default:
                    PlaySE(SE_PC_OFF);
                    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
                    gTasks[taskId].func = Task_MenuTurnOff;
                break;
            }

        }
        else{
            switch(sMenuDataPtr->fieldTabId){
                case TAB_FIELD:
                    sMenuDataPtr->currentFieldInfo = (sMenuDataPtr->currentFieldInfo + 1) % sMenuDataPtr->numFields;
                    PrintFieldTab();
                break;
                default:
                    PlaySE(SE_PC_OFF);
                    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
                    gTasks[taskId].func = Task_MenuTurnOff;
                break;
            }
        }
    }

    if (JOY_NEW(DPAD_DOWN)){
        if(IsDoubleBattle()){
            if(sMenuDataPtr->modeId != NUM_MODES - 1)
                sMenuDataPtr->modeId++;
            else
                sMenuDataPtr->modeId = 0;
        }
        else{
            if(sMenuDataPtr->modeId != MODE_BATTLER1)
                sMenuDataPtr->modeId++;
            else
                sMenuDataPtr->modeId = MODE_BATTLER0;
        }
        setBattler();
        PrintPage();
    }

    if (JOY_NEW(DPAD_UP)){
        if(IsDoubleBattle()){
            if(sMenuDataPtr->modeId != 0)
                sMenuDataPtr->modeId--;
            else
                sMenuDataPtr->modeId = NUM_MODES - 1;
        }
        else{
            if(sMenuDataPtr->modeId != MODE_BATTLER0)
                sMenuDataPtr->modeId--;
            else
                sMenuDataPtr->modeId = MODE_BATTLER1;
        }
        setBattler();
        PrintPage();
    }

    if (JOY_NEW(DPAD_LEFT)){
        switch(sMenuDataPtr->modeId){
            case MODE_FIELD:
                if(sMenuDataPtr->fieldTabId != 0)
                    sMenuDataPtr->fieldTabId--;
                else
                    sMenuDataPtr->fieldTabId = NUM_FIELD_TABS - 1;
                PrintPage();
            break;
            default: //Battlers
                if(sMenuDataPtr->tabId != 0)
                    sMenuDataPtr->tabId--;
                else
                    sMenuDataPtr->tabId = NUM_TABS - 1;
                setBattler();
                PrintPage();
            break;
        }
    }

    if (JOY_NEW(DPAD_RIGHT)){
        switch(sMenuDataPtr->modeId){
            case MODE_FIELD:
                if(sMenuDataPtr->fieldTabId != NUM_FIELD_TABS - 1)
                    sMenuDataPtr->fieldTabId++;
                else
                    sMenuDataPtr->fieldTabId = 0;
                PrintPage();
            break;
            default: //Battlers
                if(sMenuDataPtr->tabId != NUM_TABS - 1)
                    sMenuDataPtr->tabId++;
                else
                    sMenuDataPtr->tabId = 0;
                setBattler();
                PrintPage();
            break;
        }
    }
}