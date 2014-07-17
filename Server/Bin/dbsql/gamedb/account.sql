CREATE TABLE `account` (
  `account` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `registerDate` int(10) unsigned zerofill DEFAULT '0000000000',
  `registerType` tinyint(4) DEFAULT NULL COMMENT '0 表示游客登录，1表示正常注册 , 2 绑定账号',
  `email` varchar(255) COLLATE utf8_unicode_ci DEFAULT '0',
  `userUID` int(11) DEFAULT NULL,
  `channel` tinyint(4) DEFAULT '0' COMMENT '渠道标示，0. appstore  1. pp 助手，2.  91商店 3. 360商店 4.winphone store',
  `state` tinyint(3) unsigned zerofill DEFAULT '000' COMMENT '0 ,表示正常，其他值表示封号，或者其他异常情况',
  PRIMARY KEY (`account`),
  UNIQUE KEY `account_UNIQUE` (`account`),
  UNIQUE KEY `userUID_UNIQUE` (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
