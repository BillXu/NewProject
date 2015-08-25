CREATE TABLE `log` (
  `logID` bigint(20) NOT NULL AUTO_INCREMENT,
  `logType` int(11) NOT NULL,
  `targetID` int(11) NOT NULL,
  `logTime` datetime NOT NULL,
  `externJson` varchar(545) COLLATE utf8_unicode_ci DEFAULT NULL,
  `arg0` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg1` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg2` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg3` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg4` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg5` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg6` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg7` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg8` bigint(20) unsigned zerofill DEFAULT NULL,
  `arg9` bigint(20) unsigned zerofill DEFAULT NULL,
  PRIMARY KEY (`logID`,`logType`,`targetID`,`logTime`)
) ENGINE=InnoDB AUTO_INCREMENT=234 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='all game log here ';
