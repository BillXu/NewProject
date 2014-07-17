CREATE TABLE `log_moneyoffsetotheraction` (
  `userUID` int(10) unsigned NOT NULL,
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `coin` bigint(20) unsigned DEFAULT NULL,
  `diamoned` int(10) unsigned DEFAULT NULL,
  `actionType` smallint(5) unsigned DEFAULT NULL COMMENT '0 好友免费赠送，1 师傅关系获得 ， 2  徒弟关系获得',
  `coinOffset` bigint(20) unsigned zerofill DEFAULT NULL,
  `diamonedOffset` int(10) unsigned zerofill DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
