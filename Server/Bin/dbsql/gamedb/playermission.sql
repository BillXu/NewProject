CREATE TABLE `playermission` (
  `userUID` int(10) unsigned NOT NULL,
  `missionCount` smallint(5) unsigned zerofill DEFAULT '00000',
  `missionData` blob,
  `saveTime` int(10) unsigned zerofill DEFAULT '0000000000',
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
