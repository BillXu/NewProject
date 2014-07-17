CREATE TABLE `log_mission` (
  `userUID` int(10) unsigned NOT NULL,
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `missionID` smallint(5) unsigned zerofill DEFAULT NULL,
  `rewardCoin` int(10) unsigned zerofill DEFAULT NULL,
  `coin` bigint(20) unsigned DEFAULT NULL,
  `diamoned` int(10) unsigned DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
