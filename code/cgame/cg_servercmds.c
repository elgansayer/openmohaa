/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// DESCRIPTION:
// cg_servercmds.c -- text commands sent by the server

#include "cg_local.h"
#include "../fgame/bg_voteoptions.h"

static const char *IsWeaponAllowed(int dmFlags, int flags)
{
    return (dmFlags & flags) ? "0" : "1";
}

static qboolean QueryLandminesAllowed2(const char *mapname, int dmflags)
{
    if (dmflags & DF_WEAPON_NO_LANDMINE) {
        return qfalse;
    }

    if (dmflags & DF_WEAPON_LANDMINE_ALWAYS) {
        return qtrue;
    }

    if (!Q_stricmpn(mapname, "obj/obj_", 8u)) {
        return qfalse;
    }
    if (!Q_stricmpn(mapname, "dm/mohdm", 8u)) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Bahnhof_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Ardennes_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Bazaar_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Berlin_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Brest_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Druckkammern_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Gewitter_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "obj/MP_Flughafen_TOW")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Holland_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Malta_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Stadt_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Unterseite_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Verschneit_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "lib/mp_ship_lib")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "DM/MP_Verschneit_DM")) {
        return qfalse;
    }
    if (!Q_stricmp(mapname, "lib/mp_ship_lib")) {
        return qfalse;
    }
    return qtrue;
}

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo(void)
{
    const char *info;
    const char *mapname;
    char        map[MAX_QPATH];
    char       *spawnpos;
    const char *version;
    const char *mapChecksumStr;

    info           = CG_ConfigString(CS_SERVERINFO);
    cgs.gametype   = atoi(Info_ValueForKey(info, "g_gametype"));
    cgs.dmflags    = atoi(Info_ValueForKey(info, "dmflags"));
    cgs.teamflags  = atoi(Info_ValueForKey(info, "teamflags"));
    cgs.fraglimit  = atoi(Info_ValueForKey(info, "fraglimit"));
    cgs.timelimit  = atoi(Info_ValueForKey(info, "timelimit"));
    cgs.maxclients = atoi(Info_ValueForKey(info, "sv_maxclients"));

    version = Info_ValueForKey(info, "version");
    if (strstr(version, "Spearhead")) {
        cgi.Cvar_Set("g_servertype", "1");
    } else {
        cgi.Cvar_Set("g_servertype", "2");
    }

    cgi.Cvar_Set("cg_gametype", Info_ValueForKey(info, "g_gametype"));
    cgi.Cvar_Set("cg_fraglimit", Info_ValueForKey(info, "fraglimit"));
    cgi.Cvar_Set("cg_timelimit", Info_ValueForKey(info, "timelimit"));
    cgi.Cvar_Set("cg_maxclients", Info_ValueForKey(info, "sv_gametype"));
    cgi.Cvar_Set("cg_obj_alliedtext1", Info_ValueForKey(info, "g_obj_alliedtext1"));
    cgi.Cvar_Set("cg_obj_alliedtext2", Info_ValueForKey(info, "g_obj_alliedtext2"));
    cgi.Cvar_Set("cg_obj_alliedtext3", Info_ValueForKey(info, "g_obj_alliedtext3"));
    cgi.Cvar_Set("cg_obj_alliedtext4", Info_ValueForKey(info, "g_obj_alliedtext4"));
    cgi.Cvar_Set("cg_obj_alliedtext5", Info_ValueForKey(info, "g_obj_alliedtext5"));
    cgi.Cvar_Set("cg_obj_axistext1", Info_ValueForKey(info, "g_obj_axistext1"));
    cgi.Cvar_Set("cg_obj_axistext2", Info_ValueForKey(info, "g_obj_axistext2"));
    cgi.Cvar_Set("cg_obj_axistext3", Info_ValueForKey(info, "g_obj_axistext3"));
    cgi.Cvar_Set("cg_obj_axistext4", Info_ValueForKey(info, "g_obj_axistext4"));
    cgi.Cvar_Set("cg_obj_axistext5", Info_ValueForKey(info, "g_obj_axistext5"));
    cgi.Cvar_Set("cg_scoreboardpic", Info_ValueForKey(info, "g_scoreboardpic"));
    cgi.Cvar_Set("cg_scoreboardpicover", Info_ValueForKey(info, "g_scoreboardpicover"));
    mapChecksumStr = Info_ValueForKey(info, "sv_mapChecksum");
    if (mapChecksumStr && mapChecksumStr[0]) {
        cgs.mapChecksum = atoi(mapChecksumStr);
        cgs.useMapChecksum = qtrue;
    } else {
        cgs.mapChecksum = 0;
        cgs.useMapChecksum = qfalse;
    }

    mapname = Info_ValueForKey(info, "mapname");

    cgi.Cvar_Set("cg_weapon_rifle", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_RIFLE));
    cgi.Cvar_Set("cg_weapon_sniper", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SNIPER));
    cgi.Cvar_Set("cg_weapon_mg", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_MG));
    cgi.Cvar_Set("cg_weapon_smg", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SMG));
    cgi.Cvar_Set("cg_weapon_rocket", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_ROCKET));
    cgi.Cvar_Set("cg_weapon_shotgun", IsWeaponAllowed(cgs.dmflags, DF_WEAPON_NO_SHOTGUN));
    cgi.Cvar_Set("cg_weapon_landmine", QueryLandminesAllowed2(mapname, cgs.dmflags) ? "1" : "0");

    spawnpos = strchr(mapname, '$');
    if (spawnpos) {
        Q_strncpyz(map, mapname, spawnpos - mapname + 1);
    } else {
        strcpy(map, mapname);
    }

    Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", map);

    // hide/show huds
    if (cgs.gametype) {
        cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_timelimit\n");
        if (cgs.fraglimit) {
            cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_fraglimit\n");
            cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_score\n");
        } else {
            cgi.Cmd_Execute(EXEC_NOW, "ui_addhud hud_score\n");
            cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_fraglimit\n");
        }
    } else {
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_timelimit\n");
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_fraglimit\n");
        cgi.Cmd_Execute(EXEC_NOW, "ui_removehud hud_score\n");
    }
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified(int num, qboolean modelOnly)
{
    // get the gamestate from the client system, which will have the
    // new configstring already integrated
    cgi.GetGameState(&cgs.gameState);

    CG_ProcessConfigString(num, modelOnly);
}


