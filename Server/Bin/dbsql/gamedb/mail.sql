CREATE TABLE `mail` (
  `mailUID` bigint(20) unsigned NOT NULL,
  `userUID` int(10) unsigned NOT NULL,
  `postTime` int(10) unsigned DEFAULT NULL,
  `mailType` tinyint(3) unsigned DEFAULT NULL,
  `mailContent` blob,
  `processAct` tinyint(3) unsigned zerofill DEFAULT NULL COMMENT '对邮件的处理类型，0 是没有处理',
  PRIMARY KEY (`mailUID`),
  KEY `userUID` (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
