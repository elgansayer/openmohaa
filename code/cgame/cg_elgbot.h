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
#include "cg_local.h"
#include "../fgame/bg_voteoptions.h"

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

// void eb_SqlInit(void);
// void eb_Shutdown(void);
// void eb_UpdateStatByName(const char * statName);

void eb_ParsePrintedText(void);
qboolean eb_ProcessDidEnterBattle(const char *fullCmd);
void ev_ResetAll( void );
qboolean eb_ProcessGotKilled(const char *fullCmd);
qboolean eb_ProcessGotKill(const char *fullCmd);
qboolean eb_ProcessGotYouKilled(const char *combinedCmd);
void eb_PerformQuickKills(void);
void eb_PerformKillSpreeChecks(void);
void eb_Announcement(eb_Action * ebAction);
void eb_PlaySound(eb_Action * ebAction);
void eb_DisplayHud(eb_Action * ebAction);
void eb_ResetLastKillTime(void);
void eb_ResetKillsInARow(void);
void eb_ResetDeathsInARow(void);
void eb_CheckKillForHeadshot(const char* fullCmd);
void eb_UpdateStat(eb_Action * ebAction);
void eb_IncrementCvar( const char* cvarName, int amount );
void eb_CheckKillForBodyParts(const char* fullCmd);
qboolean eb_CheckStringForCVar(const char* fullCmd, cvar_t *cvar, const char *params); 