static void CG_ParsePrintedText()
{

    // // com print we are here
    // Com_Printf("CG_ServerCommand: printdeathmsg\n");
    
    // // print sql test
    // Com_Test_Sql_f(attackerName, victimName, s1 , s2, type);


    const char *cmd;
    int         iClientNum;
    const char *pszClientInfo;
    const char *pszName;
    const char *fullCmd;
    float       fX, fY;
    float       color[4];
    qhandle_t   handle;

    cmd = cgi.Argv(0);
    fullCmd = cgi.Args();

    char combinedCmd[256];
    size_t combinedCmdSize = sizeof(combinedCmd);
    snprintf(combinedCmd, combinedCmdSize, "%s %s", cmd, fullCmd);
    combinedCmd[combinedCmdSize - 1] = '\0'; // Ensure null-termination

    Com_Printf("Command is: %s\n", cgi.Argv(0));
    Com_Printf("fullCmd is: %s\n", cgi.Args());
    Com_Printf("combinedCmd is: %s\n", combinedCmd);

    pszName = cgi.Cvar_Get("name", "", 0)->string;
    Com_Printf("name is: %s\n", pszName);

    if (pszName == NULL || pszName[0] == "" || pszName[0] == '\0') {
        return;
    }

    // cvar_t *eb_bashed = cgi.Cvar_Get("eb_bashed", "", 0);
    // cvar_t *eb_gotBashed = cgi.Cvar_Get("eb_gotBashed", "", 0);
    
    cvar_t *eb_suicide = cgi.Cvar_Get("eb_suicide", "killed yourself", 0);
    if (eb_CheckStringForCVar(combinedCmd , eb_suicide))
    {
        // We killed ourselves        
        eb_ResetLastKillTime();
        eb_ResetKillsInARow();
        return;
    }

    // Don't continue if the message is just "You killed %s"
    qboolean foundYouKilled = eb_CheckForYouKilledMessage(combinedCmd);
    if (foundYouKilled)
    {
        eb_ResetDeathsInARow();
        return;
    }

    // Get body part stats
    eb_CheckForBodyParts(fullCmd);
}

