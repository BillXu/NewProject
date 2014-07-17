CREATE TABLE `log_robotcoin` (
  `robotZoneOffset1` bigint(20) NOT NULL,
  `robotZoneOffset2` bigint(20) DEFAULT NULL,
  `robotZoneOffset3` bigint(20) DEFAULT NULL,
  `robotZoneOffset4` bigint(20) DEFAULT NULL,
  `robotTotalOffset` bigint(20) DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`robotZoneOffset1`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
