#pragma once

// app.db的结构

const char SQL_init_eventTable[] = { "CREATE TABLE 'event' ('EID' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,'TYPE' INTEGER NOT NULL,'TIME' NUMERIC NOT NULL DEFAULT CURRENT_TIMESTAMP,'LINK' INTEGER,'CONT' TEXT,'NOTE' TEXT,'STATUS' INTEGER);" };

const char SQL_init_relatTable[] = { "CREATE TABLE `relationship` (`QQ` INTEGER NOT NULL UNIQUE,`Nickname` TEXT NOT NULL,`level` INTEGER NOT NULL,`amity` INTEGER NOT NULL,`from` TEXT,`note` TEXT,`lastActiv` NUMERIC DEFAULT CURRENT_TIMESTAMP,PRIMARY KEY(`QQ`));" };

const char SQL_init_activeIndex[] = { "CREATE INDEX `Activ` ON `relationship` (`lastActiv` DESC);" };
										  
const char SQL_init_powerIndex[] = { "CREATE INDEX `power` ON `relationship` (`level` ASC);" };
										   
const char SQL_init_priorIndex[] = { "CREATE INDEX `priority` ON `event` (`STATUS` DESC);" };

// 一些常用公用的SQL

const char SQL_begin[] = { "BEGIN TRANSACTION" };

const char SQL_commit[] = { "COMMIT TRANSACTION" };