qboolean eb_CheckForYouKilledMessage(const char *combinedCmd) 
{
    cvar_t *eb_gotKill = cgi.Cvar_Get("eb_gotKill", "You killed", 0);
    if (!eb_CheckStringForCVar(combinedCmd , eb_gotKill))
    {
        return qfalse;
    }
    
    time_t currentTime = time(NULL);
    int unixTimeStamp = (int)currentTime;
            
    cvar_t *eb_lastKillTime = cgi.Cvar_Get("eb_lastKillTime", "0", 0);
    cvar_t *eb_streakTime = cgi.Cvar_Get("eb_streakTime", "10", 0);

    int lastKillTime = eb_lastKillTime->integer;
    int streakTime = eb_streakTime->integer;

    if ((lastKillTime > 0 && streakTime > 0 ) && (lastKillTime > (unixTimeStamp - streakTime))) {
        // we got a killing spree
        Com_Printf("we got a quick kill spree: %d\n", time);
        eb_PerformQuickKills();
    } else {
        // We count this kill 
        cgi.Cvar_Set("eb_quickKillLevel", "1");
    }    

    char timeString[20];
    snprintf(timeString, sizeof(timeString), "%d", unixTimeStamp);
    cgi.Cvar_Set("eb_lastKillTime", timeString);

    // Update kills in a row
    cvar_t *eb_killsInARow = cgi.Cvar_Get("eb_killsInARow", "0", 0);
    int killsInARow = eb_killsInARow->integer;
    killsInARow++;
    char killsInARowString[20];
    snprintf(killsInARowString, sizeof(killsInARowString), "%d", killsInARow);
    cgi.Cvar_Set("eb_killsInARow", killsInARowString);

    // Com print we got a kill
    Com_Printf("we got a kill: %s\n", killsInARowString);

    // Perform kill spree checks (killing spree, rampage, dominating, unstoppable, godlike)
    // play sounds, show ui, announce to server
    eb_PerformKillSpreeChecks();

    return qtrue;     
}

typedef struct {
    char* name;
    int kill_count;
    char* sound_path;
    char* stuff_command;
    char* ui_menu;
} KillSpree;

typedef struct {
    char* name;
    int kills_in_a_row;
    char* sound_path;
} QuickKill;

// [Name] is on a Killing Spree!: You score 5 frags without dying.
// [Name] is on a Rampage!: You score 10 frags without dying.
// [Name] is Dominating!: You score 15 frags without dying.
// [Name] is Unstoppable!: You score 20 frags without dying.
// [Name] is Godlike!: You score 25 frags without dying.

