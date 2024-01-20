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

typedef struct {
    char* name;
    int kill_count;
    char* sound_path;
    char* stuff_command;
    char* ui_menu;
    char* stat_cvar;
} eb_Action;

typedef struct {
    char* name;
    char* check_cvar;
    char* stat_cvar;
    int count;
} eb_Stat;

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
    const char *fullCmd;

    cmd = cgi.Argv(0);
    fullCmd = cgi.Args();

    char combinedCmd[256];
    size_t combinedCmdSize = sizeof(combinedCmd);
    snprintf(combinedCmd, combinedCmdSize, "%s %s", cmd, fullCmd);
    combinedCmd[combinedCmdSize - 1] = '\0'; // Ensure null-termination

    // Com_Printf("Command is: %s\n", cgi.Argv(0));
    // Com_Printf("fullCmd is: %s\n", cgi.Args());
    // Com_Printf("combinedCmd is: %s\n", combinedCmd);

    // cvar_t *eb_bashed = cgi.Cvar_Get("eb_bashed", "", 0);
    // cvar_t *eb_gotBashed = cgi.Cvar_Get("eb_gotBashed", "", 0);

    // Reset everything if we enter a new battle
    if (eb_ProcessDidEnterBattle(combinedCmd))
    {
        return;
    }
    
    cvar_t *eb_suicide = cgi.Cvar_Get("eb_suicide", "killed yourself", 0);
    if (eb_CheckStringForCVar(combinedCmd , eb_suicide, NULL))
    {
        eb_Action suicidesAction = 
            {"suicides",0, NULL, NULL, NULL, "ui_NumSuicides"} ;
            
        eb_UpdateStat(&suicidesAction);  

        // We killed ourselves        
        eb_ResetLastKillTime();
        eb_ResetKillsInARow();
        return;
    }

    // Don't continue if the message is just "You killed %s"
    qboolean foundYouKilled = eb_ProcessGotYouKilled(combinedCmd);
    if (foundYouKilled)
    {
        eb_ResetDeathsInARow();
        return;
    }

    // The message was not just "You killed"
    // Did we die or did we kill someone?
    eb_ProcessGotKill(combinedCmd);

    // Did we die? then reset kill streaks
    eb_ProcessGotKilled(combinedCmd);
}

qboolean eb_ProcessDidEnterBattle(const char *fullCmd) 
{
    cvar_t *eb_enteredBattle = cgi.Cvar_Get("eb_mapRestart", "%s has entered the battle", 0);
    cvar_t *name_cvar = cgi.Cvar_Get("name", "", 0);
    if (!eb_CheckStringForCVar(fullCmd , eb_enteredBattle, name_cvar->string))
    {
        return qfalse;
    }       

    // Reset everything
    ev_ResetAll();

    return qtrue;
}

void ev_ResetAll( void )
{
    cgi.Cvar_Set("ui_NumSuicides", "0");
    cgi.Cvar_Set("ui_NumKills", "0");
    cgi.Cvar_Set("ui_NumDeaths", "0");
    
    cgi.Cvar_Set("ui_NumDoubleKills", "0");
    cgi.Cvar_Set("ui_NumMultikills", "0");
    cgi.Cvar_Set("ui_NumMegakills", "0");
    cgi.Cvar_Set("ui_NumMonsterKills", "0");
    cgi.Cvar_Set("ui_NumUltrakills", "0");
    cgi.Cvar_Set("ui_NumLudicrouskills", "0");
    cgi.Cvar_Set("ui_NumWickedSick", "0");
    cgi.Cvar_Set("ui_NumHolyShit", "0");

    cgi.Cvar_Set("ui_NumKillingSpree", "0");
    cgi.Cvar_Set("ui_NumRampage", "0");
    cgi.Cvar_Set("ui_NumDominating", "0");
    cgi.Cvar_Set("ui_NumUnstoppable", "0");
    cgi.Cvar_Set("ui_NumGodLike", "0");
    
    cgi.Cvar_Set("ui_bashed", "0");
    cgi.Cvar_Set("ui_gotBashed", "0");

    cgi.Cvar_Set("ui_numDeathsInARow", "0");
    cgi.Cvar_Set("ui_numKillsInARow", "0");

    // for tracking during game , not stats
    cgi.Cvar_Set("eb_deathsInARow", "0");
    cgi.Cvar_Set("eb_killsInARow", "0");    
    cgi.Cvar_Set("eb_lastKillTime", "0");
    cgi.Cvar_Set("eb_quickKillLevel", "0");        
}

