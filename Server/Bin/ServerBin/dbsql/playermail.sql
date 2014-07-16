CREATE TABLE `playermails` (
  `nMailUID` bigint(20) unsigned NOT NULL,
  `nUserUID` int(11) NOT NULL,
  `strTitle` varchar(245) DEFAULT NULL,
  `nPostTime` int(11) DEFAULT NULL,
  `bIsRead` tinyint(4) DEFAULT NULL,
  `eType` tinyint(4) DEFAULT NULL,
  `pContent` blob,
  `bProcessed` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`nMailUID`,`nUserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='mails of all players ;';