void eb_PerformQuickKills() 
{
    cvar_t *eb_sounds = cgi.Cvar_Get("eb_sounds", "1", 0);
    cvar_t *eb_quickKillLevel = cgi.Cvar_Get("eb_quickKillLevel", "1", 0);    

   // Update quick kill level
    int quickKillLevel = eb_quickKillLevel->integer;
    quickKillLevel++;

    // Reset quick kill level if it's over 9
    if (quickKillLevel > 9) {
        quickKillLevel = 2;
    }

    char quickKillLevelString[20];
    snprintf(quickKillLevelString, sizeof(quickKillLevelString), "%d", quickKillLevel);
    cgi.Cvar_Set("eb_quickKillLevel", quickKillLevelString);

    if (!eb_sounds->integer) 
    {
        return;
    }
    
    // Double Kill: 2 kills.
    // Multikill: 3 kills.
    // Mega Kill: 4 kills.
    // Monster Kill: 5 kills.
    // Ultra Kill: 6 kills.
    // Unholy: 7 kills.
    // Impossible: 8 kills.
    // Unpossible: 9 or more kills ( will continually announce "Unpossible!").

    // {"Head Hunter", 0, "sounds/headhunter.mp3"},
    // {"Headshot", 0, "sounds/headshot.mp3"},
    // {"First Blood", 0, "sounds/firstblood.mp3", "scored first blood!"},

    QuickKill QuickKills[] = {
        {"Double Kill", 2, "sounds/double_kill.mp3"},
        {"Multikill", 3, "sounds/multikill.mp3"},
        {"Megakill", 4, "sounds/megakill.mp3"},
        {"Monster Kill", 5, "sounds/monsterkill.mp3"},
        {"Ultrakill", 6, "sounds/ultrakill.mp3"},
        {"ludicrouskill", 7, "sounds/ludicrouskill.mp3"},
        {"wickedsick", 8, "sounds/wickedsick.mp3"},
        {"wickedsick", 8, "sounds/wickedsick.mp3"},
        {"Holy Shit", 9, "sounds/holyshit.mp3"},        
    };

    // What quick kill level am i?
    QuickKill *quickKill = NULL;
    int killCount = eb_quickKillLevel->integer;
    int numQuickKills = sizeof(QuickKills) / sizeof(QuickKills[0]);
    for (int i = 0; i < numQuickKills; i++) {
        if (killCount == QuickKills[i].kills_in_a_row) {
            quickKill = &QuickKills[i];
        }
    }

    if (!quickKill) 
    {
        return;
    }

    // concat quickKill->string and "play %s\n"
    char playString[256];
    size_t playStringSize = sizeof(playString);
    snprintf(playString, playStringSize, "playmp3 %s\n", quickKill->sound_path);
    playString[playStringSize - 1] = '\0'; // Ensure null-termination

    cgi.Cmd_Stuff(playString);    
}

