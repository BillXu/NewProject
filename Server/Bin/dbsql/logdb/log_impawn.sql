CREATE TABLE `log_impawn` (
  `userUID` int(10) unsigned NOT NULL DEFAULT '0',
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `impawnItemID` smallint(5) unsigned DEFAULT NULL,
  `itemCount` int(10) unsigned DEFAULT '1',
  `impawnCoin` bigint(20) unsigned zerofill DEFAULT NULL,
  `impawnDiamoned` int(10) unsigned zerofill DEFAULT NULL,
  `coin` bigint(20) unsigned zerofill DEFAULT NULL,
  `diamoned` int(10) unsigned zerofill DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