qboolean eb_ProcessGotKilled(const char *fullCmd) 
{
    cvar_t *eb_killChecks = cgi.Cvar_Get("eb_deathChecks", "%s was,%s caught,%s is picking,%s died,%s took,%s tripped,blew up %s,%s blew up,%s played,%s cratered", 0);
    cvar_t *name_cvar = cgi.Cvar_Get("name", "", 0);
    if (!eb_CheckStringForCVar(fullCmd , eb_killChecks, name_cvar->string))
    {
        return qfalse;
    }    

    // We died soo..
    eb_ResetKillsInARow();
    eb_ResetLastKillTime();
    
    // Update kills in a row
    eb_IncrementCvar( "eb_deathsInARow", 1);

    eb_Action deathAction = 
        {"deaths",0, NULL, NULL, NULL, "ui_NumDeaths"};
        
    eb_UpdateStat(&deathAction);      

    return qtrue;
}

qboolean eb_ProcessGotKill(const char *fullCmd) 
{
    cvar_t *eb_killChecks = cgi.Cvar_Get("eb_killChecks", "by %s", 0);
    cvar_t *name_cvar = cgi.Cvar_Get("name", "", 0);
    if (!eb_CheckStringForCVar(fullCmd , eb_killChecks, name_cvar->string))
    {
        return qfalse;
    }

    // We got a kill, lets chat if we got a headshot or other
    eb_CheckKillForHeadshot(fullCmd);

    // Get body part stats
    eb_CheckKillForBodyParts(fullCmd);

    return qtrue;
}

qboolean eb_ProcessGotYouKilled(const char *combinedCmd) 
{
    cvar_t *eb_gotKill = cgi.Cvar_Get("eb_gotKill", "You killed", 0);
    if (!eb_CheckStringForCVar(combinedCmd , eb_gotKill, NULL))
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
        // Com_Printf("we got a quick kill spree: %d\n", time);
        eb_PerformQuickKills();
    } else {
        // We count this kill 
        cgi.Cvar_Set("eb_quickKillLevel", "1");
    }    

    char timeString[20];
    snprintf(timeString, sizeof(timeString), "%d", unixTimeStamp);
    cgi.Cvar_Set("eb_lastKillTime", timeString);

    // Update kills in a row
    eb_IncrementCvar( "eb_killsInARow", 1);

    // Perform kill spree checks (killing spree, rampage, dominating, unstoppable, godlike)
    // play sounds, show ui, announce to server
    eb_PerformKillSpreeChecks();

    eb_Action killAction = 
        {"kills",0, NULL, NULL, NULL, "ui_NumKills"} ;
        
    eb_UpdateStat(&killAction);  

    return qtrue;     
}

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
    
    // {"First Blood", 0, "sounds/firstblood.mp3", "scored first blood!"},

    eb_Action quickKills[] = {
        {"Double Kill", 2, "sounds/double_kill.mp3", NULL, NULL, "ui_NumDoubleKills"},
        {"Multikill", 3, "sounds/multikill.mp3", NULL, NULL, "ui_NumDoubleKills"},
        {"Megakill", 4, "sounds/megakill.mp3", NULL, NULL, "ui_NumMegakills"},
        {"Monster Kill", 5, "sounds/monsterkill.mp3", NULL, NULL, "ui_NumMonsterKills"},
        {"Ultrakill", 6, "sounds/ultrakill.mp3", NULL, NULL, "ui_NumUltrakills"},
        {"ludicrouskill", 7, "sounds/ludicrouskill.mp3", NULL, NULL, "ui_NumLudicrouskills"},
        {"wickedsick", 8, "sounds/wickedsick.mp3", NULL, NULL, "ui_NumWickedSick"},
        {"Holy Shit", 9, "sounds/holyshit.mp3", NULL, NULL, "ui_NumHolyShit"},        
    };

    // What quick kill level am i?
    eb_Action *quickKill = NULL;
    int killCount = eb_quickKillLevel->integer;
    int numQuickKills = sizeof(quickKills) / sizeof(quickKills[0]);
    for (int i = 0; i < numQuickKills; i++) {
        if (killCount == quickKills[i].kill_count) {
            quickKill = &quickKills[i];
        }
    }

    if (!quickKill) 
    {
        return;
    }

    eb_PlaySound(quickKill);  
    eb_UpdateStat(quickKill);  
}