void eb_PerformKillSpreeChecks() 
{
    cvar_t *eb_sounds = cgi.Cvar_Get("eb_sounds", "1", 0);
    cvar_t *eb_displays = cgi.Cvar_Get("eb_displays", "1", 0);
    cvar_t *eb_announcements = cgi.Cvar_Get("eb_announcements", "1", 0);    
    cvar_t *eb_killsInARow = cgi.Cvar_Get("eb_killsInARow", "1", 0);    

    Com_Printf("eb_killsInARow: %s\n", eb_killsInARow->string);

    // 5 kills in a row is a killing spree start
    if (eb_killsInARow->integer < 5) 
    {
        return;
    }
    
    // Double Kill: 2 kills.
    // Multikill: 3 kills.
    // Mega Kill: 4 kills.
    // Monster Kill: 5 kills.
    // Ultra Kill: 6 kills.
    // Unholy: 7 kills.
    // Impossible: 8 kills.
    // Unpossible: 9 or more kills ( will continually announce "Unpossible!").
    // {"Head Hunter", 0, "sounds/headhunter.wav"},
    // {"Headshot", 0, "sounds/headshot.wav", "headshot"},       
    // {"First Blood", 0, "sounds/firstblood.wav", "scored first blood!"},
    // hidemenu headshot
    // hidemenu firstblood
    // hidemenu rampage
    // hidemenu dominating
    // hidemenu godlike
    // hidemenu holyshit
    // hidemenu unstoppable
    // hidemenu killingspree

    KillSpree killSprees[] = {
        {"Killing Spree", 5, "sounds/killingspree.mp3", "%s is on a Killing Spree!", "killingspree"},
        {"Rampage", 10, "sounds/rampage.mp3", "%s is on a Rampage!", "rampage"},
        {"Dominating", 15, "sounds/dominating.mp3", "%s is Dominating", "dominating"},
        {"Unstoppable", 20, "sounds/unstoppable.mp3", "%s is Unstoppable", "unstoppable"},
        {"Godlike", 25, "sounds/godlike.mp3", "%s is Godlike!", "godlike"},        
    };

    // What kill spree level am I?
    KillSpree *killSpree = NULL;
    int killCount = eb_killsInARow->integer;
    int numKillSprees = sizeof(killSprees) / sizeof(killSprees[0]);
    for (int i = 0; i < numKillSprees; i++) {
        if (killCount == killSprees[i].kill_count) {
            killSpree = &killSprees[i];
        }
    }

    if (!killSpree) 
    {   
        return;
    }

    if (eb_sounds->integer && killSpree->sound_path)
    {        
        char playString[256];
        size_t playStringSize = sizeof(playString);
        snprintf(playString, playStringSize, "playmp3 %s\n", killSpree->sound_path);
        playString[playStringSize - 1] = '\0'; // Ensure null-termination

        Com_Printf("playString is: %s\n", playString);
        cgi.Cmd_Stuff(playString);
    }    

    if (eb_displays->integer && killSpree->ui_menu)
    {               
        char playString[512];
        size_t playStringSize = sizeof(playString);
        snprintf(playString, playStringSize, "showmenu %s; wait 450; hidemenu %s\n", killSpree->ui_menu, killSpree->ui_menu);
        playString[playStringSize - 1] = '\0'; // Ensure null-termination

        // Print final command
        Com_Printf("playString is: %s\n", playString);
        cgi.Cmd_Stuff(playString);
    }  

    if (eb_announcements->integer && killSpree->stuff_command)
    {        
        // concat killSpree->stuff_command, cgi.Cvar_Get("name", "", 0)->string
        char sayString[256];
        size_t sayStringSize = sizeof(sayString);
        snprintf(sayString, sayStringSize, killSpree->stuff_command, cgi.Cvar_Get("name", "", 0)->string);
        sayString[sayStringSize - 1] = '\0'; // Ensure null-termination
    
        char playString[512];
        size_t playStringSize = sizeof(playString);
        snprintf(playString, playStringSize, "dmmessage 0 %s\n", sayString);
        playString[playStringSize - 1] = '\0'; // Ensure null-termination

        Com_Printf("sayString is: %s\n", sayString);
        Com_Printf("playString is: %s\n", playString);
        cgi.Cmd_Stuff(playString);
    }
}

void eb_ResetLastKillTime(void) 
{
    cgi.Cvar_Set("eb_lastKillTime", "0");
}

void eb_ResetKillsInARow(void) 
{
    cgi.Cvar_Set("eb_killsInARow", "0");
}

void eb_ResetDeathsInARow(void) 
{
    cgi.Cvar_Set("eb_deathsInARow", "0");
}

void eb_CheckForBodyParts(const char* fullCmd) 
{   
    // Get the location names from the cvars
    cvar_t *eb_head = cgi.Cvar_Get("eb_head", "head,helmet,neck", 0);
    cvar_t *eb_torso = cgi.Cvar_Get("eb_torso", "torso", 0);
    cvar_t *eb_pelvis = cgi.Cvar_Get("eb_pelvis", "pelvis", 0);
    cvar_t *eb_rightArm = cgi.Cvar_Get("eb_rightArm", "right arm,right hand", 0);
    cvar_t *eb_leftArm = cgi.Cvar_Get("eb_leftArm", "left arm,left hand", 0);
    cvar_t *eb_rightLeg = cgi.Cvar_Get("eb_rightLeg", "right leg,right foot", 0);
    cvar_t *eb_leftLeg = cgi.Cvar_Get("eb_leftLeg", "left leg,left foot", 0);

    cvar_t *eb_bodyPartCvars[] = {eb_head, eb_torso, eb_pelvis, eb_rightArm, eb_leftArm, eb_rightLeg, eb_leftLeg};

    // loop through eb_bodyPartCvars
    for (int i = 0; i < sizeof(eb_bodyPartCvars) / sizeof(eb_bodyPartCvars[0]); i++) 
    {
        Com_Printf("eb_bodyPartCvars[%d] is: %s\n", i, eb_bodyPartCvars[i]->string);
        if (eb_CheckStringForCVar(fullCmd, eb_bodyPartCvars[i]))
        {
            Com_Printf("Found body part: %s\n", eb_bodyPartCvars[i]->string);
            return;
        }
    }
}

