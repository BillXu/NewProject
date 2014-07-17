CREATE TABLE `log_presentasset` (
  `userUID` int(10) unsigned NOT NULL,
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `targetPlayerUID` int(10) unsigned DEFAULT NULL,
  `assetItemID` smallint(5) unsigned DEFAULT NULL,
  `assetCount` int(10) unsigned zerofill DEFAULT NULL,
  `presentReason` smallint(5) unsigned zerofill DEFAULT NULL COMMENT '0 普通赠送， 1 拜师赠送',
  `coin` bigint(20) unsigned DEFAULT NULL,
  `diamoned` int(10) unsigned DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
