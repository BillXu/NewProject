CREATE TABLE `account` (
  `account` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `userUID` int(15) NOT NULL AUTO_INCREMENT,
  `registerType` int(10) DEFAULT NULL COMMENT '0 表示游客登录，1表示正常注册 , 2 绑定账号',
  `email` varchar(255) COLLATE utf8_unicode_ci DEFAULT '0',
  `channel` tinyint(4) DEFAULT '0' COMMENT '渠道标示，0. appstore  1. pp 助手，2.  91商店 3. 360商店 4.winphone store',
  `state` tinyint(3) unsigned zerofill DEFAULT '000' COMMENT '0 ,表示正常，其他值表示封号，或者其他异常情况',
  `registerDateTime` datetime DEFAULT NULL,
  PRIMARY KEY (`userUID`,`account`),
  UNIQUE KEY `account_UNIQUE` (`account`),
  UNIQUE KEY `userUID_UNIQUE` (`userUID`)
) ENGINE=InnoDB AUTO_INCREMENT=1342 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