qboolean eb_CheckStringForCVar(const char* fullCmd, cvar_t *cvar) 
{
    if (cvar == NULL || cvar->string == NULL || fullCmd == NULL) {
        return qfalse; 
    }

    char *cvar_string = cvar->string;
    char *item;
    char *rest = cvar_string;

    while ((item = strtok_r(rest, ",", &rest))) {
        // Trim leading and trailing whitespaces
        char *end;
        if (item) {
            while(isspace((unsigned char)*item)) item++;
            end = item + strlen(item) - 1;
            while(end > item && isspace((unsigned char)*end)) end--;
            *(end+1) = 0;
        }

        if (item && strstr(fullCmd, item) != NULL) {
            return qtrue;
        }
    }

    return qfalse;
}

/*cvar_t *eb_sounds = 
================
CG_ParseStats

================
*/
static void CG_ParseStats()
{
    cgi.Cvar_Set("ui_NumObjectives", cgi.Argv(1));
    cgi.Cvar_Set("ui_NumComplete", cgi.Argv(2));
    cgi.Cvar_Set("ui_NumShotsFired", cgi.Argv(3));
    cgi.Cvar_Set("ui_NumHits", cgi.Argv(4));
    cgi.Cvar_Set("ui_Accuracy", cgi.Argv(5));
    cgi.Cvar_Set("ui_PreferredWeapon", cgi.Argv(6));
    cgi.Cvar_Set("ui_NumHitsTaken", cgi.Argv(7));
    cgi.Cvar_Set("ui_NumObjectsDestroyed", cgi.Argv(8));
    cgi.Cvar_Set("ui_NumEnemysKilled", cgi.Argv(9));
    cgi.Cvar_Set("ui_HeadShots", cgi.Argv(10));
    cgi.Cvar_Set("ui_TorsoShots", cgi.Argv(11));
    cgi.Cvar_Set("ui_LeftLegShots", cgi.Argv(12));
    cgi.Cvar_Set("ui_RightLegShots", cgi.Argv(13));
    cgi.Cvar_Set("ui_GroinShots", cgi.Argv(14));
    cgi.Cvar_Set("ui_LeftArmShots", cgi.Argv(15));
    cgi.Cvar_Set("ui_RightArmShots", cgi.Argv(16));
    cgi.Cvar_Set("ui_GunneryEvaluation", cgi.Argv(17));
    cgi.Cvar_Set("ui_gotmedal", cgi.Argv(18));
    cgi.Cvar_Set("ui_success", cgi.Argv(19));
    cgi.Cvar_Set("ui_failed", cgi.Argv(20));
}

/*
================
CG_Stopwatch_f

================
*/
static void CG_Stopwatch_f()
{
    if (cgi.Argc() < 3) {
        Com_Error(ERR_DROP, "stopwatch didn't have 2 parameters");
    }

    if (cg_protocol >= PROTOCOL_MOHTA_MIN) {
        cgi.stopWatch->iStartTime = atoi(cgi.Argv(1));

        if (cgi.Argc() > 3) {
            cgi.stopWatch->eType = atoi(cgi.Argv(3));
        } else {
            // Normal stop watch
            cgi.stopWatch->eType = SWT_NORMAL;
        }
    } else {
        // The base game has it wrong
        cgi.stopWatch->iStartTime = 1000 * atoi(cgi.Argv(1));
    }

    cgi.stopWatch->iEndTime = cgi.stopWatch->iStartTime + 1000 * atoi(cgi.Argv(2));
}

