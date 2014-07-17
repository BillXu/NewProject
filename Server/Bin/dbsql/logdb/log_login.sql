CREATE TABLE `log_login` (
  `userUID` int(11) unsigned NOT NULL DEFAULT '0',
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `loginTime` int(10) unsigned DEFAULT NULL,
  `logoutTime` int(10) unsigned DEFAULT NULL,
  `onlineTime` int(10) unsigned zerofill DEFAULT NULL COMMENT '在线时长，本次登陆，分钟位单位',
  `coin` bigint(20) unsigned DEFAULT NULL,
  `diamoned` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
