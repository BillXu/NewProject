CREATE TABLE `log_playinroom` (
  `userUID` int(10) unsigned NOT NULL,
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `enterRoomTime` int(10) unsigned DEFAULT NULL,
  `coinBeforEnter` bigint(20) unsigned DEFAULT NULL,
  `coinExit` bigint(20) unsigned DEFAULT NULL,
  `exitRoomTime` int(10) unsigned DEFAULT NULL,
  `roomID` int(10) unsigned DEFAULT NULL,
  `stayInRoomTime` int(10) unsigned DEFAULT NULL COMMENT 'by minite',
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