/*
================
CG_ServerLag_f

================
*/
static void CG_ServerLag_f()
{
    cgs.serverLagTime = cg.time;
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand(qboolean modelOnly)
{
    const char *cmd;

    cmd = cgi.Argv(0);
    
    if (!cmd[0]) {
        // server claimed the command
        return;
    }

    if (!strcmp(cmd, "cs")) {
        CG_ConfigStringModified(atoi(cgi.Argv(1)), modelOnly);
        return;
    }

    if (modelOnly) {
        return;
    }

    if (!strcmp(cmd, "print") || !strcmp(cmd, "hudprint")) {
        cgi.Printf("%s", cgi.Argv(1));
        if (!strcmp(cmd, "hudprint")) {
            CG_HudPrint_f();
        }

        // Process the cmd buffer for keywords
        CG_ParsePrintedText();

        return;
    } else if (!strcmp(cmd, "printdeathmsg")) {
        const char *s1, *s2, * attackerName, * victimName, *type;
        const char *result1, *result2;
        int         hudColor;

        result1 = NULL;
        result2 = NULL;
        s1           = cgi.Argv(1);
        s2           = cgi.Argv(2);
        attackerName = cgi.Argv(3);
        victimName   = cgi.Argv(4);
        type         = cgi.Argv(5);

        if (*type == tolower(*type)) {
            hudColor = 4;
        } else {
            hudColor = 5;
        }

        if (*s1 != 'x') {
            result1 = cgi.LV_ConvertString(s1);
        }
        if (*s2 != 'x') {
            result2 = cgi.LV_ConvertString(s2);
        }
        if (tolower(*type) == 's') {
            cgi.Printf("%c%s %s\n", hudColor, victimName, result1);
        } else if (tolower(*type) == 'p') {
            if (*s2 == 'x') {
                cgi.Printf("%c%s %s %s\n", hudColor, victimName, result1, attackerName);
            } else {
                cgi.Printf("%c%s %s %s%s\n", hudColor, victimName, result1, attackerName, result2);
            }
        } else if (tolower(*type) == 'w') {
            cgi.Printf("%c%s %s\n", hudColor, victimName, result1);
        } else {
            cgi.Printf("%s", cgi.Argv(1));
        }
        return;
    }

    if (!strcmp(cmd, "stufftext")) {
        cgi.Cmd_Stuff(cgi.Argv(1));
        cgi.Cmd_Stuff("\n");
        return;
    }

    if (!strcmp(cmd, "scores")) {
        CG_ParseScores();
        return;
    }

    if (!strcmp(cmd, "stats")) {
        CG_ParseStats();
        return;
    }

    if (!strcmp(cmd, "stopwatch")) {
        CG_Stopwatch_f();
        return;
    }

    if (!strcmp(cmd, "svlag")) {
        CG_ServerLag_f();
        return;
    }

    if (!strcmp(cmd, "voteresult")) {
        cmd = cgi.Argv(1);
        if (*cmd) {
            strcmp(cmd, "passed");
        }
    }

    if (!strcmp(cmd, "vo0")) {
        CG_VoteOptions_StartReadFromServer(cgi.Argv(1));
        return;
    }

    if (!strcmp(cmd, "vo1")) {
        CG_VoteOptions_ContinueReadFromServer(cgi.Argv(1));
        return;
    }

    if (!strcmp(cmd, "vo2")) {
        CG_VoteOptions_FinishReadFromServer(cgi.Argv(1));
        return;
    }

    cgi.Printf("Unknown client game command: %s\n", cmd);
}

/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands(int latestSequence, qboolean differentServer)
{
    int lastServerCommandSequence;

    lastServerCommandSequence = cgs.serverCommandSequence;

    while (cgs.serverCommandSequence < latestSequence) {
        if (cgi.GetServerCommand(++cgs.serverCommandSequence, differentServer)) {
            CG_ServerCommand(qtrue);
        }
    }

    cgs.serverCommandSequence = lastServerCommandSequence;
    while (cgs.serverCommandSequence < latestSequence) {
        if (cgi.GetServerCommand(++cgs.serverCommandSequence, differentServer)) {
            CG_ServerCommand(qfalse);
        }
    }
}