void eb_PerformKillSpreeChecks() 
{
    cvar_t *eb_killsInARow = cgi.Cvar_Get("eb_killsInARow", "1", 0);    
    // Com_Printf("eb_killsInARow: %s\n", eb_killsInARow->string);
    // 5 kills in a row is a killing spree start
    if (eb_killsInARow->integer < 5) 
    {
        return;
    }
    
    eb_Action ebActions[] = {
        {"Killing Spree", 5, "sounds/killingspree.mp3", "is on a Killing Spree!", "killingspree", "ui_NumKillingSpree"},
        {"Rampage", 10, "sounds/rampage.mp3", "is on a Rampage!", "rampage", "ui_NumRampage"},
        {"Dominating", 15, "sounds/dominating.mp3", "is Dominating", "dominating", "ui_NumDominating"},
        {"Unstoppable", 20, "sounds/unstoppable.mp3", "is Unstoppable", "unstoppable", "ui_NumUnstoppable"},
        {"Godlike", 25, "sounds/godlike.mp3", "is Godlike!", "godlike", "ui_NumGodLike"},        
    };

    // What kill spree level am I?
    eb_Action *ebAction = NULL;
    int killCount = eb_killsInARow->integer;
    int numebActions = sizeof(ebActions) / sizeof(ebActions[0]);
    for (int i = 0; i < numebActions; i++) {
        if (killCount == ebActions[i].kill_count) {
            ebAction = &ebActions[i];
        }
    }

    if (!ebAction) 
    {   
        return;
    }
    
    eb_PlaySound(ebAction);
    eb_DisplayHud(ebAction);
    eb_Announcement(ebAction);
    eb_UpdateStat(ebAction);  
}

void eb_Announcement(eb_Action * ebAction) 
{
   cvar_t *eb_announcements = cgi.Cvar_Get("eb_announcements", "1", 0);   
    if (!eb_announcements->integer || !ebAction->sound_path)
    {     
        return;
    }
            
    char playString[512];
    size_t playStringSize = sizeof(playString);
    snprintf(playString, playStringSize, "say %s\n", ebAction->stuff_command);
    playString[playStringSize - 1] = '\0'; // Ensure null-termination

    cgi.Cmd_Stuff(playString);
}

void eb_PlaySound(eb_Action * ebAction) 
{
    cvar_t *eb_sounds = cgi.Cvar_Get("eb_sounds", "1", 0);
    if (!eb_sounds->integer || !ebAction->sound_path)
    {     
        return;
    }
    
    cgi.S_StartLocalSound(ebAction->sound_path, qfalse);
}

