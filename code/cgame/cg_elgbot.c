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
// #include <sqlite3.h>
#include "cg_local.h"
#include "cg_elgbot.h"

// sqlite3 *elgbot_db;

// void eb_SqlInit(void)
// {
//     sqlite3_open("elgbot.db", &elgbot_db);

//     char *zErrMsg = 0;
//     int rc;

//     /* Execute SQL statement */
//     rc = sqlite3_exec(elgbot_db, "CREATE TABLE IF NOT EXISTS stat (id INTEGER PRIMARY KEY AUTOINCREMENT, stat_name TEXT, stat_value INTEGER);", NULL, 0, &zErrMsg);

//     if( rc != SQLITE_OK )
//     {
//         Com_Printf("SQL error: %s\n", zErrMsg);
//         sqlite3_free(zErrMsg);
//     }
//     else 
//     {
//         Com_Printf("Elgbot Stats Table Created Successfully\n");
//     }
// }

// void eb_Shutdown(void)
// {
//     if (elgbot_db != NULL) 
//     {
//         sqlite3_close(elgbot_db);

//         // print closed db
//         Com_Printf("Closed elgbot.db\n");
//     }
// }

// void eb_UpdateStatByName(const char * statName) 
// {
//     char *zErrMsg = 0;
//     int rc;
//     if (elgbot_db == NULL)
//     {
//             return;
//     }

//     /* Execute SQL statement */

//     char sqlStr[1024];
//     size_t sqlStrSize = sizeof(sqlStr);
//     snprintf(sqlStr, sqlStrSize, "INSERT OR REPLACE INTO stat (stat_name, stat_value) VALUES ('stat_name', COALESCE((SELECT stat_value FROM stat WHERE stat_name = '%s'), 0) + 1);", statName);
//     sqlStr[sqlStrSize - 1] = '\0'; // Ensure null-termination

//     rc = sqlite3_exec(elgbot_db, sqlStr, NULL, 0, &zErrMsg);

//     if( rc != SQLITE_OK ){
//             Com_Printf("SQL error: %s\n", zErrMsg);
//             sqlite3_free(zErrMsg);
//     } else {
//             Com_Printf("Elgbot Stats Table Updated Successfully\n");
//     }
// }

void eb_ParsePrintedText()
{
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

    // We died soo reset everything
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

    // Check for bash

    // cvar_t *eb_gotBashed = cgi.Cvar_Get("eb_gotBashed", "", 0);
    cvar_t *eb_bashed = cgi.Cvar_Get("eb_bashed", "was clubbed by %s, was bashed by %s", 0);    
    if (eb_CheckStringForCVar(fullCmd , eb_bashed, name_cvar->string))
    {
        eb_Action bashAction = 
            {"bashed",0, NULL, NULL, NULL, "ui_NumBashed"};
        
        eb_UpdateStat(&bashAction);      
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

    // What quick kill level am I?
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
    // eb_UpdateStatByName(ebAction->name);
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
            eb_UpdateStat(&eb_bodyPartActions[i]);
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

    #if defined (_WIN32) || defined (_WIN64)
        #define strtok_r strtok_s
    #endif

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
