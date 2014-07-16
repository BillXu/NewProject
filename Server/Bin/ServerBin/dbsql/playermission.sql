CREATE TABLE `playermission` (
  `nUserUID` int(10) unsigned NOT NULL,
  `nLastSaveTime` int(10) unsigned NOT NULL,
  `MissionData` blob,
  PRIMARY KEY (`nUserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