void eb_DisplayHud(eb_Action * ebAction) 
{
    cvar_t *eb_displays = cgi.Cvar_Get("eb_displays", "1", 0);
    if (!eb_displays->integer || !ebAction->ui_menu)
    {       
        return;
    }

    char playString[512];
    size_t playStringSize = sizeof(playString);
    snprintf(playString, playStringSize, "showmenu %s; wait 450; hidemenu %s\n", ebAction->ui_menu, ebAction->ui_menu);
    playString[playStringSize - 1] = '\0'; // Ensure null-termination

    // Print final command
    // Com_Printf("playString is: %s\n", playString);
    cgi.Cmd_Stuff(playString);   
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

void eb_CheckKillForHeadshot(const char* fullCmd) 
{
    cvar_t *eb_head = cgi.Cvar_Get("eb_head", "head,helmet,neck", 0);
    if (!eb_CheckStringForCVar(fullCmd, eb_head, NULL))
    { 
        return;
    }

    cvar_t *ui_HeadShots = cgi.Cvar_Get("ui_HeadShots", "0", 0);        
    int headshots = ui_HeadShots->integer + 1;
    if (headshots % 10 == 0) 
    {        
        eb_Action ebAction_headhunter = 
            {"headhunter", 10, "sounds/headhunter.mp3", "Headhunter!", "headshot", "ui_HeadShots"} ;
            
        eb_PlaySound(&ebAction_headhunter);
        eb_Announcement(&ebAction_headhunter);
        eb_UpdateStat(&ebAction_headhunter);  
    } 
    else 
    {
        eb_Action ebAction_headshot = 
            {"headshot", 10, "sounds/headshot.mp3", "Headshot!", "headshot", "ui_HeadShots"} ;

        eb_PlaySound(&ebAction_headshot);
        eb_Announcement(&ebAction_headshot);
        eb_DisplayHud(&ebAction_headshot);
        eb_UpdateStat(&ebAction_headshot);  
    }    
}

void eb_UpdateStat(eb_Action * ebAction) 
{
    eb_IncrementCvar(ebAction->stat_cvar, 1);
}

void eb_IncrementCvar( const char* cvarName, int amount ) 
{
    // Make sure the cvarName name is valid
    if (cvarName == NULL || cvarName[0] == '\0') {
        return;
    }

    cvar_t *cvar = cgi.Cvar_Get(cvarName, "0", 0);
    // Ensure the cvar is valid 
    if (cvar == NULL) {
        return;
    }

    int cvarValue = cvar->integer;
    cvarValue += amount;
    char cvarValueString[20];
    snprintf(cvarValueString, sizeof(cvarValueString), "%d", cvarValue);
    cgi.Cvar_Set(cvarName, cvarValueString);
}

void eb_DecramentCvar( const char* cvarName, int amount ) 
{
    if (cvarName == NULL || cvarName[0] == '\0') {
        return;
    }
        
    cvar_t *cvar = cgi.Cvar_Get(cvarName, "0", 0);
    int cvarValue = cvar->integer;
    cvarValue -= amount;
    char cvarValueString[20];
    snprintf(cvarValueString, sizeof(cvarValueString), "%d", cvarValue);
    cgi.Cvar_Set(cvarName, cvarValueString);
}

void eb_CheckKillForBodyParts(const char* fullCmd) 
{   
    // Get the location names from the cvars    
    eb_Action eb_bodyPartActions[] = {
        {"eb_torso", 0, NULL, NULL, NULL, "ui_TorsoShots"},
        {"eb_pelvis", 0, NULL, NULL, NULL, "ui_GroinShots"},
        {"eb_rightArm", 0, NULL, NULL, NULL, "ui_RightArmShots"},
        {"eb_leftArm", 0, NULL, NULL, NULL, "ui_LeftArmShots"},
        {"eb_rightLeg", 0, NULL, NULL, NULL, "ui_RightLegShots"},
        {"eb_leftLeg", 0, NULL, NULL, NULL, "ui_LeftLegShots"}
    };

    // loop through eb_bodyPartCvars
    for (int i = 0; i < sizeof(eb_bodyPartActions) / sizeof(eb_bodyPartActions[0]); i++) 
    {
        cvar_t *eb_bodyPartCvar = cgi.Cvar_Get(eb_bodyPartActions[i].name, "", 0);
        // Com_Printf("eb_bodyPartCvars[%d] is: %s\n", i, eb_bodyPartCvar->string);
        if (eb_CheckStringForCVar(fullCmd, eb_bodyPartCvar, NULL))
        {
            // Com_Printf("Found body part: %s\n", eb_bodyPartCvar->string);
            eb_UpdateStat(&eb_bodyPartCvar);
            return;
        }
    }
}

qboolean eb_CheckStringForCVar(const char* fullCmd, cvar_t *cvar, const char *params) 
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

        // if params is not empty or null, format our cvar string with params
        if (params != NULL && params[0] != '\0') {
            char formattedCvarString[256];
            size_t formattedCvarStringSize = sizeof(formattedCvarString);
            snprintf(formattedCvarString, formattedCvarStringSize, item, params);
            formattedCvarString[formattedCvarStringSize - 1] = '\0'; // Ensure null-termination
            item = formattedCvarString;
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
